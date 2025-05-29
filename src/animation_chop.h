#pragma once
#include "CHOP_CPlusPlusBase.h"
#include <anim/animation.hpp>
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

private:
	const OP_NodeInfo* m_nodeInfo;
	const char* m_warning;
	const char* m_error;
	float m_sampleRate { 60.0f }; 
	std::unique_ptr<anim::Animation> m_animation;

};


// static PyObject* py_chop_animationFromDict(PyObject* self, PyObject* args);
static PyObject* py_chop_create_channel(PyObject* self, PyObject* args);
static PyObject* py_chop_emplace_channel(PyObject* self, PyObject* args);
static PyObject* py_chop_insert_channel(PyObject* self, PyObject* args);
static PyObject* py_chop_channel(PyObject* self, PyObject* args);
static PyObject* py_chop_remove_channel(PyObject* self, PyObject* args);
static PyObject* py_chop_has_channel(PyObject* self, PyObject* args);
static PyObject* py_chop_clear(PyObject* self, PyObject* args);

static PyObject* py_chop_get_channels(PyObject* self, void* closure);
static PyObject* py_chop_get_channel_names(PyObject* self, void* closure);
static PyObject* py_chop_get_num_channels(PyObject* self, void* closure);
static PyObject* py_chop_get_start_time(PyObject* self, void* closure);
static int py_chop_set_start_time(PyObject* self, PyObject* args, void* closure);
static PyObject* py_chop_get_end_time(PyObject* self, void* closure);
static int py_chop_set_end_time(PyObject* self, PyObject* args, void* closure);
static PyObject* py_chop_get_length(PyObject* self, void* closure);
static int py_chop_set_length(PyObject* self, PyObject* args, void* closure);
static PyObject* py_chop_num_samples(PyObject* self, void* closure);