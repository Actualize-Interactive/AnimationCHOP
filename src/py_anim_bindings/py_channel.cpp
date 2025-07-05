#include "py_channel.h"
#include "py_keyframe.h" // For KeyframeToPY_Object, PY_KeyframeType
#include "utils.h" // For AnimationCHOP, AnimationToPY_Object
#include <vector>
#include <optional>

// Allocation/deallocation functions
static PyObject* PY_Channel_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PY_Channel *self = (PY_Channel *)type->tp_alloc(type, 0);
    if (self != NULL) {
        // id member is allocated but not constructed with a specific value yet.
        // parent is set to nullptr to indicate it's not fully formed by the factory.
        self->parent = nullptr;
    }
    return (PyObject *)self;
}

static void PY_Channel_dealloc(PY_Channel *self) {

    if (self->parent) {
        self->channel_id.~Id(); // Clean up the Id member
    }

    Py_XDECREF(self->parent);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Initialize the object - NOTE: This should not be called directly, use ChannelToPY_Object instead
static int PY_Channel_init(PY_Channel *self, PyObject *args, PyObject *kwds) {
    PyErr_SetString(PyExc_RuntimeError, "Channel objects cannot be created directly. Use AnimationCHOP methods to create channels.");
    return -1;
}

// --- Keyframe creation ---
static PyObject* create_keyframe_time_value(PY_Channel* self, PyObject* args) {
    double time, value;
    int function = (int)anim::Function::Bezier;
    int handle_mode = (int)anim::HandleMode::Smooth;
    
    auto argc = PyTuple_Size(args);
    if (argc == 2) {
        if (!PyArg_ParseTuple(args, "dd", &time, &value)) return NULL;
    } else if (argc == 4) {
        if (!PyArg_ParseTuple(args, "ddii", &time, &value, &function, &handle_mode)) return NULL;
    } else {
        return NULL; // Wrong arg count
    }
    
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    try {
        const anim::Keyframe& kf = channelData.channel->create_keyframe(time, value, (anim::Function)function, (anim::HandleMode)handle_mode);
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        return KeyframeToPY_Object(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* create_keyframe_point(PY_Channel* self, PyObject* args) {
    PyObject *position_obj;
    int function = (int)anim::Function::Bezier;
    int handle_mode = (int)anim::HandleMode::Smooth;
    
    auto argc = PyTuple_Size(args);
    if (argc == 1) {
        if (!PyArg_ParseTuple(args, "O", &position_obj)) return NULL;
    } else if (argc == 3) {
        if (!PyArg_ParseTuple(args, "Oii", &position_obj, &function, &handle_mode)) return NULL;
    } else {
        return NULL; // Wrong arg count
    }
    
    anim::Point pos;
    if (!PY_ObjectToPoint(position_obj, pos)) return NULL;
    
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    try {
        const anim::Keyframe& kf = channelData.channel->create_keyframe(pos, (anim::Function)function, (anim::HandleMode)handle_mode);
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        return KeyframeToPY_Object(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* create_keyframe_with_handles(PY_Channel* self, PyObject* args) {
    double time, value;
    PyObject *in_handle_obj, *out_handle_obj;
    int function = (int)anim::Function::Bezier;
    int handle_mode = (int)anim::HandleMode::Smooth;
    
    auto argc = PyTuple_Size(args);
    if (argc == 4) {
        if (!PyArg_ParseTuple(args, "ddOO", &time, &value, &in_handle_obj, &out_handle_obj)) return NULL;
    } else if (argc == 6) {
        if (!PyArg_ParseTuple(args, "ddOOii", &time, &value, &in_handle_obj, &out_handle_obj, &function, &handle_mode)) return NULL;
    } else {
        return NULL; // Wrong arg count
    }
    
    anim::Point in_handle, out_handle;
    if (!PY_ObjectToPoint(in_handle_obj, in_handle) || !PY_ObjectToPoint(out_handle_obj, out_handle)) return NULL;
    
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    try {
        const anim::Keyframe& kf = channelData.channel->create_keyframe(time, value, in_handle, out_handle, (anim::Function)function, (anim::HandleMode)handle_mode);
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        return KeyframeToPY_Object(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PY_Channel_create_keyframe(PY_Channel *self, PyObject *args, PyObject *kwds) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
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
    if (PyObject_TypeCheck(first_arg, &PY_PointType)) {
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
            if (PyObject_TypeCheck(third_arg, &PY_PointType) && PyObject_TypeCheck(fourth_arg, &PY_PointType)) {
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

static PyObject* PY_Channel_create_keyframe_from_state(PY_Channel* self, PyObject* args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    PyObject* arg;
    if (!PyArg_ParseTuple(args, "O", &arg)) {
        return NULL;
    }
    
    // Check if argument is a dictionary (Keyframe state)
    if (!PyDict_Check(arg)) {
        PyErr_SetString(PyExc_ValueError, "Keyframe state must be a dictionary");
        return NULL;
    }
    
    // Create temporary keyframe and set its state
    PY_Keyframe* temp_kf = KeyframeToPY_Keyframe(anim::Keyframe(0.0, 0.0));
    if (!temp_kf) return NULL;
    
    if (PY_Keyframe_set_state(temp_kf, arg, NULL) < 0) {
        Py_DECREF(temp_kf);
        return NULL;
    }
    try {
        const anim::Keyframe& result = channelData.channel->create_keyframe(temp_kf->keyframe);
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        Py_DECREF(temp_kf);
        return KeyframeToPY_Object(result);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        Py_DECREF(temp_kf);
        return NULL;
    }

}

static PyObject* PY_Channel_emplace_keyframe(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }

    PyObject* py_keyframe;
    if (!PyArg_ParseTuple(args, "O", &py_keyframe))
        return NULL;
    anim::Keyframe kf;
    if (!PY_ObjectToKeyframe(py_keyframe, kf))
        return NULL;
    try {
        const anim::Keyframe& result = channelData.channel->emplace_keyframe(std::move(kf));
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        return KeyframeToPY_Object(result);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// --- Keyframe access ---
static PyObject* PY_Channel_getitem(PY_Channel *self, Py_ssize_t index) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    try {
        const anim::Keyframe& kf = channelData.channel->keyframe(static_cast<size_t>(index));
        return KeyframeToPY_Object(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_IndexError, e.what());
        return NULL;
    }
}

static Py_ssize_t PY_Channel_len(PY_Channel *self) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) return 0;
    return (Py_ssize_t)channelData.channel->num_keyframes();
}

static int PY_Channel_contains(PY_Channel *self, PyObject *key) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) return 0;
    double time = 0.0;
    if (PyFloat_Check(key)) {
        time = PyFloat_AsDouble(key);
    } else if (PyLong_Check(key)) {
        time = (double)PyLong_AsDouble(key);
    } else {
        return 0;
    }
    return channelData.channel->has_keyframe(time) ? 1 : 0;
}

// --- Keyframe queries ---
static PyObject* PY_Channel_prev_keyframe(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double time;
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;
    try {
        const anim::Keyframe& kf = channelData.channel->prev_keyframe(time);
        return KeyframeToPY_Object(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PY_Channel_next_keyframe(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double time;
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;
    try {
        const anim::Keyframe& kf = channelData.channel->next_keyframe(time);
        return KeyframeToPY_Object(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PY_Channel_closest_keyframe(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double time;
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;
    try {
        const anim::Keyframe& kf = channelData.channel->closest_keyframe(time);
        return KeyframeToPY_Object(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// --- Keyframe update/setters ---
static PyObject* PY_Channel_update_keyframe(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    Py_ssize_t index; PyObject* value;
    if (!PyArg_ParseTuple(args, "nO", &index, &value)) return NULL;
    anim::Keyframe kf;
    if (!PY_ObjectToKeyframe(value, kf)) return NULL;
    try {
        channelData.channel->update_keyframe(static_cast<size_t>(index), kf);
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

static PyObject* PY_Channel_set_keyframe_time(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    Py_ssize_t index; double value;
    if (!PyArg_ParseTuple(args, "nd", &index, &value)) return NULL;
    try { 
        channelData.channel->set_keyframe_time(static_cast<size_t>(index), value); 
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

static PyObject* PY_Channel_set_keyframe_value(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    Py_ssize_t index; double value;
    if (!PyArg_ParseTuple(args, "nd", &index, &value)) return NULL;
    try { 
        channelData.channel->set_keyframe_value(static_cast<size_t>(index), value); 
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

static PyObject* PY_Channel_set_keyframe_position(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    Py_ssize_t index;
    PyObject* value;
    if (PyArg_ParseTuple(args, "nO", &index, &value)) {
        // Try Point object
        anim::Point pt;
        if (!PY_ObjectToPoint(value, pt)) return NULL;
        try { 
            channelData.channel->set_keyframe_position(static_cast<size_t>(index), pt); 
            if (channelData.node_struct) {
                channelData.node_struct->context->makeNodeDirty();
            }
            Py_RETURN_NONE; 
        }
        catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
    } else {
        PyErr_Clear();
        double t, v;
        if (PyArg_ParseTuple(args, "ndd", &index, &t, &v)) {
            try { 
                channelData.channel->set_keyframe_position(static_cast<size_t>(index), t, v); 
                if (channelData.node_struct) {
                    channelData.node_struct->context->makeNodeDirty();
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

static PyObject* PY_Channel_set_keyframe_in_handle(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    Py_ssize_t index; PyObject* value;
    if (!PyArg_ParseTuple(args, "nO", &index, &value)) return NULL;
    anim::Point pt;
    if (!PY_ObjectToPoint(value, pt)) return NULL;
    try { 
        channelData.channel->set_keyframe_in_handle(static_cast<size_t>(index), pt); 
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

static PyObject* PY_Channel_set_keyframe_out_handle(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    Py_ssize_t index; PyObject* value;
    if (!PyArg_ParseTuple(args, "nO", &index, &value)) return NULL;
    anim::Point pt;
    if (!PY_ObjectToPoint(value, pt)) return NULL;
    try { 
        channelData.channel->set_keyframe_out_handle(static_cast<size_t>(index), pt); 
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

static PyObject* PY_Channel_set_keyframe_function(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    Py_ssize_t index; int value;
    if (!PyArg_ParseTuple(args, "ni", &index, &value)) return NULL;
    try { 
        channelData.channel->set_keyframe_function(static_cast<size_t>(index), (anim::Function)value); 
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

static PyObject* PY_Channel_set_keyframe_handle_mode(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) { PyErr_SetString(PyExc_RuntimeError, "Channel is not valid"); return NULL; }
    Py_ssize_t index; int value;
    if (!PyArg_ParseTuple(args, "ni", &index, &value)) return NULL;
    try { 
        channelData.channel->set_keyframe_handle_mode(static_cast<size_t>(index), (anim::HandleMode)value); 
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE; 
    }
    catch (const std::exception& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); return NULL; }
}

// --- Keyframe removal ---
static PyObject* PY_Channel_remove_keyframe(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    Py_ssize_t index;
    if (!PyArg_ParseTuple(args, "n", &index))
        return NULL;
    try {
        channelData.channel->delete_keyframe(static_cast<size_t>(index));
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        Py_RETURN_NONE;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// --- Keyframe get by index ---
static PyObject* PY_Channel_get_keyframe(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    Py_ssize_t index;
    if (!PyArg_ParseTuple(args, "n", &index))
        return NULL;
    try {
        const anim::Keyframe& kf = channelData.channel->keyframe(static_cast<size_t>(index));
        return KeyframeToPY_Object(kf);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_IndexError, e.what());
        return NULL;
    }
}

// --- String representation ---
static PyObject* PY_Channel_str(PY_Channel *self) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        return PyUnicode_FromString("<Channel (invalid)>");
    }
    std::string s = std::string("<Channel name='") + channelData.channel->name() + "' keyframes=" + std::to_string(channelData.channel->num_keyframes()) + ">";
    return PyUnicode_FromString(s.c_str());
}

// --- Evaluation ---
static PyObject* PY_Channel_evaluate(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double time;
    if (!PyArg_ParseTuple(args, "d", &time))
        return NULL;
    try {
        double value = channelData.channel->evaluate(time);
        return PyFloat_FromDouble(value);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PY_Channel_evaluate_range(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double start_time, end_time;
    int num_samples;
    if (!PyArg_ParseTuple(args, "ddi", &start_time, &end_time, &num_samples))
        return NULL;
    try {
        std::vector<double> values = channelData.channel->evaluate_range(start_time, end_time, num_samples);
        PyObject* list = PyList_New(values.size());
        for (size_t i = 0; i < values.size(); ++i)
            PyList_SET_ITEM(list, i, PyFloat_FromDouble(values[i]));
        return list;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* PY_Channel_evaluate_range_by_rate(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double start_time, end_time, sample_rate;
    if (!PyArg_ParseTuple(args, "ddd", &start_time, &end_time, &sample_rate))
        return NULL;
    try {
        std::vector<double> values = channelData.channel->evaluate_range_by_rate(start_time, end_time, sample_rate);
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
static PyObject* PY_Channel_get_name(PY_Channel *self, void*) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    PyObject* result = PyUnicode_FromString(channelData.channel->name().c_str());
    // PyUnicode_FromString sets exception on failure, so just return result
    return result;
}

static int PY_Channel_set_name(PY_Channel *self, PyObject* value, void*) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return -1;
    }
    if (!PyUnicode_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "Name must be a string");
        return -1;
    }
    channelData.channel->set_name(PyUnicode_AsUTF8(value));
    if (channelData.node_struct) {
        channelData.node_struct->context->makeNodeDirty();
    }
    return 0;
}

static PyObject* PY_Channel_size(PY_Channel *self, void*) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyLong_FromSize_t(channelData.channel->size());
}

static PyObject* PY_Channel_num_keyframes(PY_Channel *self, void*) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyLong_FromSize_t(channelData.channel->num_keyframes());
}

static PyObject* PY_Channel_empty(PY_Channel *self, void*) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyBool_FromLong(channelData.channel->empty() ? 1 : 0);
}

static PyObject* PY_Channel_start_time(PY_Channel *self, void*) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyFloat_FromDouble(channelData.channel->start_time());
}

static PyObject* PY_Channel_end_time(PY_Channel *self, void*) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyFloat_FromDouble(channelData.channel->end_time());
}

static PyObject* PY_Channel_length(PY_Channel *self, void*) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    return PyFloat_FromDouble(channelData.channel->length());
}

static PyObject* PY_Channel_num_samples(PY_Channel *self, PyObject* args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    double sample_rate;
    if (!PyArg_ParseTuple(args, "d", &sample_rate))
        return NULL;
    try {
        size_t n = channelData.channel->num_samples(sample_rate);
        return PyLong_FromSize_t(n);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// --- State methods ---
PyObject* PY_Channel_get_state(PY_Channel *self, void *closure) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    PyObject* state_dict = PyDict_New();
    if (!state_dict) return NULL;
    
    try {
        // Add channel properties
        PyDict_SetItemString(state_dict, "name", PyUnicode_FromString(channelData.channel->name().c_str()));
        PyDict_SetItemString(state_dict, "start_time", PyFloat_FromDouble(channelData.channel->start_time()));
        PyDict_SetItemString(state_dict, "end_time", PyFloat_FromDouble(channelData.channel->end_time()));
        PyDict_SetItemString(state_dict, "length", PyFloat_FromDouble(channelData.channel->length()));
        PyDict_SetItemString(state_dict, "num_keyframes", PyLong_FromSize_t(channelData.channel->num_keyframes()));
        PyDict_SetItemString(state_dict, "empty", PyBool_FromLong(channelData.channel->empty() ? 1 : 0));
        
        // Create keyframes list using keyframe state
        PyObject* keyframes_list = PyList_New(channelData.channel->num_keyframes());
        if (!keyframes_list) {
            Py_DECREF(state_dict);
            return NULL;
        }
        
        for (size_t i = 0; i < channelData.channel->num_keyframes(); ++i) {
            try {
                const auto& keyframe = channelData.channel->keyframe(i);
                PY_Keyframe* py_keyframe = KeyframeToPY_Keyframe(keyframe);
                if (!py_keyframe) {
                    Py_DECREF(keyframes_list);
                    Py_DECREF(state_dict);
                    return NULL;
                }
                
                PyObject* kf_state = PY_Keyframe_get_state(py_keyframe, NULL);
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

int PY_Channel_set_state(PY_Channel *self, PyObject *value, void *closure) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return -1;
    }
    
    if (!PyDict_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "Channel state must be a dictionary");
        return -1;
    }
    
    try {
        // Clear existing keyframes
        while (channelData.channel->num_keyframes() > 0) {
            channelData.channel->delete_keyframe(0);
        }
        
        // Set channel name if provided
        PyObject* name_obj = PyDict_GetItemString(value, "name");
        if (name_obj && PyUnicode_Check(name_obj)) {
            channelData.channel->set_name(PyUnicode_AsUTF8(name_obj));
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
                PY_Keyframe* temp_kf = KeyframeToPY_Keyframe(anim::Keyframe(0.0, 0.0));
                if (!temp_kf) return -1;
                
                if (PY_Keyframe_set_state(temp_kf, kf_state, NULL) < 0) {
                    Py_DECREF(temp_kf);
                    return -1;
                }
                channelData.channel->create_keyframe(temp_kf->keyframe);

                Py_DECREF(temp_kf);
            }
        }
        
        if (channelData.node_struct) {
            channelData.node_struct->context->makeNodeDirty();
        }
        return 0;
        
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }
}

static PyObject* PY_Channel_get_state_method(PY_Channel *self, PyObject *args) {
    return PY_Channel_get_state(self, NULL);
}

static PyObject* PY_Channel_set_state_method(PY_Channel *self, PyObject *args) {
    PyObject* state;
    if (!PyArg_ParseTuple(args, "O", &state))
        return NULL;
    
    if (PY_Channel_set_state(self, state, NULL) < 0)
        return NULL;
    
    Py_RETURN_NONE;
}

static PyObject* PY_Channel_copy(PY_Channel *self) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    PyObject* object = ChannelToPY_Object(channelData.channel, self->parent);
    if (!object) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create Channel object");
        return NULL;
    }
    return object;
}

// TODO update this it is a shallow copy, we need to either implement a deep copy
// in the anim library or handle it here
static PyObject* PY_Channel_deep_copy(PY_Channel *self, PyObject *args) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    PyObject* copy = ChannelToPY_Object(channelData.channel, self->parent);
    if (!copy) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create deep copy of Channel");
        return NULL;
    }
    return copy;
}

// --- Method definitions ---
static PyMethodDef PY_Channel_methods[] = {
    {"__copy__", (PyCFunction)PY_Channel_copy, METH_NOARGS, "Create a shallow copy of the channel"},
    {"__deepcopy__", (PyCFunction)PY_Channel_deep_copy, METH_VARARGS, "Create a deep copy of the channel"},
    {"create_keyframe", (PyCFunction)PY_Channel_create_keyframe, METH_VARARGS | METH_KEYWORDS, "Create a keyframe (overloads supported)"},
    {"create_keyframe_from_state", (PyCFunction)PY_Channel_create_keyframe_from_state, METH_VARARGS, "Create a keyframe from a state dictionary"},
    {"emplace_keyframe", (PyCFunction)PY_Channel_emplace_keyframe, METH_VARARGS, "Emplace a keyframe (move) into the channel"},
    {"delete_keyframe", (PyCFunction)PY_Channel_remove_keyframe, METH_VARARGS, "Delete a keyframe by index"},
    {"keyframe", (PyCFunction)PY_Channel_get_keyframe, METH_VARARGS, "Get a keyframe by index"},
    {"prev_keyframe", (PyCFunction)PY_Channel_prev_keyframe, METH_VARARGS, "Get previous keyframe before time"},
    {"next_keyframe", (PyCFunction)PY_Channel_next_keyframe, METH_VARARGS, "Get next keyframe after time"},
    {"closest_keyframe", (PyCFunction)PY_Channel_closest_keyframe, METH_VARARGS, "Get closest keyframe to time"},
    {"update_keyframe", (PyCFunction)PY_Channel_update_keyframe, METH_VARARGS, "Update a keyframe at index with a new keyframe object"},
    {"set_keyframe_time", (PyCFunction)PY_Channel_set_keyframe_time, METH_VARARGS, "Set keyframe time at index"},
    {"set_keyframe_value", (PyCFunction)PY_Channel_set_keyframe_value, METH_VARARGS, "Set keyframe value at index"},
    {"set_keyframe_position", (PyCFunction)PY_Channel_set_keyframe_position, METH_VARARGS, "Set keyframe position at index"},
    {"set_keyframe_in_handle", (PyCFunction)PY_Channel_set_keyframe_in_handle, METH_VARARGS, "Set keyframe in handle at index"},
    {"set_keyframe_out_handle", (PyCFunction)PY_Channel_set_keyframe_out_handle, METH_VARARGS, "Set keyframe out handle at index"},
    {"set_keyframe_function", (PyCFunction)PY_Channel_set_keyframe_function, METH_VARARGS, "Set keyframe function at index"},
    {"set_keyframe_handle_mode", (PyCFunction)PY_Channel_set_keyframe_handle_mode, METH_VARARGS, "Set keyframe handle mode at index"},
    {"evaluate", (PyCFunction)PY_Channel_evaluate, METH_VARARGS, "Evaluate the channel at a specific time"},
    {"evaluate_range", (PyCFunction)PY_Channel_evaluate_range, METH_VARARGS, "Evaluate the channel over a range (start_time, end_time, num_samples)"},
    {"evaluate_range_by_rate", (PyCFunction)PY_Channel_evaluate_range_by_rate, METH_VARARGS, "Evaluate the channel over a range by sample rate (start_time, end_time, sample_rate)"},
    {"num_samples", (PyCFunction)PY_Channel_num_samples, METH_VARARGS, "Get the number of samples for a given sample rate"},
    {"get_state", (PyCFunction)PY_Channel_get_state_method, METH_NOARGS, "Get Channel state as dictionary"},
    {"set_state", (PyCFunction)PY_Channel_set_state_method, METH_VARARGS, "Set Channel state from dictionary"},
    {NULL}  // Sentinel
};

// --- Sequence protocol ---
static PySequenceMethods PY_Channel_as_sequence = {
    (lenfunc)PY_Channel_len, // sq_length
    0, // sq_concat
    0, // sq_repeat
    (ssizeargfunc)PY_Channel_getitem, // sq_item
    0, // sq_slice
    0, // sq_ass_item
    0, // sq_ass_slice
    (objobjproc)PY_Channel_contains, // sq_contains
    0, // sq_inplace_concat
    0  // sq_inplace_repeat
};

// --- Mapping protocol for subscript operator ---
static PyObject* PY_Channel_mp_subscript(PY_Channel *self, PyObject *key) {
    auto channelData = getChannelData(self, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return NULL;
    }
    
    if (PyLong_Check(key)) {
        Py_ssize_t index = PyLong_AsSsize_t(key);
        if (index == -1 && PyErr_Occurred()) {
            return NULL;
        }
        
        // Handle negative indices
        if (index < 0) {
            index += (Py_ssize_t)channelData.channel->num_keyframes();
        }
        
        try {
            const anim::Keyframe& kf = channelData.channel->keyframe(static_cast<size_t>(index));
            return KeyframeToPY_Object(kf);
        } catch (const std::exception& e) {
            PyErr_SetString(PyExc_IndexError, e.what());
            return NULL;
        }
    }
    
    PyErr_SetString(PyExc_TypeError, "Channel indices must be integers");
    return NULL;
}

static PyMappingMethods PY_Channel_as_mapping = {
    (lenfunc)PY_Channel_len,        // mp_length
    (binaryfunc)PY_Channel_mp_subscript,  // mp_subscript
    0,                              // mp_ass_subscript
};

// --- Properties ---
static PyGetSetDef PY_Channel_getset[] = {
    {"name", (getter)PY_Channel_get_name, (setter)PY_Channel_set_name, "Channel name", NULL},
    {"size", (getter)PY_Channel_size, NULL, "Number of keyframes", NULL},
    {"num_keyframes", (getter)PY_Channel_num_keyframes, NULL, "Number of keyframes", NULL},
    {"empty", (getter)PY_Channel_empty, NULL, "True if channel is empty", NULL},
    {"start_time", (getter)PY_Channel_start_time, NULL, "Start time", NULL},
    {"end_time", (getter)PY_Channel_end_time, NULL, "End time", NULL},
    {"length", (getter)PY_Channel_length, NULL, "Length", NULL},
    {"state", (getter)PY_Channel_get_state, (setter)PY_Channel_set_state, "Channel state as dictionary", NULL},
    {NULL}
};

// Type definition
PyTypeObject PY_ChannelType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "anim.Channel",          // tp_name
    sizeof(PY_Channel),       // tp_basicsize
    0,                         // tp_itemsize
    (destructor)PY_Channel_dealloc, // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    (reprfunc)PY_Channel_str,  // tp_repr
    0,                         // tp_as_number
    &PY_Channel_as_sequence,   // tp_as_sequence
    &PY_Channel_as_mapping,    // tp_as_mapping
    0,                         // tp_hash 
    0,                         // tp_call
    (reprfunc)PY_Channel_str,  // tp_str
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
    PY_Channel_methods,         // tp_methods
    0,                         // tp_members
    PY_Channel_getset,         // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    (initproc)PY_Channel_init,  // tp_init
    0,                         // tp_alloc
    PY_Channel_new,             // tp_new
};

// Type getter for external use
PyObject* get_channel_type(PyObject* self, void* closure) {
    if (PyType_Ready(&PY_ChannelType) < 0) { // Ensure type is ready
        return NULL;
    }
    Py_INCREF(&PY_ChannelType);
    return (PyObject*)&PY_ChannelType;
}

// Helper functions for conversion between C++ and Python
PyObject* ChannelToPY_Object(anim::Channel* channel, PyObject* parent) {
    // Ensure the type is initialized before creating an instance
    if (PyType_Ready(&PY_ChannelType) < 0) {
        return NULL;
    }
    // PY_Channel* py_channel = PyObject_New(PY_Channel, &PY_ChannelType);
    // if (py_channel == NULL) {
    //     return NULL;
    // }
    // py_channel->channel = channel;
    // py_channel->parent = parent;
    // Py_XINCREF(parent); // Keep parent alive
    // return (PyObject*)py_channel;


    // Allocate memory for the PY_Channel object using the type's allocator
    PY_Channel* self = (PY_Channel *)PY_ChannelType.tp_alloc(&PY_ChannelType, 0);
    if (self == NULL) {
        return NULL;
    }
    // Initialize parent to nullptr temporarily. If placement new fails,
    // self->parent will be nullptr, guiding PY_Channel_dealloc.
    self->parent = nullptr;

    try {
        new (&self->channel_id) anim::Id(channel->id().id); // Use placement new to initialize id
        Py_XINCREF(parent); // Increase reference count for parent
        self->parent = parent; // Assign parent
    } catch (const std::exception& e) {
        PY_ChannelType.tp_free(self); // Deallocate the raw memory if C++ construction fails
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    } catch (...) {
        PY_ChannelType.tp_free(self); // Deallocate the raw memory if any other exception occurs
        PyErr_SetString(PyExc_RuntimeError, "Unknown C++ exception during PY_Channel construction");
        return NULL;
    }   

    return (PyObject*)self;
}

bool PY_ObjectToChannel(PyObject* obj, anim::Channel*& channel) {
    if (!PyObject_TypeCheck(obj, &PY_ChannelType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a Channel object");
        return false;
    }
    
    PY_Channel* py_channel = (PY_Channel*)obj;
    auto channelData = getChannelData(py_channel, false);
    if (!channelData.channel) {
        PyErr_SetString(PyExc_RuntimeError, "Channel is not valid");
        return false;
    }

    channel = channelData.channel;
    return true;
}


ChannelData getChannelData(PY_Channel *self, bool autoCook) {
    if (!self || !self->parent) {
        PyErr_SetString(PyExc_RuntimeError, "PyChannel or its parent is invalid.");
        return ChannelData();
    }
    // Assuming get_td_node_struct can get you to the AnimationCHOP C++ instance
    // and then to the anim::Animation instance.
    TD::PY_Struct* td_struct = get_td_node_struct(self->parent, nullptr);
    if (!td_struct || !td_struct->context) { // Simplified check
        PyErr_SetString(PyExc_RuntimeError, "Cannot retrieve CHOP context.");
        return ChannelData();
    }

    TD::PY_GetInfo info;
    info.autoCook = autoCook;
    AnimationCHOP* inst = (AnimationCHOP*)td_struct->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot retrieve AnimationCHOP instance.");
        return ChannelData();
    }

    anim::Animation* animation = inst->animation(); // Assuming this method exists
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot retrieve anim::Animation instance.");
        return ChannelData();
    }
    // anim::Animation would need a method like getChannelById
    return { animation->channel(self->channel_id), inst, td_struct };
}