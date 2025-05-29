#include "py_point.h"
#include <string>
#include <format>

// Allocation/deallocation functions
static PyObject* PyPoint_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyPoint *self = (PyPoint *)type->tp_alloc(type, 0);
    if (self != NULL) {
        // Initialize with default values
        new (&self->point) anim::Point(0.0, 0.0);
    }
    return (PyObject *)self;
}

static void PyPoint_dealloc(PyPoint *self) {
    self->point.~Point();
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Initialize the object
static int PyPoint_init(PyPoint *self, PyObject *args, PyObject *kwds) {
    double time = 0.0, value = 0.0;
    static char *kwlist[] = {(char*)"time", (char*)"value", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|dd", kwlist, &time, &value))
        return -1;
    
    self->point = anim::Point(time, value);
    return 0;
}

static PyObject* PyPoint_get_time(PyPoint *self, void *closure) {
    return PyFloat_FromDouble(self->point.time);
}

static int PyPoint_set_time(PyPoint *self, PyObject *value, void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The time attribute must be a float");
        return -1;
    }
    
    self->point.time = PyFloat_AsDouble(value);
    return 0;
}

static PyObject* PyPoint_get_value(PyPoint *self, void *closure) {
    return PyFloat_FromDouble(self->point.value);
}

static int PyPoint_set_value(PyPoint *self, PyObject *value, void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The value attribute must be a float");
        return -1;
    }
    
    self->point.value = PyFloat_AsDouble(value);
    return 0;
}


PyGetSetDef PyPoint_getset[] = {
    {"time", (getter)PyPoint_get_time, (setter)PyPoint_set_time, "Time of the Point", NULL},
    {"value", (getter)PyPoint_get_value, (setter)PyPoint_set_value, "Value of the Point", NULL},
    {NULL}  // Sentinel
};

static PyObject* PyPoint_str(PyPoint *self) {
    auto str = std::format("Point(time={}, value={})", self->point.time, self->point.value);
    return PyUnicode_FromString(str.c_str());
}

PyTypeObject PyPointType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "anim.Point",              // tp_name
    sizeof(PyPoint),           // tp_basicsize
    0,                         // tp_itemsize
    (destructor)PyPoint_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)PyPoint_str,     // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash 
    0,                         // tp_call
    (reprfunc)PyPoint_str,     // tp_str
    PyObject_GenericGetAttr,   // tp_getattro
    PyObject_GenericSetAttr,   // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "Point objects",           // tp_doc 
    0,		                   // tp_traverse 
    0,		                   // tp_clear 
    0,		                   // tp_richcompare 
    0,		                   // tp_weaklistoffset 
    0,		                   // tp_iter 
    0,		                   // tp_iternext 
    0,		                   // tp_methods 
    0,		                   // tp_members 
    PyPoint_getset,            // tp_getset 
    0,		                   // tp_base 
    0,		                   // tp_dict 
    0,		                   // tp_descr_get 
    0,		                   // tp_descr_set 
    0,		                   // tp_dictoffset 
    (initproc)PyPoint_init,    // tp_init 
    0,		                   // tp_alloc 
    PyPoint_new,               // tp_new 
};


PyPoint* PointToPyPoint(const anim::Point& point) {
    // Ensure the type is initialized before creating an instance
    if (PyType_Ready(&PyPointType) < 0) {
        return NULL;
    }
    PyPoint* pyPoint = PyObject_New(PyPoint, &PyPointType);
    if (pyPoint == NULL) {
        return NULL; // PyErr_NoMemory() might have been set by PyObject_New
    }
    new (&pyPoint->point) anim::Point(point);
    return pyPoint;
}

// Convert from C++ type to Python object
PyObject* PointToPyObject(const anim::Point& point) {
    PyPoint* pyPoint = PointToPyPoint(point);
    if (pyPoint == NULL) {
        return NULL;
    }
    return (PyObject*)pyPoint;
}

// Convert from Python object to C++ type
bool PyObjectToPoint(PyObject* obj, anim::Point& point) {
    if (!PyObject_TypeCheck(obj, &PyPointType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a Point object");
        return false;
    }
    
    PyPoint* pyPoint = (PyPoint*)obj;
    point = pyPoint->point;
    return true;
}

// Type getter for external use
PyObject* get_point_type(PyObject* self, void* closure) {
    if (PyType_Ready(&PyPointType) < 0) {
        return NULL;
    }
    Py_INCREF(&PyPointType);
    return (PyObject*)&PyPointType;
}
