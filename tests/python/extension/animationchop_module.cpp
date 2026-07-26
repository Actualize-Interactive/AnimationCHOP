// Test-only CPython extension that exposes the AnimationCHOP Python bindings
// as an importable module, so they can be exercised with pytest WITHOUT
// TouchDesigner.
//
// It compiles the real operator sources (animation_chop.cpp and the
// py_anim_bindings/*.cpp) and stands in for TouchDesigner with a fake
// PY_Context. That is the entire TouchDesigner surface the bindings touch:
// they cast their `self` to a TD::PY_Struct, read `->context`, and call
// getNodeInstance() / makeNodeDirty() on it.
//
// The module exposes a single `op` object bound to the operator's own method
// and getset tables, so `op.create_channel(...)`, `op.state`, `op.Point`, etc.
// behave exactly as they do inside TouchDesigner.

#include "animation_chop.h"
#include "CPlusPlus_Common.h"

#include <cstddef>
#include <new>

using namespace TD;

namespace {

AnimationCHOP* g_instance = nullptr;
long           g_dirtyCount = 0;

// The only TouchDesigner interface the bindings use.
class FakeContext : public PY_Context
{
public:
    void* getNodeInstance(const PY_GetInfo&, void*) override
    {
        return g_instance;
    }

    void makeNodeDirty(void*) override
    {
        ++g_dirtyCount;
    }
};

FakeContext g_context;

// A PyObject laid out like TD::PY_Struct.
//
// Inside TouchDesigner the node's Python object *is* a PY_Struct: an opaque
// header (where CPython's own object header lives), then a PY_Context*. The
// bindings cast to that layout and read `context`, so the stand-in has to put
// its context pointer at the same offset. The static_asserts below are what
// actually guarantee that; the padding is just arithmetic to get there.
struct FakeNode
{
    PyObject_HEAD
    char        headerPad[sizeof(int32_t) * OP_STRUCT_HEADER_ENTRIES - sizeof(PyObject)];
    PY_Context* context;
};

static_assert(offsetof(FakeNode, context) == offsetof(PY_Struct, context),
              "FakeNode must place `context` where TD::PY_Struct does, since the "
              "bindings cast between them.");
static_assert(sizeof(FakeNode) <= sizeof(PY_Struct),
              "FakeNode must not claim more storage than a real PY_Struct.");

PyObject*
FakeNode_new(PyTypeObject* type, PyObject*, PyObject*)
{
    PyObject* self = type->tp_alloc(type, 0);
    if (!self)
        return nullptr;
    reinterpret_cast<FakeNode*>(self)->context = &g_context;
    return self;
}

PyTypeObject FakeNodeType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    "animationchop.AnimationCHOP",           // tp_name
    sizeof(FakeNode),                        // tp_basicsize
};

// --- module-level helpers, for assertions the bindings themselves cannot make ---

PyObject*
tm_dirty_count(PyObject*, PyObject*)
{
    return PyLong_FromLong(g_dirtyCount);
}

PyObject*
tm_reset_dirty_count(PyObject*, PyObject*)
{
    g_dirtyCount = 0;
    Py_RETURN_NONE;
}

PyMethodDef module_methods[] = {
    {"dirty_count", tm_dirty_count, METH_NOARGS,
     "Number of times the bindings have marked the node dirty."},
    {"reset_dirty_count", tm_reset_dirty_count, METH_NOARGS,
     "Reset the makeNodeDirty() counter."},
    {nullptr, nullptr, 0, nullptr},
};

PyModuleDef animationchop_module = {
    PyModuleDef_HEAD_INIT,
    "animationchop",
    "Test-only extension exposing the AnimationCHOP bindings (no TouchDesigner).",
    -1,
    module_methods,
    nullptr, nullptr, nullptr, nullptr,
};

} // namespace

PyMODINIT_FUNC
PyInit_animationchop(void)
{
    // Bind the operator's own tables, so the module under test is the real
    // binding surface rather than a reimplementation of it.
    FakeNodeType.tp_flags   = Py_TPFLAGS_DEFAULT;
    FakeNodeType.tp_doc     = "Stand-in for an AnimationCHOP node outside TouchDesigner.";
    FakeNodeType.tp_methods = AnimationCHOP_pythonMethods;
    FakeNodeType.tp_getset  = AnimationCHOP_pythonGetSets;
    FakeNodeType.tp_new     = FakeNode_new;

    if (PyType_Ready(&FakeNodeType) < 0)
        return nullptr;

    PyObject* module = PyModule_Create(&animationchop_module);
    if (!module)
        return nullptr;

    // One operator instance, alive for the session. The bindings reach it
    // through FakeContext::getNodeInstance().
    if (!g_instance) {
        static OP_NodeInfo nodeInfo{};
        nodeInfo.opPath = "/test/animation1";
        nodeInfo.opId   = 1;
        g_instance      = new AnimationCHOP(&nodeInfo);
    }

    PyObject* node = FakeNode_new(&FakeNodeType, nullptr, nullptr);
    if (!node) {
        Py_DECREF(module);
        return nullptr;
    }

    Py_INCREF(&FakeNodeType);
    if (PyModule_AddObject(module, "AnimationCHOP", (PyObject*)&FakeNodeType) < 0) {
        Py_DECREF(&FakeNodeType);
        Py_DECREF(node);
        Py_DECREF(module);
        return nullptr;
    }
    if (PyModule_AddObject(module, "op", node) < 0) {
        Py_DECREF(node);
        Py_DECREF(module);
        return nullptr;
    }

    return module;
}
