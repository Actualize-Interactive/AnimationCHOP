#include "py_animation.h"
#include "py_channel.h" // For PyChannelType, ChannelToPyObject, PyObjectToChannel
#include <vector>
#include <string>
#include <map>
#include <optional> // For std::optional

// Allocation/deallocation functions
static PyObject* PyAnimation_new(PyTypeObject *type, [[maybe_unused]] PyObject *args, [[maybe_unused]] PyObject *kwds) {
    PyAnimation *self = (PyAnimation *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->animation_ptr = new anim::Animation();
        if (!self->animation_ptr) {
            Py_DECREF(self);
            return PyErr_NoMemory();
        }
        self->is_owner = true; 
    }
    return (PyObject *)self;
}

static void PyAnimation_dealloc(PyAnimation *self) {
    if (self->is_owner && self->animation_ptr) {
        delete self->animation_ptr;
    }
    self->animation_ptr = nullptr;
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Initialize the object
static int PyAnimation_init([[maybe_unused]] PyAnimation *self, [[maybe_unused]] PyObject *args, [[maybe_unused]] PyObject *kwds) {
    return 0;
}

static PyObject* PyAnimation_append_channel(PyAnimation *self, PyObject *args) {
    PyObject* py_channel_arg; 
    
    if (!PyArg_ParseTuple(args, "O", &py_channel_arg))
        return NULL;
    
    anim::Channel channel_temp;
    if (!PyObjectToChannel(py_channel_arg, channel_temp)) {
         // PyObjectToChannel sets the error
        return NULL;
    }
        
    try {
        self->animation_ptr->append_channel(channel_temp);
        Py_RETURN_NONE;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// Methods
static PyObject* PyAnimation_insert_channel(PyAnimation *self, PyObject *args) {
    PyObject* py_channel_arg; 
    size_t index;

    if (!PyArg_ParseTuple(args, "nO", &index, &py_channel_arg)) // Changed "kO" to "nO" for size_t
        return NULL;

    anim::Channel channel_temp;
    if (!PyObjectToChannel(py_channel_arg, channel_temp)) {
        // PyObjectToChannel sets the error
        return NULL;
    }

    // Insert the channel into the animation
    try {
        self->animation_ptr->insert_channel(index, channel_temp);
        Py_RETURN_NONE;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}


static PyObject* PyAnimation_get_channel_by_index(PyAnimation *self, PyObject *args) {
    size_t index;
    
    if (!PyArg_ParseTuple(args, "n", &index)) // Changed "k" to "n" for size_t
        return NULL;
    
    try {
        // Assuming get_channel returns a pointer or optional<reference_wrapper<Channel>>
        // If it returns a copy or reference, adjust accordingly.
        // For this example, let's assume it returns a pointer to a channel managed by Animation.
        anim::Channel* channel_ptr = self->animation_ptr->get_channel(index);
        if (!channel_ptr) {
            Py_RETURN_NONE; // Or raise an IndexError
        }

        // Convert C++ Channel to PyObject. This typically involves creating a new PyChannel.
        // This means the Python side gets a *copy* or a new wrapper around the existing C++ object.
        // If the C++ object's lifetime is tied to PyAnimation, care must be taken.
        // ChannelToPyObject should handle creating a new PyChannel that wraps a *copy*
        // or a reference if the lifetime management is clear.
        // For simplicity, assume ChannelToPyObject creates a new PyChannel wrapping a copy.
        return ChannelToPyObject(*channel_ptr);
    } catch (const std::out_of_range& e) {
        PyErr_SetString(PyExc_IndexError, e.what());
        return NULL;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_get_channel_by_name(PyAnimation *self, PyObject *args) {
    const char* name;
    
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    
    try {
        anim::Channel* channel_ptr = self->animation_ptr->get_channel(std::string(name));
        if (!channel_ptr) {
            Py_RETURN_NONE; // Or raise KeyError
        }
        return ChannelToPyObject(*channel_ptr);
    } catch (const std::out_of_range& e) { // Or other specific exception for not found
        PyErr_Format(PyExc_KeyError, "Channel with name '%s' not found", name);
        return NULL;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_remove_channel_by_index(PyAnimation *self, PyObject *args) {
    size_t index;
    
    if (!PyArg_ParseTuple(args, "n", &index)) // Changed "k" to "n"
        return NULL;

    bool result = self->animation_ptr->remove_channel(index);
    return PyBool_FromLong(result ? 1 : 0);
}

static PyObject* PyAnimation_remove_channel_by_name(PyAnimation *self, PyObject *args) {
    const char* name;
    
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    bool result = self->animation_ptr->remove_channel(std::string(name));
    return PyBool_FromLong(result ? 1 : 0);
}

static PyObject* PyAnimation_get_channel_count(PyAnimation *self, [[maybe_unused]] PyObject *args) {
    size_t count = self->animation_ptr->get_channel_count();
    return PyLong_FromSize_t(count);
}

static PyObject* PyAnimation_get_channel_names(PyAnimation *self, [[maybe_unused]] PyObject *args) {
    std::vector<std::string> names = self->animation_ptr->get_channel_names();
    
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

static PyObject* PyAnimation_find_channel_index(PyAnimation *self, PyObject *args) {
    const char* name;
    
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    
    try {
        size_t index = self->animation_ptr->find_channel_index(std::string(name));
        return PyLong_FromSize_t(index);
    } catch (const std::out_of_range& e) { // Assuming find_channel_index throws if not found
        PyErr_SetString(PyExc_ValueError, e.what()); // Or KeyError
        return NULL;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_evaluate_channels(PyAnimation *self, PyObject *args) {
    double time;
    
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;
    
    try {
        std::map<std::string, double> values = self->animation_ptr->evaluate_channels(time);

        PyObject* values_dict = PyDict_New();
        if (!values_dict) {
            return NULL;
        }
        
        for (const auto& pair : values) {
            PyObject* py_name = PyUnicode_FromString(pair.first.c_str());
            PyObject* py_value = PyFloat_FromDouble(pair.second);
            
            if (!py_name || !py_value) {
                Py_XDECREF(py_name);
                Py_XDECREF(py_value);
                Py_DECREF(values_dict);
                return NULL;
            }
            
            if (PyDict_SetItem(values_dict, py_name, py_value) < 0) {
                Py_DECREF(py_name);
                Py_DECREF(py_value);
                Py_DECREF(values_dict);
                return NULL;
            }
            
            Py_DECREF(py_name);
            Py_DECREF(py_value);
        }
        
        return values_dict;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_evaluate_channels_range(PyAnimation *self, PyObject *args) {
    double start_time, end_time;
    int num_samples;
    
    if (!PyArg_ParseTuple(args, "ddi", &start_time, &end_time, &num_samples))
        return NULL;
    
    try {
        std::map<std::string, std::vector<double>> values = 
            self->animation_ptr->evaluate_channels_range(start_time, end_time, num_samples);

        PyObject* values_dict = PyDict_New();
        if (!values_dict) {
            return NULL;
        }
        
        for (const auto& pair : values) {
            PyObject* py_name = PyUnicode_FromString(pair.first.c_str());
            if (!py_name) {
                Py_DECREF(values_dict);
                return NULL;
            }

            PyObject* py_values_list = PyList_New(pair.second.size());
            if (!py_values_list) {
                Py_DECREF(py_name);
                Py_DECREF(values_dict);
                return NULL;
            }
            
            for (size_t i = 0; i < pair.second.size(); ++i) {
                PyObject* py_value = PyFloat_FromDouble(pair.second[i]);
                if (!py_value) {
                    Py_DECREF(py_name);
                    Py_DECREF(py_values_list);
                    Py_DECREF(values_dict);
                    return NULL;
                }
                PyList_SET_ITEM(py_values_list, i, py_value);
            }
            
            if (PyDict_SetItem(values_dict, py_name, py_values_list) < 0) {
                Py_DECREF(py_name);
                Py_DECREF(py_values_list);
                Py_DECREF(values_dict);
                return NULL;
            }
            
            Py_DECREF(py_name);
            Py_DECREF(py_values_list);
        }
        
        return values_dict;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_evaluate_channels_range_by_rate(PyAnimation *self, PyObject *args) {
    double start_time, end_time, sample_rate;
    
    if (!PyArg_ParseTuple(args, "ddd", &start_time, &end_time, &sample_rate))
        return NULL;
    
    try {
        std::map<std::string, std::vector<double>> values = 
            self->animation_ptr->evaluate_channels_range_by_rate(start_time, end_time, sample_rate);
        
        PyObject* values_dict = PyDict_New();
        if (!values_dict) {
            return NULL;
        }
        
        for (const auto& pair : values) {
            PyObject* py_name = PyUnicode_FromString(pair.first.c_str());
            if (!py_name) {
                Py_DECREF(values_dict);
                return NULL;
            }
            
            PyObject* py_values_list = PyList_New(pair.second.size());
            if (!py_values_list) {
                Py_DECREF(py_name);
                Py_DECREF(values_dict);
                return NULL;
            }
            
            for (size_t i = 0; i < pair.second.size(); ++i) {
                PyObject* py_value = PyFloat_FromDouble(pair.second[i]);
                if (!py_value) {
                    Py_DECREF(py_name);
                    Py_DECREF(py_values_list);
                    Py_DECREF(values_dict);
                    return NULL;
                }
                PyList_SET_ITEM(py_values_list, i, py_value);
            }
            
            if (PyDict_SetItem(values_dict, py_name, py_values_list) < 0) {
                Py_DECREF(py_name);
                Py_DECREF(py_values_list);
                Py_DECREF(values_dict);
                return NULL;
            }
            
            Py_DECREF(py_name);
            Py_DECREF(py_values_list);
        }
        
        return values_dict;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyAnimation_get_start_time(PyAnimation *self, [[maybe_unused]] PyObject *args) {
    std::optional<double> start_time = self->animation_ptr->get_start_time();
    if (start_time) {
        return PyFloat_FromDouble(*start_time);
    } else {
        Py_RETURN_NONE;
    }
}

static PyObject* PyAnimation_get_end_time(PyAnimation *self, [[maybe_unused]] PyObject *args) {
    std::optional<double> end_time = self->animation_ptr->get_end_time();
    if (end_time) {
        return PyFloat_FromDouble(*end_time);
    } else {
        Py_RETURN_NONE;
    }
}

static PyObject* PyAnimation_length(PyAnimation *self, [[maybe_unused]] PyObject *args) {
    double length = self->animation_ptr->length();
    return PyFloat_FromDouble(length);
}

static PyObject* PyAnimation_num_samples(PyAnimation *self, PyObject *args) {
    double sample_rate;
    
    if (!PyArg_ParseTuple(args, "d", &sample_rate))
        return NULL;

    int num_samples_val = self->animation_ptr->num_samples(sample_rate); // Renamed to avoid conflict
    return PyLong_FromLong(num_samples_val);
}

static PyObject* PyAnimation_is_empty(PyAnimation *self, [[maybe_unused]] PyObject *args) {
    bool is_empty = self->animation_ptr->is_empty();
    return PyBool_FromLong(is_empty ? 1 : 0);
}

static PyObject* PyAnimation_has_no_keyframes(PyAnimation *self, [[maybe_unused]] PyObject *args) {
    bool has_no_keyframes = self->animation_ptr->has_no_keyframes();
    return PyBool_FromLong(has_no_keyframes ? 1 : 0);
}

// Method definitions
static PyMethodDef PyAnimation_methods[] = {
    {"insert_channel", (PyCFunction)PyAnimation_insert_channel, METH_VARARGS,
     "Insert a channel at a specific position"},
    {"append_channel", (PyCFunction)PyAnimation_append_channel, METH_VARARGS,
     "Append a channel to the end of the animation"},
    {"get_channel_by_index", (PyCFunction)PyAnimation_get_channel_by_index, METH_VARARGS,
     "Get a channel by its index"},
    {"get_channel_by_name", (PyCFunction)PyAnimation_get_channel_by_name, METH_VARARGS,
     "Get a channel by its name"},
    {"remove_channel_by_index", (PyCFunction)PyAnimation_remove_channel_by_index, METH_VARARGS,
     "Remove a channel by its index"},
    {"remove_channel_by_name", (PyCFunction)PyAnimation_remove_channel_by_name, METH_VARARGS,
     "Remove a channel by its name"},
    {"get_channel_count", (PyCFunction)PyAnimation_get_channel_count, METH_NOARGS,
     "Get the number of channels in the animation"},
    {"get_channel_names", (PyCFunction)PyAnimation_get_channel_names, METH_NOARGS,
     "Get a list of all channel names"},
    {"find_channel_index", (PyCFunction)PyAnimation_find_channel_index, METH_VARARGS,
     "Find the index of a channel by its name"},
    {"evaluate_channels", (PyCFunction)PyAnimation_evaluate_channels, METH_VARARGS,
     "Evaluate all channels at a specific time"},
    {"evaluate_channels_range", (PyCFunction)PyAnimation_evaluate_channels_range, METH_VARARGS,
     "Evaluate all channels over a time range with fixed sample count"},
    {"evaluate_channels_range_by_rate", (PyCFunction)PyAnimation_evaluate_channels_range_by_rate, METH_VARARGS,
     "Evaluate all channels over a time range with a specific sample rate"},
    {"get_start_time", (PyCFunction)PyAnimation_get_start_time, METH_NOARGS,
     "Get the earliest keyframe time across all channels"},
    {"get_end_time", (PyCFunction)PyAnimation_get_end_time, METH_NOARGS,
     "Get the latest keyframe time across all channels"},
    {"length", (PyCFunction)PyAnimation_length, METH_NOARGS,
     "Get the total duration of the animation"},
    {"num_samples", (PyCFunction)PyAnimation_num_samples, METH_VARARGS,
     "Get the number of samples needed for a given sample rate"},
    {"is_empty", (PyCFunction)PyAnimation_is_empty, METH_NOARGS,
     "Check if the animation has no channels"},
    {"has_no_keyframes", (PyCFunction)PyAnimation_has_no_keyframes, METH_NOARGS,
     "Check if the animation has no keyframes"},
    {NULL}  // Sentinel
};

// String representation
static PyObject* PyAnimation_str(PyAnimation *self) {
    size_t channel_count = self->animation_ptr->get_channel_count();
    std::vector<std::string> channel_names_vec = self->animation_ptr->get_channel_names(); // Renamed

    std::string names_str;
    for (size_t i = 0; i < channel_names_vec.size(); ++i) {
        if (i > 0) {
            names_str += ", ";
        }
        names_str += channel_names_vec[i];
    }
    return PyUnicode_FromFormat("Animation with %zu channels: %s", channel_count, names_str.c_str());
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

// Factory function to wrap an existing C++ anim::Animation instance
// If owned_by_python_wrapper is true, it means existing_animation was 'new'ed elsewhere
// and this PyAnimation instance is now taking ownership.
[[maybe_unused]] PyObject* PyAnimation_WrapExisting(anim::Animation* existing_animation_ptr, bool owned_by_python_wrapper) {
    if (!existing_animation_ptr) {
        PyErr_SetString(PyExc_ValueError, "Cannot wrap a NULL anim::Animation pointer.");
        return NULL;
    }
    if (PyType_Ready(&PyAnimationType) < 0) return NULL;

    PyAnimation *self = (PyAnimation *)PyAnimationType.tp_alloc(&PyAnimationType, 0);
    if (self != NULL) {
        self->animation_ptr = existing_animation_ptr;
        self->is_owner = owned_by_python_wrapper;
    } else {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate PyAnimation object.");
        return NULL;
    }
    return (PyObject *)self;
}



// Type getter for external use
[[maybe_unused]] PyObject* get_animation_type([[maybe_unused]] PyObject* self, [[maybe_unused]] void* closure) {
    if (PyType_Ready(&PyAnimationType) < 0) {
        return NULL;
    }
    Py_INCREF(&PyAnimationType);
    return (PyObject*)&PyAnimationType;
}

[[maybe_unused]] PyObject* AnimationToPyObject(const anim::Animation& animation_to_copy) {
    anim::Animation* new_cpp_anim_instance = new anim::Animation(animation_to_copy);
    if (!new_cpp_anim_instance) {
        return PyErr_NoMemory();
    }
    PyObject* py_obj = PyAnimation_WrapExisting(new_cpp_anim_instance, true);
    if (!py_obj) {
        delete new_cpp_anim_instance;
        return NULL; // PyAnimation_WrapExisting should have set an error
    }
    return py_obj;
}

[[maybe_unused]] bool PyObjectToAnimation(PyObject* obj, anim::Animation& animation) {
    if (!PyObject_TypeCheck(obj, &PyAnimationType)) {
        PyErr_SetString(PyExc_TypeError, "Expected an Animation object");
        return false;
    }
    
    PyAnimation* py_animation = (PyAnimation*)obj;
    animation = *py_animation->animation_ptr; // Assumes anim::Animation is copy-assignable
    return true;
}
