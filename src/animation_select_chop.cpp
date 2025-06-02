#include "animation_select_chop.h"


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
    info->customOPInfo.opType->setString("Animationselect");
    info->customOPInfo.opLabel->setString("Animation Select");
    info->customOPInfo.opIcon->setString("ANS");
    info->customOPInfo.authorName->setString("Keith Lostracco");
    info->customOPInfo.authorEmail->setString("keith@actualize.vision");
    info->customOPInfo.minInputs = 0;
    info->customOPInfo.maxInputs = 0;
}


DLLEXPORT 
CHOP_CPlusPlusBase* CreateCHOPInstance(const OP_NodeInfo* info)
{
    return new AnimationSelectCHOP(info);
}

DLLEXPORT void 
DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
{
    delete static_cast<AnimationSelectCHOP*>(instance);
}

}; // extern "C"

AnimationSelectCHOP::AnimationSelectCHOP(const OP_NodeInfo* info)
    : m_warning(nullptr)
    , m_error(nullptr)
    , m_autoRange(true)
    , m_startTime(0.0)
    , m_endTime(30.0)
{
}

AnimationSelectCHOP::~AnimationSelectCHOP()
{
}

void 
AnimationSelectCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
    ginfo->cookEveryFrameIfAsked = false;
    ginfo->timeslice = false;
}

bool 
AnimationSelectCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
    info->sampleRate = static_cast<float>(inputs->getParDouble("Samplerate", 0));
    m_autoRange = inputs->getParInt("Autorange", 0) != 0;
    info->startIndex = 0;

    auto animationChop = getAnimationCHOP(inputs);
    if (!animationChop) {
        return false;
    }
    auto animation = animationChop->animation();
    if (!animation) {
        return false;
    }

    info->numChannels = static_cast<int32_t>(animation->size());

    if (m_autoRange) {
        auto max_length = 0.0;
        for (const auto& channel : animation->channels()) {
            max_length = std::max(max_length, channel->length());
        }
        info->numSamples = static_cast<int32_t>(max_length * info->sampleRate);
        m_startTime = 0.0;
        m_endTime = max_length;
        return true;
    }
    
    auto rangeStart = inputs->getParDouble("Range", 0);
    auto rangeEnd = inputs->getParDouble("Range", 1);
    auto range_delta = rangeEnd - rangeStart;

    auto rangeUnit = inputs->getParString("Rangeunit");
    if (strcmp(rangeUnit, "samples") == 0) { // Samples
        info->numSamples = static_cast<int32_t>(range_delta + 1.0);
        m_startTime = rangeStart / info->sampleRate;
        m_endTime = rangeEnd / info->sampleRate;
    } else { // Seconds
        info->numSamples = static_cast<int32_t>(range_delta * info->sampleRate);
        m_startTime = rangeStart;
        m_endTime = rangeEnd;
    }

    return true;
}

void 
AnimationSelectCHOP::getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void* reserved1)
{
    auto animationChop = getAnimationCHOP(inputs);
    if (!animationChop) {
        return;
    }

    auto animation = animationChop->animation();
    if (!animation) {
        return;
    }
    if (index < 0 || index >= static_cast<int32_t>(animation->channel_names().size())) {
        return;
    }
    name ->setString(animation->channel_names()[index].c_str());
}

void 
AnimationSelectCHOP::execute(CHOP_Output* output, const OP_Inputs* inputs, void* reserved1)
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

    size_t num_anim_channels = animation->num_channels();
    for (int i = 0; i < output->numChannels; i++) {
        if (i < static_cast<int>(num_anim_channels)) {
            auto samples = animation->channel(i).evaluate_range(
                m_startTime,
                m_endTime,
                output->numSamples
            );
            std::copy(samples.begin(), samples.end(), output->channels[i]);
        }
    }
}
void
AnimationSelectCHOP::getWarningString(OP_String* warning, void* reserved1)
{
    warning->setString(m_warning);
}

void
AnimationSelectCHOP::getErrorString(OP_String* error, void* reserved1)
{
    error->setString(m_error);
}

void
AnimationSelectCHOP::setupParameters(OP_ParameterManager* manager, void* reserved1)
{
    {
        OP_StringParameter sp;
        sp.name = "Animationchop";
        sp.label = "Animation CHOP";
        sp.defaultValue = "";
        manager->appendCHOP(sp);
    } {
		OP_NumericParameter	np;
		np.name = "Autorange";
		np.label = "Auto Range";
		np.defaultValues[0] = 1.0;
 
		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	} {
		OP_NumericParameter	np;
		np.name = "Range";
		np.label = "Range";
		np.defaultValues[0] = 0.0;
        np.minValues[0] = 0.0;
        np.clampMins[0] = true;
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
AnimationSelectCHOP::getAnimationCHOP(const OP_Inputs *inputs)
{
    auto target = inputs->getParCHOP("Animationchop");
    if (!target) {
        m_warning = "No Animation CHOP specified.";
        return nullptr;
    } else if (!target->customOP || strcmp(target->customOP->opType, "Animation") != 0) {
        m_error = "Invalid Animation CHOP specified.";
        return nullptr;
    }
    
    std::cout << "Target CHOP: " << target->customOP->opType << std::endl;

    return static_cast<AnimationCHOP*>(target->customOP->instance);
}

void
AnimationCHOP::applyOutputMode(const OP_Inputs *inputs)
{
    const char* output_mode = inputs->getParString("Outputmode");
    if (strcmp(output_mode, "range") == 0) {
        m_outputMode = OutputMode::range;
    } else if (strcmp(output_mode, "autorange") == 0) {
        m_outputMode = OutputMode::autoRange;
    } else {
        m_outputMode = OutputMode::autoRange;
    }
}

// void 
// AnimationSelectCHOP::updateChannelSelection(const OP_Inputs* inputs)
// {
//     // // Get parameters
//     // m_targetCHOPPath = inputs->getParString("targetpath");
//     // m_startFrame = inputs->getParInt("startframe");
//     // m_endFrame = inputs->getParInt("endframe");
//     // m_channelPattern = inputs->getParString("channelpattern");
    
//     // // Clear previous channel names (actual channel evaluation happens in evaluateTargetCHOP)
//     // m_channelNames.clear();
// }

// bool 
// AnimationSelectCHOP::matchesPattern(const std::string& channelName, const std::string& pattern)
// {
//     // if (pattern == "*") return true;
    
//     // try {
//     //     // Convert glob pattern to regex
//     //     std::string regexPattern = pattern;
//     //     std::replace(regexPattern.begin(), regexPattern.end(), '*', '.');
//     //     regexPattern = ".*" + regexPattern + ".*";
        
//     //     std::regex re(regexPattern);
//     //     return std::regex_match(channelName, re);
//     // }
//     // catch (const std::exception&) {
//     //     return false;
//     // }

//     return true; // Placeholder, always matches for now
// }

// void 
// AnimationSelectCHOP::evaluateTargetCHOP(const OP_Inputs* inputs)
// {
//     // // This is a placeholder implementation
//     // // In a real implementation, you would:
//     // // 1. Get reference to the target CHOP using m_targetCHOPPath
//     // // 2. Extract channel names and data from the target CHOP
//     // // 3. Filter channels based on m_channelPattern
//     // // 4. Extract data for the frame range [m_startFrame, m_endFrame]
    
//     // // For now, create some dummy data
//     // m_channelNames.clear();
//     // m_channelData.clear();
    
//     // // Example: create a few test channels
//     // for (int i = 0; i < 3; i++)
//     // {
//     //     std::string channelName = "chan" + std::to_string(i);
//     //     if (matchesPattern(channelName, m_channelPattern))
//     //     {
//     //         m_channelNames.push_back(channelName);
            
//     //         std::vector<double> channelData;
//     //         int numSamples = m_endFrame - m_startFrame + 1;
//     //         for (int j = 0; j < numSamples; j++)
//     //         {
//     //             channelData.push_back(static_cast<double>(i + j) * 0.1);
//     //         }
//     //         m_channelData.push_back(channelData);
//     //     }
//     // }
// }
