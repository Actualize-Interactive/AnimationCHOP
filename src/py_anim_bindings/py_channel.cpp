#include "py_channel.h"
#include "py_keyframe.h" // For KeyframeToPyKeyframe, PyKeyframeType
#include "py_point.h"    // For PyObjectToPoint2D
#include <vector>
#include <optional>

// Allocation/deallocation functions
static PyObject* PyChannel_new(PyTypeObject *type, [[maybe_unused]] PyObject *args, [[maybe_unused]] PyObject *kwds) {
    PyChannel *self = (PyChannel *)type->tp_alloc(type, 0);
    if (self != NULL) {
        new (&self->channel) anim::Channel();
    }
    return (PyObject *)self;
}

static void PyChannel_dealloc(PyChannel *self) {
    self->channel.~Channel();
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Initialize the object
static int PyChannel_init(PyChannel *self, PyObject *args, PyObject *kwds) {
    const char* name = "";
    static char *kwlist[] = {(char*)"name", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &name))
        return -1;
    
    self->channel = anim::Channel(name);
    return 0;
}

static PyObject* PyChannel_remove_keyframe(PyChannel *self, PyObject *args) {
    double time;
    
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;

    bool result = self->channel.remove_keyframe(time);
    return PyBool_FromLong(result ? 1 : 0);
}

static PyObject* PyChannel_get_keyframe(PyChannel *self, PyObject *args) {
    double time;
    
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;

    std::optional<anim::Keyframe> keyframe_opt = self->channel.get_keyframe(time);
    if (!keyframe_opt) {
        Py_RETURN_NONE;
    }
    
    return KeyframeToPyObject(keyframe_opt.value());
}

static PyObject* PyChannel_get_all_keyframes(PyChannel *self, [[maybe_unused]] PyObject *args) {
    const std::vector<anim::Keyframe>& keyframes = self->channel.get_all_keyframes();

    PyObject* keyframes_list = PyList_New(keyframes.size());
    if (!keyframes_list) {
        return NULL;
    }
    
    for (size_t i = 0; i < keyframes.size(); ++i) {
        PyObject* py_keyframe = KeyframeToPyObject(keyframes[i]);
        if (!py_keyframe) {
            Py_DECREF(keyframes_list);
            return NULL;
        }
        PyList_SET_ITEM(keyframes_list, i, py_keyframe); // Steals reference
    }
    return keyframes_list;
}

static PyObject* PyChannel_evaluate(PyChannel *self, PyObject *args) {
    double time;
    
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;
    
    try {
        double value = self->channel.evaluate(time);
        return PyFloat_FromDouble(value);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyChannel_evaluate_range(PyChannel *self, PyObject *args) {
    double start_time, end_time;
    int num_samples;
    
    if (!PyArg_ParseTuple(args, "ddi", &start_time, &end_time, &num_samples))
        return NULL;
    
    try {
        std::vector<double> values = self->channel.evaluate_range(start_time, end_time, num_samples);

        PyObject* values_list = PyList_New(values.size());
        if (!values_list) {
            return NULL;
        }
        
        for (size_t i = 0; i < values.size(); ++i) {
            PyObject* py_value = PyFloat_FromDouble(values[i]);
            if (!py_value) {
                Py_DECREF(values_list);
                return NULL;
            }
            PyList_SET_ITEM(values_list, i, py_value);
        }
        
        return values_list;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyChannel_evaluate_range_by_rate(PyChannel *self, PyObject *args) {
    double start_time, end_time, sample_rate;
    
    if (!PyArg_ParseTuple(args, "ddd", &start_time, &end_time, &sample_rate))
        return NULL;
    
    try {
        std::vector<double> values = self->channel.evaluate_range_by_rate(start_time, end_time, sample_rate);

        PyObject* values_list = PyList_New(values.size());
        if (!values_list) {
            return NULL;
        }
        
        for (size_t i = 0; i < values.size(); ++i) {
            PyObject* py_value = PyFloat_FromDouble(values[i]);
            if (!py_value) {
                Py_DECREF(values_list);
                return NULL;
            }
            PyList_SET_ITEM(values_list, i, py_value);
        }
        
        return values_list;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyChannel_is_empty(PyChannel *self, [[maybe_unused]] PyObject *args) {
    bool is_empty = self->channel.is_empty();
    return PyBool_FromLong(is_empty ? 1 : 0);
}

static PyObject* PyChannel_get_start_time(PyChannel *self, [[maybe_unused]] PyObject *args) {
    std::optional<double> start_time = self->channel.get_start_time();
    if (start_time) {
        return PyFloat_FromDouble(*start_time);
    } else {
        Py_RETURN_NONE;
    }
}

static PyObject* PyChannel_get_end_time(PyChannel *self, [[maybe_unused]] PyObject *args) {
    std::optional<double> end_time = self->channel.get_end_time();
    if (end_time) {
        return PyFloat_FromDouble(*end_time);
    } else {
        Py_RETURN_NONE;
    }
}

// Method definitions
static PyMethodDef PyChannel_methods[] = {
    {"remove_keyframe", (PyCFunction)PyChannel_remove_keyframe, METH_VARARGS,
     "Remove a keyframe at the specified time"},
    {"get_keyframe", (PyCFunction)PyChannel_get_keyframe, METH_VARARGS,
     "Get the keyframe at the specified time"},
    {"get_all_keyframes", (PyCFunction)PyChannel_get_all_keyframes, METH_NOARGS,
     "Get all keyframes in this channel"},
    {"evaluate", (PyCFunction)PyChannel_evaluate, METH_VARARGS,
     "Evaluate the channel at a specific time"},
    {"evaluate_range", (PyCFunction)PyChannel_evaluate_range, METH_VARARGS,
     "Evaluate the channel over a range with a fixed number of samples"},
    {"evaluate_range_by_rate", (PyCFunction)PyChannel_evaluate_range_by_rate, METH_VARARGS,
     "Evaluate the channel over a range with a specific sample rate"},
    {"is_empty", (PyCFunction)PyChannel_is_empty, METH_NOARGS,
     "Check if the channel has no keyframes"},
    {"get_start_time", (PyCFunction)PyChannel_get_start_time, METH_NOARGS,
     "Get the earliest keyframe time"},
    {"get_end_time", (PyCFunction)PyChannel_get_end_time, METH_NOARGS,
     "Get the latest keyframe time, or None if the channel is empty"},
    {NULL}  // Sentinel
};

// String representation
static PyObject* PyChannel_str(PyChannel *self) {
    const std::vector<anim::Keyframe>& keyframes = self->channel.get_all_keyframes();
    return PyUnicode_FromFormat("Channel with %zu keyframes", keyframes.size());
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
[[maybe_unused]] PyObject* get_channel_type([[maybe_unused]] PyObject* self, [[maybe_unused]] void* closure) {
    if (PyType_Ready(&PyChannelType) < 0) { // Ensure type is ready
        return NULL;
    }
    Py_INCREF(&PyChannelType);
    return (PyObject*)&PyChannelType;
}

// Helper functions for conversion between C++ and Python
[[maybe_unused]] PyObject* ChannelToPyObject(const anim::Channel& channel) {
    // Ensure the type is initialized before creating an instance
    if (PyType_Ready(&PyChannelType) < 0) {
        return NULL;
    }
    PyChannel* py_channel = PyObject_New(PyChannel, &PyChannelType);
    if (py_channel == NULL) {
        return NULL;
    }
    new (&py_channel->channel) anim::Channel(channel); // Use copy constructor
    return (PyObject*)py_channel;
}

[[maybe_unused]] bool PyObjectToChannel(PyObject* obj, anim::Channel& channel) {
    if (!PyObject_TypeCheck(obj, &PyChannelType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a Channel object");
        return false;
    }
    
    PyChannel* py_channel = (PyChannel*)obj;
    channel = py_channel->channel; // Assumes anim::Channel is copy-assignable
    return true;
}
