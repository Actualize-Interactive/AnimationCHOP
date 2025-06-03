#include "animation_view_chop.h"


#include <cstring>
#include <algorithm>


#include <regex>

#include <iostream>

extern "C"
{
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

    auto animationChop = getAnimationCHOP(inputs);
    if (!animationChop) {
        return false;
    }
    auto animation = animationChop->animation();
    if (!animation) {
        return false;
    }

    switch (m_viewMode) {
    case ViewMode::samples: {
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
        int32_t num_samples = 0;
        for (const auto& channel : animation->channels()) {
            num_samples += static_cast<int32_t>(channel->size());
        }
        info->numSamples = num_samples;
        m_selectedKeyframes.resize(num_samples, false); // Initialize selection state
        return true;
    } case ViewMode::segments: {
        info->numChannels = static_cast<int32_t>(m_segments_chan_names.size());
        int32_t num_samples = 0;
        for (const auto& channel : animation->channels()) {
            num_samples += static_cast<int32_t>(channel->size() - 1); // Each segment is defined by two keyframes
        }
        info->numSamples = num_samples;
        m_selectedSegments.resize(num_samples, false); // Initialize selection state
        return true;
    } case ViewMode::channels: {
        info->numChannels = static_cast<int32_t>(m_channels_chan_names.size());
        info->numSamples = static_cast<int32_t>(animation->size());
        m_selectedChannels.resize(animation->size(), false); // Initialize selection state
        return true;
    } case ViewMode::animation: {
        info->numChannels = static_cast<int32_t>(m_animation_chan_names.size());
        info->numSamples = 1; // Single sample for animation info
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
    auto animationChop = getAnimationCHOP(inputs);
    if (!animationChop) {
        return;
    }
    auto animation = animationChop->animation();
    if (!animation) {
        return;
    }
    switch (m_viewMode) {
    case ViewMode::samples: {
        if (index < 0 || index >= static_cast<int32_t>(animation->channel_names().size())) {
            return;
        }
        name ->setString(animation->channel_names()[index].c_str());
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

    auto animationChop = getAnimationCHOP(inputs);
    if (!animationChop) {
        return;
    }

    auto animation = animationChop->animation();
    if (!animation) {
        return;
    }
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
                    output->channels[10][i] = static_cast<float>(m_selectedKeyframes[i]);
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
                output->channels[10][i] = static_cast<float>(displayHandles(start_keyframe));
                output->channels[11][i] = static_cast<float>(m_selectedSegments[i]); // Selected
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
                output->channels[4][c] = static_cast<float>(m_selectedChannels[c]); // Selected
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
            for (size_t k = 0; k < channel->size(); ++k) {
                const auto& keyframe = channel->keyframe(k);
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
        sp.name = "Animationchop";
        sp.label = "Animation CHOP";
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
AnimationViewCHOP::getAnimationCHOP(const OP_Inputs *inputs)
{
    auto target = inputs->getParCHOP("Animationchop");
    if (!target) {
        m_warning = "No Animation CHOP specified.";
        return nullptr;
    } else if (!target->customOP || strcmp(target->customOP->opType, "Animation") != 0) {
        m_error = "Invalid Animation CHOP specified.";
        return nullptr;
    }
    return static_cast<AnimationCHOP*>(target->customOP->instance);
}


bool AnimationViewCHOP::displayHandles(const Keyframe& start_keyframe) const
{
    return start_keyframe.function == Function::bezier 
        && static_cast<uint8_t>(start_keyframe.handle_mode) > static_cast<uint8_t>(HandleMode::smooth);
}