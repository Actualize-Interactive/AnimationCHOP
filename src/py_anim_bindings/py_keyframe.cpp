#include "py_keyframe.h"
#include "py_point.h"
#include "py_tangent_mode.h"

// Forward declarations of PyKeyframe methods
static PyObject* PyKeyframe_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static void PyKeyframe_dealloc(PyKeyframe *self);
static int PyKeyframe_init(PyKeyframe *self, PyObject *args, PyObject *kwds);
static PyObject* PyKeyframe_get_time(PyKeyframe *self, void *closure);
static PyObject* PyKeyframe_get_value(PyKeyframe *self, void *closure);
static PyObject* PyKeyframe_get_tangent_mode(PyKeyframe *self, void *closure);
static int PyKeyframe_set_tangent_mode(PyKeyframe *self, PyObject *value, void *closure);
static PyObject* PyKeyframe_get_in_tangent(PyKeyframe *self, void *closure);
static PyObject* PyKeyframe_get_out_tangent(PyKeyframe *self, void *closure);
static PyObject* PyKeyframe_str(PyKeyframe *self);

// Define methods for PyKeyframe
static PyMethodDef PyKeyframe_methods[] = {
    {NULL}  // Sentinel
};

// Define getset for PyKeyframe
static PyGetSetDef PyKeyframe_getset[] = {
    {"time", (getter)PyKeyframe_get_time, NULL, "Get keyframe time", NULL},
    {"value", (getter)PyKeyframe_get_value, NULL, "Get keyframe value", NULL},
    {"tangent_mode", (getter)PyKeyframe_get_tangent_mode, (setter)PyKeyframe_set_tangent_mode, "Get/set tangent mode", NULL},
    {"in_tangent", (getter)PyKeyframe_get_in_tangent, NULL, "Get in tangent", NULL},
    {"out_tangent", (getter)PyKeyframe_get_out_tangent, NULL, "Get out tangent", NULL},
    {NULL}  // Sentinel
};

// Keyframe type definition
PyTypeObject PyKeyframeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "anim.Keyframe",           // tp_name
    sizeof(PyKeyframe),        // tp_basicsize
    0,                         // tp_itemsize
    (destructor)PyKeyframe_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)PyKeyframe_str,  // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash 
    0,                         // tp_call
    (reprfunc)PyKeyframe_str,  // tp_str
    PyObject_GenericGetAttr,   // tp_getattro
    PyObject_GenericSetAttr,   // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "Keyframe objects",        // tp_doc 
    0,                         // tp_traverse 
    0,                         // tp_clear 
    0,                         // tp_richcompare 
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

// Implementation of Keyframe methods

static PyObject* PyKeyframe_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyKeyframe *self = (PyKeyframe *)type->tp_alloc(type, 0);
    if (self != NULL) {
        // Initialize with NULL pointer - will be set in init
        self->keyframe = nullptr;
    }
    return (PyObject *)self;
}

static void PyKeyframe_dealloc(PyKeyframe *self) {
    // We don't own the keyframe pointer, so no need to delete it
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int PyKeyframe_init(PyKeyframe *self, PyObject *args, PyObject *kwds) {
    // This should not be called directly - keyframes should be created by channels
    PyErr_SetString(PyExc_TypeError, "Keyframe objects cannot be created directly. Use Channel.add_keyframe instead.");
    return -1;
}

static PyObject* PyKeyframe_get_time(PyKeyframe *self, void *closure) {
    if (!self->keyframe) {
        Py_RETURN_NONE;
    }
    return PyFloat_FromDouble(self->keyframe->time);
}

static PyObject* PyKeyframe_get_value(PyKeyframe *self, void *closure) {
    if (!self->keyframe) {
        Py_RETURN_NONE;
    }
    return PyFloat_FromDouble(self->keyframe->value);
}

static PyObject* PyKeyframe_get_tangent_mode(PyKeyframe *self, void *closure) {
    if (!self->keyframe) {
        Py_RETURN_NONE;
    }
    
    // Get the TangentMode enum module and extract the appropriate enum value
    PyObject* tangent_mode_enum = get_tangent_mode_enum(NULL, NULL);
    if (!tangent_mode_enum) {
        return NULL;
    }
    
    // Get the appropriate enum value
    PyObject* enum_value = NULL;
    switch(self->keyframe->tangent_mode) {
        case anim::TangentMode::linear:
            enum_value = PyObject_GetAttrString(tangent_mode_enum, "LINEAR");
            break;
        case anim::TangentMode::flat:
            enum_value = PyObject_GetAttrString(tangent_mode_enum, "FLAT");
            break;
        case anim::TangentMode::smoothManual:
            enum_value = PyObject_GetAttrString(tangent_mode_enum, "SMOOTH_MANUAL");
            break;
        case anim::TangentMode::smoothAuto:
            enum_value = PyObject_GetAttrString(tangent_mode_enum, "SMOOTH_AUTO");
            break;
        case anim::TangentMode::stepped:
            enum_value = PyObject_GetAttrString(tangent_mode_enum, "STEPPED");
            break;
    }
    
    Py_DECREF(tangent_mode_enum);
    return enum_value;
}

static int PyKeyframe_set_tangent_mode(PyKeyframe *self, PyObject *value, void *closure) {
    if (!self->keyframe) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot set tangent mode on invalid keyframe");
        return -1;
    }
    
    PyObject* tangent_mode_enum = get_tangent_mode_enum(NULL, NULL);
    if (!tangent_mode_enum) {
        return -1;
    }
    
    // Check if value is an instance of TangentMode
    int is_instance = PyObject_IsInstance(value, tangent_mode_enum);
    Py_DECREF(tangent_mode_enum);
    
    if (is_instance != 1) {
        PyErr_SetString(PyExc_TypeError, "tangent_mode must be a TangentMode enum value");
        return -1;
    }
    
    // Convert to integer value
    long mode_value = PyLong_AsLong(value);
    if (mode_value == -1 && PyErr_Occurred()) {
        return -1;
    }
    
    // Set the tangent mode
    self->keyframe->tangent_mode = static_cast<anim::TangentMode>(mode_value);
    return 0;
}

static PyObject* PyKeyframe_get_in_tangent(PyKeyframe *self, void *closure) {
    if (!self->keyframe) {
        Py_RETURN_NONE;
    }
    return Point2DToPyObject(self->keyframe->in_tangent);
}

static PyObject* PyKeyframe_get_out_tangent(PyKeyframe *self, void *closure) {
    if (!self->keyframe) {
        Py_RETURN_NONE;
    }
    return Point2DToPyObject(self->keyframe->out_tangent);
}

static PyObject* PyKeyframe_str(PyKeyframe *self) {
    if (!self->keyframe) {
        return PyUnicode_FromString("Keyframe(invalid)");
    }
    
    char buffer[200]; // Adjust size as needed
    sprintf(buffer, "Keyframe(time=%f, value=%f, mode=%d)", 
            self->keyframe->time, 
            self->keyframe->value, 
            static_cast<int>(self->keyframe->tangent_mode));
    return PyUnicode_FromString(buffer);
}

// External utility functions

PyKeyframe* KeyframeToPyKeyframe(anim::Keyframe* keyframe) {
    PyKeyframe* pyKeyframe = PyObject_New(PyKeyframe, &PyKeyframeType);
    if (pyKeyframe == NULL) {
        return NULL;
    }
    pyKeyframe->keyframe = keyframe;
    return pyKeyframe;
}

PyObject* KeyframeToPyObject(anim::Keyframe* keyframe) {
    PyKeyframe* pyKeyframe = KeyframeToPyKeyframe(keyframe);
    return (PyObject*)pyKeyframe;
}

bool PyObjectToKeyframe(PyObject* obj, anim::Keyframe*& keyframe) {
    if (!PyObject_TypeCheck(obj, &PyKeyframeType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a Keyframe object");
        return false;
    }
    
    PyKeyframe* pyKeyframe = (PyKeyframe*)obj;
    keyframe = pyKeyframe->keyframe;
    return true;
}

PyObject* get_keyframe_type(PyObject* self, void* closure) {
    Py_INCREF(&PyKeyframeType);
    return (PyObject*)&PyKeyframeType;
}