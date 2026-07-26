#include "py_point.h"
#include <string>
#include <format>

// Allocation/deallocation functions
static PyObject* PY_Point_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PY_Point *self = (PY_Point *)type->tp_alloc(type, 0);
    if (self != NULL) {
        // Initialize with default values
        new (&self->point) anim::Point(0.0, 0.0);
    }
    return (PyObject *)self;
}

static void PY_Point_dealloc(PY_Point *self) {
    self->point.~Point();
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Initialize the object
static int PY_Point_init(PY_Point *self, PyObject *args, PyObject *kwds) {
    double time = 0.0, value = 0.0;
    static char *kwlist[] = {(char*)"time", (char*)"value", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|dd", kwlist, &time, &value))
        return -1;
    
    self->point = anim::Point(time, value);
    return 0;
}

static PyObject* PY_Point_get_time(PY_Point *self, void *closure) {
    return PyFloat_FromDouble(self->point.time);
}

static int PY_Point_set_time(PY_Point *self, PyObject *value, void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The time attribute must be a float");
        return -1;
    }
    
    self->point.time = PyFloat_AsDouble(value);
    return 0;
}

static PyObject* PY_Point_get_value(PY_Point *self, void *closure) {
    return PyFloat_FromDouble(self->point.value);
}

static int PY_Point_set_value(PY_Point *self, PyObject *value, void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The value attribute must be a float");
        return -1;
    }
    
    self->point.value = PyFloat_AsDouble(value);
    return 0;
}

// --- Equality and copy protocol ---
static PyObject* PY_Point_richcompare(PyObject* a, PyObject* b, int op) {
    if (!PyObject_TypeCheck(a, &PY_PointType) || !PyObject_TypeCheck(b, &PY_PointType)) {
        Py_RETURN_NOTIMPLEMENTED;
    }
    const anim::Point& pa = ((PY_Point*)a)->point;
    const anim::Point& pb = ((PY_Point*)b)->point;
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
static PyObject* PY_Point_copy(PY_Point* self, PyObject*) {
    PY_Point* result = PointToPY_Point(self->point);
    if (!result) return NULL;
    return (PyObject*)result;
}
static PyObject* PY_Point_deepcopy(PY_Point* self, PyObject* args) {
    // Ignore memo dict
    PY_Point* result = PointToPY_Point(self->point);
    if (!result) return NULL;
    return (PyObject*)result;
}

// --- State methods ---
PyObject* PY_Point_get_state(PY_Point *self, void *closure) {
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

int PY_Point_set_state(PY_Point *self, PyObject *value, void *closure) {
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

static PyObject* PY_Point_get_state_method(PY_Point *self, PyObject *args) {
    return PY_Point_get_state(self, NULL);
}

static PyObject* PY_Point_set_state_method(PY_Point *self, PyObject *args) {
    PyObject* state;
    if (!PyArg_ParseTuple(args, "O", &state))
        return NULL;
    
    if (PY_Point_set_state(self, state, NULL) < 0)
        return NULL;
    
    Py_RETURN_NONE;
}

// --- Methods table ---
static PyMethodDef PY_Point_methods[] = {
    {"__copy__", (PyCFunction)PY_Point_copy, METH_NOARGS, "Shallow copy of Point"},
    {"__deepcopy__", (PyCFunction)PY_Point_deepcopy, METH_VARARGS, "Deep copy of Point"},
    {"get_state", (PyCFunction)PY_Point_get_state_method, METH_NOARGS, "Get Point state as dictionary"},
    {"set_state", (PyCFunction)PY_Point_set_state_method, METH_VARARGS, "Set Point state from dictionary"},
    {NULL, NULL, 0, NULL}
};

PyGetSetDef PY_Point_getset[] = {
    {"time", (getter)PY_Point_get_time, (setter)PY_Point_set_time, "Time of the Point", NULL},
    {"value", (getter)PY_Point_get_value, (setter)PY_Point_set_value, "Value of the Point", NULL},
    {"state", (getter)PY_Point_get_state, (setter)PY_Point_set_state, "Point state as dictionary", NULL},
    {NULL}  // Sentinel
};

static PyObject* PY_Point_str(PY_Point *self) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Point(time=%.1f, value=%.1f)", self->point.time, self->point.value);
    return PyUnicode_FromString(buffer);
}

PyTypeObject PY_PointType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "anim.Point",              // tp_name
    sizeof(PY_Point),           // tp_basicsize
    0,                         // tp_itemsize
    (destructor)PY_Point_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)PY_Point_str,     // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash 
    0,                         // tp_call
    (reprfunc)PY_Point_str,     // tp_str
    PyObject_GenericGetAttr,   // tp_getattro
    PyObject_GenericSetAttr,   // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    // tp_doc
    "A (time, value) pair, used for keyframe positions and Bezier handles.\n\n"
    "Like Keyframe, a Point is a value: one read from a keyframe is a detached\n"
    "copy, so kf.in_handle.time = x changes nothing. Assign a whole Point\n"
    "instead -- kf.in_handle = Point(x, y), or\n"
    "channel.set_keyframe_in_handle(index, Point(x, y)).", // tp_doc
    0,                         // tp_traverse 
    0,                         // tp_clear 
    PY_Point_richcompare,       // tp_richcompare 
    0,                         // tp_weaklistoffset 
    0,                         // tp_iter 
    0,                         // tp_iternext 
    PY_Point_methods,           // tp_methods 
    0,                         // tp_members 
    PY_Point_getset,            // tp_getset 
    0,                         // tp_base 
    0,                         // tp_dict 
    0,                         // tp_descr_get 
    0,                         // tp_descr_set 
    0,                         // tp_dictoffset 
    (initproc)PY_Point_init,    // tp_init 
    0,                         // tp_alloc 
    PY_Point_new,               // tp_new 
};


PY_Point* PointToPY_Point(const anim::Point& point) {
    // Ensure the type is initialized before creating an instance
    if (PyType_Ready(&PY_PointType) < 0) {
        return NULL;
    }
    PY_Point* py_Point = PyObject_New(PY_Point, &PY_PointType);
    if (py_Point == NULL) {
        return NULL; // PyErr_NoMemory() might have been set by PyObject_New
    }
    new (&py_Point->point) anim::Point(point);
    return py_Point;
}

// Convert from C++ type to Python object
PyObject* PointToPY_Object(const anim::Point& point) {
    PY_Point* py_Point = PointToPY_Point(point);
    if (py_Point == NULL) {
        return NULL;
    }
    return (PyObject*)py_Point;
}

// Convert from Python object to C++ type
bool PY_ObjectToPoint(PyObject* obj, anim::Point& point) {
    if (!PyObject_TypeCheck(obj, &PY_PointType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a Point object");
        return false;
    }
    
    PY_Point* py_Point = (PY_Point*)obj;
    point = py_Point->point;
    return true;
}

// Type getter for external use
PyObject* get_point_type(PyObject* self, void* closure) {
    if (PyType_Ready(&PY_PointType) < 0) {
        return NULL;
    }
    Py_INCREF(&PY_PointType);
    return (PyObject*)&PY_PointType;
}
