#include "py_point.h"
#include <string>
#include <format>

// Allocation/deallocation functions
static PyObject* PyPoint2D_new(PyTypeObject *type, [[maybe_unused]] PyObject *args, [[maybe_unused]] PyObject *kwds) {
    PyPoint2D *self = (PyPoint2D *)type->tp_alloc(type, 0);
    if (self != NULL) {
        // Initialize with default values
        new (&self->point) anim::Point2D(0.0, 0.0);
    }
    return (PyObject *)self;
}

static void PyPoint2D_dealloc(PyPoint2D *self) {
    self->point.~Point2D();
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Initialize the object
static int PyPoint2D_init(PyPoint2D *self, PyObject *args, PyObject *kwds) {
    double time = 0.0, value = 0.0;
    static char *kwlist[] = {(char*)"time", (char*)"value", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|dd", kwlist, &time, &value))
        return -1;
    
    self->point = anim::Point2D(time, value);
    return 0;
}

static PyObject* PyPoint2D_get_time(PyPoint2D *self, [[maybe_unused]]void *closure) {
    return PyFloat_FromDouble(self->point.time);
}

static int PyPoint2D_set_time(PyPoint2D *self, PyObject *value, [[maybe_unused]]void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The time attribute must be a float");
        return -1;
    }
    
    self->point.time = PyFloat_AsDouble(value);
    return 0;
}

static PyObject* PyPoint2D_get_value(PyPoint2D *self, [[maybe_unused]] void *closure) {
    return PyFloat_FromDouble(self->point.value);
}

static int PyPoint2D_set_value(PyPoint2D *self, PyObject *value, [[maybe_unused]]void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The value attribute must be a float");
        return -1;
    }
    
    self->point.value = PyFloat_AsDouble(value);
    return 0;
}


PyGetSetDef PyPoint2D_getset[] = {
    {"time", (getter)PyPoint2D_get_time, (setter)PyPoint2D_set_time, "Time of the point", NULL},
    {"value", (getter)PyPoint2D_get_value, (setter)PyPoint2D_set_value, "Value of the point", NULL},
    {NULL}  // Sentinel
};

static PyObject* PyPoint2D_str(PyPoint2D *self) {
    auto str = std::format("Point2D(time={}, value={})", self->point.time, self->point.value);
    return PyUnicode_FromString(str.c_str());
}

PyTypeObject PyPoint2DType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "anim.Point2D",             // tp_name
    sizeof(PyPoint2D),         // tp_basicsize
    0,                         // tp_itemsize
    (destructor)PyPoint2D_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)PyPoint2D_str,   // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash 
    0,                         // tp_call
    (reprfunc)PyPoint2D_str,   // tp_str
    PyObject_GenericGetAttr,   // tp_getattro
    PyObject_GenericSetAttr,   // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "Point2D objects",         // tp_doc 
    0,		                   // tp_traverse 
    0,		                   // tp_clear 
    0,		                   // tp_richcompare 
    0,		                   // tp_weaklistoffset 
    0,		                   // tp_iter 
    0,		                   // tp_iternext 
    0,		                   // tp_methods 
    0,		                   // tp_members 
    PyPoint2D_getset,          // tp_getset 
    0,		                   // tp_base 
    0,		                   // tp_dict 
    0,		                   // tp_descr_get 
    0,		                   // tp_descr_set 
    0,		                   // tp_dictoffset 
    (initproc)PyPoint2D_init,  // tp_init 
    0,		                   // tp_alloc 
    PyPoint2D_new,             // tp_new 
};


[[maybe_unused]] PyPoint2D* Point2DToPyPoint2D(const anim::Point2D& point) {
    // Ensure the type is initialized before creating an instance
    if (PyType_Ready(&PyPoint2DType) < 0) {
        return NULL;
    }
    PyPoint2D* pyPoint = PyObject_New(PyPoint2D, &PyPoint2DType);
    if (pyPoint == NULL) {
        return NULL; // PyErr_NoMemory() might have been set by PyObject_New
    }
    new (&pyPoint->point) anim::Point2D(point);
    return pyPoint;
}

// Convert from C++ type to Python object
[[maybe_unused]] PyObject* Point2DToPyObject(const anim::Point2D& point) {
    PyPoint2D* pyPoint = Point2DToPyPoint2D(point);
    if (pyPoint == NULL) {
        return NULL;
    }
    return (PyObject*)pyPoint;
}

// Convert from Python object to C++ type
[[maybe_unused]] bool PyObjectToPoint2D(PyObject* obj, anim::Point2D& point) {
    if (!PyObject_TypeCheck(obj, &PyPoint2DType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a Point2D object");
        return false;
    }
    
    PyPoint2D* pyPoint = (PyPoint2D*)obj;
    point = pyPoint->point;
    return true;
}

// Type getter for external use
[[maybe_unused]] PyObject* get_point2d_type([[maybe_unused]] PyObject* self, [[maybe_unused]] void* closure) {
    if (PyType_Ready(&PyPoint2DType) < 0) {
        return NULL;
    }
    Py_INCREF(&PyPoint2DType);
    return (PyObject*)&PyPoint2DType;
}

