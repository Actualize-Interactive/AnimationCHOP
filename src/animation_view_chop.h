#pragma once
#include "animation_chop.h"
#include <anim.hpp>
#include <vector>
#include <array>
#include <string>


using namespace TD;
using namespace anim;

class AnimationViewCHOP : public CHOP_CPlusPlusBase
{
public:
    AnimationViewCHOP(const OP_NodeInfo* info);
    virtual ~AnimationViewCHOP();

    virtual void getGeneralInfo(CHOP_GeneralInfo*, const OP_Inputs*, void* reserved1) override;
    virtual bool getOutputInfo(CHOP_OutputInfo*, const OP_Inputs*, void* reserved1) override;
    virtual void getChannelName(int32_t index, OP_String* name, const OP_Inputs*, void* reserved1) override;
    virtual void execute(CHOP_Output*, const OP_Inputs*, void* reserved1) override;
    virtual void getWarningString(OP_String* warning, void* reserved1) override;
    virtual void getErrorString(OP_String* error, void* reserved1) override;
    virtual void setupParameters(OP_ParameterManager* manager, void* reserved1) override;

private:
    enum class ViewMode {
        samples,
        keyframes,
        segments,
        channels,
        animation
    };

    const char* m_warning;
    const char* m_error;
    ViewMode m_viewMode;
    double m_samplesStartTime;
    double m_samplesEndTime;

    std::vector<bool> m_selectedKeyframes;
    std::vector<bool> m_selectedSegments;
    std::vector<bool> m_selectedStartHandles;
    std::vector<bool> m_selectedEndHandles;
    std::vector<bool> m_selectedChannels;

    std::array<const char*, 11> m_keyframes_chan_names {
        "channel_index", "keyframe_index", "time", "value", "in_handle_time", "in_handle_value",
        "out_handle_time", "out_handle_value", "function", "handle_mode", "selected"
    };

    std::array<const char*, 14> m_segments_chan_names {
        "channel_index", "segment_index", "start_time", "start_value", "end_time", "end_value", 
        "start_handle_time", "start_handle_value", "end_handle_time", "end_handle_value",
        "display_handles", "selected", "selected_start_handle", "selected_end_handle"
    };

    std::array<const char*, 5> m_channels_chan_names {
        "num_keyframes", "start_time", "end_time", "start_index", "selected"
    };

    std::array<const char*, 5> m_animation_chan_names {
        "num_channels", "min_keyframe_time", "max_keyframe_time", 
        "min_keyframe_value", "max_keyframe_value", 
    };


    AnimationCHOP* getAnimationCHOP(const OP_Inputs* inputs);
    bool displayHandles(const Keyframe& start_keyframe) const;
};
