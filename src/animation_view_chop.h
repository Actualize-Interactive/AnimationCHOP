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

    AnimationCHOP* animationCHOP();
    AnimationViewCHOP* dataInstance();
    const AnimationViewCHOP* dataInstance() const;


    // Python binding methods
    void selectKeyframes(const std::vector<size_t>& indices);
    void unselectKeyframes(const std::vector<size_t>& indices);
    void unselectAllKeyframes();
    std::vector<size_t> getSelectedKeyframes() const;

    void selectSegments(const std::vector<size_t>& indices);
    void unselectSegments(const std::vector<size_t>& indices);
    void unselectAllSegments();
    std::vector<size_t> getSelectedSegments() const;

    void selectStartHandles(const std::vector<size_t>& indices);
    void unselectStartHandles(const std::vector<size_t>& indices);
    void unselectAllStartHandles();
    std::vector<size_t> getSelectedStartHandles() const;

    void selectEndHandles(const std::vector<size_t>& indices);
    void unselectEndHandles(const std::vector<size_t>& indices);
    void unselectAllEndHandles();
    std::vector<size_t> getSelectedEndHandles() const;

    void selectChannels(const std::vector<size_t>& indices);
    void unselectChannels(const std::vector<size_t>& indices);
    void unselectAllChannels();
    std::vector<size_t> getSelectedChannels() const;
    void setChannelDisplay(size_t index, bool display);

    const std::vector<bool>& selectedKeyframes() const { return m_selectedKeyframes; }
    const std::vector<bool>& selectedSegments() const { return m_selectedSegments; }
    const std::vector<bool>& selectedStartHandles() const { return m_selectedStartHandles; }
    const std::vector<bool>& selectedEndHandles() const { return m_selectedEndHandles; }
    const std::vector<bool>& selectedChannels() const { return m_selectedChannels; }
    const std::vector<bool>& displayedChannels() const { return m_displayedChannels; }

protected:
    enum class ViewMode {
        samples,
        keyframes,
        segments,
        channels,
        animation
    };

    struct SelectedKeyframe {
        size_t channel_index;
        size_t keyframe_index;
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
    std::vector<bool> m_displayedChannels;

    std::array<const char*, 12> m_keyframes_chan_names {
        "channel_index", "keyframe_index", "time", "value", "in_handle_time", "in_handle_value",
        "out_handle_time", "out_handle_value", "function", "handle_mode", "selected", "display"
    };

    std::array<const char*, 15> m_segments_chan_names {
        "channel_index", "segment_index", "start_time", "start_value", "end_time", "end_value", 
        "start_handle_time", "start_handle_value", "end_handle_time", "end_handle_value",
        "display_start_handle", "display_end_handle", "selected", "selected_start_handles", "selected_end_handles"
    };

    std::array<const char*, 6> m_channels_chan_names {
        "num_keyframes", "start_time", "end_time", "start_index", "selected", "display"
    };

    std::array<const char*, 5> m_animation_chan_names {
        "num_channels", "min_keyframe_time", "max_keyframe_time", 
        "min_keyframe_value", "max_keyframe_value", 
    };

    AnimationCHOP* m_animationCHOP;
    AnimationViewCHOP* m_dataInstance;

    bool displayStartHandle(size_t channel_index, const Keyframe& keyframe) const;
    bool displayEndHandle(bool display_start_handle, const Keyframe& keyframe) const;

private:
    bool setDataInstance(const OP_Inputs* inputs);

};
