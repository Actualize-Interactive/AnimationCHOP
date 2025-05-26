#include "py_bezier_handle.h"
#include <string>
#include <format>

// Allocation/deallocation functions
static PyObject* PyBezierHandle_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyBezierHandle *self = (PyBezierHandle *)type->tp_alloc(type, 0);
    if (self != NULL) {
        // Initialize with default values
        new (&self->point) anim::BezierHandle(0.0, 0.0);
    }
    return (PyObject *)self;
}

static void PyBezierHandle_dealloc(PyBezierHandle *self) {
    self->point.~BezierHandle();
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Initialize the object
static int PyBezierHandle_init(PyBezierHandle *self, PyObject *args, PyObject *kwds) {
    double time = 0.0, value = 0.0;
    static char *kwlist[] = {(char*)"time", (char*)"value", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|dd", kwlist, &time, &value))
        return -1;
    
    self->point = anim::BezierHandle(time, value);
    return 0;
}

static PyObject* PyBezierHandle_get_time(PyBezierHandle *self, void *closure) {
    return PyFloat_FromDouble(self->point.time);
}

static int PyBezierHandle_set_time(PyBezierHandle *self, PyObject *value, void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The time attribute must be a float");
        return -1;
    }
    
    self->point.time = PyFloat_AsDouble(value);
    return 0;
}

static PyObject* PyBezierHandle_get_value(PyBezierHandle *self, void *closure) {
    return PyFloat_FromDouble(self->point.value);
}

static int PyBezierHandle_set_value(PyBezierHandle *self, PyObject *value, void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The value attribute must be a float");
        return -1;
    }
    
    self->point.value = PyFloat_AsDouble(value);
    return 0;
}


PyGetSetDef PyBezierHandle_getset[] = {
    {"time", (getter)PyBezierHandle_get_time, (setter)PyBezierHandle_set_time, "Time of the BezierHandle", NULL},
    {"value", (getter)PyBezierHandle_get_value, (setter)PyBezierHandle_set_value, "Value of the BezierHandle", NULL},
    {NULL}  // Sentinel
};

static PyObject* PyBezierHandle_str(PyBezierHandle *self) {
    auto str = std::format("BezierHandle(time={}, value={})", self->point.time, self->point.value);
    return PyUnicode_FromString(str.c_str());
}

PyTypeObject PyBezierHandleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "anim.BezierHandle",       // tp_name
    sizeof(PyBezierHandle),    // tp_basicsize
    0,                         // tp_itemsize
    (destructor)PyBezierHandle_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)PyBezierHandle_str,   // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash 
    0,                         // tp_call
    (reprfunc)PyBezierHandle_str,   // tp_str
    PyObject_GenericGetAttr,   // tp_getattro
    PyObject_GenericSetAttr,   // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "BezierHandle objects",    // tp_doc 
    0,		                   // tp_traverse 
    0,		                   // tp_clear 
    0,		                   // tp_richcompare 
    0,		                   // tp_weaklistoffset 
    0,		                   // tp_iter 
    0,		                   // tp_iternext 
    0,		                   // tp_methods 
    0,		                   // tp_members 
    PyBezierHandle_getset,     // tp_getset 
    0,		                   // tp_base 
    0,		                   // tp_dict 
    0,		                   // tp_descr_get 
    0,		                   // tp_descr_set 
    0,		                   // tp_dictoffset 
    (initproc)PyBezierHandle_init,  // tp_init 
    0,		                   // tp_alloc 
    PyBezierHandle_new,        // tp_new 
};


PyBezierHandle* BezierHandleToPyBezierHandle(const anim::BezierHandle& handle) {
    // Ensure the type is initialized before creating an instance
    if (PyType_Ready(&PyBezierHandleType) < 0) {
        return NULL;
    }
    PyBezierHandle* pyHandle = PyObject_New(PyBezierHandle, &PyBezierHandleType);
    if (pyHandle == NULL) {
        return NULL; // PyErr_NoMemory() might have been set by PyObject_New
    }
    new (&pyHandle->point) anim::BezierHandle(handle);
    return pyHandle;
}

// Convert from C++ type to Python object
PyObject* BezierHandleToPyObject(const anim::BezierHandle& handle) {
    PyBezierHandle* pyHandle = BezierHandleToPyBezierHandle(handle);
    if (pyHandle == NULL) {
        return NULL;
    }
    return (PyObject*)pyHandle;
}

// Convert from Python object to C++ type
bool PyObjectToBezierHandle(PyObject* obj, anim::BezierHandle& handle) {
    if (!PyObject_TypeCheck(obj, &PyBezierHandleType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a BezierHandle object");
        return false;
    }
    
    PyBezierHandle* pyHandle = (PyBezierHandle*)obj;
    handle = pyHandle->point;
    return true;
}

// Type getter for external use
PyObject* get_bezier_handle_type(PyObject* self, void* closure) {
    if (PyType_Ready(&PyBezierHandleType) < 0) {
        return NULL;
    }
    Py_INCREF(&PyBezierHandleType);
    return (PyObject*)&PyBezierHandleType;
}

