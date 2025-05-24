#include "CHOP_CPlusPlusBase.h"
#include <anim.hpp>
#include <string>
#include <vector>
#include <map>

using namespace TD;

class AnimationCHOP : public CHOP_CPlusPlusBase
{
public:
	AnimationCHOP(const OP_NodeInfo* info);
	virtual ~AnimationCHOP();

	virtual void		getGeneralInfo(CHOP_GeneralInfo*, const OP_Inputs*, [[maybe_unused]] void* ) override;
	virtual bool		getOutputInfo(CHOP_OutputInfo*, const OP_Inputs*, [[maybe_unused]] void*) override;
	virtual void		getChannelName(int32_t index, OP_String *name, const OP_Inputs*, [[maybe_unused]] void* reserved) override;

	virtual void		execute(CHOP_Output*, const OP_Inputs*, [[maybe_unused]] void* reserved1) override;

	virtual void getWarningString(OP_String *warning, [[maybe_unused]] void* reserved1) override;
	virtual void getErrorString(OP_String *warning, [[maybe_unused]] void* reserved1) override;

	virtual void		setupParameters(OP_ParameterManager* manager, [[maybe_unused]] void *reserved1) override;
	virtual void		pulsePressed(const char* name, [[maybe_unused]] void* reserved1) override;

	anim::Animation& animation() { return m_animation; }

	const anim::Channel* createChannel(const std::string& name, int32_t insertIndex = -1);
	bool removeChannel(const std::string& name);
	bool removeChannel(size_t index);

	void createChannels(const std::vector<std::string>& channelNames);
	bool removeChannels(const std::vector<std::string>& channelNames);

	bool setKeyframeAtTime(const std::string& channelName, double time, double value, anim::TangentMode mode, double in_tangent_time, double in_tangent_value, double out_tangent_time, double out_tangent_value);
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

static PyObject* pyCreateChannel(PyObject* self, PyObject* args);
static PyObject* pyGetChannel(PyObject* self, PyObject* args);
static PyObject* pyRemoveChannel(PyObject* self, PyObject* args);
static PyObject* pyGetChannelNames(PyObject* self);

static PyObject* pySetKeyframe(PyObject* self, PyObject* args);
static PyObject* pySetKeyframeAtTime(PyObject* self, PyObject* args);
static PyObject* pyGetKeyframe(PyObject* self, PyObject* args);
static PyObject* pyGetKeyframeAtTime(PyObject* self, PyObject* args);
static PyObject* pyHasKeyframe(PyObject* self, PyObject* args);
static PyObject* pyHasKeyframeAtTime(PyObject* self, PyObject* args);
static PyObject* pyRemoveKeyframe(PyObject* self, PyObject* args);
static PyObject* pyRemoveKeyframeAtTime(PyObject* self, PyObject* args);

static PyObject* pySetKeyframes(PyObject* self, PyObject* args);
static PyObject* pySetKeyframesAtTime(PyObject* self, PyObject* args);
static PyObject* pyRemoveKeyframesAtTime(PyObject* self, PyObject* args);