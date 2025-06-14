#include "animation_view_chop.h"
#include "animation_chop.h"

#include <cstring>
#include <algorithm>
#include <iostream>


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
static PyObject* py_offset_selected_keyframes(PyObject* self, PyObject* args);
static PyObject* py_offset_selected_keyframes_by_time(PyObject* self, PyObject* args);
static PyObject* py_offset_selected_keyframes_by_value(PyObject* self, PyObject* args);

static PyObject* py_select_segments(PyObject* self, PyObject* args);
static PyObject* py_unselect_segments(PyObject* self, PyObject* args);
static PyObject* py_unselect_all_segments(PyObject* self, PyObject* args);
static PyObject* py_selected_segments(PyObject* self, PyObject* args);
static PyObject* py_offset_selected_segments(PyObject* self, PyObject* args);
static PyObject* py_offset_selected_segments_by_time(PyObject* self, PyObject* args);
static PyObject* py_offset_selected_segments_by_value(PyObject* self, PyObject* args);

static PyObject* py_select_start_handles(PyObject* self, PyObject* args);
static PyObject* py_unselect_start_handles(PyObject* self, PyObject* args);
static PyObject* py_unselect_all_start_handles(PyObject* self, PyObject* args);
static PyObject* py_selected_start_handles(PyObject* self, PyObject* args);
static PyObject* py_offset_selected_start_handles(PyObject* self, PyObject* args);
static PyObject* py_offset_selected_start_handles_by_time(PyObject* self, PyObject* args);
static PyObject* py_offset_selected_start_handles_by_value(PyObject* self, PyObject* args);

static PyObject* py_select_end_handles(PyObject* self, PyObject* args);
static PyObject* py_unselect_end_handles(PyObject* self, PyObject* args);
static PyObject* py_unselect_all_end_handles(PyObject* self, PyObject* args);
static PyObject* py_selected_end_handles(PyObject* self, PyObject* args);
static PyObject* py_offset_selected_end_handles(PyObject* self, PyObject* args);
static PyObject* py_offset_selected_end_handles_by_time(PyObject* self, PyObject* args);
static PyObject* py_offset_selected_end_handles_by_value(PyObject* self, PyObject* args);

static PyObject* py_select_channels(PyObject* self, PyObject* args);
static PyObject* py_unselect_channels(PyObject* self, PyObject* args);
static PyObject* py_unselect_all_channels(PyObject* self, PyObject* args);
static PyObject* py_selected_channels(PyObject* self, PyObject* args);
static PyObject* py_set_channel_display(PyObject* self, PyObject* args);

static PyObject* py_reset_begin_set_values(PyObject* self, PyObject* args);
static PyObject* py_unselect_all(PyObject* self, PyObject* args);

// Undo/Redo Python bindings
static PyObject* py_undo(PyObject* self, PyObject* args);
static PyObject* py_redo(PyObject* self, PyObject* args);
static PyObject* py_reset_undo(PyObject* self, PyObject* args);
static PyObject* py_cache_state(PyObject* self, PyObject* args);
static PyObject* py_set_undo(PyObject* self, PyObject* args);

// Python method table for AnimationViewCHOP
static PyMethodDef viewMethods[] = {
    {"select_keyframes", (PyCFunction)py_select_keyframes, METH_VARARGS, "Select keyframes by indices."},
    {"unselect_keyframes", (PyCFunction)py_unselect_keyframes, METH_VARARGS, "Unselect keyframes by indices."},
    {"unselect_all_keyframes", (PyCFunction)py_unselect_all_keyframes, METH_NOARGS, "Unselect all keyframes."},
    {"selected_keyframes", (PyCFunction)py_selected_keyframes, METH_NOARGS, "Get selected keyframe indices."},
    {"offset_selected_keyframes", (PyCFunction)py_offset_selected_keyframes, METH_VARARGS, "Offset selected keyframes by time and value offsets."},
    {"offset_selected_keyframes_by_time", (PyCFunction)py_offset_selected_keyframes_by_time, METH_VARARGS, "Offset selected keyframes by time offset."},
    {"offset_selected_keyframes_by_value", (PyCFunction)py_offset_selected_keyframes_by_value, METH_VARARGS, "Offset selected keyframes by value offset."},

    {"select_segments", (PyCFunction)py_select_segments, METH_VARARGS, "Select segments by indices."},
    {"unselect_segments", (PyCFunction)py_unselect_segments, METH_VARARGS, "Unselect segments by indices."},
    {"unselect_all_segments", (PyCFunction)py_unselect_all_segments, METH_NOARGS, "Unselect all segments."},
    {"selected_segments", (PyCFunction)py_selected_segments, METH_NOARGS, "Get selected segment indices."},
    {"offset_selected_segments", (PyCFunction)py_offset_selected_segments, METH_VARARGS, "Offset selected segments by time and value offsets."},
    {"offset_selected_segments_by_time", (PyCFunction)py_offset_selected_segments_by_time, METH_VARARGS, "Offset selected segments by time offset."},
    {"offset_selected_segments_by_value", (PyCFunction)py_offset_selected_segments_by_value, METH_VARARGS, "Offset selected segments by value offset."},

    {"select_start_handles", (PyCFunction)py_select_start_handles, METH_VARARGS, "Select start handles by indices."},
    {"unselect_start_handles", (PyCFunction)py_unselect_start_handles, METH_VARARGS, "Unselect start handles by indices."},
    {"unselect_all_start_handles", (PyCFunction)py_unselect_all_start_handles, METH_NOARGS, "Unselect all start handles."},
    {"selected_start_handles", (PyCFunction)py_selected_start_handles, METH_NOARGS, "Get selected start handle indices."},
    {"offset_selected_start_handles", (PyCFunction)py_offset_selected_start_handles, METH_VARARGS, "Offset selected start handles by time and value offsets."},
    {"offset_selected_start_handles_by_time", (PyCFunction)py_offset_selected_start_handles_by_time, METH_VARARGS, "Offset selected start handles by time offset."},
    {"offset_selected_start_handles_by_value", (PyCFunction)py_offset_selected_start_handles_by_value, METH_VARARGS, "Offset selected start handles by value offset."},

    {"select_end_handles", (PyCFunction)py_select_end_handles, METH_VARARGS, "Select end handles by indices."},
    {"unselect_end_handles", (PyCFunction)py_unselect_end_handles, METH_VARARGS, "Unselect end handles by indices."},
    {"unselect_all_end_handles", (PyCFunction)py_unselect_all_end_handles, METH_NOARGS, "Unselect all end handles."},
    {"selected_end_handles", (PyCFunction)py_selected_end_handles, METH_NOARGS, "Get selected end handle indices."},
    {"offset_selected_end_handles", (PyCFunction)py_offset_selected_end_handles, METH_VARARGS, "Offset selected end handles by time and value offsets."},
    {"offset_selected_end_handles_by_time", (PyCFunction)py_offset_selected_end_handles_by_time, METH_VARARGS, "Offset selected end handles by time offset."},
    {"offset_selected_end_handles_by_value", (PyCFunction)py_offset_selected_end_handles_by_value, METH_VARARGS, "Offset selected end handles by value offset."},

    {"select_channels", (PyCFunction)py_select_channels, METH_VARARGS, "Select channels by indices."},
    {"unselect_channels", (PyCFunction)py_unselect_channels, METH_VARARGS, "Unselect channels by indices."},
    {"unselect_all_channels", (PyCFunction)py_unselect_all_channels, METH_NOARGS, "Unselect all channels."},
    {"selected_channels", (PyCFunction)py_selected_channels, METH_NOARGS, "Get selected channel indices."},
    {"set_channel_display", (PyCFunction)py_set_channel_display, METH_VARARGS, "Set channel display state by indices."},

    
    {"reset_begin_set_values", (PyCFunction)py_reset_begin_set_values, METH_NOARGS, "Reset begin set values for all selected items."},
    {"unselect_all", (PyCFunction)py_unselect_all, METH_NOARGS, "Unselect all selected items."},

    {"undo", (PyCFunction)py_undo, METH_NOARGS, "Undo the last operation."},
    {"redo", (PyCFunction)py_redo, METH_NOARGS, "Redo the next operation."},
    {"reset_undo", (PyCFunction)py_reset_undo, METH_NOARGS, "Reset the undo stack."},
    {"cache_state", (PyCFunction)py_cache_state, METH_NOARGS, "Cache the current state for undo/redo."},
    {"set_undo", (PyCFunction)py_set_undo, METH_NOARGS, "Set an undo point."},

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
        
        // Initialize keyframe views with proper indices
        m_keyframeViews.resize(total_keyframes, {0, 0, false, anim::Point()});
        size_t keyframe_index = 0;
        for (size_t c = 0; c < animation->size(); ++c) {
            auto& channel = animation->channel(c);
            for (size_t k = 0; k < channel.size(); ++k) {
                m_keyframeViews[keyframe_index].channel_index = c;
                m_keyframeViews[keyframe_index].keyframe_index = k;
                ++keyframe_index;
            }
        }
        
        m_segmentViews.resize(total_segments, {0, 0, false, anim::Point(), anim::Point(), false, anim::Point(), false, anim::Point()});
        size_t segment_index = 0;
        for (size_t c = 0; c < animation->size(); ++c) {
            auto& channel = animation->channel(c);
			if (channel.size() < 2) {
				continue; // Skip channels with less than 2 keyframes
			}
            for (size_t k = 0; k < channel.size() - 1; ++k) {
                m_segmentViews[segment_index].channel_index = c;
                m_segmentViews[segment_index].keyframe_index = k;
                ++segment_index;
            }
        }
        m_channelViews.resize(animation->size(), {false, true});
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
        info->numSamples = static_cast<int32_t>(dataInstance()->m_keyframeViews.size());
        return true;
    } case ViewMode::segments: {
        info->numChannels = static_cast<int32_t>(m_segments_chan_names.size());
        info->numSamples = static_cast<int32_t>(dataInstance()->m_segmentViews.size());
        return true;
    } case ViewMode::channels: {
        info->numChannels = static_cast<int32_t>(m_channels_chan_names.size());
        info->numSamples = static_cast<int32_t>(dataInstance()->m_channelViews.size());
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
                    output->channels[10][i] = static_cast<float>(dataInstance()->m_keyframeViews[i].selected);
                    output->channels[11][i] = static_cast<float>(dataInstance()->m_channelViews[c].displayed);
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
                output->channels[12][i] = static_cast<float>(dataInstance()->m_segmentViews[i].selected);
                output->channels[13][i] = static_cast<float>(dataInstance()->m_segmentViews[i].start_handle_selected);
                output->channels[14][i] = static_cast<float>(dataInstance()->m_segmentViews[i].end_handle_selected);
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
            if (c < output->numSamples) {
                auto num_keyframes = channel.size();
                output->channels[0][c] = static_cast<float>(num_keyframes);
                output->channels[1][c] = static_cast<float>(channel.start_time());
                output->channels[2][c] = static_cast<float>(channel.end_time());
                output->channels[3][c] = static_cast<float>(start_index);
                start_index += static_cast<int32_t>(num_keyframes);
                output->channels[4][c] = static_cast<float>(dataInstance()->m_channelViews[c].selected);
                output->channels[5][c] = static_cast<float>(dataInstance()->m_channelViews[c].displayed);
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
    return inst->m_channelViews[channel_index].displayed
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

// Helper function to convert KeyframeView to Python dict
PyObject* keyframeViewToPyDict(const KeyframeView& kv) {
    PyObject* dict = PyDict_New();
    if (!dict) return nullptr;
    
    PyDict_SetItemString(dict, "channel_index", PyLong_FromSize_t(kv.channel_index));
    PyDict_SetItemString(dict, "keyframe_index", PyLong_FromSize_t(kv.keyframe_index));
    PyDict_SetItemString(dict, "selected", PyBool_FromLong(kv.selected));
    PyDict_SetItemString(dict, "begin_set_time", PyFloat_FromDouble(kv.begin_set_position.time));
    PyDict_SetItemString(dict, "begin_set_value", PyFloat_FromDouble(kv.begin_set_position.value));

    return dict;
}

// AnimationViewCHOP selection methods implementation
void AnimationViewCHOP::selectKeyframes(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }
    
    for (size_t idx : indices) {
        if (idx < inst->keyframeViews().size()) {
            auto& kv = inst->m_keyframeViews[idx];
            kv.selected = true;
            
            // Set begin_set values using the stored indices
            auto& channel = animation->channel(kv.channel_index);
            auto& keyframe = channel.keyframe(kv.keyframe_index);
            kv.begin_set_position = keyframe.position;
            // std::cout << "Selected keyframe sample index: " << idx
                    //   << ", at channel " << kv.channel_index
                    //   << ", index " << kv.keyframe_index
                    //   << ", time " << kv.begin_set_time
                    //   << ", value " << kv.begin_set_value << std::endl;
        }
    }
}

void AnimationViewCHOP::unselectKeyframes(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->keyframeViews().size()) {
            inst->m_keyframeViews[idx].selected = false;
        }
    }
}

void AnimationViewCHOP::unselectAllKeyframes() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (auto& kv : inst->m_keyframeViews) {
        kv.selected = false;
    }
}

void AnimationViewCHOP::offsetSelectedKeyframes(double time_offset, double value_offset) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }
    
    for (auto& kv : inst->m_keyframeViews) {
        if (kv.selected) {
            auto& channel = animation->channel(kv.channel_index);

            channel.set_keyframe_position(kv.keyframe_index, 
                kv.begin_set_position + anim::Point(time_offset, value_offset)
            );
        }
    }
}

void AnimationViewCHOP::offsetSelectedKeyframesByTime(double time_offset) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }
    
    for (auto& kv : inst->m_keyframeViews) {
        if (kv.selected) {
            auto& channel = animation->channel(kv.channel_index);
            channel.set_keyframe_time(kv.keyframe_index, 
                kv.begin_set_position.time + time_offset
            );
        }
    }
}

void AnimationViewCHOP::offsetSelectedKeyframesByValue(double value_offset) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }
    
    for (auto& kv : inst->m_keyframeViews) {
        if (kv.selected) {
            auto& channel = animation->channel(kv.channel_index);
            channel.set_keyframe_value(kv.keyframe_index, 
                kv.begin_set_position.value + value_offset
            );
        }
    }
}

void AnimationViewCHOP::selectSegments(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }

    for (size_t idx : indices) {
        if (idx < inst->segmentViews().size()) {
            auto& sv = inst->m_segmentViews[idx];
            sv.selected = true;
            auto& channel = animation->channel(sv.channel_index);
            auto& start_kf = channel.keyframe(sv.keyframe_index);
            sv.begin_set_start_position = start_kf.position;
            auto& end_kf = channel.keyframe(sv.keyframe_index + 1);
            sv.begin_set_end_position = end_kf.position;
        }
    }
}

void AnimationViewCHOP::unselectSegments(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->segmentViews().size()) {
            inst->m_segmentViews[idx].selected = false;
        }
    }
}

void AnimationViewCHOP::unselectAllSegments() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (auto& sv : inst->m_segmentViews) {
        sv.selected = false;
    }
}

// Helper function to convert SegmentView to Python dict
PyObject* segmentViewToPyDict(const SegmentView& sv) {
    PyObject* dict = PyDict_New();
    if (!dict) return nullptr;

    PyDict_SetItemString(dict, "channel_index", PyLong_FromSize_t(sv.channel_index));
    PyDict_SetItemString(dict, "keyframe_index", PyLong_FromSize_t(sv.keyframe_index));
    PyDict_SetItemString(dict, "selected", PyBool_FromLong(sv.selected));

    PyDict_SetItemString(dict, "begin_set_start_time", PyFloat_FromDouble(sv.begin_set_start_position.time));
    PyDict_SetItemString(dict, "begin_set_start_value", PyFloat_FromDouble(sv.begin_set_start_position.value));

    PyDict_SetItemString(dict, "begin_set_end_time", PyFloat_FromDouble(sv.begin_set_end_position.time));
    PyDict_SetItemString(dict, "begin_set_end_value", PyFloat_FromDouble(sv.begin_set_end_position.value));

    PyDict_SetItemString(dict, "start_handle_selected", PyBool_FromLong(sv.start_handle_selected));
    PyDict_SetItemString(dict, "begin_set_start_handle_time", PyFloat_FromDouble(sv.begin_set_start_handle.time));
    PyDict_SetItemString(dict, "begin_set_start_handle_value", PyFloat_FromDouble(sv.begin_set_start_handle.value));

    PyDict_SetItemString(dict, "end_handle_selected", PyBool_FromLong(sv.end_handle_selected));
    PyDict_SetItemString(dict, "begin_set_end_handle_time", PyFloat_FromDouble(sv.begin_set_end_handle.time));
    PyDict_SetItemString(dict, "begin_set_end_handle_value", PyFloat_FromDouble(sv.begin_set_end_handle.value));

    return dict;
}

void AnimationViewCHOP::offsetSelectedSegments(double time_offset, double value_offset)
{
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }
    
    for (auto& sv : inst->m_segmentViews) {
        if (sv.selected) {
            auto& channel = animation->channel(sv.channel_index);
            
            channel.set_keyframe_position(sv.keyframe_index, 
                sv.begin_set_start_position + anim::Point(time_offset, value_offset)
            );
            
            channel.set_keyframe_position(sv.keyframe_index + 1, 
                sv.begin_set_end_position + anim::Point(time_offset, value_offset)
            );
        }
    }
}

void AnimationViewCHOP::offsetSelectedSegmentsByTime(double time_offset) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }

    for (auto& sv : inst->m_segmentViews) {
        if (sv.selected) {
            auto& channel = animation->channel(sv.channel_index);

            channel.set_keyframe_position(sv.keyframe_index,
                sv.begin_set_start_position + anim::Point(time_offset, 0)
            );

            channel.set_keyframe_position(sv.keyframe_index + 1,
                sv.begin_set_end_position + anim::Point(time_offset, 0)
            );
        }
    }
}

void AnimationViewCHOP::offsetSelectedSegmentsByValue(double value_offset) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }

    for (auto& sv : inst->m_segmentViews) {
        if (sv.selected) {
            auto& channel = animation->channel(sv.channel_index);

            channel.set_keyframe_position(sv.keyframe_index,
                sv.begin_set_start_position + anim::Point(0, value_offset)
            );

            channel.set_keyframe_position(sv.keyframe_index + 1,
                sv.begin_set_end_position + anim::Point(0, value_offset)
            );
        }
    }
}

void AnimationViewCHOP::selectStartHandles(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }

    for (size_t idx : indices) {
        if (idx < inst->segmentViews().size()) {
            auto& sv = inst->m_segmentViews[idx];
            sv.start_handle_selected = true;
            auto& channel = animationCHOP()->animation()->channel(sv.channel_index);
            auto& start_keyframe = channel.keyframe(sv.keyframe_index);
            sv.begin_set_start_handle = start_keyframe.out_handle;
        }
    }
}

void AnimationViewCHOP::unselectStartHandles(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->segmentViews().size()) {
            inst->m_segmentViews[idx].start_handle_selected = false;
        }
    }
}

void AnimationViewCHOP::unselectAllStartHandles() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (auto& sv : inst->m_segmentViews) {
        sv.start_handle_selected = false;
    }
}

void AnimationViewCHOP::offsetSelectedStartHandles(double time_offset, double value_offset)
{
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }
    
    for (auto& sv : inst->m_segmentViews) {
        if (sv.start_handle_selected) {
            auto& channel = animation->channel(sv.channel_index);
            channel.set_keyframe_out_handle(sv.keyframe_index, 
                sv.begin_set_start_handle + anim::Point(time_offset, value_offset)
            );  // Set the out handle position for the keyframe
        }
    }
}

void AnimationViewCHOP::offsetSelectedStartHandlesByTime(double time_offset) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }

    for (auto& sv : inst->m_segmentViews) {
        if (sv.start_handle_selected) {
            auto& channel = animation->channel(sv.channel_index);
            channel.set_keyframe_out_handle(sv.keyframe_index,
                sv.begin_set_start_handle + anim::Point(time_offset, 0)
            );  // Set the out handle position for the keyframe
        }
    }
}

void AnimationViewCHOP::offsetSelectedStartHandlesByValue(double value_offset) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }

    for (auto& sv : inst->m_segmentViews) {
        if (sv.start_handle_selected) {
            auto& channel = animation->channel(sv.channel_index);
            channel.set_keyframe_out_handle(sv.keyframe_index,
                sv.begin_set_start_handle + anim::Point(0, value_offset)
            );  // Set the out handle position for the keyframe
        }
    }
}

void AnimationViewCHOP::selectEndHandles(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }

    for (size_t idx : indices) {
        if (idx < inst->segmentViews().size()) {
            auto& sv = inst->m_segmentViews[idx];
            sv.end_handle_selected = true;
            auto& channel = animation->channel(sv.channel_index);
            auto& end_keyframe = channel.keyframe(sv.keyframe_index + 1);
            sv.begin_set_end_handle = end_keyframe.in_handle;
        }
    }
}

void AnimationViewCHOP::unselectEndHandles(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->segmentViews().size()) {
            inst->m_segmentViews[idx].end_handle_selected = false;
        }
    }
}

void AnimationViewCHOP::unselectAllEndHandles() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (auto& sv : inst->m_segmentViews) {
        sv.end_handle_selected = false;
    }
}

void AnimationViewCHOP::offsetSelectedEndHandles(double time_offset, double value_offset)
{
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }
    
    for (auto& sv : inst->m_segmentViews) {
        if (sv.end_handle_selected) {
            auto& channel = animation->channel(sv.channel_index);
            channel.set_keyframe_in_handle(sv.keyframe_index + 1, 
                sv.begin_set_end_handle + anim::Point(time_offset, value_offset)
            );  // Set the in handle position for the next keyframe
        }
    }
}

void AnimationViewCHOP::offsetSelectedEndHandlesByTime(double time_offset) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }

    for (auto& sv : inst->m_segmentViews) {
        if (sv.end_handle_selected) {
            auto& channel = animation->channel(sv.channel_index);
            channel.set_keyframe_in_handle(sv.keyframe_index + 1,
                sv.begin_set_end_handle + anim::Point(time_offset, 0)
            );  // Set the in handle position for the next keyframe
        }
    }
}

void AnimationViewCHOP::offsetSelectedEndHandlesByValue(double value_offset) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }

    for (auto& sv : inst->m_segmentViews) {
        if (sv.end_handle_selected) {
            auto& channel = animation->channel(sv.channel_index);
            channel.set_keyframe_in_handle(sv.keyframe_index + 1,
                sv.begin_set_end_handle + anim::Point(0, value_offset)
            );  // Set the in handle position for the next keyframe
        }
    }
}

void AnimationViewCHOP::selectChannels(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->channelViews().size()) {
            inst->m_channelViews[idx].selected = true;
        }
    }
}

void AnimationViewCHOP::unselectChannels(const std::vector<size_t>& indices) {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (size_t idx : indices) {
        if (idx < inst->channelViews().size()) {
            inst->m_channelViews[idx].selected = false;
        }
    }
}

void AnimationViewCHOP::unselectAllChannels() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    for (auto& cv : inst->m_channelViews) {
        cv.selected = false;
    }
}

std::vector<size_t> AnimationViewCHOP::getSelectedChannels() const {
    std::vector<size_t> selected;
    auto inst = dataInstance();
    if (!inst) {
        return selected;
    }
    const auto& channelViews = inst->channelViews();
    for (size_t i = 0; i < channelViews.size(); ++i) {
        if (channelViews[i].selected) {
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
    if (index < inst->channelViews().size()) {
        inst->m_channelViews[index].displayed = display;
    }
}

void AnimationViewCHOP::resetBeginSetValues() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animation = animationCHOP()->animation();
    if (!animation) {
        return;
    }
    
    // Reset begin_set values for all selected keyframes
    for (auto& kv : inst->m_keyframeViews) {
        if (kv.selected) {
            auto& channel = animation->channel(kv.channel_index);
            auto& keyframe = channel.keyframe(kv.keyframe_index);
            kv.begin_set_position = keyframe.position;
        }
    }

    // Reset begin_set values for all selected segments
    for (auto& sv : inst->m_segmentViews) {
        auto& channel = animation->channel(sv.channel_index);
        auto& start_kf = channel.keyframe(sv.keyframe_index);
        auto& end_kf = channel.keyframe(sv.keyframe_index + 1);
        if (sv.selected) {
            sv.begin_set_start_position = start_kf.position;
            sv.begin_set_end_position = end_kf.position;
        }
        if (sv.start_handle_selected) {
            sv.begin_set_start_handle = start_kf.out_handle;
        }
        if (sv.end_handle_selected) {
            sv.begin_set_end_handle = end_kf.in_handle;
        }
    }
}

void AnimationViewCHOP::undo() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animationCHOP = inst->animationCHOP();
    if (!animationCHOP) {
        return;
    }
    
    // animationCHOP->newAnimation();

    if (inst->m_undoStack.empty()) {
        return; // Nothing to undo
    }

    auto lastUndo = std::move(inst->m_undoStack.back());
    inst->m_undoStack.pop_back();
    inst->m_redoStack.push_back(std::make_unique<anim::Animation>(
        animationCHOP->animation()->copy()
    ));
    animationCHOP->replaceAnimation(std::move(lastUndo));

    // std::cout << "AnimationViewCHOP: Undo items left on stack: " 
    //           << inst->m_undoStack.size() << std::endl;
}
void AnimationViewCHOP::redo() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    auto animationCHOP = inst->animationCHOP();
    if (!animationCHOP) {
        return;
    }
    
    if (inst->m_redoStack.empty()) {
        return; // Nothing to redo
    }

    auto lastRedo = std::move(inst->m_redoStack.back());
    inst->m_redoStack.pop_back();
    inst->m_undoStack.push_back(std::make_unique<anim::Animation>(
        animationCHOP->animation()->copy()
    ));
    animationCHOP->replaceAnimation(std::move(lastRedo));

    // std::cout << "AnimationViewCHOP: Redo items left on stack: " 
    //           << inst->m_redoStack.size() << std::endl;
}

void AnimationViewCHOP::resetUndo() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }
    
    inst->m_undoStack.clear();
    inst->m_redoStack.clear();
    inst->m_stateCache = nullptr;
}

void AnimationViewCHOP::cacheState() {
    auto inst = dataInstance();
    if (!inst) {
        return;
    }

    inst->m_stateCache = std::make_unique<anim::Animation>(inst->m_animationCHOP->animation()->copy());
    // std::cout << "AnimationViewCHOP: Cached current animation state for undo." << std::endl;
}

bool AnimationViewCHOP::setUndo() {
    auto inst = dataInstance();
    if (!inst) {
        return false;
    }
    auto animationCHOP = inst->animationCHOP();
    if (!animationCHOP || !animationCHOP->animation()) {
        return false;
    }
    auto& cache = *inst->m_stateCache.get();
    auto& currentAnimation = *animationCHOP->animation();

    if (cache == currentAnimation) {
        // No changes since last cache, nothing to do
        // std::cout << "AnimationViewCHOP: No changes detected, skipping undo." << std::endl;
        return false;
    }
    
    // Remove all redo entries (everything after current index)
    inst->m_redoStack.clear();
    // If we have a cached state, use it    
    // Add current animation state to undo stack
    if (inst->m_stateCache) {
        // If we have a cached state, use it
        inst->m_undoStack.push_back(std::move(inst->m_stateCache));
        inst->m_stateCache = nullptr; // Clear cache after using it
        // std::cout << "AnimationViewCHOP: Added state to undo stack, index: " 
        //           << inst->m_undoStack.size() - 1 << std::endl;
    }

    return true;
}

static PyObject* py_select_keyframes(PyObject* self, PyObject* args) {
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
    inst->selectKeyframes(indices);
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

    // Get selected keyframes and return as list of dicts
    const auto& keyframeViews = dataInstance->keyframeViews();
    std::vector<KeyframeView> selectedKeyframes;
    for (const auto& kv : keyframeViews) {
        if (kv.selected) {
            selectedKeyframes.push_back(kv);
        }
    }
    
    PyObject* list = PyList_New(selectedKeyframes.size());
    if (!list) return nullptr;
    
    for (size_t i = 0; i < selectedKeyframes.size(); ++i) {
        PyObject* dict = keyframeViewToPyDict(selectedKeyframes[i]);
        if (!dict) {
            Py_DECREF(list);
            return nullptr;
        }
        PyList_SET_ITEM(list, i, dict);
    }
    
    return list;
}

static PyObject* py_offset_selected_keyframes(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    double time_offset, value_offset;
    if (!PyArg_ParseTuple(args, "dd", &time_offset, &value_offset)) {
        return NULL;
    }

    inst->offsetSelectedKeyframes(time_offset, value_offset);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_offset_selected_keyframes_by_time(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    double time_offset;
    if (!PyArg_ParseTuple(args, "d", &time_offset)) {
        return NULL;
    }

    inst->offsetSelectedKeyframesByTime(time_offset);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_offset_selected_keyframes_by_value(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    double value_offset;
    if (!PyArg_ParseTuple(args, "d", &value_offset)) {
        return NULL;
    }

    inst->offsetSelectedKeyframesByValue(value_offset);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
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

    auto dataInstance = inst->dataInstance();
    if (!dataInstance) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get data instance");
        return NULL;
    }

    // Get selected segments and return as list of dicts
    const auto& segmentViews = dataInstance->segmentViews();
    std::vector<SegmentView> selectedSegments;
    for (const auto& sv : segmentViews) {
        if (sv.selected) {
            selectedSegments.push_back(sv);
        }
    }

    PyObject* result = PyList_New(selectedSegments.size());
    if (!result) return NULL;

    for (size_t i = 0; i < selectedSegments.size(); ++i) {
        PyObject* dict = segmentViewToPyDict(selectedSegments[i]);
        if (!dict) {
            Py_DECREF(result);
            return NULL;
        }
        PyList_SetItem(result, i, dict);
    }

    return result;
}

static PyObject* py_offset_selected_segments(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    double time_offset, value_offset;
    if (!PyArg_ParseTuple(args, "dd", &time_offset, &value_offset)) {
        return NULL;
    }

    inst->offsetSelectedSegments(time_offset, value_offset);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_offset_selected_segments_by_time(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    double time_offset;
    if (!PyArg_ParseTuple(args, "d", &time_offset)) {
        return NULL;
    }

    inst->offsetSelectedSegmentsByTime(time_offset);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_offset_selected_segments_by_value(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    double value_offset;
    if (!PyArg_ParseTuple(args, "d", &value_offset)) {
        return NULL;
    }

    inst->offsetSelectedSegmentsByValue(value_offset);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
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

    auto dataInstance = inst->dataInstance();
    if (!dataInstance) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get data instance");
        return NULL;
    }
    
    // Get selected start handles and return a list of dicts
    const auto& segmentViews = dataInstance->segmentViews();
    std::vector<SegmentView> selectedStartHandles;
    for (const auto& sv : segmentViews) {
        if (sv.start_handle_selected) {
            selectedStartHandles.push_back(sv);
        }
    }

    PyObject* result = PyList_New(selectedStartHandles.size());
    if (!result) return NULL;

    for (size_t i = 0; i < selectedStartHandles.size(); ++i) {
        PyObject* dict = segmentViewToPyDict(selectedStartHandles[i]);
        if (!dict) {
            Py_DECREF(result);
            return NULL;
        }
        PyList_SetItem(result, i, dict);
    }

    return result;
}

static PyObject* py_offset_selected_start_handles(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    double time_offset, value_offset;
    if (!PyArg_ParseTuple(args, "dd", &time_offset, &value_offset)) {
        return NULL;
    }

    inst->offsetSelectedStartHandles(time_offset, value_offset);
    me->context->makeNodeDirty();

    Py_RETURN_NONE;
}

static PyObject* py_offset_selected_start_handles_by_time(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    double time_offset;
    if (!PyArg_ParseTuple(args, "d", &time_offset)) {
        return NULL;
    }

    inst->offsetSelectedStartHandlesByTime(time_offset);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_offset_selected_start_handles_by_value(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    double value_offset;
    if (!PyArg_ParseTuple(args, "d", &value_offset)) {
        return NULL;
    }

    inst->offsetSelectedStartHandlesByValue(value_offset);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
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

    auto dataInstance = inst->dataInstance();
    if (!dataInstance) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get data instance");
        return NULL;
    }
    
    // Get selected end handles and return a list of dicts
    const auto& segmentViews = dataInstance->segmentViews();
    std::vector<SegmentView> selectedEndHandles;
    for ( const auto& sv : segmentViews) {
        if (sv.end_handle_selected) {
            selectedEndHandles.push_back(sv);
        }
    }

    PyObject* result = PyList_New(selectedEndHandles.size());
    if (!result) return NULL;

    for (size_t i = 0; i < selectedEndHandles.size(); ++i) {
        PyObject* dict = segmentViewToPyDict(selectedEndHandles[i]);
        if (!dict) {
            Py_DECREF(result);
            return NULL;
        }
        PyList_SetItem(result, i, dict);
    }

    return result;
}

static PyObject* py_offset_selected_end_handles(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    double time_offset, value_offset;
    if (!PyArg_ParseTuple(args, "dd", &time_offset, &value_offset)) {
        return NULL;
    }

    inst->offsetSelectedEndHandles(time_offset, value_offset);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_offset_selected_end_handles_by_time(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    double time_offset;
    if (!PyArg_ParseTuple(args, "d", &time_offset)) {
        return NULL;
    }

    inst->offsetSelectedEndHandlesByTime(time_offset);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_offset_selected_end_handles_by_value(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    double value_offset;
    if (!PyArg_ParseTuple(args, "d", &value_offset)) {
        return NULL;
    }

    inst->offsetSelectedEndHandlesByValue(value_offset);
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
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

static PyObject* py_reset_begin_set_values(PyObject* self, PyObject* args) {
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

    dataInstance->resetBeginSetValues();
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_unselect_all(PyObject* self, PyObject* args) {
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
    dataInstance->unselectAllSegments();
    dataInstance->unselectAllStartHandles();
    dataInstance->unselectAllEndHandles();
    dataInstance->unselectAllChannels();
    
    me->context->makeNodeDirty();
    
    Py_RETURN_NONE;
}

static PyObject* py_undo(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    inst->undo();
    me->context->makeNodeDirty();
    Py_RETURN_NONE;
}

static PyObject* py_redo(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    inst->redo();
    me->context->makeNodeDirty();
    Py_RETURN_NONE;
}

static PyObject* py_reset_undo(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    inst->resetUndo();
    Py_RETURN_NONE;
}

static PyObject* py_cache_state(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    inst->cacheState();
    Py_RETURN_NONE;
}

static PyObject* py_set_undo(PyObject* self, PyObject* args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationViewCHOP* inst = (AnimationViewCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get AnimationViewCHOP instance");
        return NULL;
    }

    auto result = inst->setUndo();
    if (result) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

// ...existing code...