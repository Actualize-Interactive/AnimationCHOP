/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

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
	
	// Channel management methods
	
	// Create a new animation channel with the given name
	// Returns true if successful, false if a channel with that name already exists
	bool                createChannel(const std::string& name);
	
	// Remove a channel by name
	// Returns true if the channel was found and removed, false otherwise
	bool                removeChannel(const std::string& name);
	
	// Remove a channel by index
	// Returns true if the channel was found and removed, false otherwise
	bool                removeChannelByIndex(size_t index);
	
	// Add multiple channels at once
	// Returns the number of successfully added channels
	int                 addChannels(const std::vector<std::string>& channelNames);
	
	// Remove multiple channels by name
	// Returns the number of successfully removed channels
	int                 removeChannels(const std::vector<std::string>& channelNames);
		// Keyframe management methods
	
	// Set a keyframe in a channel by name
	// If channel doesn't exist, returns false
	bool                setKeyframe(const std::string& channelName, double time, double value, 
	                               anim::TangentMode mode = anim::TangentMode::smoothAuto,
	                               double in_tangent_time = 0, double in_tangent_value = 0,
	                               double out_tangent_time = 0, double out_tangent_value = 0);
	
	// Set a keyframe in a channel by index
	// If index is out of range, returns false
	bool                setKeyframeInChannel(size_t channelIndex, double time, double value,
	                                      anim::TangentMode mode = anim::TangentMode::smoothAuto,
	                                      double in_tangent_time = 0, double in_tangent_value = 0,
	                                      double out_tangent_time = 0, double out_tangent_value = 0);
	
	// Remove a keyframe from a channel by name at the specified time
	// Returns true if keyframe was removed, false if channel doesn't exist or no keyframe at that time
	bool                removeKeyframe(const std::string& channelName, double time);
	
	// Remove a keyframe from a channel by index at the specified time
	// Returns true if keyframe was removed, false if index is out of range or no keyframe at that time
	bool                removeKeyframeFromChannel(size_t channelIndex, double time);
		// Set multiple keyframes in a channel by name
	// Returns the number of keyframes successfully set
	int                 setKeyframes(const std::string& channelName, 
	                               const std::vector<std::pair<double, double>>& timeValuePairs,
	                               anim::TangentMode mode = anim::TangentMode::smoothAuto);
	
	// Set multiple keyframes in a channel by index
	// Returns the number of keyframes successfully set
	int                 setKeyframesInChannel(size_t channelIndex, 
	                                       const std::vector<std::pair<double, double>>& timeValuePairs,
	                                       anim::TangentMode mode = anim::TangentMode::smoothAuto);
	
	// Remove multiple keyframes from a channel by name
	// Returns the number of keyframes successfully removed
	int                 removeKeyframes(const std::string& channelName, const std::vector<double>& times);
	
	// Remove multiple keyframes from a channel by index
	// Returns the number of keyframes successfully removed
	int                 removeKeyframesFromChannel(size_t channelIndex, const std::vector<double>& times);
	
	// Evaluation methods
	
	// Evaluate a channel at a specific time
	// Returns the value, or 0.0 if the channel doesn't exist
	double              evaluateChannel(const std::string& channelName, double time);
	
	// Evaluate all channels at a specific time
	// Returns a map of channel names to their evaluated values
	std::map<std::string, double> evaluateAllChannels(double time);
	
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
