#include "py_channel.h"
#include "py_keyframe.h" // For KeyframeToPyObject, PyKeyframeType
#include <vector>
#include <optional>

// Allocation/deallocation functions
static PyObject* PyChannel_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyChannel *self = (PyChannel *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->channel = nullptr;
        self->parent = nullptr;
    }
    return (PyObject *)self;
}

static void PyChannel_dealloc(PyChannel *self) {
    Py_XDECREF(self->parent);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Initialize the object - NOTE: This should not be called directly, use ChannelToPyObject instead
static int PyChannel_init(PyChannel *self, PyObject *args, PyObject *kwds) {
    PyErr_SetString(PyExc_RuntimeError, "Channel objects cannot be created directly. Use AnimationCHOP methods to create channels.");
    return -1;
}

static PyObject* PyChannel_remove_keyframe(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    size_t index;
    
    if (!PyArg_ParseTuple(args, "k", &index))
        return NULL;

    try {
        self->channel->delete_keyframe(index);
        Py_RETURN_TRUE;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyChannel_remove_keyframe_at_time(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    double time;
    
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;

    try {
        self->channel->delete_keyframe(time);
        Py_RETURN_TRUE;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyChannel_get_keyframe(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    size_t index;
    if (!PyArg_ParseTuple(args, "k", &index))
        return NULL;
    
    try {
        const anim::Keyframe& keyframe = self->channel->keyframe(index);
        return KeyframeToPyObject(keyframe);
    } catch (const std::out_of_range& e) {
        PyErr_SetString(PyExc_IndexError, e.what());
        return NULL;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyChannel_get_keyframe_at_time(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    double time;

    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;

    try {
        const anim::Keyframe& keyframe = self->channel->keyframe(time);
        return KeyframeToPyObject(keyframe);
    } catch (const std::exception& e) {
        // No keyframe at this time
        Py_RETURN_NONE;
    }
}

static PyObject* PyChannel_get_all_keyframes(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    size_t num_keyframes = self->channel->num_keyframes();

    PyObject* keyframes_list = PyList_New(num_keyframes);
    if (!keyframes_list) {
        return NULL;
    }
    
    for (size_t i = 0; i < num_keyframes; ++i) {
        try {
            const anim::Keyframe& keyframe = self->channel->keyframe(i);
            PyObject* py_keyframe = KeyframeToPyObject(keyframe);
            if (!py_keyframe) {
                Py_DECREF(keyframes_list);
                return NULL;
            }
            PyList_SET_ITEM(keyframes_list, i, py_keyframe); // Steals reference
        } catch (const std::exception& e) {
            Py_DECREF(keyframes_list);
            PyErr_SetString(PyExc_RuntimeError, e.what());
            return NULL;
        }
    }
    return keyframes_list;
}

static PyObject* PyChannel_evaluate(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    double time;
    
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;
    
    try {
        double value = self->channel->evaluate(time);
        return PyFloat_FromDouble(value);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyChannel_is_empty(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    bool is_empty = (self->channel->num_keyframes() == 0);
    return PyBool_FromLong(is_empty ? 1 : 0);
}

static PyObject* PyChannel_get_start_time(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    if (self->channel->num_keyframes() == 0) {
        Py_RETURN_NONE;
    }
    
    return PyFloat_FromDouble(self->channel->start_time());
}

static PyObject* PyChannel_get_end_time(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    if (self->channel->num_keyframes() == 0) {
        Py_RETURN_NONE;
    }
    
    return PyFloat_FromDouble(self->channel->end_time());
}

static PyObject* PyChannel_get_name(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    return PyUnicode_FromString(self->channel->name().c_str());
}

static PyObject* PyChannel_get_num_keyframes(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    return PyLong_FromSize_t(self->channel->num_keyframes());
}

// Method definitions
static PyMethodDef PyChannel_methods[] = {
    {"remove_keyframe", (PyCFunction)PyChannel_remove_keyframe, METH_VARARGS,
     "Remove a keyframe at the specified index"},
    {"remove_keyframe_at_time", (PyCFunction)PyChannel_remove_keyframe_at_time, METH_VARARGS,
     "Remove a keyframe at the specified time"},
    {"get_keyframe", (PyCFunction)PyChannel_get_keyframe, METH_VARARGS,
     "Get the keyframe at the specified index"},
    {"get_keyframe_at_time", (PyCFunction)PyChannel_get_keyframe_at_time, METH_VARARGS,
     "Get the keyframe at the specified time, or None if not found"},
    {"get_all_keyframes", (PyCFunction)PyChannel_get_all_keyframes, METH_NOARGS,
     "Get all keyframes in this channel"},
    {"evaluate", (PyCFunction)PyChannel_evaluate, METH_VARARGS,
     "Evaluate the channel at a specific time"},
    {"is_empty", (PyCFunction)PyChannel_is_empty, METH_NOARGS,
     "Check if the channel has no keyframes"},
    {"get_start_time", (PyCFunction)PyChannel_get_start_time, METH_NOARGS,
     "Get the earliest keyframe time"},
    {"get_end_time", (PyCFunction)PyChannel_get_end_time, METH_NOARGS,
     "Get the latest keyframe time, or None if the channel is empty"},
    {"get_name", (PyCFunction)PyChannel_get_name, METH_NOARGS,
     "Get the channel name"},
    {"get_num_keyframes", (PyCFunction)PyChannel_get_num_keyframes, METH_NOARGS,
     "Get the number of keyframes in this channel"},
    {NULL}  // Sentinel
};

// String representation
static PyObject* PyChannel_str(PyChannel *self) {
    if (!self->channel) {
        return PyUnicode_FromString("Channel (invalid)");
    }
    
    size_t num_keyframes = self->channel->num_keyframes();
    return PyUnicode_FromFormat("Channel('%s') with %zu keyframes", 
                                self->channel->name().c_str(), num_keyframes);
}

// Type definition
PyTypeObject PyChannelType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "anim.Channel",          // tp_name
    sizeof(PyChannel),       // tp_basicsize
    0,                         // tp_itemsize
    (destructor)PyChannel_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)PyChannel_str,  // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash 
    0,                         // tp_call
    (reprfunc)PyChannel_str,  // tp_str
    PyObject_GenericGetAttr,   // tp_getattro
    PyObject_GenericSetAttr,   // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "Channel object",         // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    PyChannel_methods,         // tp_methods
    0,                         // tp_members
    0,                         // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)PyChannel_init,  // tp_init
    0,                         // tp_alloc
    PyChannel_new,             // tp_new
};

// Type getter for external use
PyObject* get_channel_type(PyObject* self, void* closure) {
    if (PyType_Ready(&PyChannelType) < 0) { // Ensure type is ready
        return NULL;
    }
    Py_INCREF(&PyChannelType);
    return (PyObject*)&PyChannelType;
}

// Helper functions for conversion between C++ and Python
PyObject* ChannelToPyObject(anim::Channel* channel, PyObject* parent) {
    // Ensure the type is initialized before creating an instance
    if (PyType_Ready(&PyChannelType) < 0) {
        return NULL;
    }
    PyChannel* py_channel = PyObject_New(PyChannel, &PyChannelType);
    if (py_channel == NULL) {
        return NULL;
    }
    py_channel->channel = channel;
    py_channel->parent = parent;
    Py_XINCREF(parent); // Keep parent alive
    return (PyObject*)py_channel;
}

bool PyObjectToChannel(PyObject* obj, anim::Channel*& channel) {
    if (!PyObject_TypeCheck(obj, &PyChannelType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a Channel object");
        return false;
    }
    
    PyChannel* py_channel = (PyChannel*)obj;
    if (!py_channel->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return false;
    }
    
    channel = py_channel->channel;
    return true;
}
