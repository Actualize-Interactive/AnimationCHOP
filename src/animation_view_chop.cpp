#include "animation_view_chop.h"

#include <cstring>
#include <algorithm>


#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
	#include <modsupport.h>

#else
	#include <Python.h>
	#include <structmember.h>
#endif


extern "C"
{
// Python binding methods
static PyObject* py_select_keyframes(PyObject* self, PyObject* args);
static PyObject* py_unselect_keyframes(PyObject* self, PyObject* args);
static PyObject* py_unselect_all_keyframes(PyObject* self, PyObject* args);
static PyObject* py_selected_keyframes(PyObject* self, PyObject* args);

static PyObject* py_select_segments(PyObject* self, PyObject* args);
static PyObject* py_unselect_segments(PyObject* self, PyObject* args);
static PyObject* py_unselect_all_segments(PyObject* self, PyObject* args);
static PyObject* py_selected_segments(PyObject* self, PyObject* args);

static PyObject* py_select_start_handles(PyObject* self, PyObject* args);
static PyObject* py_unselect_start_handles(PyObject* self, PyObject* args);
static PyObject* py_unselect_all_start_handles(PyObject* self, PyObject* args);
static PyObject* py_selected_start_handles(PyObject* self, PyObject* args);

static PyObject* py_select_end_handles(PyObject* self, PyObject* args);
static PyObject* py_unselect_end_handles(PyObject* self, PyObject* args);
static PyObject* py_unselect_all_end_handles(PyObject* self, PyObject* args);
static PyObject* py_selected_end_handles(PyObject* self, PyObject* args);

static PyObject* py_select_channels(PyObject* self, PyObject* args);
static PyObject* py_unselect_channels(PyObject* self, PyObject* args);
static PyObject* py_unselect_all_channels(PyObject* self, PyObject* args);
static PyObject* py_selected_channels(PyObject* self, PyObject* args);
static PyObject* py_set_channel_display(PyObject* self, PyObject* args);

// Python method table for AnimationViewCHOP
static PyMethodDef viewMethods[] = {
    {"select_keyframes", (PyCFunction)py_select_keyframes, METH_VARARGS, "Select keyframes by indices."},
    {"unselect_keyframes", (PyCFunction)py_unselect_keyframes, METH_VARARGS, "Unselect keyframes by indices."},
    {"unselect_all_keyframes", (PyCFunction)py_unselect_all_keyframes, METH_NOARGS, "Unselect all keyframes."},
    {"selected_keyframes", (PyCFunction)py_selected_keyframes, METH_NOARGS, "Get selected keyframe indices."},
    
    {"select_segments", (PyCFunction)py_select_segments, METH_VARARGS, "Select segments by indices."},
    {"unselect_segments", (PyCFunction)py_unselect_segments, METH_VARARGS, "Unselect segments by indices."},
    {"unselect_all_segments", (PyCFunction)py_unselect_all_segments, METH_NOARGS, "Unselect all segments."},
    {"selected_segments", (PyCFunction)py_selected_segments, METH_NOARGS, "Get selected segment indices."},
    
    {"select_start_handles", (PyCFunction)py_select_start_handles, METH_VARARGS, "Select start handles by indices."},
    {"unselect_start_handles", (PyCFunction)py_unselect_start_handles, METH_VARARGS, "Unselect start handles by indices."},
    {"unselect_all_start_handles", (PyCFunction)py_unselect_all_start_handles, METH_NOARGS, "Unselect all start handles."},
    {"selected_start_handles", (PyCFunction)py_selected_start_handles, METH_NOARGS, "Get selected start handle indices."},
    
    {"select_end_handles", (PyCFunction)py_select_end_handles, METH_VARARGS, "Select end handles by indices."},
    {"unselect_end_handles", (PyCFunction)py_unselect_end_handles, METH_VARARGS, "Unselect end handles by indices."},
    {"unselect_all_end_handles", (PyCFunction)py_unselect_all_end_handles, METH_NOARGS, "Unselect all end handles."},
    {"selected_end_handles", (PyCFunction)py_selected_end_handles, METH_NOARGS, "Get selected end handle indices."},
    
    {"select_channels", (PyCFunction)py_select_channels, METH_VARARGS, "Select channels by indices."},
    {"unselect_channels", (PyCFunction)py_unselect_channels, METH_VARARGS, "Unselect channels by indices."},
    {"unselect_all_channels", (PyCFunction)py_unselect_all_channels, METH_NOARGS, "Unselect all channels."},
    {"selected_channels", (PyCFunction)py_selected_channels, METH_NOARGS, "Get selected channel indices."},
    {"set_channel_display", (PyCFunction)py_set_channel_display, METH_VARARGS, "Set channel display state by indices."},
    
    {nullptr, nullptr, 0, nullptr}
};

DLLEXPORT void 
FillCHOPPluginInfo(CHOP_PluginInfo* info)
{
    info->apiVersion = CHOPCPlusPlusAPIVersion;
    info->customOPInfo.opType->setString("Animationview");
    info->customOPInfo.opLabel->setString("Animation View");
    info->customOPInfo.opIcon->setString("AMV");
    info->customOPInfo.authorName->setString("Keith Lostracco");
    info->customOPInfo.authorEmail->setString("keith@actualize.vision");
    info->customOPInfo.minInputs = 0;
    info->customOPInfo.maxInputs = 0;
    info->customOPInfo.pythonVersion->setString(PY_VERSION);
    info->customOPInfo.pythonMethods = viewMethods;
}

DLLEXPORT 
CHOP_CPlusPlusBase* CreateCHOPInstance(const OP_NodeInfo* info)
{
    return new AnimationViewCHOP(info);
}

DLLEXPORT void 
DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
{
    delete static_cast<AnimationViewCHOP*>(instance);
}

}; // extern "C"

AnimationViewCHOP::AnimationViewCHOP(const OP_NodeInfo* info)
    : m_warning(nullptr)
    , m_error(nullptr)
    , m_viewMode(ViewMode::samples)
    , m_samplesStartTime(0.0)
    , m_samplesEndTime(30.0)
    , m_animationCHOP(nullptr)
    , m_dataInstance(nullptr)
{
}

AnimationViewCHOP::~AnimationViewCHOP()
{
}

void 
AnimationViewCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
    ginfo->cookEveryFrameIfAsked = false;
    ginfo->timeslice = false;
}

bool 
AnimationViewCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
    info->sampleRate = static_cast<float>(inputs->getParDouble("Samplerate", 0));
    m_viewMode = static_cast<ViewMode>(inputs->getParInt("Viewmode", 0));
    
    info->startIndex = 0;

    if (!setDataInstance(inputs)) {
        return false;
    }
    
    // If we are the data instance, store inputs and initialize all data members
    if (m_dataInstance) {

        auto animation = animationCHOP()->animation();
        if (!animation) {
            return false;
        }
        
        // Initialize all data members regardless of current view mode
        int32_t total_keyframes = 0;
        int32_t total_segments = 0;
        for (const auto& channel : animation->channels()) {
            total_keyframes += static_cast<int32_t>(channel->size());
            total_segments += static_cast<int32_t>(channel->size() - 1);
        }
        
        m_selectedKeyframes.resize(total_keyframes, false);
        m_selectedSegments.resize(total_segments, false);
        m_selectedStartHandles.resize(total_segments, false);
        m_selectedEndHandles.resize(total_segments, false);
        m_selectedChannels.resize(animation->size(), false);
        m_displayedChannels.resize(animation->size(), true);
    }

    // Set output info based on current view mode
    switch (m_viewMode) {
    case ViewMode::samples: {
        auto animation = animationCHOP()->animation();
        if (!animation) {
            return false;
        }
        info->numChannels = static_cast<int32_t>(animation->size());
        auto rangeStart = inputs->getParDouble("Range", 0);
        auto rangeEnd = inputs->getParDouble("Range", 1);
        auto range_delta = rangeEnd - rangeStart;

        auto rangeUnit = inputs->getParString("Rangeunit");
        if (strcmp(rangeUnit, "samples") == 0) { // Samples
            info->numSamples = static_cast<int32_t>(range_delta + 1.0);
            m_samplesStartTime = rangeStart / info->sampleRate;
            m_samplesEndTime = rangeEnd / info->sampleRate;
        } else { // Seconds
            info->numSamples = static_cast<int32_t>(range_delta * info->sampleRate);
            m_samplesStartTime = rangeStart;
            m_samplesEndTime = rangeEnd;
        }
        return true;
    } case ViewMode::keyframes: {
        info->numChannels = static_cast<int32_t>(m_keyframes_chan_names.size());
        info->numSamples = static_cast<int32_t>(dataInstance()->m_selectedKeyframes.size());
        return true;
    } case ViewMode::segments: {
        info->numChannels = static_cast<int32_t>(m_segments_chan_names.size());
        info->numSamples = static_cast<int32_t>(dataInstance()->m_selectedSegments.size());
        return true;
    } case ViewMode::channels: {
        info->numChannels = static_cast<int32_t>(m_channels_chan_names.size());
        info->numSamples = static_cast<int32_t>(dataInstance()->m_selectedChannels.size());
        return true;
    } case ViewMode::animation: {
        info->numChannels = static_cast<int32_t>(m_animation_chan_names.size());
        info->numSamples = 1;
        return true;
    } default: {
        m_error = "Invalid select mode specified.";
        return false;
    }
    }
    return true;
}

void 
AnimationViewCHOP::getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void* reserved1)
{

    if (!setDataInstance(inputs)) {
        return;
    }
    switch (m_viewMode) {
    case ViewMode::samples: {
        auto animation = animationCHOP()->animation();
        if (!animation) {
            return;
        }
        if (index < 0 || index >= static_cast<int32_t>(animation->channel_names().size())) {
            return;
        }
        name->setString(animation->channel_names()[index].c_str());
        break;
    } case ViewMode::keyframes: {
        if (index < 0 || index >= static_cast<int32_t>(m_keyframes_chan_names.size())) {
            return;
        }
        name->setString(m_keyframes_chan_names[index]);
        break;
    } case ViewMode::segments: {
        if (index < 0 || index >= static_cast<int32_t>(m_segments_chan_names.size())) {
            return;
        }
        name->setString(m_segments_chan_names[index]);
        break;
    } case ViewMode::channels: {
        if (index < 0 || index >= static_cast<int32_t>(m_channels_chan_names.size())) {
            return;
        }
        name->setString(m_channels_chan_names[index]);
        break;
    } case ViewMode::animation: {
        if (index < 0 || index >= static_cast<int32_t>(m_animation_chan_names.size())) {
            return;
        }
        name->setString(m_animation_chan_names[index]);
        break;
    } default:
        m_error = "Invalid select mode specified.";
        return;
    }
}

void 
AnimationViewCHOP::execute(CHOP_Output* output, const OP_Inputs* inputs, void* reserved1)
{
    m_error = nullptr;
    m_warning = nullptr;

    if (!setDataInstance(inputs)) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }
    
    // Use existing execute logic with animation and our data members
    switch (m_viewMode) {
    case ViewMode::samples: {
        size_t num_anim_channels = animation->num_channels();
        for (int i = 0; i < output->numChannels; ++i) {
            if (i < static_cast<int>(num_anim_channels)) {
                auto samples = animation->channel(i).evaluate_range(
                    m_samplesStartTime,
                    m_samplesEndTime,
                    output->numSamples
                );
                std::copy(samples.begin(), samples.end(), output->channels[i]);
            }
        }
        break;
    } case ViewMode::keyframes: {
        size_t num_keyframe_channels = m_keyframes_chan_names.size();
        if (output->numChannels > num_keyframe_channels) {
            m_error = "Not enough channels allocated";
            return;
        }
        size_t i = 0;
        for (size_t c = 0; c < animation->size(); ++c) {
            auto& channel = animation->channel(c);
            for(size_t k = 0; k < channel.size(); ++k) {
                if (i < output->numSamples) {
                    output->channels[0][i] = static_cast<float>(c);
                    output->channels[1][i] = static_cast<float>(k);
                    output->channels[2][i] = static_cast<float>(channel.keyframe(k).time());
                    output->channels[3][i] = static_cast<float>(channel.keyframe(k).value());
                    output->channels[4][i] = static_cast<float>(channel.keyframe(k).in_handle.time);
                    output->channels[5][i] = static_cast<float>(channel.keyframe(k).in_handle.value);
                    output->channels[6][i] = static_cast<float>(channel.keyframe(k).out_handle.time);
                    output->channels[7][i] = static_cast<float>(channel.keyframe(k).out_handle.value);
                    output->channels[8][i] = static_cast<float>(channel.keyframe(k).function);
                    output->channels[9][i] = static_cast<float>(channel.keyframe(k).handle_mode);
                    output->channels[10][i] = static_cast<float>(dataInstance()->m_selectedKeyframes[i]);
                    output->channels[11][i] = static_cast<float>(dataInstance()->m_displayedChannels[c]);
                }
                ++i;
            }
        }
        break;
    } case ViewMode::segments: {
        size_t num_segment_info_channels = m_segments_chan_names.size();
        if (output->numChannels > num_segment_info_channels) {
            m_error = "Not enough channels allocated";
            return;
        }
        size_t i = 0;
        for (size_t c = 0; c < animation->size(); ++c) {
            auto& channel = animation->channel(c);
            for (size_t k = 0; k < channel.size() - 1; ++k) {
                auto start_keyframe = channel.keyframe(k);
                auto end_keyframe = channel.keyframe(k + 1);
                output->channels[0][i] = static_cast<float>(c); // Channel index
                output->channels[1][i] = static_cast<float>(k); // Segment index
                output->channels[2][i] = static_cast<float>(start_keyframe.time()); // Start time
                output->channels[3][i] = static_cast<float>(start_keyframe.value()); // Start value
                output->channels[4][i] = static_cast<float>(end_keyframe.time()); // End time
                output->channels[5][i] = static_cast<float>(end_keyframe.value()); // End value
                output->channels[6][i] = static_cast<float>(start_keyframe.out_handle.time); // Start handle time
                output->channels[7][i] = static_cast<float>(start_keyframe.out_handle.value); // Start handle value
                output->channels[8][i] = static_cast<float>(end_keyframe.in_handle.time); // End handle time
                output->channels[9][i] = static_cast<float>(end_keyframe.in_handle.value); // End handle value
                auto display_start_handle = dataInstance()->displayStartHandle(c, start_keyframe);
                output->channels[10][i] = static_cast<float>(display_start_handle);
                output->channels[11][i] = static_cast<float>(dataInstance()->displayEndHandle(display_start_handle, end_keyframe));
                output->channels[12][i] = static_cast<float>(dataInstance()->m_selectedSegments[i]); // Selected
                output->channels[13][i] = static_cast<float>(dataInstance()->m_selectedStartHandles[i]); // Selected start handles
                output->channels[14][i] = static_cast<float>(dataInstance()->m_selectedEndHandles[i]); // Selected end handles
                ++i;
            }
        }
        break;
    } case ViewMode::channels: {
        size_t num_channel_info_channels = m_channels_chan_names.size();
        if (output->numChannels > num_channel_info_channels) {
            m_error = "Not enough channels allocated";
            return;
        }
        int32_t start_index = 0;
        for (size_t c = 0; c < animation->size(); ++c) {
                auto& channel = animation->channel(c);
            if (c < output->numChannels) {
                auto num_keyframes = channel.size();
                output->channels[0][c] = static_cast<float>(num_keyframes);
                output->channels[1][c] = static_cast<float>(channel.start_time());
                output->channels[2][c] = static_cast<float>(channel.end_time());
                output->channels[3][c] = static_cast<float>(start_index);
                start_index += static_cast<int32_t>(num_keyframes);
                output->channels[4][c] = static_cast<float>(dataInstance()->m_selectedChannels[c]); // Selected
                output->channels[5][c] = static_cast<float>(dataInstance()->m_displayedChannels[c]); // Display
            }
        }
        break;
    } case ViewMode::animation: {
        size_t num_animation_info_channels = m_animation_chan_names.size();
        if (output->numChannels > num_animation_info_channels) {
            m_error = "Not enough channels allocated";
            return;
        }
        double min_keyframe_time = std::numeric_limits<double>::max();
        double max_keyframe_time = std::numeric_limits<double>::lowest();
        double min_keyframe_value = std::numeric_limits<double>::max();
        double max_keyframe_value = std::numeric_limits<double>::lowest();
        for (const auto& channel : animation->channels()) {
            for (const auto& keyframe : channel->keyframes()) {
                min_keyframe_time = std::min(min_keyframe_time, keyframe.time());
                max_keyframe_time = std::max(max_keyframe_time, keyframe.time());
                min_keyframe_value = std::min(min_keyframe_value, keyframe.value());
                max_keyframe_value = std::max(max_keyframe_value, keyframe.value());
            }
        }
        output->channels[0][0] = static_cast<float>(animation->size());
        output->channels[1][0] = static_cast<float>(min_keyframe_time);
        output->channels[2][0] = static_cast<float>(max_keyframe_time);
        output->channels[3][0] = static_cast<float>(min_keyframe_value);
        output->channels[4][0] = static_cast<float>(max_keyframe_value);
        break;
    } default:
        m_error = "Invalid select mode specified.";
        return;
    }
}



void
AnimationViewCHOP::getWarningString(OP_String* warning, void* reserved1)
{
    warning->setString(m_warning);
}

void
AnimationViewCHOP::getErrorString(OP_String* error, void* reserved1)
{
    error->setString(m_error);
}

void
AnimationViewCHOP::setupParameters(OP_ParameterManager* manager, void* reserved1)
{
    {
        OP_StringParameter sp;
        sp.name = "Datasource";
        sp.label = "Animation / Animation View CHOP";
        sp.defaultValue = "";
        manager->appendCHOP(sp);
    } {
        OP_StringParameter	sp;
		sp.name = "Viewmode";
        sp.label = "View Mode";
        sp.defaultValue = "samples";
        const char *names[] = { "samples", 
            "keyframes", "segments", "channel", "animation" };
        const char *labels[] = { "Samples View (range)", 
            "Keyframe View", "Segments View", "Channels View", "Animation View" };

        OP_ParAppendResult res = manager->appendMenu(sp, 5, names, labels);
        assert(res == OP_ParAppendResult::Success);
    } {
		OP_NumericParameter	np;
		np.name = "Range";
		np.label = "Range";
		np.defaultValues[0] = 0.0;
        // np.minValues[0] = 0.0;
        // np.clampMins[0] = true;
        np.defaultValues[1] = 30.0;
		
		OP_ParAppendResult res = manager->appendFloat(np, 2);
		assert(res == OP_ParAppendResult::Success);
	} {
        OP_StringParameter	sp;
		sp.name = "Rangeunit";
        sp.label = "Range Unit";
        sp.defaultValue = "seconds";
        const char *names[] = { "seconds", "samples" };
        const char *labels[] = { "Seconds", "Samples" };

        OP_ParAppendResult res = manager->appendMenu(sp, 2, names, labels);
        assert(res == OP_ParAppendResult::Success);
    } {
		OP_NumericParameter	np;
		np.name = "Samplerate";
		np.label = "Sample Rate";
		np.defaultValues[0] = 60.0;
		np.minSliders[0] = 120.0;
        np.minValues[0] = 1.0;
		np.maxSliders[0] =  30.0;
        np.clampMins[0] = true;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
    

}

AnimationCHOP*
AnimationViewCHOP::animationCHOP()
{
    return m_animationCHOP;
}

AnimationViewCHOP*
AnimationViewCHOP::dataInstance()
{
    return m_dataInstance;
}

const AnimationViewCHOP*
AnimationViewCHOP::dataInstance() const
{
    return m_dataInstance;
}

bool
AnimationViewCHOP::setDataInstance(const OP_Inputs *inputs)
{
    auto target = inputs->getParCHOP("Datasource");
    if (!target) {
        m_warning = "No AnimationCHOP or AnimationViewCHOP specified.";
        m_animationCHOP = nullptr;
        m_dataInstance = nullptr;
        return false;
    }
    
    if (target->customOP && strcmp(target->customOP->opType, "Animation") == 0) {
        m_animationCHOP = static_cast<AnimationCHOP*>(target->customOP->instance);
        if (!m_animationCHOP) {
            m_error = "Invalid Animation CHOP specified.";
            m_animationCHOP = nullptr;
            m_dataInstance = nullptr;
            return false;
        }
        m_dataInstance = this;
        return true;
    } else if (target->customOP && strcmp(target->customOP->opType, "Animationview") == 0) {
        auto chainedInstance = static_cast<AnimationViewCHOP*>(target->customOP->instance);
        m_dataInstance = chainedInstance->dataInstance();
        if (!m_dataInstance) {
            m_error = "Invalid AnimationView CHOP specified.";
            m_animationCHOP = nullptr;
            m_dataInstance = nullptr;
            return false;
        }
        m_animationCHOP = chainedInstance->animationCHOP();
        return true;
    } else {
        m_error = "Invalid Animation CHOP specified.";
        m_animationCHOP = nullptr;
        m_dataInstance = nullptr;
        return false;
    }
}

bool AnimationViewCHOP::displayStartHandle(size_t channel_index, const Keyframe& start_keyframe) const
{
    auto inst = dataInstance();
    if (!inst) {
        return false;
    }
    return inst->m_displayedChannels[channel_index]
        && start_keyframe.function == Function::bezier 
        && static_cast<uint8_t>(start_keyframe.handle_mode) > static_cast<uint8_t>(HandleMode::smooth);
}

bool AnimationViewCHOP::displayEndHandle(bool display_start_handle, const Keyframe& end_keyframe) const
{
    auto inst = dataInstance();
    if (!inst) {
        return false;
    }
    return display_start_handle
        && (static_cast<uint8_t>(end_keyframe.handle_mode) > static_cast<uint8_t>(HandleMode::smooth)
        || end_keyframe.function != Function::bezier);
}

// Helper function to convert Python list to vector of indices
std::vector<size_t> pyListToIndices(PyObject* list) {
    std::vector<size_t> indices;
    if (!PyList_Check(list)) {
        return indices;
    }
    
    Py_ssize_t size = PyList_Size(list);
    indices.reserve(size);
    
    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject* item = PyList_GetItem(list, i);
        if (PyLong_Check(item)) {
            long idx = PyLong_AsLong(item);
            if (idx >= 0) {
                indices.push_back(static_cast<size_t>(idx));
            }
        }
    }
    return indices;
}

// Helper function to convert vector of indices to Python list
PyObject* indicesToPyList(const std::vector<size_t>& indices) {
    PyObject* list = PyList_New(indices.size());
    if (!list) return nullptr;
    
    for (size_t i = 0; i < indices.size(); ++i) {
        PyObject* idx = PyLong_FromSize_t(indices[i]);
        if (!idx) {
            Py_DECREF(list);
            return nullptr;
        }
        PyList_SET_ITEM(list, i, idx);
    }
    return list;
}

// Helper function to convert vector of pairs to Python list of lists
PyObject* pairsToPyList(const std::vector<std::pair<size_t, size_t>>& pairs) {
    PyObject* list = PyList_New(pairs.size());
    if (!list) return nullptr;
    
    for (size_t i = 0; i < pairs.size(); ++i) {
        PyObject* sublist = PyList_New(2);
        if (!sublist) {
            Py_DECREF(list);
            return nullptr;
        }
        
        PyObject* channel_idx = PyLong_FromSize_t(pairs[i].first);
        PyObject* item_idx = PyLong_FromSize_t(pairs[i].second);
        
        if (!channel_idx || !item_idx) {
            Py_XDECREF(channel_idx);
            Py_XDECREF(item_idx);
            Py_DECREF(sublist);
            Py_DECREF(list);
            return nullptr;
        }
        
        PyList_SET_ITEM(sublist, 0, channel_idx);
        PyList_SET_ITEM(sublist, 1, item_idx);
        PyList_SET_ITEM(list, i, sublist);
    }
    return list;
}

// AnimationViewCHOP selection methods implementation
void AnimationViewCHOP::selectKeyframes(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->m_selectedKeyframes.size()) {
            inst->m_selectedKeyframes[idx] = true;
        }
    }
}

void AnimationViewCHOP::unselectKeyframes(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->m_selectedKeyframes.size()) {
            inst->m_selectedKeyframes[idx] = false;
        }
    }
}

void AnimationViewCHOP::unselectAllKeyframes() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    std::fill(inst->m_selectedKeyframes.begin(), inst->m_selectedKeyframes.end(), false);
}

std::vector<size_t> AnimationViewCHOP::getSelectedKeyframes() const {
    std::vector<size_t> selected;
    auto inst = dataInstance();
    if (!inst) {
        return selected;
    }
    for (size_t i = 0; i < inst->m_selectedKeyframes.size(); ++i) {
        if (inst->m_selectedKeyframes[i]) {
            selected.push_back(i);
        }
    }
    return selected;
}

void AnimationViewCHOP::selectSegments(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->m_selectedSegments.size()) {
            inst->m_selectedSegments[idx] = true;
        }
    }
}

void AnimationViewCHOP::unselectSegments(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->m_selectedSegments.size()) {
            inst->m_selectedSegments[idx] = false;
        }
    }
}

void AnimationViewCHOP::unselectAllSegments() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    std::fill(inst->m_selectedSegments.begin(), inst->m_selectedSegments.end(), false);
}

std::vector<size_t> AnimationViewCHOP::getSelectedSegments() const {
    std::vector<size_t> selected;
    auto inst = dataInstance();
    if (!inst) {
        return selected;
    }
    for (size_t i = 0; i < inst->m_selectedSegments.size(); ++i) {
        if (inst->m_selectedSegments[i]) {
            selected.push_back(i);
        }
    }
    return selected;
}

void AnimationViewCHOP::selectStartHandles(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->m_selectedStartHandles.size()) {
            inst->m_selectedStartHandles[idx] = true;
        }
    }
}

void AnimationViewCHOP::unselectStartHandles(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->m_selectedStartHandles.size()) {
            inst->m_selectedStartHandles[idx] = false;
        }
    }
}

void AnimationViewCHOP::unselectAllStartHandles() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    std::fill(inst->m_selectedStartHandles.begin(), inst->m_selectedStartHandles.end(), false);
}

std::vector<size_t> AnimationViewCHOP::getSelectedStartHandles() const {
    std::vector<size_t> selected;
    auto inst = dataInstance();
    if (!inst) {
        return selected;
    }
    for (size_t i = 0; i < inst->m_selectedStartHandles.size(); ++i) {
        if (inst->m_selectedStartHandles[i]) {
            selected.push_back(i);
        }
    }
    return selected;
}

void AnimationViewCHOP::selectEndHandles(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->m_selectedEndHandles.size()) {
            inst->m_selectedEndHandles[idx] = true;
        }
    }
}

void AnimationViewCHOP::unselectEndHandles(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->m_selectedEndHandles.size()) {
            inst->m_selectedEndHandles[idx] = false;
        }
    }
}

void AnimationViewCHOP::unselectAllEndHandles() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    std::fill(inst->m_selectedEndHandles.begin(), inst->m_selectedEndHandles.end(), false);
}

std::vector<size_t> AnimationViewCHOP::getSelectedEndHandles() const {
    std::vector<size_t> selected;
    auto inst = dataInstance();
    if (!inst) {
        return selected;
    }
    for (size_t i = 0; i < inst->m_selectedEndHandles.size(); ++i) {
        if (inst->m_selectedEndHandles[i]) {
            selected.push_back(i);
        }
    }
    return selected;
}

void AnimationViewCHOP::selectChannels(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->m_selectedChannels.size()) {
            inst->m_selectedChannels[idx] = true;
        }
    }
}

void AnimationViewCHOP::unselectChannels(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->m_selectedChannels.size()) {
            inst->m_selectedChannels[idx] = false;
        }
    }
}

void AnimationViewCHOP::unselectAllChannels() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    std::fill(inst->m_selectedChannels.begin(), inst->m_selectedChannels.end(), false);
}

std::vector<size_t> AnimationViewCHOP::getSelectedChannels() const {
    std::vector<size_t> selected;
    auto inst = dataInstance();
    if (!inst) {
        return selected;
    }
    for (size_t i = 0; i < inst->m_selectedChannels.size(); ++i) {
        if (inst->m_selectedChannels[i]) {
            selected.push_back(i);
        }
    }
    return selected;
}

void AnimationViewCHOP::setChannelDisplay(size_t index, bool display)
{
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    if (index < inst->m_displayedChannels.size()) {
        inst->m_displayedChannels[index] = display;
    }
}

// Python binding implementations
static PyObject* py_select_keyframes(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    // Get data instance for actual data manipulation
    auto dataInstance = inst->dataInstance();
    if (!dataInstance) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get data instance");
        return NULL;
    }

    PyObject* list;
    if (!PyArg_ParseTuple(args, "O", &list)) {
        return NULL;
    }

    std::vector<size_t> indices = pyListToIndices(list);
    dataInstance->selectKeyframes(indices);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_unselect_keyframes(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    // Get data instance for actual data manipulation
    auto dataInstance = inst->dataInstance();
    if (!dataInstance) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get data instance");
        return NULL;
    }

    PyObject* list;
    if (!PyArg_ParseTuple(args, "O", &list)) {
        return NULL;
    }

    std::vector<size_t> indices = pyListToIndices(list);
    dataInstance->unselectKeyframes(indices);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_unselect_all_keyframes(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    auto dataInstance = inst->dataInstance();
    if (!dataInstance) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get data instance");
        return NULL;
    }

    dataInstance->unselectAllKeyframes();
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_selected_keyframes(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    auto dataInstance = inst->dataInstance();
    if (!dataInstance) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get data instance");
        return NULL;
    }

    std::vector<size_t> selected = dataInstance->getSelectedKeyframes();
    return indicesToPyList(selected);
}

static PyObject* py_select_segments(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    PyObject* list;
    if (!PyArg_ParseTuple(args, "O", &list)) {
        return NULL;
    }

    std::vector<size_t> indices = pyListToIndices(list);
    inst->selectSegments(indices);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_unselect_segments(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    PyObject* list;
    if (!PyArg_ParseTuple(args, "O", &list)) {
        return NULL;
    }

    std::vector<size_t> indices = pyListToIndices(list);
    inst->unselectSegments(indices);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_unselect_all_segments(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    inst->unselectAllSegments();
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_selected_segments(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    std::vector<size_t> selected = inst->getSelectedSegments();
    return indicesToPyList(selected);
}

static PyObject* py_select_start_handles(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    PyObject* list;
    if (!PyArg_ParseTuple(args, "O", &list)) {
        return NULL;
    }

    std::vector<size_t> indices = pyListToIndices(list);
    inst->selectStartHandles(indices);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_unselect_start_handles(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    PyObject* list;
    if (!PyArg_ParseTuple(args, "O", &list)) {
        return NULL;
    }

    std::vector<size_t> indices = pyListToIndices(list);
    inst->unselectStartHandles(indices);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_unselect_all_start_handles(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    inst->unselectAllStartHandles();
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_selected_start_handles(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    std::vector<size_t> selected = inst->getSelectedStartHandles();
    return indicesToPyList(selected);
}

static PyObject* py_select_end_handles(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    PyObject* list;
    if (!PyArg_ParseTuple(args, "O", &list)) {
        return NULL;
    }

    std::vector<size_t> indices = pyListToIndices(list);
    inst->selectEndHandles(indices);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_unselect_end_handles(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    PyObject* list;
    if (!PyArg_ParseTuple(args, "O", &list)) {
        return NULL;
    }

    std::vector<size_t> indices = pyListToIndices(list);
    inst->unselectEndHandles(indices);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_unselect_all_end_handles(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    inst->unselectAllEndHandles();
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_selected_end_handles(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    std::vector<size_t> selected = inst->getSelectedEndHandles();
    return indicesToPyList(selected);
}

static PyObject* py_select_channels(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    PyObject* list;
    if (!PyArg_ParseTuple(args, "O", &list)) {
        return NULL;
    }

    std::vector<size_t> indices = pyListToIndices(list);
    inst->selectChannels(indices);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_unselect_channels(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    PyObject* list;
    if (!PyArg_ParseTuple(args, "O", &list)) {
        return NULL;
    }

    std::vector<size_t> indices = pyListToIndices(list);
    inst->unselectChannels(indices);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_unselect_all_channels(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    inst->unselectAllChannels();
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_selected_channels(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    std::vector<size_t> selected = inst->getSelectedChannels();
    return indicesToPyList(selected);
}

static PyObject* py_set_channel_display(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    Py_ssize_t index_ssize;
    int display_int;
    if (!PyArg_ParseTuple(args, "np", &index_ssize, &display_int)) {
        return NULL;
    }
    if (index_ssize < 0) {
        PyErr_SetString(PyExc_ValueError, "Channel index must be non-negative");
        return NULL;
    }
    
    size_t index = static_cast<size_t>(index_ssize);
    bool display = (display_int != 0);
    inst->setChannelDisplay(index, display);

    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}