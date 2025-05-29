#include "py_keyframe.h"
#include <format>

// Allocation/deallocation functions
static PyObject* PyKeyframe_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyKeyframe *self = (PyKeyframe *)type->tp_alloc(type, 0);
    if (self != NULL) {
        new (&self->keyframe) anim::Keyframe(0.0, 0.0);
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
    PyObject* function_obj = NULL;
    PyObject* handle_mode_obj = NULL;
    
    static char *kwlist[] = {
        (char*)"time", (char*)"value", (char*)"in_handle", 
        (char*)"out_handle", (char*)"function", (char*)"handle_mode", NULL
    };
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ddOOOO", kwlist, 
                                     &time, &value, &in_handle_obj, 
                                     &out_handle_obj, &function_obj, &handle_mode_obj))
        return -1;
    
    // Default handles
    anim::Point in_handle(time - 1.0, value);
    anim::Point out_handle(time + 1.0, value);
    anim::Function function = anim::Function::bezier;
    anim::HandleMode handle_mode = anim::HandleMode::smooth;
    
    // Parse in_handle if provided
    if (in_handle_obj) {
        if (!PyObjectToPoint(in_handle_obj, in_handle)) {
            // PyObjectToPoint already sets the error
            return -1;
        }
    }
    
    // Parse out_handle if provided
    if (out_handle_obj) {
        if (!PyObjectToPoint(out_handle_obj, out_handle)) {
            // PyObjectToPoint already sets the error
            return -1;
        }
    }
    
    // Parse function if provided
    if (function_obj) {
        if (PyLong_Check(function_obj)) {
            long function_val = PyLong_AsLong(function_obj);
            
            if (function_val >= 0 && function_val < static_cast<long>(anim::Function::count)) { 
                function = static_cast<anim::Function>(function_val);
            } else {
                PyErr_SetString(PyExc_ValueError, "Invalid function value");
                return -1;
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Function must be a Function enum value");
            return -1;
        }
    }
    
    // Parse handle_mode if provided
    if (handle_mode_obj) {
        if (PyLong_Check(handle_mode_obj)) {
            long mode_val = PyLong_AsLong(handle_mode_obj);
            
            if (mode_val >= 0 && mode_val < static_cast<long>(anim::HandleMode::count)) { 
                handle_mode = static_cast<anim::HandleMode>(mode_val);
            } else {
                PyErr_SetString(PyExc_ValueError, "Invalid handle mode value");
                return -1;
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Handle mode must be a HandleMode enum value");
            return -1;
        }
    }

    // Create new keyframe using the new API constructor
    self->keyframe = anim::Keyframe(anim::Point(time, value), function, handle_mode, in_handle, out_handle);
    
    return 0;
}

// Getter/setter functions for properties
static PyObject* PyKeyframe_get_time(PyKeyframe *self, void *closure) {
    return PyFloat_FromDouble(self->keyframe.position.time);
}

static int PyKeyframe_set_time(PyKeyframe *self, PyObject *value, void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The time attribute must be a float");
        return -1;
    }

    self->keyframe.position.time = PyFloat_AsDouble(value);
    return 0;
}

static PyObject* PyKeyframe_get_value(PyKeyframe *self, void *closure) {
    return PyFloat_FromDouble(self->keyframe.position.value);
}

static int PyKeyframe_set_value(PyKeyframe *self, PyObject *value, void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The value attribute must be a float");
        return -1;
    }

    self->keyframe.position.value = PyFloat_AsDouble(value);
    return 0;
}

static PyObject* PyKeyframe_get_in_handle(PyKeyframe *self, void *closure) {
    return PointToPyObject(self->keyframe.in_handle);
}

static int PyKeyframe_set_in_handle(PyKeyframe *self, PyObject *value, void *closure) {
    anim::Point handle;
    if (!PyObjectToPoint(value, handle)) {
        // PyObjectToPoint already sets the error
        return -1;
    }
    
    self->keyframe.in_handle = handle;
    
    return 0;
}

static PyObject* PyKeyframe_get_out_handle(PyKeyframe *self, void *closure) {
    return PointToPyObject(self->keyframe.out_handle);
}

static int PyKeyframe_set_out_handle(PyKeyframe *self, PyObject *value, void *closure) {
    anim::Point handle;
    if (!PyObjectToPoint(value, handle)) {
        // PyObjectToPoint already sets the error
        return -1;
    }
    
    self->keyframe.out_handle = handle;
    
    return 0;
}

static PyObject* PyKeyframe_get_function(PyKeyframe *self, void *closure) {
    return PyLong_FromLong(static_cast<long>(self->keyframe.function));
}

static int PyKeyframe_set_function(PyKeyframe *self, PyObject *value, void *closure) {
    if (PyLong_Check(value)) {
        long function_val = PyLong_AsLong(value);
        if (function_val >= 0 && function_val < static_cast<long>(anim::Function::count)) {
            self->keyframe.function = static_cast<anim::Function>(function_val);
            return 0;
        } else {
            PyErr_SetString(PyExc_ValueError, "Invalid function value");
            return -1;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Function must be a Function enum value");
        return -1;
    }
}

static PyObject* PyKeyframe_get_handle_mode(PyKeyframe *self, void *closure) {
    return PyLong_FromLong(static_cast<long>(self->keyframe.handle_mode));
}

static int PyKeyframe_set_handle_mode(PyKeyframe *self, PyObject *value, void *closure) {
    if (PyLong_Check(value)) {
        long mode_val = PyLong_AsLong(value);
        if (mode_val >= 0 && mode_val < static_cast<long>(anim::HandleMode::count)) {
            self->keyframe.handle_mode = static_cast<anim::HandleMode>(mode_val);
            return 0;
        } else {
            PyErr_SetString(PyExc_ValueError, "Invalid handle mode value");
            return -1;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Handle mode must be a HandleMode enum value");
        return -1;
    }
}

// Property definitions
static PyGetSetDef PyKeyframe_getset[] = {
    {"time", (getter)PyKeyframe_get_time, (setter)PyKeyframe_set_time, "Time of the keyframe", NULL},
    {"value", (getter)PyKeyframe_get_value, (setter)PyKeyframe_set_value, "Value of the keyframe", NULL},
    {"in_handle", (getter)PyKeyframe_get_in_handle, (setter)PyKeyframe_set_in_handle, "Incoming handle point", NULL},
    {"out_handle", (getter)PyKeyframe_get_out_handle, (setter)PyKeyframe_set_out_handle, "Outgoing handle point", NULL},
    {"function", (getter)PyKeyframe_get_function, (setter)PyKeyframe_set_function, "Interpolation function", NULL},
    {"handle_mode", (getter)PyKeyframe_get_handle_mode, (setter)PyKeyframe_set_handle_mode, "Handle mode", NULL},
    {NULL}  // Sentinel
};

// String representation
static PyObject* PyKeyframe_str(PyKeyframe *self) {
    auto str = std::format("Keyframe(time={}, value={}, function={}, handle_mode={})", 
                            self->keyframe.position.time, 
                            self->keyframe.position.value, 
                            static_cast<int>(self->keyframe.function),
                            static_cast<int>(self->keyframe.handle_mode));
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
PyObject* get_keyframe_type(PyObject* self, void* closure) {
    if (PyType_Ready(&PyKeyframeType) < 0) {
        return NULL;
    }
    Py_INCREF(&PyKeyframeType);
    return (PyObject*)&PyKeyframeType;
}

PyKeyframe* KeyframeToPyKeyframe(const anim::Keyframe& keyframe) {
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
PyObject* KeyframeToPyObject(const anim::Keyframe& keyframe) {
    return (PyObject*)KeyframeToPyKeyframe(keyframe);
}

bool PyKeyframeToKeyframe(PyKeyframe* py_keyframe, anim::Keyframe& keyframe) {
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

bool PyObjectToKeyframe(PyObject* obj, anim::Keyframe& keyframe) {
    if (!PyObject_TypeCheck(obj, &PyKeyframeType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a Keyframe object");
        return false;
    }
    
    PyKeyframe* py_keyframe = (PyKeyframe*)obj;
    keyframe = py_keyframe->keyframe; // Direct assignment if Keyframe is copyable
    return true;
}

