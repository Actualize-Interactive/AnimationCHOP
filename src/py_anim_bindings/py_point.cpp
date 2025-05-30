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

// --- Equality and copy protocol ---
static PyObject* PyPoint_richcompare(PyObject* a, PyObject* b, int op) {
    if (!PyObject_TypeCheck(a, &PyPointType) || !PyObject_TypeCheck(b, &PyPointType)) {
        Py_RETURN_NOTIMPLEMENTED;
    }
    const anim::Point& pa = ((PyPoint*)a)->point;
    const anim::Point& pb = ((PyPoint*)b)->point;
    switch (op) {
        case Py_EQ:
            return PyBool_FromLong(pa == pb);
        case Py_NE:
            return PyBool_FromLong(pa != pb);
        default:
            Py_RETURN_NOTIMPLEMENTED;
    }
}

// --- Copy protocol ---
static PyObject* PyPoint_copy(PyPoint* self, PyObject*) {
    PyPoint* result = PointToPyPoint(self->point);
    if (!result) return NULL;
    return (PyObject*)result;
}
static PyObject* PyPoint_deepcopy(PyPoint* self, PyObject* args) {
    // Ignore memo dict
    PyPoint* result = PointToPyPoint(self->point);
    if (!result) return NULL;
    return (PyObject*)result;
}

// --- State methods ---
PyObject* PyPoint_get_state(PyPoint *self, void *closure) {
    if (!self) {
        PyErr_SetString(PyExc_RuntimeError, "Point object is invalid");
        return NULL;
    }
    
    PyObject* state_dict = PyDict_New();
    if (!state_dict) return NULL;
    
    PyObject* time_obj = PyFloat_FromDouble(self->point.time);
    PyObject* value_obj = PyFloat_FromDouble(self->point.value);
    
    if (!time_obj || !value_obj) {
        Py_XDECREF(time_obj);
        Py_XDECREF(value_obj);
        Py_DECREF(state_dict);
        return NULL;
    }
    
    if (PyDict_SetItemString(state_dict, "time", time_obj) < 0 ||
        PyDict_SetItemString(state_dict, "value", value_obj) < 0) {
        Py_DECREF(time_obj);
        Py_DECREF(value_obj);
        Py_DECREF(state_dict);
        return NULL;
    }
    
    Py_DECREF(time_obj);
    Py_DECREF(value_obj);
    
    return state_dict;
}

int PyPoint_set_state(PyPoint *self, PyObject *value, void *closure) {
    if (!self) {
        PyErr_SetString(PyExc_RuntimeError, "Point object is invalid");
        return -1;
    }
    
    if (!PyDict_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "Point state must be a dictionary");
        return -1;
    }
    
    PyObject* time_obj = PyDict_GetItemString(value, "time");
    PyObject* value_obj = PyDict_GetItemString(value, "value");
    
    if (!time_obj) {
        PyErr_SetString(PyExc_ValueError, "Point state must have 'time' field");
        return -1;
    }
    
    if (!value_obj) {
        PyErr_SetString(PyExc_ValueError, "Point state must have 'value' field");
        return -1;
    }
    
    if (!PyFloat_Check(time_obj) && !PyLong_Check(time_obj)) {
        PyErr_SetString(PyExc_TypeError, "Point state 'time' must be a number");
        return -1;
    }
    
    if (!PyFloat_Check(value_obj) && !PyLong_Check(value_obj)) {
        PyErr_SetString(PyExc_TypeError, "Point state 'value' must be a number");
        return -1;
    }
    
    double time_val = PyFloat_AsDouble(time_obj);
    double value_val = PyFloat_AsDouble(value_obj);
    
    if (PyErr_Occurred()) {
        return -1;
    }
    
    self->point.time = time_val;
    self->point.value = value_val;
    
    return 0;
}

static PyObject* PyPoint_get_state_method(PyPoint *self, PyObject *args) {
    return PyPoint_get_state(self, NULL);
}

static PyObject* PyPoint_set_state_method(PyPoint *self, PyObject *args) {
    PyObject* state;
    if (!PyArg_ParseTuple(args, "O", &state))
        return NULL;
    
    if (PyPoint_set_state(self, state, NULL) < 0)
        return NULL;
    
    Py_RETURN_NONE;
}

// --- Methods table ---
static PyMethodDef PyPoint_methods[] = {
    {"__copy__", (PyCFunction)PyPoint_copy, METH_NOARGS, "Shallow copy of Point"},
    {"__deepcopy__", (PyCFunction)PyPoint_deepcopy, METH_VARARGS, "Deep copy of Point"},
    {"get_state", (PyCFunction)PyPoint_get_state_method, METH_NOARGS, "Get Point state as dictionary"},
    {"set_state", (PyCFunction)PyPoint_set_state_method, METH_VARARGS, "Set Point state from dictionary"},
    {NULL, NULL, 0, NULL}
};

PyGetSetDef PyPoint_getset[] = {
    {"time", (getter)PyPoint_get_time, (setter)PyPoint_set_time, "Time of the Point", NULL},
    {"value", (getter)PyPoint_get_value, (setter)PyPoint_set_value, "Value of the Point", NULL},
    {"state", (getter)PyPoint_get_state, (setter)PyPoint_set_state, "Point state as dictionary", NULL},
    {NULL}  // Sentinel
};

static PyObject* PyPoint_str(PyPoint *self) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Point(time=%.1f, value=%.1f)", self->point.time, self->point.value);
    return PyUnicode_FromString(buffer);
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
    0,                         // tp_traverse 
    0,                         // tp_clear 
    PyPoint_richcompare,       // tp_richcompare 
    0,                         // tp_weaklistoffset 
    0,                         // tp_iter 
    0,                         // tp_iternext 
    PyPoint_methods,           // tp_methods 
    0,                         // tp_members 
    PyPoint_getset,            // tp_getset 
    0,                         // tp_base 
    0,                         // tp_dict 
    0,                         // tp_descr_get 
    0,                         // tp_descr_set 
    0,                         // tp_dictoffset 
    (initproc)PyPoint_init,    // tp_init 
    0,                         // tp_alloc 
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
