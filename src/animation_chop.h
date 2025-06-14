#pragma once
#include "CHOP_CPlusPlusBase.h"

#include <anim.hpp>
#include <string>
#include <vector>
#include <map>
#include <memory>


using namespace TD;

class AnimationCHOP : public CHOP_CPlusPlusBase
{
public:
	AnimationCHOP(const OP_NodeInfo* info);
	~AnimationCHOP() override;

	virtual void		getGeneralInfo(CHOP_GeneralInfo*, const OP_Inputs*, void* ) override;
	virtual bool		getOutputInfo(CHOP_OutputInfo*, const OP_Inputs*, void*) override;
	virtual void		getChannelName(int32_t index, OP_String *name, const OP_Inputs*, void* reserved1) override;

	virtual void		execute(CHOP_Output*, const OP_Inputs*, void* reserved1) override;

	virtual void getWarningString(OP_String *warning, void* reserved1) override;
	virtual void getErrorString(OP_String *warning, void* reserved1) override;

	virtual void		setupParameters(OP_ParameterManager* manager, void *reserved1) override;
	virtual void		pulsePressed(const char* name, void* reserved1) override;

	anim::Animation* animation() { return m_animation.get(); }
	const anim::Animation& animation() const { return *m_animation; }
	float sampleRate() const { return m_sampleRate; }

	void replaceAnimation(std::unique_ptr<anim::Animation> newAnimation) {
		if (newAnimation) {
			m_animation = std::move(newAnimation);
		}
	}


private:
	const OP_NodeInfo* m_nodeInfo;
	const char* m_warning;
	const char* m_error;
	float m_sampleRate { 60.0f };
	std::unique_ptr<anim::Animation> m_animation;

	enum class OutputMode {
		range,
		autoRange,
		input,
		sequence
	};

	OutputMode m_outputMode { OutputMode::range };
	double m_lastEvalTime { 0.0 };

};


