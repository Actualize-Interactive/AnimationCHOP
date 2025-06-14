#pragma once
#include "animation_chop.h"
#include <anim.hpp>
#include <vector>
#include <array>
#include <string>
#include <memory>


using namespace TD;
using namespace anim;

struct KeyframeView {
    size_t channel_index;
    size_t keyframe_index;
    bool selected;
    anim::Point begin_set_position;
};

struct SegmentView {
    size_t channel_index;
    size_t keyframe_index;
    bool selected;
    anim::Point begin_set_start_position;
    anim::Point begin_set_end_position;
    bool start_handle_selected;
    anim::Point begin_set_start_handle;
    bool end_handle_selected;
    anim::Point begin_set_end_handle;
};
struct ChannelView {
    bool selected;
    bool displayed;
};


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
    void offsetSelectedKeyframes(double time_offset, double value_offset);
    void offsetSelectedKeyframesByTime(double time_offset);
    void offsetSelectedKeyframesByValue(double value_offset);

    void selectSegments(const std::vector<size_t>& indices);
    void unselectSegments(const std::vector<size_t>& indices);
    void unselectAllSegments();
    void offsetSelectedSegments(double time_offset, double value_offset);
    void offsetSelectedSegmentsByTime(double time_offset);
    void offsetSelectedSegmentsByValue(double value_offset);

    void selectStartHandles(const std::vector<size_t>& indices);
    void unselectStartHandles(const std::vector<size_t>& indices);
    void unselectAllStartHandles();
    void offsetSelectedStartHandles(double time_offset, double value_offset);
    void offsetSelectedStartHandlesByTime(double time_offset);
    void offsetSelectedStartHandlesByValue(double value_offset);

    void selectEndHandles(const std::vector<size_t>& indices);
    void unselectEndHandles(const std::vector<size_t>& indices);
    void unselectAllEndHandles();
    void offsetSelectedEndHandles(double time_offset, double value_offset);
    void offsetSelectedEndHandlesByTime(double time_offset);
    void offsetSelectedEndHandlesByValue(double value_offset);

    void selectChannels(const std::vector<size_t>& indices);
    void unselectChannels(const std::vector<size_t>& indices);
    void unselectAllChannels();
    std::vector<size_t> getSelectedChannels() const;
    void setChannelDisplay(size_t index, bool display);

    void resetBeginSetValues();

    // Undo/Redo functionality
    void undo();
    void redo();
    void resetUndo();
    void cacheState();
    void setUndo();

    const std::vector<KeyframeView>& keyframeViews() const { return m_keyframeViews; }
    const std::vector<SegmentView>& segmentViews() const { return m_segmentViews; }
    const std::vector<ChannelView>& channelViews() const { return m_channelViews; }
    
protected:
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

    std::vector<KeyframeView> m_keyframeViews;
    std::vector<SegmentView> m_segmentViews;
    std::vector<ChannelView> m_channelViews;

    // Undo/Redo stack
    std::vector<std::unique_ptr<anim::Animation>> m_undoStack;
    std::vector<std::unique_ptr<anim::Animation>> m_redoStack;
    std::unique_ptr<anim::Animation> m_stateCache;

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
