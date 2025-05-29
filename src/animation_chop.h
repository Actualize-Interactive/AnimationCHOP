#include "CHOP_CPlusPlusBase.h"
#include <anim/animation.hpp>
#include <string>
#include <vector>
#include <map>

using namespace TD;

class AnimationCHOP : public CHOP_CPlusPlusBase
{
public:
	AnimationCHOP(const OP_NodeInfo* info);
	virtual ~AnimationCHOP();

	virtual void		getGeneralInfo(CHOP_GeneralInfo*, const OP_Inputs*, void* ) override;
	virtual bool		getOutputInfo(CHOP_OutputInfo*, const OP_Inputs*, void*) override;
	virtual void		getChannelName(int32_t index, OP_String *name, const OP_Inputs*, void* reserved1) override;

	virtual void		execute(CHOP_Output*, const OP_Inputs*, void* reserved1) override;

	virtual void getWarningString(OP_String *warning, void* reserved1) override;
	virtual void getErrorString(OP_String *warning, void* reserved1) override;

	virtual void		setupParameters(OP_ParameterManager* manager, void *reserved1) override;
	virtual void		pulsePressed(const char* name, void* reserved1) override;

	anim::Animation& animation() { return m_animation; }

	anim::Channel& createChannel(const std::string& name);
	anim::Channel* getChannel(const std::string& name);
	const anim::Channel* getChannel(const std::string& name) const;
	bool removeChannel(const std::string& name);
	bool removeChannel(size_t index);

	void createChannels(const std::vector<std::string>& channelNames);
	bool removeChannels(const std::vector<std::string>& channelNames);
	void clearChannels();

	bool setKeyframeAtTime(const std::string& channelName, double time, double value, 
                         anim::Function in_function = anim::Function::bezier, 
                         anim::Function out_function = anim::Function::bezier, 
                         anim::HandleMode handle_mode = anim::HandleMode::smooth);
	bool removeKeyframeAtTime(const std::string& channelName, double time);
	bool removeKeyframes(const std::string& channelName, const std::vector<double>& times);

	size_t getChannelCount() const;
	std::vector<std::string> getChannelNames() const;
	bool channelExists(const std::string& name) const;
	size_t getKeyframeCount(const std::string& channelName) const;

private:
	const OP_NodeInfo*	m_nodeInfo;
	const char* m_warning;
	const char* m_error;

	anim::Animation     m_animation;

};

static PyObject* py_animationFromDict(PyObject* self, PyObject* args);

static PyObject* py_createChannel(PyObject* self, PyObject* args);
static PyObject* py_getChannel(PyObject* self, PyObject* args);
static PyObject* py_removeChannel(PyObject* self, PyObject* args);
static PyObject* py_getChannelNames(PyObject* self);
static PyObject* py_clearChannels(PyObject* self);
static PyObject* py_getAnimation(PyObject* self);

static PyObject* py_setKeyframe(PyObject* self, PyObject* args);
static PyObject* py_setKeyframeAtTime(PyObject* self, PyObject* args);
static PyObject* py_getKeyframe(PyObject* self, PyObject* args);
static PyObject* py_getKeyframeAtTime(PyObject* self, PyObject* args);
static PyObject* py_hasKeyframe(PyObject* self, PyObject* args);
static PyObject* py_hasKeyframeAtTime(PyObject* self, PyObject* args);
static PyObject* py_removeKeyframe(PyObject* self, PyObject* args);
static PyObject* py_removeKeyframeAtTime(PyObject* self, PyObject* args);

static PyObject* py_setKeyframes(PyObject* self, PyObject* args);
static PyObject* py_setKeyframesAtTime(PyObject* self, PyObject* args);
static PyObject* py_removeKeyframesAtTime(PyObject* self, PyObject* args);

static PyObject* py_debugChannel(PyObject* self, PyObject* args);