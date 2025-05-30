#include "py_channel.h"
#include "py_keyframe.h" // For KeyframeToPyObject, PyKeyframeType
#include "utils.h" // For AnimationCHOP, AnimationToPyObject
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

// --- Keyframe creation ---
static PyObject* create_keyframe_time_value(PyChannel* self, PyObject* args) {
    double time, value;
    int function = (int)anim::Function::bezier;
    int handle_mode = (int)anim::HandleMode::smooth;
    
    auto argc = PyTuple_Size(args);
    if (argc == 2) {
        if (!PyArg_ParseTuple(args, "dd", &time, &value)) return NULL;
    } else if (argc == 4) {
        if (!PyArg_ParseTuple(args, "ddii", &time, &value, &function, &handle_mode)) return NULL;
    } else {
        return NULL; // Wrong arg count
    }
    
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    
    try {
        const anim::Keyframe& kf = self->channel->create_keyframe(time, value, (anim::Function)function, (anim::HandleMode)handle_mode);
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        return KeyframeToPyObject(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* create_keyframe_point(PyChannel* self, PyObject* args) {
    PyObject *position_obj;
    int function = (int)anim::Function::bezier;
    int handle_mode = (int)anim::HandleMode::smooth;
    
    auto argc = PyTuple_Size(args);
    if (argc == 1) {
        if (!PyArg_ParseTuple(args, "O", &position_obj)) return NULL;
    } else if (argc == 3) {
        if (!PyArg_ParseTuple(args, "Oii", &position_obj, &function, &handle_mode)) return NULL;
    } else {
        return NULL; // Wrong arg count
    }
    
    anim::Point pos;
    if (!PyObjectToPoint(position_obj, pos)) return NULL;
    
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    
    try {
        const anim::Keyframe& kf = self->channel->create_keyframe(pos, (anim::Function)function, (anim::HandleMode)handle_mode);
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        return KeyframeToPyObject(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* create_keyframe_with_handles(PyChannel* self, PyObject* args) {
    double time, value;
    PyObject *in_handle_obj, *out_handle_obj;
    int function = (int)anim::Function::bezier;
    int handle_mode = (int)anim::HandleMode::smooth;
    
    auto argc = PyTuple_Size(args);
    if (argc == 4) {
        if (!PyArg_ParseTuple(args, "ddOO", &time, &value, &in_handle_obj, &out_handle_obj)) return NULL;
    } else if (argc == 6) {
        if (!PyArg_ParseTuple(args, "ddOOii", &time, &value, &in_handle_obj, &out_handle_obj, &function, &handle_mode)) return NULL;
    } else {
        return NULL; // Wrong arg count
    }
    
    anim::Point in_handle, out_handle;
    if (!PyObjectToPoint(in_handle_obj, in_handle) || !PyObjectToPoint(out_handle_obj, out_handle)) return NULL;
    
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    
    try {
        const anim::Keyframe& kf = self->channel->create_keyframe(time, value, in_handle, out_handle, (anim::Function)function, (anim::HandleMode)handle_mode);
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        return KeyframeToPyObject(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PyChannel_create_keyframe(PyChannel *self, PyObject *args, PyObject *kwds) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }

    auto argc = PyTuple_Size(args);
    if (argc == 0) {
        PyErr_SetString(PyExc_TypeError, "create_keyframe requires at least 1 argument");
        return NULL;
    }
    
    PyObject* first_arg = PyTuple_GetItem(args, 0);
    
    // Check if first arg is a Point object
    if (PyObject_TypeCheck(first_arg, &PyPointType)) {
        // Overload 2: create_keyframe(Point [, function, handle_mode])
        PyObject* result = create_keyframe_point(self, args);
        if (result) return result;
        PyErr_Clear(); // Clear any errors from failed attempt
    }
    
    // Check if first arg is numeric (time/value overloads)
    if (PyFloat_Check(first_arg) || PyLong_Check(first_arg)) {
        if (argc >= 4) {
            // Check if args 2 and 3 are Points (handle overload)
            PyObject* third_arg = PyTuple_GetItem(args, 2);
            PyObject* fourth_arg = PyTuple_GetItem(args, 3);
            if (PyObject_TypeCheck(third_arg, &PyPointType) && PyObject_TypeCheck(fourth_arg, &PyPointType)) {
                // Overload 3: create_keyframe(time, value, in_handle, out_handle [, function, handle_mode])
                PyObject* result = create_keyframe_with_handles(self, args);
                if (result) return result;
                PyErr_Clear();
            }
        }
        
        // Overload 1: create_keyframe(time, value [, function, handle_mode])
        PyObject* result = create_keyframe_time_value(self, args);
        if (result) return result;
        PyErr_Clear();
    }
    
    PyErr_SetString(PyExc_TypeError, 
        "Invalid arguments for create_keyframe. Expected:\n"
        "  (time, value [, function, handle_mode])\n"
        "  (Point [, function, handle_mode])\n"
        "  (time, value, in_handle, out_handle [, function, handle_mode])");
    return NULL;
}

static PyObject* PyChannel_emplace_keyframe(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }

    PyObject* py_keyframe;
    if (!PyArg_ParseTuple(args, "O", &py_keyframe))
        return NULL;
    anim::Keyframe kf;
    if (!PyObjectToKeyframe(py_keyframe, kf))
        return NULL;
    try {
        const anim::Keyframe& result = self->channel->emplace_keyframe(std::move(kf));
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        return KeyframeToPyObject(result);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// --- Keyframe access ---
static PyObject* PyChannel_getitem(PyChannel *self, Py_ssize_t index) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    try {
        const anim::Keyframe& kf = self->channel->keyframe(static_cast<size_t>(index));
        return KeyframeToPyObject(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_IndexError, e.what());
        return NULL;
    }
}

static Py_ssize_t PyChannel_len(PyChannel *self) {
    if (!self->channel) return 0;
    return (Py_ssize_t)self->channel->num_keyframes();
}

static int PyChannel_contains(PyChannel *self, PyObject *key) {
    if (!self->channel) return 0;
    double time = 0.0;
    if (PyFloat_Check(key)) {
        time = PyFloat_AsDouble(key);
    } else if (PyLong_Check(key)) {
        time = (double)PyLong_AsDouble(key);
    } else {
        return 0;
    }
    return self->channel->has_keyframe(time) ? 1 : 0;
}

// --- Keyframe queries ---
static PyObject* PyChannel_prev_keyframe(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double time;
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;
    try {
        const anim::Keyframe& kf = self->channel->prev_keyframe(time);
        return KeyframeToPyObject(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}
static PyObject* PyChannel_next_keyframe(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double time;
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;
    try {
        const anim::Keyframe& kf = self->channel->next_keyframe(time);
        return KeyframeToPyObject(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}
static PyObject* PyChannel_closest_keyframe(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double time;
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;
    try {
        const anim::Keyframe& kf = self->channel->closest_keyframe(time);
        return KeyframeToPyObject(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// --- Keyframe update/setters ---
static PyObject* PyChannel_update_keyframe(PyChannel *self, PyObject *args) {
    if (!self->channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    Py_ssize_t index; PyObject* value;
    if (!PyArg_ParseTuple(args, "nO", &index, &value)) return NULL;
    anim::Keyframe kf;
    if (!PyObjectToKeyframe(value, kf)) return NULL;
    try {
        self->channel->update_keyframe(static_cast<size_t>(index), kf);
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

static PyObject* PyChannel_set_keyframe_time(PyChannel *self, PyObject *args) {
    if (!self->channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    Py_ssize_t index; double value;
    if (!PyArg_ParseTuple(args, "nd", &index, &value)) return NULL;
    try { 
        self->channel->set_keyframe_time(static_cast<size_t>(index), value); 
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

static PyObject* PyChannel_set_keyframe_value(PyChannel *self, PyObject *args) {
    if (!self->channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    Py_ssize_t index; double value;
    if (!PyArg_ParseTuple(args, "nd", &index, &value)) return NULL;
    try { 
        self->channel->set_keyframe_value(static_cast<size_t>(index), value); 
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

static PyObject* PyChannel_set_keyframe_position(PyChannel *self, PyObject *args) {
    if (!self->channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    Py_ssize_t index;
    PyObject* value;
    if (PyArg_ParseTuple(args, "nO", &index, &value)) {
        // Try Point object
        anim::Point pt;
        if (!PyObjectToPoint(value, pt)) return NULL;
        try { 
            self->channel->set_keyframe_position(static_cast<size_t>(index), pt); 
            if (node_struct) {
                node_struct->context->makeNodeDirty();
            }
            Py_RETURN_NONE; 
        }
        catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
    } else {
        PyErr_Clear();
        double t, v;
        if (PyArg_ParseTuple(args, "ndd", &index, &t, &v)) {
            try { 
                self->channel->set_keyframe_position(static_cast<size_t>(index), t, v); 
                if (node_struct) {
                    node_struct->context->makeNodeDirty();
                }
                Py_RETURN_NONE; 
            }
            catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
        } else {
            PyErr_SetString(PyExc_TypeError, "set_keyframe_position expects (index, Point) or (index, time, value)");
            return NULL;
        }
    }
}

static PyObject* PyChannel_set_keyframe_in_handle(PyChannel *self, PyObject *args) {
    if (!self->channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    Py_ssize_t index; PyObject* value;
    if (!PyArg_ParseTuple(args, "nO", &index, &value)) return NULL;
    anim::Point pt;
    if (!PyObjectToPoint(value, pt)) return NULL;
    try { 
        self->channel->set_keyframe_in_handle(static_cast<size_t>(index), pt); 
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

static PyObject* PyChannel_set_keyframe_out_handle(PyChannel *self, PyObject *args) {
    if (!self->channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    Py_ssize_t index; PyObject* value;
    if (!PyArg_ParseTuple(args, "nO", &index, &value)) return NULL;
    anim::Point pt;
    if (!PyObjectToPoint(value, pt)) return NULL;
    try { 
        self->channel->set_keyframe_out_handle(static_cast<size_t>(index), pt); 
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

static PyObject* PyChannel_set_keyframe_function(PyChannel *self, PyObject *args) {
    if (!self->channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    Py_ssize_t index; int value;
    if (!PyArg_ParseTuple(args, "ni", &index, &value)) return NULL;
    try { 
        self->channel->set_keyframe_function(static_cast<size_t>(index), (anim::Function)value); 
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

static PyObject* PyChannel_set_keyframe_handle_mode(PyChannel *self, PyObject *args) {
    if (!self->channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    Py_ssize_t index; int value;
    if (!PyArg_ParseTuple(args, "ni", &index, &value)) return NULL;
    try { 
        self->channel->set_keyframe_handle_mode(static_cast<size_t>(index), (anim::HandleMode)value); 
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

// --- Keyframe removal ---
static PyObject* PyChannel_remove_keyframe(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    Py_ssize_t index;
    if (!PyArg_ParseTuple(args, "n", &index))
        return NULL;
    try {
        self->channel->delete_keyframe(static_cast<size_t>(index));
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// --- Keyframe get by index ---
static PyObject* PyChannel_get_keyframe(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    Py_ssize_t index;
    if (!PyArg_ParseTuple(args, "n", &index))
        return NULL;
    try {
        const anim::Keyframe& kf = self->channel->keyframe(static_cast<size_t>(index));
        return KeyframeToPyObject(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_IndexError, e.what());
        return NULL;
    }
}

// --- String representation ---
static PyObject* PyChannel_str(PyChannel *self) {
    if (!self->channel) {
        return PyUnicode_FromString("<Channel (invalid)>");
    }
    std::string s = std::string("<Channel name='") + self->channel->name() + "' keyframes=" + std::to_string(self->channel->num_keyframes()) + ">";
    return PyUnicode_FromString(s.c_str());
}

// --- Evaluation ---
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
static PyObject* PyChannel_evaluate_range(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double start_time, end_time;
    int num_samples;
    if (!PyArg_ParseTuple(args, "ddi", &start_time, &end_time, &num_samples))
        return NULL;
    try {
        std::vector<double> values = self->channel->evaluate_range(start_time, end_time, num_samples);
        PyObject* list = PyList_New(values.size());
        for (size_t i = 0; i < values.size(); ++i)
            PyList_SET_ITEM(list, i, PyFloat_FromDouble(values[i]));
        return list;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}
static PyObject* PyChannel_evaluate_range_by_rate(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double start_time, end_time, sample_rate;
    if (!PyArg_ParseTuple(args, "ddd", &start_time, &end_time, &sample_rate))
        return NULL;
    try {
        std::vector<double> values = self->channel->evaluate_range_by_rate(start_time, end_time, sample_rate);
        PyObject* list = PyList_New(values.size());
        for (size_t i = 0; i < values.size(); ++i)
            PyList_SET_ITEM(list, i, PyFloat_FromDouble(values[i]));
        return list;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// --- Properties ---
static PyObject* PyChannel_get_name(PyChannel *self, void*) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyUnicode_FromString(self->channel->name().c_str());
}
static int PyChannel_set_name(PyChannel *self, PyObject* value, void*) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return -1;
    }
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    if (!PyUnicode_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "Name must be a string");
        return -1;
    }
    self->channel->set_name(PyUnicode_AsUTF8(value));
    if (node_struct) {
        node_struct->context->makeNodeDirty();
    }
    return 0;
}
static PyObject* PyChannel_size(PyChannel *self, void*) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyLong_FromSize_t(self->channel->size());
}
static PyObject* PyChannel_num_keyframes(PyChannel *self, void*) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyLong_FromSize_t(self->channel->num_keyframes());
}
static PyObject* PyChannel_empty(PyChannel *self, void*) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyBool_FromLong(self->channel->empty() ? 1 : 0);
}
static PyObject* PyChannel_start_time(PyChannel *self, void*) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyFloat_FromDouble(self->channel->start_time());
}
static PyObject* PyChannel_end_time(PyChannel *self, void*) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyFloat_FromDouble(self->channel->end_time());
}
static PyObject* PyChannel_length(PyChannel *self, void*) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyFloat_FromDouble(self->channel->length());
}
static PyObject* PyChannel_num_samples(PyChannel *self, PyObject* args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double sample_rate;
    if (!PyArg_ParseTuple(args, "d", &sample_rate))
        return NULL;
    try {
        size_t n = self->channel->num_samples(sample_rate);
        return PyLong_FromSize_t(n);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// --- State methods ---
PyObject* PyChannel_get_state(PyChannel *self, void *closure) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    PyObject* state_dict = PyDict_New();
    if (!state_dict) return NULL;
    
    try {
        // Add channel properties
        PyDict_SetItemString(state_dict, "name", PyUnicode_FromString(self->channel->name().c_str()));
        PyDict_SetItemString(state_dict, "start_time", PyFloat_FromDouble(self->channel->start_time()));
        PyDict_SetItemString(state_dict, "end_time", PyFloat_FromDouble(self->channel->end_time()));
        PyDict_SetItemString(state_dict, "length", PyFloat_FromDouble(self->channel->length()));
        PyDict_SetItemString(state_dict, "num_keyframes", PyLong_FromSize_t(self->channel->num_keyframes()));
        PyDict_SetItemString(state_dict, "empty", PyBool_FromLong(self->channel->empty() ? 1 : 0));
        
        // Create keyframes list using keyframe state
        PyObject* keyframes_list = PyList_New(self->channel->num_keyframes());
        if (!keyframes_list) {
            Py_DECREF(state_dict);
            return NULL;
        }
        
        for (size_t i = 0; i < self->channel->num_keyframes(); ++i) {
            try {
                const auto& keyframe = self->channel->keyframe(i);
                PyKeyframe* py_keyframe = KeyframeToPyKeyframe(keyframe);
                if (!py_keyframe) {
                    Py_DECREF(keyframes_list);
                    Py_DECREF(state_dict);
                    return NULL;
                }
                
                PyObject* kf_state = PyKeyframe_get_state(py_keyframe, NULL);
                Py_DECREF(py_keyframe); // We only needed it for the state
                
                if (!kf_state) {
                    Py_DECREF(keyframes_list);
                    Py_DECREF(state_dict);
                    return NULL;
                }
                
                PyList_SET_ITEM(keyframes_list, i, kf_state);
                
            } catch (const std::exception& e) {
                Py_DECREF(keyframes_list);
                Py_DECREF(state_dict);
                PyErr_Format(PyExc_RuntimeError, "Error processing keyframe %zu: %s", i, e.what());
                return NULL;
            }
        }
        
        PyDict_SetItemString(state_dict, "keyframes", keyframes_list);
        return state_dict;
        
    } catch (const std::exception& e) {
        Py_DECREF(state_dict);
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

int PyChannel_set_state(PyChannel *self, PyObject *value, void *closure) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return -1;
    }
    
    if (!PyDict_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "Channel state must be a dictionary");
        return -1;
    }
    
    TD::PY_Struct* node_struct = nullptr;
    if (self->parent) {
        node_struct = get_td_node_struct((PyObject*)self->parent, nullptr);
    }
    
    try {
        // Clear existing keyframes
        while (self->channel->num_keyframes() > 0) {
            self->channel->delete_keyframe(0);
        }
        
        // Set channel name if provided
        PyObject* name_obj = PyDict_GetItemString(value, "name");
        if (name_obj && PyUnicode_Check(name_obj)) {
            self->channel->set_name(PyUnicode_AsUTF8(name_obj));
        }
        
        // Process keyframes
        PyObject* keyframes_obj = PyDict_GetItemString(value, "keyframes");
        if (keyframes_obj) {
            if (!PyList_Check(keyframes_obj)) {
                PyErr_SetString(PyExc_TypeError, "Channel state 'keyframes' must be a list");
                return -1;
            }
            
            Py_ssize_t num_keyframes = PyList_Size(keyframes_obj);
            
            for (Py_ssize_t i = 0; i < num_keyframes; ++i) {
                PyObject* kf_state = PyList_GetItem(keyframes_obj, i);
                if (!PyDict_Check(kf_state)) {
                    PyErr_Format(PyExc_ValueError, "Keyframe %zd state must be a dictionary", i);
                    return -1;
                }
                
                // Create temporary keyframe and set its state
                PyKeyframe* temp_kf = KeyframeToPyKeyframe(anim::Keyframe(0.0, 0.0));
                if (!temp_kf) return -1;
                
                if (PyKeyframe_set_state(temp_kf, kf_state, NULL) < 0) {
                    Py_DECREF(temp_kf);
                    return -1;
                }
                
                // Create keyframe in channel
                self->channel->create_keyframe(
                    temp_kf->keyframe.position.time,
                    temp_kf->keyframe.position.value,
                    temp_kf->keyframe.in_handle,
                    temp_kf->keyframe.out_handle,
                    temp_kf->keyframe.function,
                    temp_kf->keyframe.handle_mode
                );
                
                Py_DECREF(temp_kf);
            }
        }
        
        if (node_struct) {
            node_struct->context->makeNodeDirty();
        }
        return 0;
        
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }
}

static PyObject* PyChannel_get_state_method(PyChannel *self, PyObject *args) {
    return PyChannel_get_state(self, NULL);
}

static PyObject* PyChannel_set_state_method(PyChannel *self, PyObject *args) {
    PyObject* state;
    if (!PyArg_ParseTuple(args, "O", &state))
        return NULL;
    
    if (PyChannel_set_state(self, state, NULL) < 0)
        return NULL;
    
    Py_RETURN_NONE;
}

static PyObject* PyChannel_copy(PyChannel *self) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    PyObject* object = ChannelToPyObject(self->channel, self->parent);
    if (!object) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create Channel object");
        return NULL;
    }
    Py_INCREF(object); // Ensure we return a new reference
    return object;
}

static PyObject* PyChannel_deep_copy(PyChannel *self, PyObject *args) {
    if (!self->channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    // Create a new Channel object with the same properties
    PyObject* copy = ChannelToPyObject(self->channel, self->parent);
    if (!copy) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create deep copy of Channel");
        return NULL;
    }
    
    // Ensure we return a new reference
    Py_INCREF(copy);
    return copy;
}

// --- Method definitions ---
static PyMethodDef PyChannel_methods[] = {
    {"__copy__", (PyCFunction)PyChannel_copy, METH_NOARGS, "Create a shallow copy of the channel"},
    {"__deepcopy__", (PyCFunction)PyChannel_deep_copy, METH_VARARGS, "Create a deep copy of the channel"},
    {"create_keyframe", (PyCFunction)PyChannel_create_keyframe, METH_VARARGS | METH_KEYWORDS, "Create a keyframe (overloads supported)"},
    {"emplace_keyframe", (PyCFunction)PyChannel_emplace_keyframe, METH_VARARGS, "Emplace a keyframe (move) into the channel"},
    {"delete_keyframe", (PyCFunction)PyChannel_remove_keyframe, METH_VARARGS, "Delete a keyframe by index"},
    {"keyframe", (PyCFunction)PyChannel_get_keyframe, METH_VARARGS, "Get a keyframe by index"},
    {"prev_keyframe", (PyCFunction)PyChannel_prev_keyframe, METH_VARARGS, "Get previous keyframe before time"},
    {"next_keyframe", (PyCFunction)PyChannel_next_keyframe, METH_VARARGS, "Get next keyframe after time"},
    {"closest_keyframe", (PyCFunction)PyChannel_closest_keyframe, METH_VARARGS, "Get closest keyframe to time"},
    {"update_keyframe", (PyCFunction)PyChannel_update_keyframe, METH_VARARGS, "Update a keyframe at index with a new keyframe object"},
    {"set_keyframe_time", (PyCFunction)PyChannel_set_keyframe_time, METH_VARARGS, "Set keyframe time at index"},
    {"set_keyframe_value", (PyCFunction)PyChannel_set_keyframe_value, METH_VARARGS, "Set keyframe value at index"},
    {"set_keyframe_position", (PyCFunction)PyChannel_set_keyframe_position, METH_VARARGS, "Set keyframe position at index"},
    {"set_keyframe_in_handle", (PyCFunction)PyChannel_set_keyframe_in_handle, METH_VARARGS, "Set keyframe in handle at index"},
    {"set_keyframe_out_handle", (PyCFunction)PyChannel_set_keyframe_out_handle, METH_VARARGS, "Set keyframe out handle at index"},
    {"set_keyframe_function", (PyCFunction)PyChannel_set_keyframe_function, METH_VARARGS, "Set keyframe function at index"},
    {"set_keyframe_handle_mode", (PyCFunction)PyChannel_set_keyframe_handle_mode, METH_VARARGS, "Set keyframe handle mode at index"},
    {"evaluate", (PyCFunction)PyChannel_evaluate, METH_VARARGS, "Evaluate the channel at a specific time"},
    {"evaluate_range", (PyCFunction)PyChannel_evaluate_range, METH_VARARGS, "Evaluate the channel over a range (start_time, end_time, num_samples)"},
    {"evaluate_range_by_rate", (PyCFunction)PyChannel_evaluate_range_by_rate, METH_VARARGS, "Evaluate the channel over a range by sample rate (start_time, end_time, sample_rate)"},
    {"num_samples", (PyCFunction)PyChannel_num_samples, METH_VARARGS, "Get the number of samples for a given sample rate"},
    {"get_state", (PyCFunction)PyChannel_get_state_method, METH_NOARGS, "Get Channel state as dictionary"},
    {"set_state", (PyCFunction)PyChannel_set_state_method, METH_VARARGS, "Set Channel state from dictionary"},
    {NULL}  // Sentinel
};

// --- Sequence protocol ---
static PySequenceMethods PyChannel_as_sequence = {
    (lenfunc)PyChannel_len, // sq_length
    0, // sq_concat
    0, // sq_repeat
    (ssizeargfunc)PyChannel_getitem, // sq_item
    0, // sq_slice
    0, // sq_ass_item
    0, // sq_ass_slice
    (objobjproc)PyChannel_contains, // sq_contains
    0, // sq_inplace_concat
    0  // sq_inplace_repeat
};

// --- Properties ---
static PyGetSetDef PyChannel_getset[] = {
    {"name", (getter)PyChannel_get_name, (setter)PyChannel_set_name, "Channel name", NULL},
    {"size", (getter)PyChannel_size, NULL, "Number of keyframes", NULL},
    {"num_keyframes", (getter)PyChannel_num_keyframes, NULL, "Number of keyframes", NULL},
    {"empty", (getter)PyChannel_empty, NULL, "True if channel is empty", NULL},
    {"start_time", (getter)PyChannel_start_time, NULL, "Start time", NULL},
    {"end_time", (getter)PyChannel_end_time, NULL, "End time", NULL},
    {"length", (getter)PyChannel_length, NULL, "Length", NULL},
    {"state", (getter)PyChannel_get_state, (setter)PyChannel_set_state, "Channel state as dictionary", NULL},
    {NULL}
};

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
    &PyChannel_as_sequence,   // tp_as_sequence
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
    PyChannel_getset,         // tp_getset
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
