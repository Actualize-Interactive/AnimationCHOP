#include "py_keyframe.h"
#include "py_function.h"
#include "py_handle_mode.h"
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

// --- State methods ---
PyObject* PyKeyframe_get_state(PyKeyframe *self, void *closure) {
    if (!self) {
        PyErr_SetString(PyExc_RuntimeError, "Keyframe object is invalid");
        return NULL;
    }
    
    PyObject* state_dict = PyDict_New();
    if (!state_dict) return NULL;
    
    try {
        // Use PyPoint state functions for position and handles
        PyPoint* position_point = PointToPyPoint(self->keyframe.position);
        if (!position_point) {
            Py_DECREF(state_dict);
            return NULL;
        }
        
        PyObject* position_state = PyPoint_get_state(position_point, NULL);
        Py_DECREF(position_point);
        if (!position_state) {
            Py_DECREF(state_dict);
            return NULL;
        }
        
        PyPoint* in_handle_point = PointToPyPoint(self->keyframe.in_handle);
        if (!in_handle_point) {
            Py_DECREF(position_state);
            Py_DECREF(state_dict);
            return NULL;
        }
        
        PyObject* in_handle_state = PyPoint_get_state(in_handle_point, NULL);
        Py_DECREF(in_handle_point);
        if (!in_handle_state) {
            Py_DECREF(position_state);
            Py_DECREF(state_dict);
            return NULL;
        }
        
        PyPoint* out_handle_point = PointToPyPoint(self->keyframe.out_handle);
        if (!out_handle_point) {
            Py_DECREF(position_state);
            Py_DECREF(in_handle_state);
            Py_DECREF(state_dict);
            return NULL;
        }
        
        PyObject* out_handle_state = PyPoint_get_state(out_handle_point, NULL);
        Py_DECREF(out_handle_point);
        if (!out_handle_state) {
            Py_DECREF(position_state);
            Py_DECREF(in_handle_state);
            Py_DECREF(state_dict);
            return NULL;
        }
        
        // Add function and handle_mode as strings for better readability
        PyObject* function_str = PyUnicode_FromString(function_to_string(self->keyframe.function));
        PyObject* handle_mode_str = PyUnicode_FromString(handle_mode_to_string(self->keyframe.handle_mode));
        
        if (!function_str || !handle_mode_str) {
            Py_XDECREF(function_str);
            Py_XDECREF(handle_mode_str);
            Py_DECREF(position_state);
            Py_DECREF(in_handle_state);
            Py_DECREF(out_handle_state);
            Py_DECREF(state_dict);
            return NULL;
        }
        
        // Set all items in state dict
        if (PyDict_SetItemString(state_dict, "position", position_state) < 0 ||
            PyDict_SetItemString(state_dict, "in_handle", in_handle_state) < 0 ||
            PyDict_SetItemString(state_dict, "out_handle", out_handle_state) < 0 ||
            PyDict_SetItemString(state_dict, "function", function_str) < 0 ||
            PyDict_SetItemString(state_dict, "handle_mode", handle_mode_str) < 0) {
            
            Py_DECREF(function_str);
            Py_DECREF(handle_mode_str);
            Py_DECREF(position_state);
            Py_DECREF(in_handle_state);
            Py_DECREF(out_handle_state);
            Py_DECREF(state_dict);
            return NULL;
        }
        
        Py_DECREF(function_str);
        Py_DECREF(handle_mode_str);
        Py_DECREF(position_state);
        Py_DECREF(in_handle_state);
        Py_DECREF(out_handle_state);
        
        return state_dict;
        
    } catch (const std::exception& e) {
        Py_DECREF(state_dict);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

int PyKeyframe_set_state(PyKeyframe *self, PyObject *value, void *closure) {
    if (!self) {
        PyErr_SetString(PyExc_RuntimeError, "Keyframe object is invalid");
        return -1;
    }
    
    if (!PyDict_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "Keyframe state must be a dictionary");
        return -1;
    }
    
    try {
        // Get position using PyPoint state functions
        PyObject* position_obj = PyDict_GetItemString(value, "position");
        if (!position_obj) {
            PyErr_SetString(PyExc_ValueError, "Keyframe state must have 'position' field");
            return -1;
        }
        
        PyPoint* position_point = PointToPyPoint(anim::Point(0.0, 0.0));
        if (!position_point) {
            return -1;
        }
        
        if (PyPoint_set_state(position_point, position_obj, NULL) < 0) {
            Py_DECREF(position_point);
            return -1;
        }
        
        anim::Point position = position_point->point;
        Py_DECREF(position_point);
        
        // Get handles (with defaults based on position) using PyPoint state functions
        anim::Point in_handle(position.time - 0.3, position.value);
        anim::Point out_handle(position.time + 0.3, position.value);
        
        PyObject* in_handle_obj = PyDict_GetItemString(value, "in_handle");
        if (in_handle_obj) {
            PyPoint* in_handle_point = PointToPyPoint(in_handle);
            if (!in_handle_point) {
                return -1;
            }
            
            if (PyPoint_set_state(in_handle_point, in_handle_obj, NULL) < 0) {
                Py_DECREF(in_handle_point);
                return -1;
            }
            
            in_handle = in_handle_point->point;
            Py_DECREF(in_handle_point);
        }
        
        PyObject* out_handle_obj = PyDict_GetItemString(value, "out_handle");
        if (out_handle_obj) {
            PyPoint* out_handle_point = PointToPyPoint(out_handle);
            if (!out_handle_point) {
                return -1;
            }
            
            if (PyPoint_set_state(out_handle_point, out_handle_obj, NULL) < 0) {
                Py_DECREF(out_handle_point);
                return -1;
            }
            
            out_handle = out_handle_point->point;
            Py_DECREF(out_handle_point);
        }
        
        // Get function and handle mode (with defaults)
        anim::Function function = anim::Function::bezier;
        anim::HandleMode handle_mode = anim::HandleMode::smooth;
        
        PyObject* function_obj = PyDict_GetItemString(value, "function");
        if (function_obj) {
            if (PyLong_Check(function_obj)) {
                // Accept integer values for backward compatibility
                long func_val = PyLong_AsLong(function_obj);
                if (func_val < 0 || func_val >= static_cast<long>(anim::Function::count)) {
                    PyErr_SetString(PyExc_ValueError, "Invalid function value in keyframe state");
                    return -1;
                }
                function = static_cast<anim::Function>(func_val);
            } else if (PyUnicode_Check(function_obj)) {
                // Accept string values
                const char* func_str = PyUnicode_AsUTF8(function_obj);
                function = string_to_function(func_str);
            } else {
                PyErr_SetString(PyExc_TypeError, "Keyframe state 'function' must be an integer or string");
                return -1;
            }
        }
        
        PyObject* handle_mode_obj = PyDict_GetItemString(value, "handle_mode");
        if (handle_mode_obj) {
            if (PyLong_Check(handle_mode_obj)) {
                // Accept integer values for backward compatibility
                long mode_val = PyLong_AsLong(handle_mode_obj);
                if (mode_val < 0 || mode_val >= static_cast<long>(anim::HandleMode::count)) {
                    PyErr_SetString(PyExc_ValueError, "Invalid handle_mode value in keyframe state");
                    return -1;
                }
                handle_mode = static_cast<anim::HandleMode>(mode_val);
            } else if (PyUnicode_Check(handle_mode_obj)) {
                // Accept string values
                const char* mode_str = PyUnicode_AsUTF8(handle_mode_obj);
                handle_mode = string_to_handle_mode(mode_str);
            } else {
                PyErr_SetString(PyExc_TypeError, "Keyframe state 'handle_mode' must be an integer or string");
                return -1;
            }
        }
        
        // Update keyframe
        self->keyframe = anim::Keyframe(position, function, handle_mode, in_handle, out_handle);
        
        return 0;
        
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }
}

static PyObject* PyKeyframe_get_state_method(PyKeyframe *self, PyObject *args) {
    return PyKeyframe_get_state(self, NULL);
}

static PyObject* PyKeyframe_set_state_method(PyKeyframe *self, PyObject *args) {
    PyObject* state;
    if (!PyArg_ParseTuple(args, "O", &state))
        return NULL;
    
    if (PyKeyframe_set_state(self, state, NULL) < 0)
        return NULL;
    
    Py_RETURN_NONE;
}

static PyObject* PyKeyframe_copy(PyKeyframe* self, PyObject*) {
    PyKeyframe* result = KeyframeToPyKeyframe(self->keyframe);
    if (!result) return NULL;
    return (PyObject*)result;
}
static PyObject* PyKeyframe_deepcopy(PyKeyframe* self, PyObject* args) {
    // Ignore memo dict
    PyKeyframe* result = KeyframeToPyKeyframe(self->keyframe);
    if (!result) return NULL;
    return (PyObject*)result;
}


// --- Methods table ---
static PyMethodDef PyKeyframe_methods[] = {
    {"__copy__", (PyCFunction)PyKeyframe_copy, METH_NOARGS, "Shallow copy of Keyframe"},
    {"__deepcopy__", (PyCFunction)PyKeyframe_deepcopy, METH_VARARGS, "Deep copy of Keyframe"},
    {"get_state", (PyCFunction)PyKeyframe_get_state_method, METH_NOARGS, "Get Keyframe state as dictionary"},
    {"set_state", (PyCFunction)PyKeyframe_set_state_method, METH_VARARGS, "Set Keyframe state from dictionary"},
    {NULL, NULL, 0, NULL}
};

// Property definitions
static PyGetSetDef PyKeyframe_getset[] = {
    {"time", (getter)PyKeyframe_get_time, (setter)PyKeyframe_set_time, "Time of the keyframe", NULL},
    {"value", (getter)PyKeyframe_get_value, (setter)PyKeyframe_set_value, "Value of the keyframe", NULL},
    {"in_handle", (getter)PyKeyframe_get_in_handle, (setter)PyKeyframe_set_in_handle, "Incoming handle point", NULL},
    {"out_handle", (getter)PyKeyframe_get_out_handle, (setter)PyKeyframe_set_out_handle, "Outgoing handle point", NULL},
    {"function", (getter)PyKeyframe_get_function, (setter)PyKeyframe_set_function, "Interpolation function", NULL},
    {"handle_mode", (getter)PyKeyframe_get_handle_mode, (setter)PyKeyframe_set_handle_mode, "Handle mode", NULL},
    {"state", (getter)PyKeyframe_get_state, (setter)PyKeyframe_set_state, "Keyframe state as dictionary", NULL},
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

// --- Equality and copy protocol ---
static PyObject* PyKeyframe_richcompare(PyObject* a, PyObject* b, int op) {
    if (!PyObject_TypeCheck(a, &PyKeyframeType) || !PyObject_TypeCheck(b, &PyKeyframeType)) {
        Py_RETURN_NOTIMPLEMENTED;
    }
    const anim::Keyframe& kfa = ((PyKeyframe*)a)->keyframe;
    const anim::Keyframe& kfb = ((PyKeyframe*)b)->keyframe;
    switch (op) {
        case Py_EQ:
            return PyBool_FromLong(kfa == kfb);
        case Py_NE:
            return PyBool_FromLong(kfa != kfb);
        default:
            Py_RETURN_NOTIMPLEMENTED;
    }
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
    PyKeyframe_richcompare,    // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    PyKeyframe_methods,        // tp_methods
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

// Forward declarations for state functions (make them accessible to other files)
extern PyObject* PyKeyframe_get_state(PyKeyframe *self, void *closure);
extern int PyKeyframe_set_state(PyKeyframe *self, PyObject *value, void *closure);

