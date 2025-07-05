#include "py_keyframe.h"
#include "py_function.h"
#include "py_handle_mode.h"
#include <format>

// Allocation/deallocation functions
static PyObject* PY_Keyframe_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PY_Keyframe *self = (PY_Keyframe *)type->tp_alloc(type, 0);
    if (self != NULL) {
        new (&self->keyframe) anim::Keyframe(0.0, 0.0);
    }
    return (PyObject *)self;
}

static void PY_Keyframe_dealloc(PY_Keyframe *self) {
    self->keyframe.~Keyframe(); 
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Initialize the object
static int PY_Keyframe_init(PY_Keyframe *self, PyObject *args, PyObject *kwds) {
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
    anim::Function function = anim::Function::Bezier;
    anim::HandleMode handle_mode = anim::HandleMode::Smooth;
    
    // Parse in_handle if provided
    if (in_handle_obj) {
        if (!PY_ObjectToPoint(in_handle_obj, in_handle)) {
            // PY_ObjectToPoint already sets the error
            return -1;
        }
    }
    
    // Parse out_handle if provided
    if (out_handle_obj) {
        if (!PY_ObjectToPoint(out_handle_obj, out_handle)) {
            // PY_ObjectToPoint already sets the error
            return -1;
        }
    }
    
    // Parse function if provided
    if (function_obj) {
        if (PyLong_Check(function_obj)) {
            long function_val = PyLong_AsLong(function_obj);
            
            if (function_val >= 0 && function_val < static_cast<long>(anim::Function::Count)) { 
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
            
            if (mode_val >= 0 && mode_val < static_cast<long>(anim::HandleMode::Count)) { 
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
static PyObject* PY_Keyframe_get_time(PY_Keyframe *self, void *closure) {
    return PyFloat_FromDouble(self->keyframe.position.time);
}

static int PY_Keyframe_set_time(PY_Keyframe *self, PyObject *value, void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The time attribute must be a float");
        return -1;
    }

    self->keyframe.position.time = PyFloat_AsDouble(value);
    return 0;
}

static PyObject* PY_Keyframe_get_value(PY_Keyframe *self, void *closure) {
    return PyFloat_FromDouble(self->keyframe.position.value);
}

static int PY_Keyframe_set_value(PY_Keyframe *self, PyObject *value, void *closure) {
    if (!PyFloat_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "The value attribute must be a float");
        return -1;
    }

    self->keyframe.position.value = PyFloat_AsDouble(value);
    return 0;
}

static PyObject* PY_Keyframe_get_in_handle(PY_Keyframe *self, void *closure) {
    return PointToPY_Object(self->keyframe.in_handle);
}

static int PY_Keyframe_set_in_handle(PY_Keyframe *self, PyObject *value, void *closure) {
    anim::Point handle;
    if (!PY_ObjectToPoint(value, handle)) {
        // PY_ObjectToPoint already sets the error
        return -1;
    }
    
    self->keyframe.in_handle = handle;
    
    return 0;
}

static PyObject* PY_Keyframe_get_out_handle(PY_Keyframe *self, void *closure) {
    return PointToPY_Object(self->keyframe.out_handle);
}

static int PY_Keyframe_set_out_handle(PY_Keyframe *self, PyObject *value, void *closure) {
    anim::Point handle;
    if (!PY_ObjectToPoint(value, handle)) {
        // PY_ObjectToPoint already sets the error
        return -1;
    }
    
    self->keyframe.out_handle = handle;
    
    return 0;
}

static PyObject* PY_Keyframe_get_function(PY_Keyframe *self, void *closure) {
    return PyLong_FromLong(static_cast<long>(self->keyframe.function));
}

static int PY_Keyframe_set_function(PY_Keyframe *self, PyObject *value, void *closure) {
    if (PyLong_Check(value)) {
        long function_val = PyLong_AsLong(value);
        if (function_val >= 0 && function_val < static_cast<long>(anim::Function::Count)) {
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

static PyObject* PY_Keyframe_get_handle_mode(PY_Keyframe *self, void *closure) {
    return PyLong_FromLong(static_cast<long>(self->keyframe.handle_mode));
}

static int PY_Keyframe_set_handle_mode(PY_Keyframe *self, PyObject *value, void *closure) {
    if (PyLong_Check(value)) {
        long mode_val = PyLong_AsLong(value);
        if (mode_val >= 0 && mode_val < static_cast<long>(anim::HandleMode::Count)) {
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
PyObject* PY_Keyframe_get_state(PY_Keyframe *self, void *closure) {
    if (!self) {
        PyErr_SetString(PyExc_RuntimeError, "Keyframe object is invalid");
        return NULL;
    }
    
    PyObject* state_dict = PyDict_New();
    if (!state_dict) return NULL;
    
    try {
        // Use PY_Point state functions for position and handles
        PY_Point* position_point = PointToPY_Point(self->keyframe.position);
        if (!position_point) {
            Py_DECREF(state_dict);
            return NULL;
        }
        
        PyObject* position_state = PY_Point_get_state(position_point, NULL);
        Py_DECREF(position_point);
        if (!position_state) {
            Py_DECREF(state_dict);
            return NULL;
        }
        
        PY_Point* in_handle_point = PointToPY_Point(self->keyframe.in_handle);
        if (!in_handle_point) {
            Py_DECREF(position_state);
            Py_DECREF(state_dict);
            return NULL;
        }
        
        PyObject* in_handle_state = PY_Point_get_state(in_handle_point, NULL);
        Py_DECREF(in_handle_point);
        if (!in_handle_state) {
            Py_DECREF(position_state);
            Py_DECREF(state_dict);
            return NULL;
        }
        
        PY_Point* out_handle_point = PointToPY_Point(self->keyframe.out_handle);
        if (!out_handle_point) {
            Py_DECREF(position_state);
            Py_DECREF(in_handle_state);
            Py_DECREF(state_dict);
            return NULL;
        }
        
        PyObject* out_handle_state = PY_Point_get_state(out_handle_point, NULL);
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

int PY_Keyframe_set_state(PY_Keyframe *self, PyObject *value, void *closure) {
    if (!self) {
        PyErr_SetString(PyExc_RuntimeError, "Keyframe object is invalid");
        return -1;
    }
    
    if (!PyDict_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "Keyframe state must be a dictionary");
        return -1;
    }
    
    try {
        // Get position using PY_Point state functions
        PyObject* position_obj = PyDict_GetItemString(value, "position");
        if (!position_obj) {
            PyErr_SetString(PyExc_ValueError, "Keyframe state must have 'position' field");
            return -1;
        }
        
        PY_Point* position_point = PointToPY_Point(anim::Point(0.0, 0.0));
        if (!position_point) {
            return -1;
        }
        
        if (PY_Point_set_state(position_point, position_obj, NULL) < 0) {
            Py_DECREF(position_point);
            return -1;
        }
        
        anim::Point position = position_point->point;
        Py_DECREF(position_point);
        
        // Get handles (with defaults based on position) using PY_Point state functions
        anim::Point in_handle(position.time - 0.3, position.value);
        anim::Point out_handle(position.time + 0.3, position.value);
        
        PyObject* in_handle_obj = PyDict_GetItemString(value, "in_handle");
        if (in_handle_obj) {
            PY_Point* in_handle_point = PointToPY_Point(in_handle);
            if (!in_handle_point) {
                return -1;
            }
            
            if (PY_Point_set_state(in_handle_point, in_handle_obj, NULL) < 0) {
                Py_DECREF(in_handle_point);
                return -1;
            }
            
            in_handle = in_handle_point->point;
            Py_DECREF(in_handle_point);
        }
        
        PyObject* out_handle_obj = PyDict_GetItemString(value, "out_handle");
        if (out_handle_obj) {
            PY_Point* out_handle_point = PointToPY_Point(out_handle);
            if (!out_handle_point) {
                return -1;
            }
            
            if (PY_Point_set_state(out_handle_point, out_handle_obj, NULL) < 0) {
                Py_DECREF(out_handle_point);
                return -1;
            }
            
            out_handle = out_handle_point->point;
            Py_DECREF(out_handle_point);
        }
        
        // Get function and handle mode (with defaults)
        anim::Function function = anim::Function::Bezier;
        anim::HandleMode handle_mode = anim::HandleMode::Smooth;
        
        PyObject* function_obj = PyDict_GetItemString(value, "function");
        if (function_obj) {
            if (PyLong_Check(function_obj)) {
                // Accept integer values for backward compatibility
                long func_val = PyLong_AsLong(function_obj);
                if (func_val < 0 || func_val >= static_cast<long>(anim::Function::Count)) {
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
                if (mode_val < 0 || mode_val >= static_cast<long>(anim::HandleMode::Count)) {
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

static PyObject* PY_Keyframe_get_state_method(PY_Keyframe *self, PyObject *args) {
    return PY_Keyframe_get_state(self, NULL);
}

static PyObject* PY_Keyframe_set_state_method(PY_Keyframe *self, PyObject *args) {
    PyObject* state;
    if (!PyArg_ParseTuple(args, "O", &state))
        return NULL;
    
    if (PY_Keyframe_set_state(self, state, NULL) < 0)
        return NULL;
    
    Py_RETURN_NONE;
}

static PyObject* PY_Keyframe_copy(PY_Keyframe* self, PyObject*) {
    PY_Keyframe* result = KeyframeToPY_Keyframe(self->keyframe);
    if (!result) return NULL;
    return (PyObject*)result;
}
static PyObject* PY_Keyframe_deepcopy(PY_Keyframe* self, PyObject* args) {
    // Ignore memo dict
    PY_Keyframe* result = KeyframeToPY_Keyframe(self->keyframe);
    if (!result) return NULL;
    return (PyObject*)result;
}


// --- Methods table ---
static PyMethodDef PY_Keyframe_methods[] = {
    {"__copy__", (PyCFunction)PY_Keyframe_copy, METH_NOARGS, "Shallow copy of Keyframe"},
    {"__deepcopy__", (PyCFunction)PY_Keyframe_deepcopy, METH_VARARGS, "Deep copy of Keyframe"},
    {"get_state", (PyCFunction)PY_Keyframe_get_state_method, METH_NOARGS, "Get Keyframe state as dictionary"},
    {"set_state", (PyCFunction)PY_Keyframe_set_state_method, METH_VARARGS, "Set Keyframe state from dictionary"},
    {NULL, NULL, 0, NULL}
};

// Property definitions
static PyGetSetDef PY_Keyframe_getset[] = {
    {"time", (getter)PY_Keyframe_get_time, (setter)PY_Keyframe_set_time, "Time of the keyframe", NULL},
    {"value", (getter)PY_Keyframe_get_value, (setter)PY_Keyframe_set_value, "Value of the keyframe", NULL},
    {"in_handle", (getter)PY_Keyframe_get_in_handle, (setter)PY_Keyframe_set_in_handle, "Incoming handle point", NULL},
    {"out_handle", (getter)PY_Keyframe_get_out_handle, (setter)PY_Keyframe_set_out_handle, "Outgoing handle point", NULL},
    {"function", (getter)PY_Keyframe_get_function, (setter)PY_Keyframe_set_function, "Interpolation function", NULL},
    {"handle_mode", (getter)PY_Keyframe_get_handle_mode, (setter)PY_Keyframe_set_handle_mode, "Handle mode", NULL},
    {"state", (getter)PY_Keyframe_get_state, (setter)PY_Keyframe_set_state, "Keyframe state as dictionary", NULL},
    {NULL}  // Sentinel
};

// String representation
static PyObject* PY_Keyframe_str(PY_Keyframe *self) {
    auto str = std::format("Keyframe(time={}, value={}, function={}, handle_mode={})", 
                            self->keyframe.position.time, 
                            self->keyframe.position.value, 
                            static_cast<int>(self->keyframe.function),
                            static_cast<int>(self->keyframe.handle_mode));
    return PyUnicode_FromString(str.c_str());
}

// --- Equality and copy protocol ---
static PyObject* PY_Keyframe_richcompare(PyObject* a, PyObject* b, int op) {
    if (!PyObject_TypeCheck(a, &PY_KeyframeType) || !PyObject_TypeCheck(b, &PY_KeyframeType)) {
        Py_RETURN_NOTIMPLEMENTED;
    }
    const anim::Keyframe& kfa = ((PY_Keyframe*)a)->keyframe;
    const anim::Keyframe& kfb = ((PY_Keyframe*)b)->keyframe;
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
PyTypeObject PY_KeyframeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "anim.Keyframe",          // tp_name
    sizeof(PY_Keyframe),       // tp_basicsize
    0,                         // tp_itemsize
    (destructor)PY_Keyframe_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)PY_Keyframe_str, // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash 
    0,                         // tp_call
    (reprfunc)PY_Keyframe_str, // tp_str
    PyObject_GenericGetAttr,   // tp_getattro
    PyObject_GenericSetAttr,   // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "Keyframe object",        // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    PY_Keyframe_richcompare,    // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    PY_Keyframe_methods,        // tp_methods
    0,                         // tp_members
    PY_Keyframe_getset,         // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)PY_Keyframe_init, // tp_init
    0,                         // tp_alloc
    PY_Keyframe_new,            // tp_new
};

// Type getter for external use
PyObject* get_keyframe_type(PyObject* self, void* closure) {
    if (PyType_Ready(&PY_KeyframeType) < 0) {
        return NULL;
    }
    Py_INCREF(&PY_KeyframeType);
    return (PyObject*)&PY_KeyframeType;
}

PY_Keyframe* KeyframeToPY_Keyframe(const anim::Keyframe& keyframe) {
    // Ensure the type is initialized before creating an instance
    if (PyType_Ready(&PY_KeyframeType) < 0) {
        return NULL;
    }
    PY_Keyframe* py_keyframe = PyObject_New(PY_Keyframe, &PY_KeyframeType);
    if (py_keyframe == NULL) {
        return NULL;
    }
    new(&py_keyframe->keyframe) anim::Keyframe(keyframe);
    return py_keyframe;
}

// Helper functions for conversion between C++ and Python
PyObject* KeyframeToPY_Object(const anim::Keyframe& keyframe) {
    return (PyObject*)KeyframeToPY_Keyframe(keyframe);
}

bool PY_KeyframeToKeyframe(PY_Keyframe* py_keyframe, anim::Keyframe& keyframe) {
    if (!py_keyframe) { // This check might be redundant if type checking is done before calling
        PyErr_SetString(PyExc_TypeError, "Expected a Keyframe object, got NULL");
        return false;
    }
    if (!PyObject_TypeCheck(py_keyframe, &PY_KeyframeType)) { // Ensure it's the correct type
        PyErr_SetString(PyExc_TypeError, "Expected a Keyframe object");
        return false;
    }
    keyframe = py_keyframe->keyframe;
    return true;
}

bool PY_ObjectToKeyframe(PyObject* obj, anim::Keyframe& keyframe) {
    if (!PyObject_TypeCheck(obj, &PY_KeyframeType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a Keyframe object");
        return false;
    }
    
    PY_Keyframe* py_keyframe = (PY_Keyframe*)obj;
    keyframe = py_keyframe->keyframe; // Direct assignment if Keyframe is copyable
    return true;
}

// Forward declarations for state functions (make them accessible to other files)
extern PyObject* PY_Keyframe_get_state(PY_Keyframe *self, void *closure);
extern int PY_Keyframe_set_state(PY_Keyframe *self, PyObject *value, void *closure);

