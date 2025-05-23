#include "py_animation.h"
#include "py_channel.h"

// Forward declarations of PyAnimation methods
static PyObject* PyAnimation_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static void PyAnimation_dealloc(PyAnimation *self);
static int PyAnimation_init(PyAnimation *self, PyObject *args, PyObject *kwds);
static PyObject* PyAnimation_create_channel(PyAnimation *self, PyObject *args);
static PyObject* PyAnimation_get_channel(PyAnimation *self, PyObject *args);
static PyObject* PyAnimation_remove_channel(PyAnimation *self, PyObject *args);
static PyObject* PyAnimation_get_channel_names(PyAnimation *self);
static PyObject* PyAnimation_get_channel_count(PyAnimation *self);
static PyObject* PyAnimation_evaluate_channel(PyAnimation *self, PyObject *args);
static PyObject* PyAnimation_evaluate_all_channels(PyAnimation *self, PyObject *args);
static PyObject* PyAnimation_str(PyAnimation *self);

// Define methods for PyAnimation
static PyMethodDef PyAnimation_methods[] = {
    {"create_channel", (PyCFunction)PyAnimation_create_channel, METH_VARARGS, "Create a new channel in the animation"},
    {"get_channel", (PyCFunction)PyAnimation_get_channel, METH_VARARGS, "Get an existing channel by name"},
    {"remove_channel", (PyCFunction)PyAnimation_remove_channel, METH_VARARGS, "Remove a channel by name"},
    {"get_channel_names", (PyCFunction)PyAnimation_get_channel_names, METH_NOARGS, "Get a list of all channel names"},
    {"get_channel_count", (PyCFunction)PyAnimation_get_channel_count, METH_NOARGS, "Get the number of channels"},
    {"evaluate_channel", (PyCFunction)PyAnimation_evaluate_channel, METH_VARARGS, "Evaluate a channel at a specific time"},
    {"evaluate_all_channels", (PyCFunction)PyAnimation_evaluate_all_channels, METH_VARARGS, "Evaluate all channels at a specific time"},
    {NULL}  // Sentinel
};

// Define getset for PyAnimation
static PyGetSetDef PyAnimation_getset[] = {
    {NULL}  // Sentinel
};

// Animation type definition
PyTypeObject PyAnimationType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "anim.Animation",          // tp_name
    sizeof(PyAnimation),       // tp_basicsize
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
    "Animation objects",       // tp_doc 
    0,                         // tp_traverse 
    0,                         // tp_clear 
    0,                         // tp_richcompare 
    0,                         // tp_weaklistoffset 
    0,                         // tp_iter 
    0,                         // tp_iternext 
    PyAnimation_methods,       // tp_methods 
    0,                         // tp_members 
    PyAnimation_getset,        // tp_getset 
    0,                         // tp_base 
    0,                         // tp_dict 
    0,                         // tp_descr_get 
    0,                         // tp_descr_set 
    0,                         // tp_dictoffset 
    (initproc)PyAnimation_init, // tp_init 
    0,                         // tp_alloc 
    PyAnimation_new,           // tp_new 
};

// Implementation of Animation methods

static PyObject* PyAnimation_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyAnimation *self = (PyAnimation *)type->tp_alloc(type, 0);
    if (self != NULL) {
        // Initialize with a new animation object
        self->animation = new anim::Animation();
        self->ownsAnimation = true;
    }
    return (PyObject *)self;
}

static void PyAnimation_dealloc(PyAnimation *self) {
    // If we own the animation, delete it
    if (self->ownsAnimation && self->animation) {
        delete self->animation;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int PyAnimation_init(PyAnimation *self, PyObject *args, PyObject *kwds) {
    // Nothing to do, animation is already created in PyAnimation_new
    return 0;
}

static PyObject* PyAnimation_create_channel(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot create channel in invalid animation");
        return NULL;
    }
    
    const char* name;
    int index = -1;
    
    if (!PyArg_ParseTuple(args, "s|i", &name, &index)) {
        return NULL;
    }
    
    // Create a new channel
    anim::Channel* channel = nullptr;
    
    if (index >= 0) {
        // Create at specific index
        channel = self->animation->create_channel(name, index);
    } else {
        // Create at end
        channel = self->animation->create_channel(name);
    }
    
    if (!channel) {
        PyErr_Format(PyExc_ValueError, "Failed to create channel '%s' (might already exist)", name);
        return NULL;
    }
    
    // Return the channel as a PyChannel object
    return ChannelToPyObject(channel, false);
}

static PyObject* PyAnimation_get_channel(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot get channel from invalid animation");
        return NULL;
    }
    
    // Try to parse a string (channel name)
    const char* name = nullptr;
    if (PyArg_ParseTuple(args, "s", &name)) {
        // Get channel by name
        anim::Channel* channel = self->animation->get_channel(name);
        
        if (!channel) {
            PyErr_Format(PyExc_KeyError, "Channel '%s' not found", name);
            return NULL;
        }
        
        // Return the channel as a PyChannel object
        return ChannelToPyObject(channel, false);
    }
    
    // Clear any error from the previous parse attempt
    PyErr_Clear();
    
    // Try to parse an integer (channel index)
    int index;
    if (PyArg_ParseTuple(args, "i", &index)) {
        // Get channel by index
        anim::Channel* channel = self->animation->get_channel(index);
        
        if (!channel) {
            PyErr_Format(PyExc_IndexError, "Channel index %d out of range", index);
            return NULL;
        }
        
        // Return the channel as a PyChannel object
        return ChannelToPyObject(channel, false);
    }
    
    // If we get here, parsing failed for both methods
    PyErr_SetString(PyExc_TypeError, "Channel name (str) or index (int) expected");
    return NULL;
}

static PyObject* PyAnimation_remove_channel(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot remove channel from invalid animation");
        return NULL;
    }
    
    // Try to parse a string (channel name)
    const char* name = nullptr;
    if (PyArg_ParseTuple(args, "s", &name)) {
        // Remove channel by name
        bool success = self->animation->remove_channel(name);
        
        if (success) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }
    
    // Clear any error from the previous parse attempt
    PyErr_Clear();
    
    // Try to parse an integer (channel index)
    int index;
    if (PyArg_ParseTuple(args, "i", &index)) {
        // Remove channel by index
        bool success = self->animation->remove_channel(index);
        
        if (success) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }
    
    // If we get here, parsing failed for both methods
    PyErr_SetString(PyExc_TypeError, "Channel name (str) or index (int) expected");
    return NULL;
}

static PyObject* PyAnimation_get_channel_names(PyAnimation *self) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot get channel names from invalid animation");
        return NULL;
    }
    
    // Get all channel names
    std::vector<std::string> names = self->animation->get_channel_names();
    
    // Create a Python list to hold the names
    PyObject* name_list = PyList_New(names.size());
    if (!name_list) {
        return NULL;
    }
    
    // Fill the list with channel names
    for (size_t i = 0; i < names.size(); i++) {
        PyObject* name = PyUnicode_FromString(names[i].c_str());
        if (!name) {
            Py_DECREF(name_list);
            return NULL;
        }
        
        // PyList_SetItem steals a reference, no need to Py_DECREF name
        PyList_SetItem(name_list, i, name);
    }
    
    return name_list;
}

static PyObject* PyAnimation_get_channel_count(PyAnimation *self) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot get channel count from invalid animation");
        return NULL;
    }
    
    return PyLong_FromSize_t(self->animation->get_channel_count());
}

static PyObject* PyAnimation_evaluate_channel(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot evaluate channel in invalid animation");
        return NULL;
    }
    
    const char* name;
    double time;
    
    if (!PyArg_ParseTuple(args, "sd", &name, &time)) {
        return NULL;
    }
    
    // Evaluate the channel at the specified time
    double value = self->animation->evaluate_channel(name, time);
    
    return PyFloat_FromDouble(value);
}

static PyObject* PyAnimation_evaluate_all_channels(PyAnimation *self, PyObject *args) {
    if (!self->animation) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot evaluate channels in invalid animation");
        return NULL;
    }
    
    double time;
    
    if (!PyArg_ParseTuple(args, "d", &time)) {
        return NULL;
    }
    
    // Evaluate all channels at the specified time
    std::map<std::string, double> values = self->animation->evaluate_all_channels(time);
    
    // Create a Python dictionary to hold the results
    PyObject* result_dict = PyDict_New();
    if (!result_dict) {
        return NULL;
    }
    
    // Fill the dictionary with channel names and values
    for (const auto& pair : values) {
        PyObject* name = PyUnicode_FromString(pair.first.c_str());
        if (!name) {
            Py_DECREF(result_dict);
            return NULL;
        }
        
        PyObject* value = PyFloat_FromDouble(pair.second);
        if (!value) {
            Py_DECREF(name);
            Py_DECREF(result_dict);
            return NULL;
        }
        
        // PyDict_SetItem does not steal references
        int result = PyDict_SetItem(result_dict, name, value);
        Py_DECREF(name);
        Py_DECREF(value);
        
        if (result < 0) {
            Py_DECREF(result_dict);
            return NULL;
        }
    }
    
    return result_dict;
}

static PyObject* PyAnimation_str(PyAnimation *self) {
    if (!self->animation) {
        return PyUnicode_FromString("Animation(invalid)");
    }
    
    std::string str = "Animation(channels=" + 
                      std::to_string(self->animation->get_channel_count()) + ")";
    return PyUnicode_FromString(str.c_str());
}

// External utility functions

PyAnimation* AnimationToPyAnimation(anim::Animation* animation, bool ownsAnimation) {
    PyAnimation* pyAnimation = PyObject_New(PyAnimation, &PyAnimationType);
    if (pyAnimation == NULL) {
        return NULL;
    }
    pyAnimation->animation = animation;
    pyAnimation->ownsAnimation = ownsAnimation;
    return pyAnimation;
}

PyObject* AnimationToPyObject(anim::Animation* animation, bool ownsAnimation) {
    PyAnimation* pyAnimation = AnimationToPyAnimation(animation, ownsAnimation);
    return (PyObject*)pyAnimation;
}

bool PyObjectToAnimation(PyObject* obj, anim::Animation*& animation) {
    if (!PyObject_TypeCheck(obj, &PyAnimationType)) {
        PyErr_SetString(PyExc_TypeError, "Expected an Animation object");
        return false;
    }
    
    PyAnimation* pyAnimation = (PyAnimation*)obj;
    animation = pyAnimation->animation;
    return true;
}

PyObject* get_animation_type(PyObject* self, void* closure) {
    Py_INCREF(&PyAnimationType);
    return (PyObject*)&PyAnimationType;
}