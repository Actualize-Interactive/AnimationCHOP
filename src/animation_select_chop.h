#pragma once
#include "animation_chop.h"
#include <anim.hpp>
#include <vector>
#include <array>
#include <string>


using namespace TD;

class AnimationSelectCHOP : public CHOP_CPlusPlusBase
{
public:
    AnimationSelectCHOP(const OP_NodeInfo* info);
    virtual ~AnimationSelectCHOP();

    virtual void getGeneralInfo(CHOP_GeneralInfo*, const OP_Inputs*, void* reserved1) override;
    virtual bool getOutputInfo(CHOP_OutputInfo*, const OP_Inputs*, void* reserved1) override;
    virtual void getChannelName(int32_t index, OP_String* name, const OP_Inputs*, void* reserved1) override;
    virtual void execute(CHOP_Output*, const OP_Inputs*, void* reserved1) override;
    virtual void getWarningString(OP_String* warning, void* reserved1) override;
    virtual void getErrorString(OP_String* error, void* reserved1) override;
    virtual void setupParameters(OP_ParameterManager* manager, void* reserved1) override;

private:
    enum class SelectMode {
        autoRange,
        range,
        keyframes,
        channel_info,
    };

    const char* m_warning;
    const char* m_error;
    SelectMode m_selectMode;
    double m_startTime;
    double m_endTime;

    std::array<const char*, 9> m_keyframes_chan_names {
        "channel_index", "time", "value", "in_handle_time", "in_handle_value",
        "out_handle_time", "out_handle_value", "function", "handle_mode"
    };

    std::array<const char*, 4> m_channel_info_chan_names {
        "start_time", "end_time", "start_index", "num_keyframes"
    };


    AnimationCHOP* getAnimationCHOP(const OP_Inputs* inputs);

    void updateChannelSelection(const OP_Inputs* inputs);
    bool matchesPattern(const std::string& channelName, const std::string& pattern);
    void evaluateTargetCHOP(const OP_Inputs* inputs);
};
