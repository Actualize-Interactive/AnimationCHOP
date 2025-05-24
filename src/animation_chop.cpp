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
#include "py_anim_bindings/py_bindings.h"


#include <stdio.h>
#include <string.h>
#include <cmath>
#include <assert.h>
#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
	#include <modsupport.h>

#else
	#include <Python/Python.h>
	#include <Python/structmember.h>
#endif


static PyMethodDef methods[] =
{
	{"create_channel", (PyCFunction)pyCreateChannel, METH_VARARGS, "Creates a new animation channel with the given name."},
	{"get_channel", (PyCFunction)pyGetChannel, METH_VARARGS, "Gets an animation channel by name."},
	{"remove_channel", (PyCFunction)pyRemoveChannel, METH_VARARGS, "Removes an animation channel by name."},
	{"get_channel_names", (PyCFunction)pyGetChannelNames, METH_NOARGS, "Returns a list of all channel names."},

	{"set_keyframe", (PyCFunction)pySetKeyframe, METH_VARARGS, "Sets a keyframe in a channel."},
	{"set_keyframe_at_time", (PyCFunction)pySetKeyframeAtTime, METH_VARARGS, "Sets a keyframe in a channel with control over handle points."},
	{"get_keyframe", (PyCFunction)pyGetKeyframe, METH_VARARGS, "Gets a keyframe from a channel at the specified time."},
	{"get_keyframe_at_time", (PyCFunction)pyGetKeyframeAtTime, METH_VARARGS, "Gets a keyframe from a channel at the specified time."},
	{"has_keyframe", (PyCFunction)pyHasKeyframe, METH_VARARGS, "Checks if a keyframe exists at the specified time."},
	{"has_keyframe_at_time", (PyCFunction)pyHasKeyframeAtTime, METH_VARARGS, "Checks if a keyframe exists at the specified time."},
	{"remove_keyframe", (PyCFunction)pyRemoveKeyframe, METH_VARARGS, "Removes a keyframe from a channel."},
	{"remove_keyframe_at_time", (PyCFunction)pyRemoveKeyframeAtTime, METH_VARARGS, "Removes a keyframe from a channel at the specified time."},

	{"set_keyframes", (PyCFunction)pySetKeyframes, METH_VARARGS, "Sets multiple keyframes in a channel."},
	{"set_keyframes_at_time", (PyCFunction)pySetKeyframesAtTime, METH_VARARGS, "Sets multiple keyframes in a channel."},
	{"remove_keyframes_at_time", (PyCFunction)pyRemoveKeyframesAtTime, METH_VARARGS, "Removes multiple keyframes from a channel."},
	{0}
};

// This struct lists the different getters and/or settings the Custom Operator will expose.
static PyGetSetDef getSets[] =
{
    {"Point2D", get_point2d_type, nullptr, "Point2D type for representing time-value pairs.", nullptr},
    {"BezierHandle", get_bezier_handle_type, nullptr, "BezierHandle type for representing handle control points.", nullptr},
    {"TangentMode", get_tangent_mode_enum, nullptr, "TangentMode enum for keyframe handle behavior.", nullptr},
    {"Keyframe", get_keyframe_type, nullptr, "Keyframe type for animation curves.", nullptr},
    {"Channel", get_channel_type, nullptr, "Channel type for animation data.", nullptr},
    {"Animation", get_animation_type, nullptr, "Animation container for channels and keyframes.", nullptr},
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
"\n";

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


AnimationCHOP::AnimationCHOP(const OP_NodeInfo* info) 
	: m_nodeInfo(info)
	, m_warning(nullptr)
	, m_error(nullptr)
	, m_animation()
{
}

AnimationCHOP::~AnimationCHOP()
{
}


void
AnimationCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, [[maybe_unused]] const OP_Inputs* inputs, [[maybe_unused]] void* reserved1)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = false;

	// Note: To disable timeslicing you'll need to turn this off, as well as ensure that
	// getOutputInfo() returns true, and likely also set the info->numSamples to how many
	// samples you want to generate for this CHOP. Otherwise it'll take on length of the
	// input CHOP, which may be timesliced.
	// ginfo->timeslice = true;

	// ginfo->inputMatchIndex = 0;
}

bool
AnimationCHOP::getOutputInfo(CHOP_OutputInfo* info, [[maybe_unused]] const OP_Inputs* inputs, [[maybe_unused]] void* reserved1)
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
	info->numChannels = static_cast<int32_t>(m_animation.get_channel_count());
	info->numSamples = m_animation.num_samples(info->sampleRate);
	return true;

}

void
AnimationCHOP::getChannelName(int32_t index, OP_String *name, [[maybe_unused]]const OP_Inputs* inputs, [[maybe_unused]]void* reserved1)
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
AnimationCHOP::execute(CHOP_Output* output, [[maybe_unused]] const OP_Inputs* inputs, [[maybe_unused]]void* reserved1)
{
	m_error = nullptr;
	m_warning = nullptr;

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
}

void 
AnimationCHOP::getWarningString(OP_String *warning, [[maybe_unused]] void* reserved1)
{
	warning->setString(m_warning);
}

void
AnimationCHOP::getErrorString(OP_String *error, [[maybe_unused]] void* reserved1)
{
	error->setString(m_error);
}

void
AnimationCHOP::setupParameters(OP_ParameterManager* manager,[[maybe_unused]] void *reserved1)
{
	// speed
	{
		OP_NumericParameter	np;

		np.name = "Speed";
		np.label = "Speed";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = -10.0;
		np.maxSliders[0] =  10.0;
		
		[[maybe_unused]] OP_ParAppendResult res = manager->appendFloat(np);
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
		
		[[maybe_unused]] OP_ParAppendResult res = manager->appendFloat(np);
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

		[[maybe_unused]] OP_ParAppendResult res = manager->appendMenu(sp, 3, names, labels);
		assert(res == OP_ParAppendResult::Success);
	}


	// pulse
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "Reset";
		
		[[maybe_unused]] OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}

}

void
AnimationCHOP::pulsePressed(const char* name, [[maybe_unused]] void* reserved1)
{
    // Handle parameter pulses here
    if (strcmp(name, "Reset") == 0)
    {

    }
}



/*** Channel and Keyframe Management Implementation ***/

// Channel management methods
const anim::Channel*
AnimationCHOP::createChannel(const std::string& name, int32_t insertIndex) 
{
    // Check if channel already exists
    if (m_animation.get_channel(name) != nullptr) {
        return nullptr;
    }
    
	if (insertIndex < 0) {
		return m_animation.create_channel(name);
	} else {
		return m_animation.insert_channel(insertIndex, anim::Channel(name));
	}
}

bool 
AnimationCHOP::removeChannel(const std::string& name) 
{
    return m_animation.remove_channel(name);
}

bool 
AnimationCHOP::removeChannel(size_t index) 
{
    return m_animation.remove_channel(index);
}

void 
AnimationCHOP::createChannels(const std::vector<std::string>& channelNames) 
{
    for (const auto& name : channelNames) {
        if (createChannel(name)) {
        }
    }
}

bool 
AnimationCHOP::removeChannels(const std::vector<std::string>& channelNames) 
{
    bool allRemoved = true;

    for (const auto& name : channelNames) {
        if (!removeChannel(name)) {
            allRemoved = false;
        }
    }
    return allRemoved;
}

// Keyframe management methods
bool 
AnimationCHOP::setKeyframeAtTime(const std::string& channelName, double time, double value, 
                         anim::TangentMode mode, double in_handle_time, double in_handle_value,
                         double out_handle_time, double out_handle_value) 
{
    auto* channel = m_animation.get_channel(channelName);
    if (!channel) {
        return false;
    }
    channel->set_keyframe_at_time(time, value, 
		anim::BezierHandle(in_handle_time, in_handle_value), 
		anim::BezierHandle(out_handle_time, out_handle_value), 
		mode);
    return true;
}

bool 
AnimationCHOP::removeKeyframeAtTime(const std::string& channelName, double time) 
{
    auto* channel = m_animation.get_channel(channelName);
    if (!channel) {
        return false;
    }
    return channel->remove_keyframe_at_time(time);
}

bool 
AnimationCHOP::removeKeyframes(const std::string& channelName, const std::vector<double>& times) 
{
    auto* channel = m_animation.get_channel(channelName);
    if (!channel) {
        return 0;
    }
    
	bool removedAll = true;
	for (double time : times) {
		if (!channel->remove_keyframe_at_time(time)) {
			removedAll = false;
		}
	}
	return removedAll;
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
    return m_animation.has_channel(name);
}

size_t 
AnimationCHOP::getKeyframeCount(const std::string& channelName) const 
{
    auto channel = m_animation.get_channel(channelName);
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
    if (!PyArg_ParseTuple(args, "s", &name)) {
        return nullptr;
    }
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
    
    auto channel = inst->createChannel(name);
	if (!channel) {
		PyErr_SetString(PyExc_RuntimeError, "Failed to create channel");
		return nullptr;
	}
	me->context->makeNodeDirty();

	auto pyChannel = ChannelToPyObject(*channel);
	if (!pyChannel) {
		PyErr_SetString(PyExc_RuntimeError, "Failed to create Python channel object");
		return nullptr;
	}

	return pyChannel;
}

static PyObject*
pyGetChannel(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    PyObject* identifier_obj;
	if (!PyArg_ParseTuple(args, "O", &identifier_obj)) {
		return nullptr;
	}

	PY_GetInfo Info;
	Info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(Info);
	if (!inst) {
		return nullptr;
	}
	anim::Channel* channel = nullptr;

	if (PyUnicode_Check(identifier_obj)) {
		const char* name = PyUnicode_AsUTF8(identifier_obj);
		channel = inst->animation().get_channel(name);
	} else if (PyLong_Check(identifier_obj)) {
		int index = static_cast<int>(PyLong_AsLong(identifier_obj));
		channel = inst->animation().get_channel(index);
	} else {
		PyErr_SetString(PyExc_TypeError, "Identifier must be a string or an integer.");
		return nullptr;
	}
	if (!channel) {
		PyErr_SetString(PyExc_RuntimeError, "Failed to get channel");
		return nullptr;
	}
	PyObject* pyChannel = ChannelToPyObject(*channel);
	if (!pyChannel) {
		PyErr_SetString(PyExc_RuntimeError, "Failed to create Python channel object");
		return nullptr;
	}
	return pyChannel;
}

static PyObject*
pyRemoveChannel(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    const char* name;
    
    if (!PyArg_ParseTuple(args, "s", &name)) {
        return nullptr;
    }
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
    
    inst->removeChannel(name);
    me->context->makeNodeDirty();
    Py_RETURN_NONE;
}

static PyObject*
pyGetChannelNames(PyObject* self)
{
    PY_Struct* me = (PY_Struct*)self;
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
    
    std::vector<std::string> names = inst->getChannelNames();
    PyObject* namesList = PyList_New(names.size());
    
    for (size_t i = 0; i < names.size(); i++)  {
        PyList_SetItem(namesList, i, PyUnicode_FromString(names[i].c_str()));
    }
    return namesList;
}

static PyObject*
pySetKeyframe(PyObject* self, PyObject* args)
{
	PY_Struct* me = (PY_Struct*)self;
	const char* name;
	size_t index;
    double value;
    int mode = static_cast<int>(anim::TangentMode::smoothAuto); // Default to smoothAuto
    double in_handle_time = 0.0;
    double in_handle_value = 0.0;
    double out_handle_time = 0.0;
    double out_handle_value = 0.0;

	// Parse the arguments: channel name, index, time, value, [mode, in_handle_time, in_handle_value, out_handle_time, out_handle_value]
	if (!PyArg_ParseTuple(args, "sid|idddd", &name, &index, &value, &mode, 
		&in_handle_time, &in_handle_value, &out_handle_time, &out_handle_value)) {
		return nullptr;
	}

	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	if (!inst)
	{
		return nullptr;
	}

	auto channel = inst->animation().get_channel(name);
	if (!channel){
		return nullptr;
	}
	if (index < 0 || index >= channel->keyframe_count()) {
		PyErr_SetString(PyExc_IndexError, "Keyframe index out of range");
		return nullptr;
	}
	try {
		auto& keyframe = channel->get_keyframe(index);
		keyframe.set_value(value);
		keyframe.set_in_handle(anim::BezierHandle(in_handle_time, in_handle_value));
		keyframe.set_out_handle(anim::BezierHandle(out_handle_time, out_handle_value));
		keyframe.set_mode(static_cast<anim::TangentMode>(mode));
	}
	catch (const std::exception& e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
		return nullptr;
	}
	me->context->makeNodeDirty();
	Py_RETURN_NONE;
}

static PyObject*
pySetKeyframeAtTime(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    const char* name;
    double time, value;
    int mode = static_cast<int>(anim::TangentMode::smoothAuto); // Default to smoothAuto
    double in_handle_time = 0.0;
    double in_handle_value = 0.0;
    double out_handle_time = 0.0;
    double out_handle_value = 0.0;
    
    // Parse the arguments: channel name, time, value, [mode, in_handle_time, in_handle_value, out_handle_time, out_handle_value]
    if (!PyArg_ParseTuple(args, "sdd|idddd", &name, &time, &value, &mode, 
                         &in_handle_time, &in_handle_value, &out_handle_time, &out_handle_value)) {
        return nullptr;
    }
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
    
    bool success = inst->setKeyframeAtTime(name, time, value, 
                                   static_cast<anim::TangentMode>(mode),
                                   in_handle_time, in_handle_value, 
                                   out_handle_time, out_handle_value);
    me->context->makeNodeDirty();
    
    return PyBool_FromLong(success);
}

static PyObject* 
pyGetKeyframe(PyObject* self, PyObject* args)
{
	PY_Struct* me = (PY_Struct*)self;
	const char* name;
	size_t index;

	if (!PyArg_ParseTuple(args, "si", &name, &index)) {
		return nullptr;
	}

	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	if (!inst) {
		return nullptr;
	}

	auto channel = inst->animation().get_channel(name);
	if (!channel) {
		return nullptr;
	}

	if (index < 0 || index >= channel->keyframe_count()) {
		PyErr_SetString(PyExc_IndexError, "Keyframe index out of range");
		return nullptr;
	}

	auto keyframe = channel->get_keyframe(index);

	auto pyKeyframe = KeyframeToPyObject(keyframe);
	if (!pyKeyframe) {
		return nullptr;
	}
	return pyKeyframe;
}

static PyObject* 
pyGetKeyframeAtTime(PyObject* self, PyObject* args)
{
	PY_Struct* me = (PY_Struct*)self;
	const char* name;
	double time;

	if (!PyArg_ParseTuple(args, "sd", &name, &time)) {
		return nullptr;
	}

	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	if (!inst) {
		return nullptr;
	}

	auto channel = inst->animation().get_channel(name);
	if (!channel) {
		return nullptr;
	}

	auto res = channel->get_keyframe_at_time(time);
	if (!res) {
		Py_RETURN_NONE;
	}
	auto keyframe = res.value();

	auto pyKeyframe = KeyframeToPyObject(keyframe);
	if (!pyKeyframe) {
		return nullptr;
	}
	return pyKeyframe;
}

static PyObject* 
pyHasKeyframe(PyObject* self, PyObject* args)
{
	PY_Struct* me = (PY_Struct*)self;
	const char* name;
	size_t index;

	if (!PyArg_ParseTuple(args, "si", &name, &index)) {
		return nullptr;
	}

	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	if (!inst) {
		return nullptr;
	}

	auto channel = inst->animation().get_channel(name);
	if (!channel) {
		return nullptr;
	}

	auto has_keyframe = channel->has_keyframe(index);
	return PyBool_FromLong(has_keyframe ? 1 : 0);
}

static PyObject*
pyHasKeyframeAtTime(PyObject* self, PyObject* args)
{
	PY_Struct* me = (PY_Struct*)self;
	const char* name;
	double time;

	if (!PyArg_ParseTuple(args, "sd", &name, &time)) {
		return nullptr;
	}

	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	if (!inst) {
		return nullptr;
	}

	auto channel = inst->animation().get_channel(name);
	if (!channel) {
		return nullptr;
	}

	auto has_keyframe = channel->has_keyframe_at_time(time);
	return PyBool_FromLong(has_keyframe ? 1 : 0);
}

static PyObject* 
pyRemoveKeyframe(PyObject* self, PyObject* args)
{
	PY_Struct* me = (PY_Struct*)self;
	const char* name;
	size_t index;

	if (!PyArg_ParseTuple(args, "si", &name, &index)) {
		return nullptr;
	}

	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	if (!inst) {
		return nullptr;
	}

	auto channel = inst->animation().get_channel(name);
	if (!channel) {
		return nullptr;
	}

	channel->remove_keyframe(index);
	me->context->makeNodeDirty();

	Py_RETURN_NONE;
}

static PyObject*
pyRemoveKeyframeAtTime(PyObject* self, PyObject* args)
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

    bool success = inst->removeKeyframeAtTime(name, time);
    me->context->makeNodeDirty();
    
    return PyBool_FromLong(success);
}

static PyObject*
pySetKeyframes(PyObject* self, PyObject* args)
{
	PY_Struct* me = (PY_Struct*)self;
	const char* name;
	// a list of dicts 
	// {'index': 0.0, 'value': 1.0, 'mode': 0, 
	// 'in_handle_time': 0.0, 'in_handle_value': 0.0, 
	// 'out_handle_time': 0.0, 'out_handle_value': 0.0}
	PyObject* indexKeyframeList; 

	if (!PyArg_ParseTuple(args, "sO", &name, &indexKeyframeList)) {
		return nullptr;
	}

	if (!PyList_Check(indexKeyframeList)) {
		PyErr_SetString(PyExc_TypeError, "Expected a list of keyframes");
		return nullptr;
	}

	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	if (!inst) {
		return nullptr;
	}
	auto channel = inst->animation().get_channel(name);
	if (!channel) {
		PyErr_SetString(PyExc_RuntimeError, "Failed to get channel");
		return nullptr;
	}
	auto size = PyList_Size(indexKeyframeList);

	for (auto i = 0; i < size; i++)
	{
		PyObject* item = PyList_GetItem(indexKeyframeList, i);

		if (!PyDict_Check(item))
		{
			PyErr_SetString(PyExc_TypeError, "Each keyframe must be a dictionary");
			return nullptr;
		}
		// Extract the keyframe data from the dictionary
		PyObject* indexObj = PyDict_GetItemString(item, "index");
		PyObject* valueObj = PyDict_GetItemString(item, "value");
		if (!indexObj || !valueObj)
		{
			PyErr_SetString(PyExc_TypeError, "Keyframe must have 'index' and 'value' keys");
			return nullptr;
		}
		auto index = static_cast<size_t>(PyLong_AsLong(indexObj));
		if (index < 0 || index >= channel->keyframe_count()) {
			PyErr_SetString(PyExc_IndexError, "Keyframe index out of range");
			return nullptr;
		}
		auto value = PyFloat_AsDouble(valueObj);
		anim::TangentMode mode = anim::TangentMode::smoothAuto;
		PyObject* modeObj = PyDict_GetItemString(item, "mode");
		PyObject* inHandleTimeObj = PyDict_GetItemString(item, "in_handle_time");
		PyObject* inHandleValueObj = PyDict_GetItemString(item, "in_handle_value");
		PyObject* outHandleTimeObj = PyDict_GetItemString(item, "out_handle_time");
		PyObject* outHandleValueObj = PyDict_GetItemString(item, "out_handle_value");

		if (modeObj) {
			if (!PyLong_Check(modeObj)) {
				PyErr_SetString(PyExc_TypeError, "Keyframe 'mode' must be an integer");
				return nullptr;
			}
			mode = static_cast<anim::TangentMode>(PyLong_AsLong(modeObj));
		}
		double inHandleTime = 0.0;
		if (inHandleTimeObj) {
			inHandleTime = PyFloat_AsDouble(inHandleTimeObj);
		}
		double inHandleValue = 0.0;
		if (inHandleValueObj) {
			inHandleValue = PyFloat_AsDouble(inHandleValueObj);
		}
		double outHandleTime = 0.0;
		if (outHandleTimeObj) {
			outHandleTime = PyFloat_AsDouble(outHandleTimeObj);
		}
		double outHandleValue = 0.0;
		if (outHandleValueObj) {
			outHandleValue = PyFloat_AsDouble(outHandleValueObj);
		}

		auto& keyframe = channel->get_keyframe(index);
		keyframe.set_value(value);
		keyframe.set_in_handle(anim::BezierHandle(inHandleTime, inHandleValue));
		keyframe.set_out_handle(anim::BezierHandle(outHandleTime, outHandleValue));
		keyframe.set_mode(static_cast<anim::TangentMode>(mode));
		
	}

	me->context->makeNodeDirty();

	Py_RETURN_NONE;
}

static PyObject*
pySetKeyframesAtTime(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    const char* name;

	// a list of dicts 
	// {'time': 0.0, 'value': 1.0, 'mode': 0, 
	// 'in_handle_time': 0.0, 'in_handle_value': 0.0, 
	// 'out_handle_time': 0.0, 'out_handle_value': 0.0}
	PyObject* keyframeList; 

	if (!PyArg_ParseTuple(args, "sO", &name, &keyframeList)) {
		return nullptr;
	}
	
	if (!PyList_Check(keyframeList)) {
		PyErr_SetString(PyExc_TypeError, "Expected a list of keyframes");
		return nullptr;
	}
	
	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	if (!inst) {
		return nullptr;
	}

	auto channel = inst->animation().get_channel(name);
	if (!channel) {
		PyErr_SetString(PyExc_RuntimeError, "Failed to get channel");
		return nullptr;
	}
	
	Py_ssize_t size = PyList_Size(keyframeList);
	for (Py_ssize_t i = 0; i < size; i++)
	{
		PyObject* item = PyList_GetItem(keyframeList, i);
		
		if (!PyDict_Check(item))
		{
			PyErr_SetString(PyExc_TypeError, "Each keyframe must be a dictionary");
			return nullptr;
		}
		// Extract the keyframe data from the dictionary
		PyObject* timeObj = PyDict_GetItemString(item, "time");
		PyObject* valueObj = PyDict_GetItemString(item, "value");
		PyObject* modeObj = PyDict_GetItemString(item, "mode");
		PyObject* inHandleTimeObj = PyDict_GetItemString(item, "in_handle_time");
		PyObject* inHandleValueObj = PyDict_GetItemString(item, "in_handle_value");
		PyObject* outHandleTimeObj = PyDict_GetItemString(item, "out_handle_time");
		PyObject* outHandleValueObj = PyDict_GetItemString(item, "out_handle_value");
		if (!timeObj || !valueObj)
		{
			PyErr_SetString(PyExc_TypeError, "Keyframe must have 'time' and 'value' keys");
			return nullptr;
		}
		auto time = PyFloat_AsDouble(timeObj);
		auto value = PyFloat_AsDouble(valueObj);
		anim::TangentMode mode = anim::TangentMode::smoothAuto;
		if (modeObj) {
			if (!PyLong_Check(modeObj)) {
				PyErr_SetString(PyExc_TypeError, "Keyframe 'mode' must be an integer");
				return nullptr;
			}
			mode = static_cast<anim::TangentMode>(PyLong_AsLong(modeObj));
		}
		double inHandleTime = 0.0;
		if (inHandleTimeObj) {
			inHandleTime = PyFloat_AsDouble(inHandleTimeObj);
		}
		double inHandleValue = 0.0;
		if (inHandleValueObj) {
			inHandleValue = PyFloat_AsDouble(inHandleValueObj);
		}
		double outHandleTime = 0.0;
		if (outHandleTimeObj) {
			outHandleTime = PyFloat_AsDouble(outHandleTimeObj);
		}
		double outHandleValue = 0.0;
		if (outHandleValueObj) {
			outHandleValue = PyFloat_AsDouble(outHandleValueObj);
		}

		channel->set_keyframe_at_time(
			time, 
			value, 
			anim::BezierHandle(inHandleTime, inHandleValue), 
			anim::BezierHandle(outHandleTime, outHandleValue), 
			mode);
	}

	me->context->makeNodeDirty();

	Py_RETURN_NONE;
}

static PyObject*
pyRemoveKeyframesAtTime(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    const char* name;
    PyObject* timeList;
    
    if (!PyArg_ParseTuple(args, "sO", &name, &timeList)) {
        return nullptr;
    }
    
    // Check if we got a proper list
    if (!PyList_Check(timeList)) {
        PyErr_SetString(PyExc_TypeError, "Expected a list of times");
        return nullptr;
    }
    
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
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
    
    auto removed_all = inst->removeKeyframes(name, times);
    me->context->makeNodeDirty();

    return PyBool_FromLong(removed_all ? 1 : 0);
}


