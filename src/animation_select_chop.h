#pragma once
#include "animation_chop.h"
#include <anim.hpp>
#include <vector>
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
    const char* m_warning;
    const char* m_error;
    bool m_autoRange;
    double m_startTime;
    double m_endTime;

    AnimationCHOP* getAnimationCHOP(const OP_Inputs* inputs);

    void updateChannelSelection(const OP_Inputs* inputs);
    bool matchesPattern(const std::string& channelName, const std::string& pattern);
    void evaluateTargetCHOP(const OP_Inputs* inputs);
};
