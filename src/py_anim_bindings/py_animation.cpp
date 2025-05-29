#include "py_animation.h"
#include "py_channel.h" // For PyChannelType, ChannelToPyObject, PyObjectToChannel
#include <vector>
#include <string>
#include <map>
#include <optional> // For std::optional

// Allocation/deallocation functions
static PyObject* PyAnimation_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyAnimation *self = (PyAnimation *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->animation = nullptr;
        self->parent = nullptr;
    }
    return (PyObject *)self;
}

static void PyAnimation_dealloc(PyAnimation *self) {
    Py_XDECREF(self->parent);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Initialize the object - NOTE: This should not be called directly, use AnimationToPyObject instead
static int PyAnimation_init(PyAnimation *self, PyObject *args, PyObject *kwds) {
    PyErr_SetString(PyExc_RuntimeError, "Animation objects cannot be created directly. Use AnimationCHOP methods to access animation.");
    return -1;
}

static PyObject* PyAnimation_create_channel(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    const char* name;
    
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    
    try {
        anim::Channel& channel = self->animation->create_channel(std::string(name));
        return ChannelToPyObject(&channel, (PyObject*)self);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_get_channel_by_index(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    size_t index;
    
    if (!PyArg_ParseTuple(args, "k", &index))
        return NULL;
    
    try {
        anim::Channel& channel = self->animation->channel(index);
        return ChannelToPyObject(&channel, (PyObject*)self);
    } catch (const std::out_of_range&) {
        PyErr_SetString(PyExc_IndexError, "Channel with given index not found");
        return NULL;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_get_channel_by_name(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    const char* name;
    
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    
    try {
        anim::Channel& channel = self->animation->channel(std::string(name));
        return ChannelToPyObject(&channel, (PyObject*)self);
    } catch (const std::out_of_range&) {
        PyErr_Format(PyExc_KeyError, "Channel with name '%s' not found", name);
        return NULL;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_remove_channel_by_index(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    size_t index;
    
    if (!PyArg_ParseTuple(args, "k", &index))
        return NULL;

    try {
        self->animation->remove_channel(index);
        Py_RETURN_NONE;
    } catch (const std::out_of_range&) {
        PyErr_SetString(PyExc_IndexError, "Channel with given index not found");
        return NULL;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_remove_channel_by_name(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    const char* name;
    
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    try {
        self->animation->remove_channel(std::string(name));
        Py_RETURN_NONE;
    } catch (const std::out_of_range&) {
        PyErr_Format(PyExc_KeyError, "Channel with name '%s' not found", name);
        return NULL;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_num_channels(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    size_t count = self->animation->num_channels();
    return PyLong_FromSize_t(count);
}

static PyObject* PyAnimation_get_channel_names(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    std::vector<std::string> names = self->animation->channel_names();
    
    PyObject* names_list = PyList_New(names.size());
    if (!names_list) {
        return NULL;
    }
    
    for (size_t i = 0; i < names.size(); ++i) {
        PyObject* py_name = PyUnicode_FromString(names[i].c_str());
        if (!py_name) {
            Py_DECREF(names_list);
            return NULL;
        }
        PyList_SET_ITEM(names_list, i, py_name);
    }
    
    return names_list;
}

static PyObject* PyAnimation_has_channel(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    const char* name;
    
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    
    bool has_ch = self->animation->has_channel(std::string(name));
    return PyBool_FromLong(has_ch ? 1 : 0);
}

static PyObject* PyAnimation_get_start_time(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    return PyFloat_FromDouble(self->animation->start_time());
}

static PyObject* PyAnimation_set_start_time(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    double start_time;
    
    if (!PyArg_ParseTuple(args, "d", &start_time))
        return NULL;
    
    self->animation->set_start_time(start_time);
    Py_RETURN_NONE;
}

static PyObject* PyAnimation_get_end_time(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    return PyFloat_FromDouble(self->animation->end_time());
}

static PyObject* PyAnimation_set_end_time(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    double end_time;
    
    if (!PyArg_ParseTuple(args, "d", &end_time))
        return NULL;
    
    self->animation->set_end_time(end_time);
    Py_RETURN_NONE;
}

static PyObject* PyAnimation_get_length(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    try {
        return PyFloat_FromDouble(self->animation->length());
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_set_length(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    double length;
    
    if (!PyArg_ParseTuple(args, "d", &length))
        return NULL;
    
    try {
        self->animation->set_length(length);
        Py_RETURN_NONE;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_num_samples(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    double sample_rate;
    
    if (!PyArg_ParseTuple(args, "d", &sample_rate))
        return NULL;

    try {
        int samples = self->animation->num_samples(sample_rate);
        return PyLong_FromLong(samples);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_is_empty(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    bool is_empty = self->animation->empty();
    return PyBool_FromLong(is_empty ? 1 : 0);
}

static PyObject* PyAnimation_clear(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    self->animation->clear();
    Py_RETURN_NONE;
}

static PyObject* PyAnimation_get_name(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    return PyUnicode_FromString(self->animation->name().c_str());
}

static PyObject* PyAnimation_set_name(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    const char* name;
    
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    
    self->animation->set_name(std::string(name));
    Py_RETURN_NONE;
}
        
// Method definitions
static PyMethodDef PyAnimation_methods[] = {
    {"create_channel", (PyCFunction)PyAnimation_create_channel, METH_VARARGS,
     "Create a new channel with the specified name"},
    {"get_channel_by_index", (PyCFunction)PyAnimation_get_channel_by_index, METH_VARARGS,
     "Get a channel by its index"},
    {"get_channel_by_name", (PyCFunction)PyAnimation_get_channel_by_name, METH_VARARGS,
     "Get a channel by its name"},
    {"remove_channel_by_index", (PyCFunction)PyAnimation_remove_channel_by_index, METH_VARARGS,
     "Remove a channel by its index"},
    {"remove_channel_by_name", (PyCFunction)PyAnimation_remove_channel_by_name, METH_VARARGS,
     "Remove a channel by its name"},
    {"num_channels", (PyCFunction)PyAnimation_num_channels, METH_NOARGS,
     "Get the number of channels in the animation"},
    {"get_channel_names", (PyCFunction)PyAnimation_get_channel_names, METH_NOARGS,
     "Get a list of all channel names"},
    {"has_channel", (PyCFunction)PyAnimation_has_channel, METH_VARARGS,
     "Check if a channel with the specified name exists"},
    {"get_start_time", (PyCFunction)PyAnimation_get_start_time, METH_NOARGS,
     "Get the animation start time"},
    {"set_start_time", (PyCFunction)PyAnimation_set_start_time, METH_VARARGS,
     "Set the animation start time"},
    {"get_end_time", (PyCFunction)PyAnimation_get_end_time, METH_NOARGS,
     "Get the animation end time"},
    {"set_end_time", (PyCFunction)PyAnimation_set_end_time, METH_VARARGS,
     "Set the animation end time"},
    {"get_length", (PyCFunction)PyAnimation_get_length, METH_NOARGS,
     "Get the total duration of the animation"},
    {"set_length", (PyCFunction)PyAnimation_set_length, METH_VARARGS,
     "Set the total duration of the animation"},
    {"num_samples", (PyCFunction)PyAnimation_num_samples, METH_VARARGS,
     "Get the number of samples needed for a given sample rate"},
    {"is_empty", (PyCFunction)PyAnimation_is_empty, METH_NOARGS,
     "Check if the animation has no channels"},
    {"clear", (PyCFunction)PyAnimation_clear, METH_NOARGS,
     "Remove all channels from the animation"},
    {"get_name", (PyCFunction)PyAnimation_get_name, METH_NOARGS,
     "Get the animation name"},
    {"set_name", (PyCFunction)PyAnimation_set_name, METH_VARARGS,
     "Set the animation name"},
    {NULL}  // Sentinel
};

// String representation
static PyObject* PyAnimation_str(PyAnimation *self) {
    if (!self->animation) {
        return PyUnicode_FromString("Animation (invalid)");
    }
    
    size_t channel_count = self->animation->num_channels();
    std::vector<std::string> channel_names_vec = self->animation->channel_names();

    std::string names_str;
    for (size_t i = 0; i < channel_names_vec.size(); ++i) {
        if (i > 0) {
            names_str += ", ";
        }
        names_str += channel_names_vec[i];
    }
    return PyUnicode_FromFormat("Animation('%s') with %zu channels: [%s]", 
                                self->animation->name().c_str(), 
                                channel_count, 
                                names_str.c_str());
}

// Type definition
PyTypeObject PyAnimationType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "anim.Animation",        // tp_name
    sizeof(PyAnimation),     // tp_basicsize
    0,                         // tp_itemsize
    (destructor)PyAnimation_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)PyAnimation_str, // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash 
    0,                         // tp_call
    (reprfunc)PyAnimation_str, // tp_str
    PyObject_GenericGetAttr,   // tp_getattro
    PyObject_GenericSetAttr,   // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "Animation object",       // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    PyAnimation_methods,       // tp_methods
    0,                         // tp_members
    0,                         // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)PyAnimation_init, // tp_init
    0,                         // tp_alloc
    PyAnimation_new,           // tp_new
};

// Type getter for external use
PyObject* get_animation_type(PyObject* self, void* closure) {
    if (PyType_Ready(&PyAnimationType) < 0) {
        return NULL;
    }
    Py_INCREF(&PyAnimationType);
    return (PyObject*)&PyAnimationType;
}

// Helper functions for conversion between C++ and Python
PyObject* AnimationToPyObject(anim::Animation* animation, PyObject* parent) {
    // Ensure the type is initialized before creating an instance
    if (PyType_Ready(&PyAnimationType) < 0) {
        return NULL;
    }
    PyAnimation* py_animation = PyObject_New(PyAnimation, &PyAnimationType);
    if (py_animation == NULL) {
        return NULL;
    }
    py_animation->animation = animation;
    py_animation->parent = parent;
    Py_XINCREF(parent); // Keep parent alive
    return (PyObject*)py_animation;
}

bool PyObjectToAnimation(PyObject* obj, anim::Animation*& animation) {
    if (!PyObject_TypeCheck(obj, &PyAnimationType)) {
        PyErr_SetString(PyExc_TypeError, "Expected an Animation object");
        return false;
    }
    
    PyAnimation* py_animation = (PyAnimation*)obj;
    if (!py_animation->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return false;
    }
    
    animation = py_animation->animation;
    return true;
}
