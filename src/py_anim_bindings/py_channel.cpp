#include "py_channel.h"
#include "py_keyframe.h"

// Forward declarations of PyChannel methods
static PyObject* PyChannel_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static void PyChannel_dealloc(PyChannel *self);
static int PyChannel_init(PyChannel *self, PyObject *args, PyObject *kwds);
static PyObject* PyChannel_get_name(PyChannel *self, void *closure);
static PyObject* PyChannel_add_keyframe(PyChannel *self, PyObject *args, PyObject *kwds);
static PyObject* PyChannel_get_keyframe_at_time(PyChannel *self, PyObject *args);
static PyObject* PyChannel_remove_keyframe_at_time(PyChannel *self, PyObject *args);
static PyObject* PyChannel_get_keyframe_count(PyChannel *self);
static PyObject* PyChannel_evaluate(PyChannel *self, PyObject *args);
static PyObject* PyChannel_str(PyChannel *self);

// Define methods for PyChannel
static PyMethodDef PyChannel_methods[] = {
    {"add_keyframe", (PyCFunction)PyChannel_add_keyframe, METH_VARARGS | METH_KEYWORDS, "Add a keyframe to the channel"},
    {"get_keyframe_at_time", (PyCFunction)PyChannel_get_keyframe_at_time, METH_VARARGS, "Get a keyframe at a specific time"},
    {"remove_keyframe_at_time", (PyCFunction)PyChannel_remove_keyframe_at_time, METH_VARARGS, "Remove a keyframe at a specific time"},
    {"get_keyframe_count", (PyCFunction)PyChannel_get_keyframe_count, METH_NOARGS, "Get the number of keyframes in the channel"},
    {"evaluate", (PyCFunction)PyChannel_evaluate, METH_VARARGS, "Evaluate the channel at a specific time"},
    {NULL}  // Sentinel
};

// Define getset for PyChannel
static PyGetSetDef PyChannel_getset[] = {
    {"name", (getter)PyChannel_get_name, NULL, "Channel name", NULL},
    {NULL}  // Sentinel
};

// Channel type definition
PyTypeObject PyChannelType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "anim.Channel",            // tp_name
    sizeof(PyChannel),         // tp_basicsize
    0,                         // tp_itemsize
    (destructor)PyChannel_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)PyChannel_str,   // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash 
    0,                         // tp_call
    (reprfunc)PyChannel_str,   // tp_str
    PyObject_GenericGetAttr,   // tp_getattro
    PyObject_GenericSetAttr,   // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "Channel objects",         // tp_doc 
    0,                         // tp_traverse 
    0,                         // tp_clear 
    0,                         // tp_richcompare 
    0,                         // tp_weaklistoffset 
    0,                         // tp_iter 
    0,                         // tp_iternext 
    PyChannel_methods,         // tp_methods 
    0,                         // tp_members 
    PyChannel_getset,          // tp_getset 
    0,                         // tp_base 
    0,                         // tp_dict 
    0,                         // tp_descr_get 
    0,                         // tp_descr_set 
    0,                         // tp_dictoffset 
    (initproc)PyChannel_init,  // tp_init 
    0,                         // tp_alloc 
    PyChannel_new,             // tp_new 
};

// Implementation of Channel methods

static PyObject* PyChannel_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyChannel *self = (PyChannel *)type->tp_alloc(type, 0);
    if (self != NULL) {
        // Initialize with NULL pointer - will be set in init
        self->channel = nullptr;
        self->ownsChannel = false;
    }
    return (PyObject *)self;
}

static void PyChannel_dealloc(PyChannel *self) {
    // If we own the channel, delete it
    if (self->ownsChannel && self->channel) {
        delete self->channel;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int PyChannel_init(PyChannel *self, PyObject *args, PyObject *kwds) {
    // This should not be called directly - channels should be created by animations
    PyErr_SetString(PyExc_TypeError, "Channel objects cannot be created directly. Use Animation.add_channel instead.");
    return -1;
}

static PyObject* PyChannel_get_name(PyChannel *self, void *closure) {
    if (!self->channel) {
        Py_RETURN_NONE;
    }
    return PyUnicode_FromString(self->channel->name.c_str());
}

static PyObject* PyChannel_add_keyframe(PyChannel *self, PyObject *args, PyObject *kwds) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot add keyframe to invalid channel");
        return NULL;
    }
    
    double time, value;
    PyObject* tangent_mode_obj = NULL;
    double in_tangent_time = 0.0, in_tangent_value = 0.0;
    double out_tangent_time = 0.0, out_tangent_value = 0.0;
    
    static char *kwlist[] = {
        (char*)"time", (char*)"value", (char*)"tangent_mode", 
        (char*)"in_tangent_time", (char*)"in_tangent_value", 
        (char*)"out_tangent_time", (char*)"out_tangent_value", NULL
    };
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "dd|Odddd", kwlist, 
                                     &time, &value, &tangent_mode_obj,
                                     &in_tangent_time, &in_tangent_value,
                                     &out_tangent_time, &out_tangent_value)) {
        return NULL;
    }
    
    // Default tangent mode is smoothAuto
    anim::TangentMode tangent_mode = anim::TangentMode::smoothAuto;
    
    // If tangent_mode was provided, convert it
    if (tangent_mode_obj) {
        // Make sure it's an integer (or IntEnum which is also an int)
        if (!PyLong_Check(tangent_mode_obj)) {
            PyErr_SetString(PyExc_TypeError, "tangent_mode must be a TangentMode enum value");
            return NULL;
        }
        
        long mode_value = PyLong_AsLong(tangent_mode_obj);
        if (mode_value == -1 && PyErr_Occurred()) {
            return NULL;
        }
        
        tangent_mode = static_cast<anim::TangentMode>(mode_value);
    }
    
    // Create in and out tangent points
    anim::Point2D in_tangent(in_tangent_time, in_tangent_value);
    anim::Point2D out_tangent(out_tangent_time, out_tangent_value);
    
    // Add keyframe to the channel
    anim::Keyframe* keyframe = self->channel->add_keyframe(time, value, tangent_mode, in_tangent, out_tangent);
    
    // Return the keyframe as a PyKeyframe object
    return KeyframeToPyObject(keyframe);
}

static PyObject* PyChannel_get_keyframe_at_time(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot get keyframe from invalid channel");
        return NULL;
    }
    
    double time;
    if (!PyArg_ParseTuple(args, "d", &time)) {
        return NULL;
    }
    
    // Get the keyframe at the specified time
    anim::Keyframe* keyframe = self->channel->get_keyframe_at_time(time);
    
    if (!keyframe) {
        Py_RETURN_NONE;
    }
    
    // Return the keyframe as a PyKeyframe object
    return KeyframeToPyObject(keyframe);
}

static PyObject* PyChannel_remove_keyframe_at_time(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot remove keyframe from invalid channel");
        return NULL;
    }
    
    double time;
    if (!PyArg_ParseTuple(args, "d", &time)) {
        return NULL;
    }
    
    // Remove the keyframe at the specified time
    bool success = self->channel->remove_keyframe_at_time(time);
    
    if (success) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

static PyObject* PyChannel_get_keyframe_count(PyChannel *self) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot get keyframe count from invalid channel");
        return NULL;
    }
    
    return PyLong_FromSize_t(self->channel->get_keyframe_count());
}

static PyObject* PyChannel_evaluate(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot evaluate invalid channel");
        return NULL;
    }
    
    double time;
    if (!PyArg_ParseTuple(args, "d", &time)) {
        return NULL;
    }
    
    // Evaluate the channel at the specified time
    double value = self->channel->evaluate(time);
    
    return PyFloat_FromDouble(value);
}

static PyObject* PyChannel_str(PyChannel *self) {
    if (!self->channel) {
        return PyUnicode_FromString("Channel(invalid)");
    }
    
    std::string str = "Channel('" + self->channel->name + "', keyframes=" + 
                      std::to_string(self->channel->get_keyframe_count()) + ")";
    return PyUnicode_FromString(str.c_str());
}

// External utility functions

PyChannel* ChannelToPyChannel(anim::Channel* channel, bool ownsChannel) {
    PyChannel* pyChannel = PyObject_New(PyChannel, &PyChannelType);
    if (pyChannel == NULL) {
        return NULL;
    }
    pyChannel->channel = channel;
    pyChannel->ownsChannel = ownsChannel;
    return pyChannel;
}

PyObject* ChannelToPyObject(anim::Channel* channel, bool ownsChannel) {
    PyChannel* pyChannel = ChannelToPyChannel(channel, ownsChannel);
    return (PyObject*)pyChannel;
}

bool PyObjectToChannel(PyObject* obj, anim::Channel*& channel) {
    if (!PyObject_TypeCheck(obj, &PyChannelType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a Channel object");
        return false;
    }
    
    PyChannel* pyChannel = (PyChannel*)obj;
    channel = pyChannel->channel;
    return true;
}

PyObject* get_channel_type(PyObject* self, void* closure) {
    Py_INCREF(&PyChannelType);
    return (PyObject*)&PyChannelType;
}