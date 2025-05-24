#include "py_keyframe.h"
#include <format>

// Allocation/deallocation functions
static PyObject* PyKeyframe_new(PyTypeObject *type, [[maybe_unused]] PyObject *args, [[maybe_unused]] PyObject *kwds) {
    PyKeyframe *self = (PyKeyframe *)type->tp_alloc(type, 0);
    if (self != NULL) {
        new (&self->keyframe) anim::Keyframe(0.0, 0.0, anim::BezierHandle(0, 0), anim::BezierHandle(0, 0), anim::TangentMode::linear);
    }
    return (PyObject *)self;
}

static void PyKeyframe_dealloc(PyKeyframe *self) {
    self->keyframe.~Keyframe(); 
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Initialize the object
static int PyKeyframe_init(PyKeyframe *self, PyObject *args, PyObject *kwds) {
    double time = 0.0, value = 0.0;
    PyObject* in_handle_obj = NULL;
    PyObject* out_handle_obj = NULL;
    PyObject* mode_obj = NULL;
    
    static char *kwlist[] = {
        (char*)"time", (char*)"value", (char*)"in_handle", 
        (char*)"out_handle", (char*)"mode", NULL
    };
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ddOOO", kwlist, 
                                     &time, &value, &in_handle_obj, 
                                     &out_handle_obj, &mode_obj))
        return -1;
    
    // Default handles
    anim::BezierHandle in_handle(time - 1.0, value);
    anim::BezierHandle out_handle(time + 1.0, value);
    anim::TangentMode mode = anim::TangentMode::linear;
    
    // Parse in_handle if provided
    if (in_handle_obj) {
        anim::Point2D temp_point;
        if (!PyObjectToPoint2D(in_handle_obj, temp_point)) {
            // PyObjectToPoint2D already sets the error
            return -1;
        }
        in_handle.time = temp_point.time;
        in_handle.value = temp_point.value;
    }
    
    // Parse out_handle if provided
    if (out_handle_obj) {
        anim::Point2D temp_point;
        if (!PyObjectToPoint2D(out_handle_obj, temp_point)) {
            // PyObjectToPoint2D already sets the error
            return -1;
        }
        out_handle.time = temp_point.time;
        out_handle.value = temp_point.value;
    }
    
    // Parse mode if provided
    if (mode_obj) {
        if (PyLong_Check(mode_obj)) {
            long mode_val = PyLong_AsLong(mode_obj);
            
            // TODO: Add count to the TangentMode enum to properly validate
            if (mode_val >= 0 && mode_val <= 5) { 
                mode = static_cast<anim::TangentMode>(mode_val);
            } else {
                PyErr_SetString(PyExc_ValueError, "Invalid handle mode value");
                return -1;
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Mode must be a TangentMode enum value");
            return -1;
        }
    }

    // Create new keyframe
    self->keyframe = anim::Keyframe(time, value, in_handle, out_handle, mode);
    
    return 0;
}

// Getter/setter functions for properties
static PyObject* PyKeyframe_get_time(PyKeyframe *self, [[maybe_unused]] void *closure) {
    return PyFloat_FromDouble(self->keyframe.time());
}

static int PyKeyframe_set_time(PyKeyframe *self, PyObject *value, [[maybe_unused]] void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The time attribute must be a float");
        return -1;
    }

    self->keyframe.set_time(PyFloat_AsDouble(value));
    return 0;
}

static PyObject* PyKeyframe_get_value(PyKeyframe *self, [[maybe_unused]] void *closure) {
    return PyFloat_FromDouble(self->keyframe.value());
}

static int PyKeyframe_set_value(PyKeyframe *self, PyObject *value, [[maybe_unused]] void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The value attribute must be a float");
        return -1;
    }

    self->keyframe.set_value(PyFloat_AsDouble(value));
    return 0;
}

static PyObject* PyKeyframe_get_in_handle(PyKeyframe *self, [[maybe_unused]] void *closure) {
    const anim::BezierHandle& in_handle_handle = self->keyframe.in_handle();
    anim::Point2D point(in_handle_handle.time, in_handle_handle.value);
    return Point2DToPyObject(point);
}

static int PyKeyframe_set_in_handle(PyKeyframe *self, PyObject *value, [[maybe_unused]] void *closure) {
    anim::Point2D point;
    if (!PyObjectToPoint2D(value, point)) {
        // PyObjectToPoint2D already sets the error
        return -1;
    }
    
    anim::BezierHandle handle(point.time, point.value);
    self->keyframe.set_in_handle(handle);
    
    return 0;
}

static PyObject* PyKeyframe_get_out_handle(PyKeyframe *self, [[maybe_unused]] void *closure) {
    const anim::BezierHandle& out_handle_handle = self->keyframe.out_handle();
    anim::Point2D point(out_handle_handle.time, out_handle_handle.value);
    return Point2DToPyObject(point);
}

static int PyKeyframe_set_out_handle(PyKeyframe *self, PyObject *value, [[maybe_unused]] void *closure) {
    anim::Point2D point;
    if (!PyObjectToPoint2D(value, point)) {
        // PyObjectToPoint2D already sets the error
        return -1;
    }
    
    anim::BezierHandle handle(point.time, point.value);
    self->keyframe.set_out_handle(handle);
    
    return 0;
}

static PyObject* PyKeyframe_get_mode(PyKeyframe *self, [[maybe_unused]] void *closure) {
    // Return a Python integer representing the enum value
    return PyLong_FromLong(static_cast<long>(self->keyframe.mode()));
}

static int PyKeyframe_set_mode(PyKeyframe *self, PyObject *value, [[maybe_unused]] void *closure) {
    if (PyLong_Check(value)) {
        long mode_val = PyLong_AsLong(value);
        auto mode_count = static_cast<long>(anim::TangentMode::count);
        if (mode_val >= 0 && mode_val < mode_count) {
            self->keyframe.set_mode(static_cast<anim::TangentMode>(mode_val));
            return 0;
        } else {
            PyErr_SetString(PyExc_ValueError, "Invalid handle mode value");
            return -1;
        }
    } else {
        // Try to extract an int from a TangentMode enum object
        PyObject* value_attr = PyObject_GetAttrString(value, "value");
        if (value_attr && PyLong_Check(value_attr)) {
            long mode_val = PyLong_AsLong(value_attr);
            Py_DECREF(value_attr);
            
            // Validate mode value
            if (mode_val >= 0 && mode_val <= 5) { // Assuming 6 modes from the enum class
                self->keyframe.set_mode(static_cast<anim::TangentMode>(mode_val));
                return 0;
            } else {
                PyErr_SetString(PyExc_ValueError, "Invalid handle mode value");
                return -1;
            }
        } else {
            Py_XDECREF(value_attr);
            PyErr_SetString(PyExc_TypeError, "Mode must be a TangentMode enum value");
            return -1;
        }
    }
}

// Property definitions
static PyGetSetDef PyKeyframe_getset[] = {
    {"time", (getter)PyKeyframe_get_time, (setter)PyKeyframe_set_time, "Time of the keyframe", NULL},
    {"value", (getter)PyKeyframe_get_value, (setter)PyKeyframe_set_value, "Value of the keyframe", NULL},
    {"in_handle", (getter)PyKeyframe_get_in_handle, (setter)PyKeyframe_set_in_handle, "Incoming handle", NULL},
    {"out_handle", (getter)PyKeyframe_get_out_handle, (setter)PyKeyframe_set_out_handle, "Outgoing handle", NULL},
    {"mode", (getter)PyKeyframe_get_mode, (setter)PyKeyframe_set_mode, "Handle mode", NULL},
    {NULL}  // Sentinel
};

// String representation
static PyObject* PyKeyframe_str(PyKeyframe *self) {
    auto str = std::format("Keyframe(time={}, value={}, mode={})", 
                            self->keyframe.time(), self->keyframe.value(), 
                            static_cast<int>(self->keyframe.mode()));
    return PyUnicode_FromString(str.c_str());
}

// Type definition
PyTypeObject PyKeyframeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "anim.Keyframe",          // tp_name
    sizeof(PyKeyframe),       // tp_basicsize
    0,                         // tp_itemsize
    (destructor)PyKeyframe_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)PyKeyframe_str, // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash 
    0,                         // tp_call
    (reprfunc)PyKeyframe_str, // tp_str
    PyObject_GenericGetAttr,   // tp_getattro
    PyObject_GenericSetAttr,   // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "Keyframe object",        // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    0,                         // tp_methods
    0,                         // tp_members
    PyKeyframe_getset,         // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)PyKeyframe_init, // tp_init
    0,                         // tp_alloc
    PyKeyframe_new,            // tp_new
};

// Type getter for external use
[[maybe_unused]] PyObject* get_keyframe_type([[maybe_unused]] PyObject* self, [[maybe_unused]] void* closure) {
    if (PyType_Ready(&PyKeyframeType) < 0) {
        return NULL;
    }
    Py_INCREF(&PyKeyframeType);
    return (PyObject*)&PyKeyframeType;
}

[[maybe_unused]] PyKeyframe* KeyframeToPyKeyframe(const anim::Keyframe& keyframe) {
    // Ensure the type is initialized before creating an instance
    if (PyType_Ready(&PyKeyframeType) < 0) {
        return NULL;
    }
    PyKeyframe* py_keyframe = PyObject_New(PyKeyframe, &PyKeyframeType);
    if (py_keyframe == NULL) {
        return NULL;
    }
    new(&py_keyframe->keyframe) anim::Keyframe(keyframe);
    return py_keyframe;
}

// Helper functions for conversion between C++ and Python
[[maybe_unused]] PyObject* KeyframeToPyObject(const anim::Keyframe& keyframe) {
    return (PyObject*)KeyframeToPyKeyframe(keyframe);
}

[[maybe_unused]] bool PyKeyframeToKeyframe(PyKeyframe* py_keyframe, anim::Keyframe& keyframe) {
    if (!py_keyframe) { // This check might be redundant if type checking is done before calling
        PyErr_SetString(PyExc_TypeError, "Expected a Keyframe object, got NULL");
        return false;
    }
    if (!PyObject_TypeCheck(py_keyframe, &PyKeyframeType)) { // Ensure it's the correct type
        PyErr_SetString(PyExc_TypeError, "Expected a Keyframe object");
        return false;
    }
    keyframe = py_keyframe->keyframe;
    return true;
}

[[maybe_unused]] bool PyObjectToKeyframe(PyObject* obj, anim::Keyframe& keyframe) {
    if (!PyObject_TypeCheck(obj, &PyKeyframeType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a Keyframe object");
        return false;
    }
    
    PyKeyframe* py_keyframe = (PyKeyframe*)obj;
    keyframe = py_keyframe->keyframe; // Direct assignment if Keyframe is copyable
    return true;
}

