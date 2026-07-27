#pragma once
#include "CHOP_CPlusPlusBase.h"

#include <anim.hpp>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include <Python.h>


using namespace TD;

// The Python method/getset tables this operator registers with TouchDesigner.
// Defined in animation_chop.cpp and exposed so the pytest extension under
// tests/python can bind the identical tables without TouchDesigner.
extern PyMethodDef AnimationCHOP_pythonMethods[];
extern PyGetSetDef AnimationCHOP_pythonGetSets[];

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

	// Persist the animation into the .toe. TouchDesigner calls saveData() on
	// every project save and whenever the operator is unloaded, and loadData()
	// on load or reload -- so the channels a user keyframed survive a restart
	// with no external file and no Python.
	virtual void		saveData(OP_NodeSaveState* saver, void* reserved1) override;
	virtual void		loadData(const OP_NodeLoadState* loader, void* reserved1) override;

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
	// A failure to restore the saved animation, kept separately because
	// execute() clears m_error on every cook and this has to stay visible: the
	// user's keyframes were in that .toe. Owns its text, unlike m_error, which
	// only ever points at string literals.
	std::string m_loadError;
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


