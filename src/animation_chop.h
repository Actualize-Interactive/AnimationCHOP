#include "CHOP_CPlusPlusBase.h"
#include <anim.hpp>
#include <string>
#include <vector>
#include <map>

using namespace TD;

/*
This is a TouchDesigner CHOP that provides animation curve functionality using the anim library.
It allows creating keyframe animations with various interpolation modes, and provides methods
for managing channels and keyframes through Python bindings.

The AnimationCHOP manages a collection of animation channels, each with keyframes that can be
evaluated over time. Each channel can be accessed by name or index, and keyframes can be added,
removed, or modified.

For our initial implementation, we'll only create channels and keyframes through functions/python
bindings (no OP_Inputs* will have an effect).
*/

class AnimationCHOP;

// To get more help about these functions, look at CHOP_CPlusPlusBase.h
class AnimationCHOP : public CHOP_CPlusPlusBase
{
public:
	AnimationCHOP(const OP_NodeInfo* info);
	virtual ~AnimationCHOP();

	virtual void		getGeneralInfo(CHOP_GeneralInfo*, const OP_Inputs*, void* ) override;
	virtual bool		getOutputInfo(CHOP_OutputInfo*, const OP_Inputs*, void*) override;
	virtual void		getChannelName(int32_t index, OP_String *name, const OP_Inputs*, void* reserved) override;

	virtual void		execute(CHOP_Output*, const OP_Inputs*, void* reserved1) override;

	virtual int32_t		getNumInfoCHOPChans(void* reserved1) override;
	virtual void		getInfoCHOPChan(int index,
										OP_InfoCHOPChan* chan,
										void* reserved1) override;

	virtual bool		getInfoDATSize(OP_InfoDATSize* infoSize, void* resereved1) override;
	virtual void		getInfoDATEntries(int32_t index,
											int32_t nEntries,
											OP_InfoDATEntries* entries,
											void* reserved1) override;

	virtual void		setupParameters(OP_ParameterManager* manager, void *reserved1) override;
	virtual void		pulsePressed(const char* name, void* reserved1) override;

	// Original methods
	void				resetFilter();
	double  			getSpeedMod() const { return m_speedMod; }
	void    			setSpeedMod(double v) { m_speedMod = v; }
	int     			getExecuteCount() const { return m_executeCount; }
	
	// Animation access
	anim::Animation& 	animation() { return m_animation; }
	
	// Channel management methods
	
	// Create a new animation channel with the given name and optional insert index
	// Returns channel pointer if successful, nullptr if a channel with that name already exists
	const anim::Channel* createChannel(const std::string& name, int32_t insertIndex = -1);
	
	// Remove a channel by name
	// Returns true if the channel was found and removed, false otherwise
	bool                removeChannel(const std::string& name);
	
	// Remove a channel by index
	// Returns true if the channel was found and removed, false otherwise
	bool                removeChannel(size_t index);
	
	// Add multiple channels at once
	// Returns the number of successfully added channels
	void                createChannels(const std::vector<std::string>& channelNames);
	
	// Remove multiple channels by name
	// Returns the number of successfully removed channels
	bool                removeChannels(const std::vector<std::string>& channelNames);
	
	// Keyframe management methods
	
	// Set a keyframe in a channel by name
	// If channel doesn't exist, returns false
	bool                setKeyframeAtTime(const std::string& channelName, double time, double value, 
	                               anim::TangentMode mode = anim::TangentMode::smoothAuto,
	                               double in_tangent_time = 0, double in_tangent_value = 0,
	                               double out_tangent_time = 0, double out_tangent_value = 0);
	
	// Remove a keyframe from a channel by name at the specified time
	// Returns true if keyframe was removed, false if channel doesn't exist or no keyframe at that time
	bool                removeKeyframeAtTime(const std::string& channelName, double time);
	
	// Remove multiple keyframes from a channel by name
	// Returns true if all keyframes were removed, false otherwise
	bool                removeKeyframes(const std::string& channelName, const std::vector<double>& times);
	
	// Query methods
	
	// Get the number of channels
	size_t              getChannelCount() const;
	
	// Get list of all channel names
	std::vector<std::string> getChannelNames() const;
	
	// Check if a channel exists
	bool                channelExists(const std::string& name) const;
	
	// Get the number of keyframes in a channel
	size_t              getKeyframeCount(const std::string& channelName) const;
	
private:
	// We don't need to store this pointer, but we do for the example.
	// The OP_NodeInfo class store information about the node that's using
	// this instance of the class (like its name).
	const OP_NodeInfo*	m_nodeInfo;

	// In this example this value will be incremented each time the execute()
	// function is called, then passes back to the CHOP 
	int32_t				m_executeCount;

	double				m_offset;
	double				m_speedMod;
	
	// The animation object that manages all channels
	anim::Animation     m_animation;

};

// Python binding function declarations
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
