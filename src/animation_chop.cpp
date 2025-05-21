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

#include "animation_chop.h"

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <assert.h>
#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
#else
	#include <Python/Python.h>
	#include <Python/structmember.h>
#endif

static PyObject*
pyReset(PyObject* self)
{
	PY_Struct* me = (PY_Struct*)self;

	PY_GetInfo info;
	// We don't want to cook the node before we set this, since it doesn't depend on it's current state
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	// It's possible the instance will be nullptr, such as if the node has been deleted
	// while the Python class is still being held on and used elsewhere.
	if (inst)
	{
		inst->resetFilter();
		// Make the node dirty so it will cook an output a newly reset filter when asked next
		me->context->makeNodeDirty();
	}

	// We need to inc-ref the None object if we are going to return it.
	Py_INCREF(Py_None);
	return Py_None;
}

// Python binding function definitions
static PyObject* pyCreateChannel(PyObject* self, PyObject* args);
static PyObject* pyRemoveChannel(PyObject* self, PyObject* args);
static PyObject* pyGetChannelNames(PyObject* self);
static PyObject* pySetKeyframe(PyObject* self, PyObject* args);
static PyObject* pyRemoveKeyframe(PyObject* self, PyObject* args);
static PyObject* pySetKeyframes(PyObject* self, PyObject* args);
static PyObject* pyRemoveKeyframes(PyObject* self, PyObject* args);
static PyObject* pyEvaluateChannel(PyObject* self, PyObject* args);
static PyObject* pyEvaluateAllChannels(PyObject* self, PyObject* args);

static PyMethodDef methods[] =
{
	{"reset", (PyCFunction)pyReset, METH_NOARGS, "Resets the Filter."},
	
	// Channel management methods
	{"createChannel", (PyCFunction)pyCreateChannel, METH_VARARGS, "Creates a new animation channel with the given name."},
	{"removeChannel", (PyCFunction)pyRemoveChannel, METH_VARARGS, "Removes an animation channel by name."},
	{"getChannelNames", (PyCFunction)pyGetChannelNames, METH_NOARGS, "Returns a list of all channel names."},
	
	// Keyframe management methods
	{"setKeyframe", (PyCFunction)pySetKeyframe, METH_VARARGS, "Sets a keyframe in a channel with control over tangent handles."},
	{"removeKeyframe", (PyCFunction)pyRemoveKeyframe, METH_VARARGS, "Removes a keyframe from a channel at the specified time."},
	{"setKeyframes", (PyCFunction)pySetKeyframes, METH_VARARGS, "Sets multiple keyframes in a channel."},
	{"removeKeyframes", (PyCFunction)pyRemoveKeyframes, METH_VARARGS, "Removes multiple keyframes from a channel."},
	
	// Evaluation methods
	{"evaluateChannel", (PyCFunction)pyEvaluateChannel, METH_VARARGS, "Evaluates a channel at a specific time."},
	{"evaluateAllChannels", (PyCFunction)pyEvaluateAllChannels, METH_VARARGS, "Evaluates all channels at a specific time."},
	
	{0},
	
	// Evaluation methods
	{"evaluateChannel", (PyCFunction)pyEvaluateChannel, METH_VARARGS, "Evaluates a channel at a specific time."},
	{"evaluateAllChannels", (PyCFunction)pyEvaluateAllChannels, METH_VARARGS, "Evaluates all channels at a specific time."},
	
	{0}
};

static PyObject*
pyGetSpeedMod(PyObject* self, void*)
{
	PY_Struct* me = (PY_Struct*)self;

	PY_GetInfo info;
	// Since thie variable is internally held in the class instance and not a product of the 'cook'
	// we don't need to cook the node before getting it.
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	// It's possible the instance will be nullptr, such as if the node has been deleted
	// while the Python class is still being held on and used elsewhere.
	if (inst)
	{
		return PyFloat_FromDouble(inst->getSpeedMod());
	}

	// an error has occured
	return nullptr;
}

static int
pySetSpeedMod(PyObject* self, PyObject* value, void*)
{
	// Do nothing in this case
	if (!value)
		return 0;

	PY_Struct* me = (PY_Struct*)self;

	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	// It's possible the instance will be nullptr, such as if the node has been deleted
	// while the Python class is still being held on and used elsewhere.
	if (inst)
	{
		// Try to cast it to a double object
		PyObject* cast = PyNumber_Float(value);

		if (cast && !PyErr_Occurred())
		{
			double v = PyFloat_AsDouble(cast);
			inst->setSpeedMod(v);
			me->context->makeNodeDirty();
			Py_XDECREF(cast);
			// success
			return 0;
		}
	}
	else
	{
		// getNodeInstance() will have already added a Python error if it returned a null
	}

	// an error has occured
	return -1;
}

static PyObject*
pyGetExecuteCount(PyObject* self, void*)
{
	PY_Struct* me = (PY_Struct*)self;

	PY_GetInfo info;
	// We want to cook the node in this case before getting the execute count
	// so we have an accurate result.
	info.autoCook = true;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	// It's possible the instance will be nullptr, such as if the node has been deleted
	// while the Python class is still being held on and used elsewhere.
	if (inst)
	{
		return PyLong_FromLong(inst->getExecuteCount());
	}

	// an error has occured
	return nullptr;
}

// This struct lists the different getters and/or settings the Custom Operator will expose.
static PyGetSetDef getSets[] =
{
	{"speedMod", pyGetSpeedMod, pySetSpeedMod, "Get or Set the speed modulation.", nullptr},
	// This one doesn't define a 'setter', so it's a read-only value.
	{"executeCount", pyGetExecuteCount, nullptr, "Get execute count.", nullptr},
	{0}
};

const char* PythonCallbacksDATStubs =
"# This is an example callbacks DAT.\n"
"\n"
"# op - The OP that is doing the callback.\n"
"# curSpeed - The current speed value the node will be using.\n"
"#\n"
"# Change the 0.0 to make the speed get adjusted by this callback.\n"
"def getSpeedAdjust(op, curSpeed):\n"
"	return curSpeed + 0.0\n"
"\n"
"# Animation example functions\n"
"\n"
"# Create a simple animation with one channel and keyframes\n"
"def createSimpleAnimation(op):\n"
"	# Create a channel\n"
"	op.createChannel('position')\n"
"	\n"
"	# Set keyframes in the channel\n"
"	op.setKeyframe('position', 0.0, 0.0)  # At time 0, value 0\n"
"	op.setKeyframe('position', 1.0, 1.0)  # At time 1, value 1\n"
"	op.setKeyframe('position', 2.0, 0.0)  # At time 2, value 0\n"
"\n"
"# Create a multi-channel animation\n"
"def createMultiChannelAnimation(op):\n"
"	# Create multiple channels\n"
"	op.createChannel('tx')\n"
"	op.createChannel('ty')\n"
"	op.createChannel('tz')\n"
"	\n"
"	# Set keyframes in each channel\n"
"	# X position - move left to right\n"
"	op.setKeyframe('tx', 0.0, -1.0)\n"
"	op.setKeyframe('tx', 2.0, 1.0)\n"
"	\n"
"	# Y position - move up and down\n"
"	op.setKeyframe('ty', 0.0, 0.0)\n"
"	op.setKeyframe('ty', 1.0, 1.0)\n"
"	op.setKeyframe('ty', 2.0, 0.0)\n"
"	\n"
"	# Z position - move forward and back\n"
"	op.setKeyframe('tz', 0.0, 0.0)\n"
"	op.setKeyframe('tz', 1.0, -0.5)\n"
"	op.setKeyframe('tz', 2.0, 0.0)\n"
"\n"
"# Add multiple keyframes at once\n"
"def addMultipleKeyframes(op):\n"
"	# Create a channel\n"
"	op.createChannel('wave')\n"
"	\n"
"	# Create time-value pairs for a wave pattern\n"
"	keyframes = []\n"
"	for i in range(21):\n"
"		t = i * 0.1  # Time from 0 to 2.0\n"
"		v = math.sin(t * math.pi)  # Value between -1 and 1\n"
"		keyframes.append((t, v))\n"
"	\n"
"	# Set all keyframes at once\n"
"	op.setKeyframes('wave', keyframes)\n"
"\n"
"# Evaluate animation at specific times\n"
"def evaluateAnimationExample(op):\n"
"	# Create a simple animation\n"
"	op.createChannel('bounce')\n"
"	op.setKeyframe('bounce', 0.0, 0.0)\n"
"	op.setKeyframe('bounce', 0.5, 1.0)\n"
"	op.setKeyframe('bounce', 1.0, 0.0)\n"
"	\n"
"	# Evaluate at specific time\n"
"	value = op.evaluateChannel('bounce', 0.25)  # Should be around 0.5\n"
"	print(f'Bounce animation at t=0.25: {value}')\n"
"	\n"
"	# Evaluate all channels\n"
"	allValues = op.evaluateAllChannels(0.25)\n"
"	for channel, value in allValues.items():\n"
"		print(f'Channel {channel} at t=0.25: {value}')\n"
"\n"
"# Example using custom tangent handles\n"
"def createAdvancedAnimation(op):\n"
"	# Create a channel\n"
"	op.createChannel('custom_ease')\n"
"	\n"
"	# Set keyframes with custom tangent handles\n"
"	# Format: setKeyframe(channel, time, value, mode, in_tangent_time, in_tangent_value, out_tangent_time, out_tangent_value)\n"
"	\n"
"	# First keyframe with custom out tangent (flat start, then steep acceleration)\n"
"	op.setKeyframe('custom_ease', 0.0, 0.0, 2, 0.0, 0.0, 0.3, 0.0)  # TangentMode::smoothManual=2\n"
"	\n"
"	# Middle keyframe with custom in/out tangents for an overshoot effect\n"
"	op.setKeyframe('custom_ease', 1.0, 1.0, 2, 0.8, 1.2, 1.2, 0.8)\n"
"	\n"
"	# Final keyframe with custom in tangent\n"
"	op.setKeyframe('custom_ease', 2.0, 0.0, 2, 1.7, 0.2, 2.0, 0.0)\n"
"	\n"
"	# You can also use different tangent modes:\n"
"	# 0 = linear (straight lines between keyframes)\n"
"	# 1 = flat (flat tangents)\n"
"	# 2 = smoothManual (manually positioned tangent handles)\n"
"	# 3 = smoothAuto (automatically calculated smooth tangents)\n"
"	# 4 = stepped (value jumps at keyframes)\n"
"	\n"
"	# Evaluate and print the curve at several points\n"
"	print(f\"Custom easing curve values:\")\n"
"	for t in [0.0, 0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 1.75, 2.0]:\n"
"		value = op.evaluateChannel('custom_ease', t)\n"
"		print(f\"  t={t:.2f}: {value:.3f}\")\n"
"";

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

DLLEXPORT
void
FillCHOPPluginInfo(CHOP_PluginInfo *info)
{
	// Always set this to CHOPCPlusPlusAPIVersion.
	info->apiVersion = CHOPCPlusPlusAPIVersion;

	// The opType is the unique name for this BasicCHOP. It must start with a 
	// capital A-Z character, and all the following characters must lower case
	// or numbers (a-z, 0-9)
	info->customOPInfo.opType->setString("Animationchop");

	// The opLabel is the text that will show up in the OP Create Dialog
	info->customOPInfo.opLabel->setString("Animation CHOP");

	// Will be turned into a 3 letter icon on the nodes
	info->customOPInfo.opIcon->setString("ANM");

	// Information about the author of this OP
	info->customOPInfo.authorName->setString("Keith Lostracco");
	info->customOPInfo.authorEmail->setString("keith@actualize.vision");
	
	// This CHOP can work with 0 inputs
	info->customOPInfo.minInputs = 0;

	// It can accept up to 1 input though, which changes it's behavior
	info->customOPInfo.maxInputs = 1;

	info->customOPInfo.pythonVersion->setString(PY_VERSION);
	info->customOPInfo.pythonMethods = methods;
	info->customOPInfo.pythonGetSets = getSets;
	info->customOPInfo.pythonCallbacksDAT = PythonCallbacksDATStubs;
}

DLLEXPORT
CHOP_CPlusPlusBase*
CreateCHOPInstance(const OP_NodeInfo* info)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per CHOP that is using the .dll
	return new AnimationCHOP(info);
}

DLLEXPORT
void
DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the CHOP using that instance is deleted, or
	// if the CHOP loads a different DLL
	delete (AnimationCHOP*)instance;
}

};


AnimationCHOP::AnimationCHOP(const OP_NodeInfo* info) : m_nodeInfo(info)
{
	m_executeCount = 0;
	m_offset = 0.0;
	m_speedMod = 1.0;
}

AnimationCHOP::~AnimationCHOP()
{
}

void
AnimationCHOP::resetFilter()
{
	m_offset = 0;
}

void
AnimationCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = true;

	// Note: To disable timeslicing you'll need to turn this off, as well as ensure that
	// getOutputInfo() returns true, and likely also set the info->numSamples to how many
	// samples you want to generate for this CHOP. Otherwise it'll take on length of the
	// input CHOP, which may be timesliced.
	// ginfo->timeslice = true;

	// ginfo->inputMatchIndex = 0;
}

bool
AnimationCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
	// // If there is an input connected, we are going to match it's channel names etc
	// // otherwise we'll specify our own.
	// if (inputs->getNumInputs() > 0)
	// {
	// 	return false;
	// }
	// else
	// {
	// 	// Set the number of channels to the number of animation channels we have
	// 	// If we don't have any channels, we'll just output a default channel
	// 	size_t numAnimChannels = m_animation.get_channel_count();
	// 	info->numChannels = numAnimChannels > 0 ? static_cast<int32_t>(numAnimChannels) : 1;

	// 	// Since we are outputting a timeslice, the system will dictate
	// 	// the numSamples and startIndex of the CHOP data
	// 	//info->numSamples = 1;
	// 	//info->startIndex = 0

	// 	// For illustration we are going to output 120hz data
	// 	info->sampleRate = 120;
	// 	return true;
	// }

	info->sampleRate = 60.0f;
	info->numChannels = m_animation.get_channel_count();
	info->numSamples = m_animation.num_samples(info->sampleRate);
	return true;

}

void
AnimationCHOP::getChannelName(int32_t index, OP_String *name, const OP_Inputs* inputs, void* reserved1)
{
	// If we have animation channels, use their names
	std::vector<std::string> channelNames = m_animation.get_channel_names();
	
	if (index < static_cast<int32_t>(channelNames.size())) {
		name->setString(channelNames[index].c_str());
	} else {
		// Default fallback name
		name->setString("chan1");
	}
}

void
AnimationCHOP::execute(CHOP_Output* output, const OP_Inputs* inputs, void* reserved1)
{
	m_executeCount++;
	
	auto num_anim_channels = m_animation.get_channel_count();
	auto num_anim_samples = m_animation.num_samples(output->sampleRate);
	for (int i = 0 ; i < output->numChannels; i++) {
		if (i < num_anim_channels) {
			for (int j = 0; j < output->numSamples; j++) {
				if (j < num_anim_samples) {
					auto channel = m_animation.get_channel(i);
					if (channel) {
						output->channels[i][j] = static_cast<float>(channel->evaluate(j / output->sampleRate));
					} else {
						output->channels[i][j] = 0.0f;
					}
				}
			}
		}
	}

// 	double	 scale = inputs->getParDouble("Scale");

// 	// In this case we'll just take the first input and re-output it scaled.

// 	if (inputs->getNumInputs() > 0)
// 	{
// 		// We know the first CHOP has the same number of channels
// 		// because we returned false from getOutputInfo. 

// 		inputs->enablePar("Speed", 0);		inputs->enablePar("Reset", 0);		inputs->enablePar("Shape", 0);
// 		int ind = 0;
// 		const OP_CHOPInput	*cinput = inputs->getInputCHOP(0);

// 		for (int i = 0 ; i < output->numChannels; i++)
// 		{
// 			for (int j = 0; j < output->numSamples; j++)
// 			{
// 				output->channels[i][j] = float(cinput->getChannelData(i)[ind] * scale);
// 				ind++;

// 				// Make sure we don't read past the end of the CHOP input
// 				ind = ind % cinput->numSamples;
// 			}
// 		}

// 	}	else // If not input is connected, output our animation channels
// 	{
// 		inputs->enablePar("Speed", 1);
// 		inputs->enablePar("Reset", 1);

// 		double speed = inputs->getParDouble("Speed");

// 		// Apply Python class modifications
// 		speed *= m_speedMod;

// 		// We'll only be adding one extra argument
// 		PyObject* args = m_nodeInfo->context->createArgumentsTuple(1, nullptr);
// 		// The first argument is already set to the 'op' variable, so we set the second argument to our speed value
// 		PyTuple_SET_ITEM(args, 1, PyFloat_FromDouble(speed));

// 		PyObject *result = m_nodeInfo->context->callPythonCallback("getSpeedAdjust", args, nullptr, nullptr);
// 		// callPythonCallback doesn't take ownership of the args
// 		Py_DECREF(args);

// 		// We own result now, so we need to Py_DECREF it unless we want to hold onto it
// 		if (result)
// 		{
// 			// If we got a float back, replace our current speed with the returned on
// 			if (PyFloat_Check(result))
// 			{
// 				speed = PyFloat_AsDouble(result);
// 			}
// 			Py_DECREF(result);
// 		}

// 		double step = speed * 0.01f;
		
// 	// Get the current time from the inputs
// 		double currentTime = inputs->getTimeInfo()->frame;
		
// 		// If we have animation channels, output their values at the current time
// 		std::vector<std::string> channelNames = m_animation.get_channel_names();
		
// 		if (!channelNames.empty())
// 		{
// 			// We have animation channels, evaluate them at current time
// 			std::map<std::string, double> channelValues = m_animation.evaluate_channels(currentTime);
			
// 			for (int i = 0; i < output->numChannels; i++)
// 			{
// 				// Get the channel name for this index
// 				std::string channelName = (i < static_cast<int>(channelNames.size())) ? 
//                                          channelNames[i] : "default";
				
// 				// Get the evaluated value for this channel
// 				double value = (channelValues.find(channelName) != channelValues.end()) ? 
// 					channelValues[channelName] * scale : 0.0;
				
// 				// Output the value for all samples
// 				for (int j = 0; j < output->numSamples; j++)
// 				{
// 					output->channels[i][j] = float(value);
// 				}
// 			}
// 		}
// 		else
// 		{
// 			// No animation channels, output a default sine wave as before
// 			// menu items can be evaluated as either an integer menu position, or a string
// 			int shape = inputs->getParInt("Shape");
			
// 			// keep each channel at a different phase
// 			double phase = 2.0f * 3.14159f / (float)(output->numChannels);
			
// 			for (int i = 0; i < output->numChannels; i++)
// 			{
// 				double offset = m_offset + phase*i;
// 				double v = 0.0f;
// 				switch(shape)
// 				{
// 					case 0:		// sine
// 						v = sin(offset);
// 						break;

// 					case 1:		// square
// 						v = fabs(fmod(offset, 1.0)) > 0.5;
// 						break;

// 					case 2:		// ramp	
// 						v = fabs(fmod(offset, 1.0));
// 						break;
// 				}
// 				v *= scale;
// 				for (int j = 0; j < output->numSamples; j++)
// 				{
// 					output->channels[i][j] = float(v);
// 					offset += step;
// 				}
// 			}
// 			m_offset += step * output->numSamples; 
// 		}
// 	}
}

int32_t
AnimationCHOP::getNumInfoCHOPChans(void * reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 2;
}

void
AnimationCHOP::getInfoCHOPChan(int32_t index,
										OP_InfoCHOPChan* chan,
										void* reserved1)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = (float)m_executeCount;
	}

	if (index == 1)
	{
		chan->name->setString("offset");
		chan->value = (float)m_offset;
	}
}

bool		
AnimationCHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 2;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
AnimationCHOP::getInfoDATEntries(int32_t index,
										int32_t nEntries,
										OP_InfoDATEntries* entries, 
										void* reserved1)
{
	char tempBuffer[4096];

	if (index == 0)
	{
		// Set the value for the first column
		entries->values[0]->setString("executeCount");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", m_executeCount);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%d", m_executeCount);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 1)
	{
		// Set the value for the first column
		entries->values[0]->setString("offset");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%g", m_offset);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", m_offset);
#endif
		entries->values[1]->setString( tempBuffer);
	}
}

void
AnimationCHOP::setupParameters(OP_ParameterManager* manager, void *reserved1)
{
	// speed
	{
		OP_NumericParameter	np;

		np.name = "Speed";
		np.label = "Speed";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = -10.0;
		np.maxSliders[0] =  10.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// scale
	{
		OP_NumericParameter	np;

		np.name = "Scale";
		np.label = "Scale";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = -10.0;
		np.maxSliders[0] =  10.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// shape
	{
		OP_StringParameter	sp;

		sp.name = "Shape";
		sp.label = "Shape";

		sp.defaultValue = "Sine";

		const char *names[] = { "Sine", "Square", "Ramp" };
		const char *labels[] = { "Sine", "Square", "Ramp" };

		OP_ParAppendResult res = manager->appendMenu(sp, 3, names, labels);
		assert(res == OP_ParAppendResult::Success);
	}

	// pulse
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "Reset";
		
		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}

}



/*** Channel and Keyframe Management Implementation ***/

// Channel management methods
bool 
AnimationCHOP::createChannel(const std::string& name) 
{
    // Check if channel already exists
    if (m_animation.get_channel(name) != nullptr) {
        return false;
    }
    
    // Create and add new channel
    anim::Channel channel(name);
    m_animation.add_channel(channel);
    return true;
}

bool 
AnimationCHOP::removeChannel(const std::string& name) 
{
    return m_animation.remove_channel(name);
}

bool 
AnimationCHOP::removeChannelByIndex(size_t index) 
{
    return m_animation.remove_channel(index);
}

int 
AnimationCHOP::addChannels(const std::vector<std::string>& channelNames) 
{
    int addedCount = 0;
    
    for (const auto& name : channelNames) {
        if (createChannel(name)) {
            addedCount++;
        }
    }
    
    return addedCount;
}

int 
AnimationCHOP::removeChannels(const std::vector<std::string>& channelNames) 
{
    int removedCount = 0;
    
    for (const auto& name : channelNames) {
        if (removeChannel(name)) {
            removedCount++;
        }
    }
    
    return removedCount;
}

// Keyframe management methods
bool 
AnimationCHOP::setKeyframe(const std::string& channelName, double time, double value, 
                         anim::TangentMode mode, double in_tangent_time, double in_tangent_value,
                         double out_tangent_time, double out_tangent_value) 
{
    auto* channel = m_animation.get_channel(channelName);
    if (!channel) {
        return false;
    }
    
    // Create tangent handles from provided values or use defaults
    anim::Point2D in_tangent;
    anim::Point2D out_tangent;
    
    // If the tangent parameters are zero (default), calculate default values
    if (in_tangent_time == 0 && in_tangent_value == 0) {
        in_tangent = anim::Point2D(time - 0.1, value);
    } else {
        in_tangent = anim::Point2D(in_tangent_time, in_tangent_value);
    }
    
    if (out_tangent_time == 0 && out_tangent_value == 0) {
        out_tangent = anim::Point2D(time + 0.1, value);
    } else {
        out_tangent = anim::Point2D(out_tangent_time, out_tangent_value);
    }
    
    channel->set_keyframe(time, value, in_tangent, out_tangent, mode);
    return true;
}

bool 
AnimationCHOP::setKeyframeInChannel(size_t channelIndex, double time, double value, 
                                  anim::TangentMode mode, double in_tangent_time, double in_tangent_value,
                                  double out_tangent_time, double out_tangent_value) 
{
    auto* channel = m_animation.get_channel(channelIndex);
    if (!channel) {
        return false;
    }
    
    // Create tangent handles from provided values or use defaults
    anim::Point2D in_tangent;
    anim::Point2D out_tangent;
    
    // If the tangent parameters are zero (default), calculate default values
    if (in_tangent_time == 0 && in_tangent_value == 0) {
        in_tangent = anim::Point2D(time - 0.1, value);
    } else {
        in_tangent = anim::Point2D(in_tangent_time, in_tangent_value);
    }
    
    if (out_tangent_time == 0 && out_tangent_value == 0) {
        out_tangent = anim::Point2D(time + 0.1, value);
    } else {
        out_tangent = anim::Point2D(out_tangent_time, out_tangent_value);
    }
    
    channel->set_keyframe(time, value, in_tangent, out_tangent, mode);
    return true;
}

bool 
AnimationCHOP::removeKeyframe(const std::string& channelName, double time) 
{
    auto* channel = m_animation.get_channel(channelName);
    if (!channel) {
        return false;
    }
    
    return channel->remove_keyframe(time);
}

bool 
AnimationCHOP::removeKeyframeFromChannel(size_t channelIndex, double time) 
{
    auto* channel = m_animation.get_channel(channelIndex);
    if (!channel) {
        return false;
    }
    
    return channel->remove_keyframe(time);
}

int 
AnimationCHOP::setKeyframes(const std::string& channelName, 
                         const std::vector<std::pair<double, double>>& timeValuePairs,
                         anim::TangentMode mode) 
{
    auto* channel = m_animation.get_channel(channelName);
    if (!channel) {
        return 0;
    }
    
    int setCount = 0;
    for (const auto& [time, value] : timeValuePairs) {
        anim::Point2D in_tangent(time - 0.1, value);
        anim::Point2D out_tangent(time + 0.1, value);
        
        channel->set_keyframe(time, value, in_tangent, out_tangent, mode);
        setCount++;
    }
    
    return setCount;
}

int 
AnimationCHOP::setKeyframesInChannel(size_t channelIndex, 
                                  const std::vector<std::pair<double, double>>& timeValuePairs,
                                  anim::TangentMode mode) 
{
    auto* channel = m_animation.get_channel(channelIndex);
    if (!channel) {
        return 0;
    }
    
    int setCount = 0;
    for (const auto& [time, value] : timeValuePairs) {
        anim::Point2D in_tangent(time - 0.1, value);
        anim::Point2D out_tangent(time + 0.1, value);
        
        channel->set_keyframe(time, value, in_tangent, out_tangent, mode);
        setCount++;
    }
    
    return setCount;
}

int 
AnimationCHOP::removeKeyframes(const std::string& channelName, const std::vector<double>& times) 
{
    auto* channel = m_animation.get_channel(channelName);
    if (!channel) {
        return 0;
    }
    
    int removedCount = 0;
    for (double time : times) {
        if (channel->remove_keyframe(time)) {
            removedCount++;
        }
    }
    
    return removedCount;
}

int 
AnimationCHOP::removeKeyframesFromChannel(size_t channelIndex, const std::vector<double>& times) 
{
    auto* channel = m_animation.get_channel(channelIndex);
    if (!channel) {
        return 0;
    }
    
    int removedCount = 0;
    for (double time : times) {
        if (channel->remove_keyframe(time)) {
            removedCount++;
        }
    }
    
    return removedCount;
}

// Evaluation methods
double 
AnimationCHOP::evaluateChannel(const std::string& channelName, double time) 
{
    auto* channel = m_animation.get_channel(channelName);
    if (!channel) {
        return 0.0;
    }
    
    return channel->evaluate(time);
}

std::map<std::string, double> 
AnimationCHOP::evaluateAllChannels(double time) 
{
    return m_animation.evaluate_channels(time);
}

// Query methods
size_t 
AnimationCHOP::getChannelCount() const 
{
    return m_animation.get_channel_count();
}

std::vector<std::string> 
AnimationCHOP::getChannelNames() const 
{
    return m_animation.get_channel_names();
}

bool 
AnimationCHOP::channelExists(const std::string& name) const 
{
    return m_animation.get_channel(name) != nullptr;
}

size_t 
AnimationCHOP::getKeyframeCount(const std::string& channelName) const 
{
    const auto* channel = m_animation.get_channel(channelName);
    if (!channel) {
        return 0;
    }
    
    return channel->get_all_keyframes().size();
}

// Python bindings for animation methods

// Channel methods
static PyObject*
pyCreateChannel(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    const char* name;
    
    if (!PyArg_ParseTuple(args, "s", &name))
    {
        return nullptr;
    }
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst)
    {
        return nullptr;
    }
    
    bool success = inst->createChannel(name);
    me->context->makeNodeDirty();
    
    return PyBool_FromLong(success);
}

static PyObject*
pyRemoveChannel(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    const char* name;
    
    if (!PyArg_ParseTuple(args, "s", &name))
    {
        return nullptr;
    }
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst)
    {
        return nullptr;
    }
    
    bool success = inst->removeChannel(name);
    me->context->makeNodeDirty();
    
    return PyBool_FromLong(success);
}

static PyObject*
pyGetChannelNames(PyObject* self)
{
    PY_Struct* me = (PY_Struct*)self;
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst)
    {
        return nullptr;
    }
    
    std::vector<std::string> names = inst->getChannelNames();
    PyObject* namesList = PyList_New(names.size());
    
    for (size_t i = 0; i < names.size(); i++)
    {
        PyList_SetItem(namesList, i, PyUnicode_FromString(names[i].c_str()));
    }
    
    return namesList;
}

// Keyframe methods
static PyObject*
pySetKeyframe(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    const char* name;
    double time, value;
    int mode = static_cast<int>(anim::TangentMode::smoothAuto); // Default to smoothAuto
    double in_tangent_time = 0.0;
    double in_tangent_value = 0.0;
    double out_tangent_time = 0.0;
    double out_tangent_value = 0.0;
    
    // Parse the arguments: channel name, time, value, [mode, in_tangent_time, in_tangent_value, out_tangent_time, out_tangent_value]
    if (!PyArg_ParseTuple(args, "sdd|idddd", &name, &time, &value, &mode, 
                         &in_tangent_time, &in_tangent_value, &out_tangent_time, &out_tangent_value))
    {
        return nullptr;
    }
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst)
    {
        return nullptr;
    }
    
    bool success = inst->setKeyframe(name, time, value, 
                                   static_cast<anim::TangentMode>(mode),
                                   in_tangent_time, in_tangent_value, 
                                   out_tangent_time, out_tangent_value);
    me->context->makeNodeDirty();
    
    return PyBool_FromLong(success);
}

static PyObject*
pyRemoveKeyframe(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    const char* name;
    double time;
    
    if (!PyArg_ParseTuple(args, "sd", &name, &time))
    {
        return nullptr;
    }
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst)
    {
        return nullptr;
    }
    
    bool success = inst->removeKeyframe(name, time);
    me->context->makeNodeDirty();
    
    return PyBool_FromLong(success);
}

static PyObject*
pySetKeyframes(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    const char* name;
    PyObject* timeValueList;
    int mode = static_cast<int>(anim::TangentMode::smoothAuto); // Default to smoothAuto
    
    if (!PyArg_ParseTuple(args, "sO|i", &name, &timeValueList, &mode))
    {
        return nullptr;
    }
    
    // Check if we got a proper list
    if (!PyList_Check(timeValueList))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a list of (time, value) pairs");
        return nullptr;
    }
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst)
    {
        return nullptr;
    }
    
    std::vector<std::pair<double, double>> timeValuePairs;
    Py_ssize_t size = PyList_Size(timeValueList);
    
    for (Py_ssize_t i = 0; i < size; i++)
    {
        PyObject* item = PyList_GetItem(timeValueList, i);
        
        // Each item should be a tuple with two elements
        if (!PyTuple_Check(item) || PyTuple_Size(item) != 2)
        {
            PyErr_SetString(PyExc_TypeError, "Each item must be a (time, value) tuple");
            return nullptr;
        }
        
        PyObject* timeObj = PyTuple_GetItem(item, 0);
        PyObject* valueObj = PyTuple_GetItem(item, 1);
        
        if (!PyFloat_Check(timeObj) && !PyLong_Check(timeObj))
        {
            PyErr_SetString(PyExc_TypeError, "Time must be a number");
            return nullptr;
        }
        
        if (!PyFloat_Check(valueObj) && !PyLong_Check(valueObj))
        {
            PyErr_SetString(PyExc_TypeError, "Value must be a number");
            return nullptr;
        }
        
        double time = PyFloat_AsDouble(timeObj);
        double value = PyFloat_AsDouble(valueObj);
        
        timeValuePairs.push_back(std::make_pair(time, value));
    }
    
    int setCount = inst->setKeyframes(name, timeValuePairs, static_cast<anim::TangentMode>(mode));
    me->context->makeNodeDirty();
    
    return PyLong_FromLong(setCount);
}

static PyObject*
pyRemoveKeyframes(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    const char* name;
    PyObject* timeList;
    
    if (!PyArg_ParseTuple(args, "sO", &name, &timeList))
    {
        return nullptr;
    }
    
    // Check if we got a proper list
    if (!PyList_Check(timeList))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a list of times");
        return nullptr;
    }
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst)
    {
        return nullptr;
    }
    
    std::vector<double> times;
    Py_ssize_t size = PyList_Size(timeList);
    
    for (Py_ssize_t i = 0; i < size; i++)
    {
        PyObject* item = PyList_GetItem(timeList, i);
        
        if (!PyFloat_Check(item) && !PyLong_Check(item))
        {
            PyErr_SetString(PyExc_TypeError, "Each time must be a number");
            return nullptr;
        }
        
        double time = PyFloat_AsDouble(item);
        times.push_back(time);
    }
    
    int removedCount = inst->removeKeyframes(name, times);
    me->context->makeNodeDirty();
    
    return PyLong_FromLong(removedCount);
}

static PyObject*
pyEvaluateChannel(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    const char* name;
    double time;
    
    if (!PyArg_ParseTuple(args, "sd", &name, &time))
    {
        return nullptr;
    }
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst)
    {
        return nullptr;
    }
    
    double value = inst->evaluateChannel(name, time);
    
    return PyFloat_FromDouble(value);
}

static PyObject*
pyEvaluateAllChannels(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    double time;
    
    if (!PyArg_ParseTuple(args, "d", &time))
    {
        return nullptr;
    }
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst)
    {
        return nullptr;
    }
    
    std::map<std::string, double> values = inst->evaluateAllChannels(time);
    PyObject* dict = PyDict_New();
    
    for (const auto& pair : values)
    {
        PyDict_SetItemString(dict, pair.first.c_str(), PyFloat_FromDouble(pair.second));
    }
    
    return dict;
}

void
AnimationCHOP::pulsePressed(const char* name, void* reserved1)
{
    // Handle parameter pulses here
    if (strcmp(name, "Reset") == 0)
    {
        // Reset the filter
        resetFilter();
    }
}

