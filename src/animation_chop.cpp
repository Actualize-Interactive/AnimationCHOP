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
	{"create_channel", (PyCFunction)py_createChannel, METH_VARARGS, "Creates a new animation channel with the given name."},
	{"get_channel", (PyCFunction)py_getChannel, METH_VARARGS, "Gets an animation channel by name."},
	{"remove_channel", (PyCFunction)py_removeChannel, METH_VARARGS, "Removes an animation channel by name."},
	{"get_channel_names", (PyCFunction)py_getChannelNames, METH_NOARGS, "Returns a list of all channel names."},
	{"clear_channels", (PyCFunction)py_clearChannels, METH_NOARGS, "Clears all animation channels."},
	{"get_animation", (PyCFunction)py_getAnimation, METH_NOARGS, "Gets the animation object."},

	{"set_keyframe", (PyCFunction)py_setKeyframe, METH_VARARGS, "Sets a keyframe in a channel."},
	{"set_keyframe_at_time", (PyCFunction)py_setKeyframeAtTime, METH_VARARGS, "Sets a keyframe in a channel with control over tangent handles."},
	{"get_keyframe", (PyCFunction)py_getKeyframe, METH_VARARGS, "Gets a keyframe from a channel at the specified time."},
	{"get_keyframe_at_time", (PyCFunction)py_getKeyframeAtTime, METH_VARARGS, "Gets a keyframe from a channel at the specified time."},
	{"has_keyframe", (PyCFunction)py_hasKeyframe, METH_VARARGS, "Checks if a keyframe exists at the specified time."},
	{"has_keyframe_at_time", (PyCFunction)py_hasKeyframeAtTime, METH_VARARGS, "Checks if a keyframe exists at the specified time."},
	{"remove_keyframe", (PyCFunction)py_removeKeyframe, METH_VARARGS, "Removes a keyframe from a channel."},
	{"remove_keyframe_at_time", (PyCFunction)py_removeKeyframeAtTime, METH_VARARGS, "Removes a keyframe from a channel at the specified time."},

	{"set_keyframes", (PyCFunction)py_setKeyframes, METH_VARARGS, "Sets multiple keyframes in a channel."},
	{"set_keyframes_at_time", (PyCFunction)py_setKeyframesAtTime, METH_VARARGS, "Sets multiple keyframes in a channel."},
	{"remove_keyframes_at_time", (PyCFunction)py_removeKeyframesAtTime, METH_VARARGS, "Removes multiple keyframes from a channel."},
	{"debug_channel", (PyCFunction)py_debugChannel, METH_VARARGS, "Debug function to inspect channel state."},
	{0}
};

// This struct lists the different getters and/or settings the Custom Operator will expose.
static PyGetSetDef getSets[] =
{
    {"Point", get_point_type, nullptr, "Point type for representing time-value pairs.", nullptr},
    {"HandleMode", get_handle_mode_enum, nullptr, "HandleMode enum for keyframe handle behavior.", nullptr},
    {"Function", get_function_enum, nullptr, "Function enum for keyframe interpolation type.", nullptr},
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
AnimationCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
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
AnimationCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
    info->sampleRate = 60.0f;
    info->numChannels = static_cast<int32_t>(m_animation.num_channels());

    // Compute the maximum channel length (end_time - start_time) across all channels
    double max_length = 0.0;
    for (size_t i = 0; i < m_animation.num_channels(); ++i) {
        try {
            const auto& channel = m_animation.channel(i);
            double length = channel.length();
            if (length > max_length) {
                max_length = length;
            }
        } catch (const std::out_of_range&) {
            // skip
        }
    }
    // If no channels, default to 1 sample
    if (max_length <= 0.0) {
        info->numSamples = 1;
    } else {
        // Output samples from t=0 to t=max_length at the current sample rate
        info->numSamples = static_cast<int32_t>(std::ceil(max_length * info->sampleRate));
        if (info->numSamples < 1) info->numSamples = 1;
    }
    return true;
}

void
AnimationCHOP::getChannelName(int32_t index, OP_String *name, const OP_Inputs* inputs, void* reserved1)
{
	// If we have animation channels, use their names
	if (index < static_cast<int32_t>(m_animation.num_channels())) {
		try {
			const auto& channel = m_animation.channel(index);
			name->setString(channel.name().c_str());
		} catch (const std::out_of_range&) {
			// Default fallback name
			name->setString("chan1");
		}
	} else {
		// Default fallback name
		name->setString("chan1");
	}
}

void
AnimationCHOP::execute(CHOP_Output* output, const OP_Inputs* inputs, void* reserved1)
{
	m_error = nullptr;
	m_warning = nullptr;

	size_t num_anim_channels = m_animation.num_channels();
	for (int i = 0; i < output->numChannels; i++) {
		if (i < static_cast<int>(num_anim_channels)) {
			try {
				const auto& channel = m_animation.channel(i);
				for (int j = 0; j < output->numSamples; j++) {
					double time = j / output->sampleRate;
					output->channels[i][j] = static_cast<float>(channel.evaluate(time));
				}
			} catch (const std::out_of_range&) {
				// Channel not found, fill with zeros
				for (int j = 0; j < output->numSamples; j++) {
					output->channels[i][j] = 0.0f;
				}
			}
		} else {
			// No animation channel for this output channel, fill with zeros
			for (int j = 0; j < output->numSamples; j++) {
				output->channels[i][j] = 0.0f;
			}
		}
	}
}

void 
AnimationCHOP::getWarningString(OP_String *warning, void* reserved1)
{
	warning->setString(m_warning);
}

void
AnimationCHOP::getErrorString(OP_String *error, void* reserved1)
{
	error->setString(m_error);
}

void
AnimationCHOP::setupParameters(OP_ParameterManager* manager,void *reserved1)
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

void
AnimationCHOP::pulsePressed(const char* name, void* reserved1)
{
    // Handle parameter pulses here
    if (strcmp(name, "Reset") == 0)
    {

    }
}



/*** Channel and Keyframe Management Implementation ***/

// Channel management methods
anim::Channel&
AnimationCHOP::createChannel(const std::string& name) 
{
    return m_animation.create_channel(name);
}

anim::Channel*
AnimationCHOP::getChannel(const std::string& name) 
{
    try {
        return &m_animation.channel(name);
    } catch (const std::out_of_range&) {
        return nullptr;
    }
}

const anim::Channel*
AnimationCHOP::getChannel(const std::string& name) const 
{
    try {
        return &m_animation.channel(name);
    } catch (const std::out_of_range&) {
        return nullptr;
    }
}

bool 
AnimationCHOP::removeChannel(const std::string& name) 
{
    try {
        m_animation.remove_channel(name);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

bool 
AnimationCHOP::removeChannel(size_t index) 
{
    try {
        m_animation.remove_channel(index);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

void 
AnimationCHOP::createChannels(const std::vector<std::string>& channelNames) 
{
    for (const auto& name : channelNames) {
        createChannel(name);
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

void
AnimationCHOP::clearChannels() 
{
	m_animation.channels().clear();
	// Reset start and end times to default values when all channels are cleared
	m_animation.set_start_time(0.0); 
	m_animation.set_end_time(30.0); // Default end time from anim::Animation
}

// Keyframe management methods
bool 
AnimationCHOP::setKeyframeAtTime(const std::string& channelName, double time, double value, 
                         anim::Function in_handle, anim::Function out_handle, anim::HandleMode mode) 
{
    try {
        auto& channel = m_animation.channel(channelName);
        channel.create_keyframe(time, value, anim::Point(), anim::Point(), in_handle, mode);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

bool 
AnimationCHOP::removeKeyframeAtTime(const std::string& channelName, double time) 
{
    try {
        auto& channel = m_animation.channel(channelName);
        for (size_t i = 0; i < channel.num_keyframes(); ++i) {
            if (std::abs(channel.keyframe(i).position.time - time) < 1e-9) { // Using a small epsilon for float comparison
                channel.delete_keyframe(i);
                return true;
            }
        }
        return false; // Keyframe at the specified time not found
    } catch (const std::out_of_range&) {
        return false;
    }
}

bool 
AnimationCHOP::removeKeyframes(const std::string& channelName, const std::vector<double>& times) 
{
    try {
        auto& channel = m_animation.channel(channelName);
        bool allRemoved = true;
        std::vector<size_t> indices_to_delete;

        for (double time_to_delete : times) {
            bool found = false;
            for (size_t i = 0; i < channel.num_keyframes(); ++i) {
                // Check if this keyframe is already marked for deletion to handle duplicate times in the input vector
                bool already_marked = false;
                for(size_t marked_idx : indices_to_delete) {
                    if (marked_idx == i) {
                        already_marked = true;
                        break;
                    }
                }
                if (already_marked) continue;

                if (std::abs(channel.keyframe(i).position.time - time_to_delete) < 1e-9) {
                    indices_to_delete.push_back(i);
                    found = true;
                    break; 
                }
            }
            if (!found) {
                allRemoved = false;
            }
        }

        // Sort indices in descending order to avoid issues with shifting indices upon deletion
        std::sort(indices_to_delete.rbegin(), indices_to_delete.rend());

        for (size_t index : indices_to_delete) {
            channel.delete_keyframe(index);
        }
        return allRemoved;
    } catch (const std::out_of_range&) {
        return false;
    }
}

// Query methods
size_t 
AnimationCHOP::getChannelCount() const 
{
    return m_animation.num_channels();
}

std::vector<std::string> 
AnimationCHOP::getChannelNames() const 
{
    std::vector<std::string> names;
    for (size_t i = 0; i < m_animation.num_channels(); ++i) {
        try {
            const auto& channel = m_animation.channel(i);
            names.push_back(channel.name());
        } catch (const std::out_of_range&) {
            // Skip invalid channel indices
        }
    }
    return names;
}

bool 
AnimationCHOP::channelExists(const std::string& name) const 
{
    try {
        m_animation.channel(name);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

size_t 
AnimationCHOP::getKeyframeCount(const std::string& channelName) const 
{
    try {
        const auto& channel = m_animation.channel(channelName);
        return channel.num_keyframes();
    } catch (const std::out_of_range&) {
        return 0;
    }
}

// Python bindings for animation methods

// Channel methods
static PyObject*
py_createChannel(PyObject* self, PyObject* args)
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
    // Check for duplicate channel
    if (inst->channelExists(name)) {
        Py_RETURN_NONE;
    }
    try {
        auto& channel = inst->createChannel(name);
        me->context->makeNodeDirty();

        auto pyChannel = ChannelToPyObject(&channel, self);
        if (!pyChannel) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create Python channel object");
            return nullptr;
        }

        return pyChannel;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject*
py_getChannel(PyObject* self, PyObject* args)
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
	try {
		if (PyUnicode_Check(identifier_obj)) {
			const char* name = PyUnicode_AsUTF8(identifier_obj);
			const auto& channel = inst->animation().channel(name);
			
			PyObject* pyChannel = ChannelToPyObject(const_cast<anim::Channel*>(&channel), self);
			if (!pyChannel) {
				PyErr_SetString(PyExc_RuntimeError, "Failed to create Python channel object");
				return nullptr;
			}
			return pyChannel;
		} else if (PyLong_Check(identifier_obj)) {
			int index = static_cast<int>(PyLong_AsLong(identifier_obj));
			const auto& channel = inst->animation().channel(index);
			
			PyObject* pyChannel = ChannelToPyObject(const_cast<anim::Channel*>(&channel), self);
			if (!pyChannel) {
				PyErr_SetString(PyExc_RuntimeError, "Failed to create Python channel object");
				return nullptr;
			}
			return pyChannel;
		} else {
			PyErr_SetString(PyExc_TypeError, "Identifier must be a string or an integer.");
			return nullptr;
		}
	} catch (const std::out_of_range&) {
		PyErr_SetString(PyExc_RuntimeError, "Channel not found");
		return nullptr;
	}
}

static PyObject*
py_removeChannel(PyObject* self, PyObject* args)
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
py_getChannelNames(PyObject* self)
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
py_clearChannels(PyObject* self)
{
    PY_Struct* me = (PY_Struct*)self;

    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }    inst->clearChannels();
    me->context->makeNodeDirty();
    Py_RETURN_NONE;
}

static PyObject*
py_getAnimation(PyObject* self)
{
    PY_Struct* me = (PY_Struct*)self;

    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }

    auto& animation = inst->animation();
    PyObject* pyAnimation = AnimationToPyObject(&animation, self);
    if (!pyAnimation) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create Python animation object");
        return nullptr;
    }

    return pyAnimation;
}

static PyObject*
py_setKeyframe(PyObject* self, PyObject* args)
{
	PY_Struct* me = (PY_Struct*)self;	const char* name;
	int index;
    double value;
    int mode = static_cast<int>(anim::HandleMode::smooth); // Default to smooth
    PyObject* in_function_obj = nullptr;
    PyObject* out_function_obj = nullptr;

	// Parse the arguments: channel name, index, value, [mode, in_function, out_function]
	if (!PyArg_ParseTuple(args, "sid|iOO", &name, &index, &value, &mode, 
		&in_function_obj, &out_function_obj)) {
		return nullptr;
	}

	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	if (!inst)
	{
		return nullptr;
	}

	try {
		auto& channel = inst->animation().channel(name);
		if (index < 0 || static_cast<size_t>(index) >= channel.num_keyframes()) {
			PyErr_SetString(PyExc_IndexError, "Keyframe index out of range");
			return nullptr;
		}
				channel.set_keyframe_value(static_cast<size_t>(index), value);
		channel.set_keyframe_handle_mode(static_cast<size_t>(index), static_cast<anim::HandleMode>(mode));
		// TODO: Update handles when Function API is clarified
		me->context->makeNodeDirty();
		Py_RETURN_NONE;
	}
	catch (const std::out_of_range&) {
		PyErr_SetString(PyExc_RuntimeError, "Channel not found");
		return nullptr;
	}
	catch (const std::exception& e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
		return nullptr;
	}
}

static PyObject*
py_setKeyframeAtTime(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;    // Accept up to 8 arguments: name, time, value, mode, in_tangent_time, in_tangent_value, out_tangent_time, out_tangent_value
    const char* name;
    double time, value;
    int mode = static_cast<int>(anim::HandleMode::smooth); // Default to smooth
    double in_tangent_time = 0.0, in_tangent_value = 0.0;
    double out_tangent_time = 0.0, out_tangent_value = 0.0;

    int arg_count = PyTuple_Size(args);
    if (arg_count < 3) {
        PyErr_SetString(PyExc_TypeError, "set_keyframe_at_time requires at least 3 arguments: channel, time, value");
        return nullptr;
    }
    if (!PyArg_ParseTuple(args, "sdd|iddddd", &name, &time, &value, &mode, &in_tangent_time, &in_tangent_value, &out_tangent_time, &out_tangent_value)) {
        return nullptr;
    }

    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }

    anim::Point in_handle(in_tangent_time, in_tangent_value);
    anim::Point out_handle(out_tangent_time, out_tangent_value);

    try {
        auto& channel = inst->animation().channel(name);
        channel.create_keyframe(time, value, in_handle, out_handle, anim::Function::bezier, static_cast<anim::HandleMode>(mode));
        me->context->makeNodeDirty();
        Py_RETURN_TRUE;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject* 
py_getKeyframe(PyObject* self, PyObject* args)
{
	PY_Struct* me = (PY_Struct*)self;
	const char* name;
	int index;

	if (!PyArg_ParseTuple(args, "si", &name, &index)) {
		return nullptr;
	}

	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	if (!inst) {
		return nullptr;
	}

	try {
		const auto& channel = inst->animation().channel(name);
		
		// Check for negative index
		if (index < 0) {
			PyErr_SetString(PyExc_IndexError, "Keyframe index cannot be negative");
			return nullptr;
		}

		// Check if index is within range
		if (static_cast<size_t>(index) >= channel.num_keyframes()) {
			char error_msg[256];
			snprintf(error_msg, sizeof(error_msg), 
				"Keyframe index %d out of range (channel has %zu keyframes)", 
				index, channel.num_keyframes());
			PyErr_SetString(PyExc_IndexError, error_msg);
			return nullptr;
		}

		const auto& keyframe = channel.keyframe(static_cast<size_t>(index));

		auto pyKeyframe = KeyframeToPyObject(keyframe);
		if (!pyKeyframe) {
			return nullptr;
		}
		return pyKeyframe;
	}
	catch (const std::out_of_range&) {
		PyErr_SetString(PyExc_RuntimeError, "Channel not found");
		return nullptr;
	}
}

static PyObject* 
py_getKeyframeAtTime(PyObject* self, PyObject* args)
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

	try {
		const auto& channel = inst->animation().channel(name);
				// Check if keyframe exists at time
		if (!channel.has_keyframe(time)) {
			Py_RETURN_NONE;
		}
		// Find the keyframe index at the given time
		for (size_t i = 0; i < channel.num_keyframes(); ++i) {
			if (std::abs(channel.keyframe(i).position.time - time) < 1e-9) {
				const auto& keyframe = channel.keyframe(i);
				auto pyKeyframe = KeyframeToPyObject(keyframe);
				if (!pyKeyframe) {
					return nullptr;
				}
				return pyKeyframe;
			}
		}
		Py_RETURN_NONE;
	}
	catch (const std::out_of_range&) {
		PyErr_SetString(PyExc_RuntimeError, "Channel not found");
		return nullptr;
	}
}

static PyObject* 
py_hasKeyframe(PyObject* self, PyObject* args)
{
	PY_Struct* me = (PY_Struct*)self;
	const char* name;
	int index;

	if (!PyArg_ParseTuple(args, "si", &name, &index)) {
		return nullptr;
	}

	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	if (!inst) {
		return PyBool_FromLong(0);
	}

	try {
		const auto& channel = inst->animation().channel(name);
		
		// Check for negative index
		if (index < 0) {
			return PyBool_FromLong(0);
		}

		// Check if index is within range
		bool has_keyframe = (static_cast<size_t>(index) < channel.num_keyframes());
		return PyBool_FromLong(has_keyframe ? 1 : 0);
	}
	catch (const std::out_of_range&) {
		return PyBool_FromLong(0);
	}
}

static PyObject*
py_hasKeyframeAtTime(PyObject* self, PyObject* args)
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
		return PyBool_FromLong(0);
	}

	try {
		const auto& channel = inst->animation().channel(name);		bool has_keyframe = channel.has_keyframe(time);
		return PyBool_FromLong(has_keyframe ? 1 : 0);
	}
	catch (const std::out_of_range&) {
		return PyBool_FromLong(0);
	}
}

static PyObject* 
py_removeKeyframe(PyObject* self, PyObject* args)
{
	PY_Struct* me = (PY_Struct*)self;
	const char* name;
	int index;

	if (!PyArg_ParseTuple(args, "si", &name, &index)) {
		return nullptr;
	}

	PY_GetInfo info;
	info.autoCook = false;
	AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
	if (!inst) {
		return nullptr;
	}

	try {
		auto& channel = inst->animation().channel(name);
		
		if (index < 0 || static_cast<size_t>(index) >= channel.num_keyframes()) {
			PyErr_SetString(PyExc_IndexError, "Keyframe index out of range");
			return nullptr;
		}
		channel.delete_keyframe(static_cast<size_t>(index));
		me->context->makeNodeDirty();

		Py_RETURN_NONE;
	}
	catch (const std::out_of_range&) {
		PyErr_SetString(PyExc_RuntimeError, "Channel not found");
		return nullptr;
	}
}

static PyObject*
py_removeKeyframeAtTime(PyObject* self, PyObject* args)
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
py_setKeyframes(PyObject* self, PyObject* args)
{
	PY_Struct* me = (PY_Struct*)self;
	const char* name;
	// a list of dicts 
	// {'index': 0, 'value': 1.0, 'mode': 0}
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
	
	try {
		auto& channel = inst->animation().channel(name);
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
			int index = static_cast<int>(PyLong_AsLong(indexObj));
			if (index < 0 || static_cast<size_t>(index) >= channel.num_keyframes()) {
				PyErr_SetString(PyExc_IndexError, "Keyframe index out of range");
				return nullptr;
			}
			auto value = PyFloat_AsDouble(valueObj);		anim::HandleMode mode = anim::HandleMode::smooth;
		PyObject* modeObj = PyDict_GetItemString(item, "mode");

		if (modeObj) {
			if (!PyLong_Check(modeObj)) {
				PyErr_SetString(PyExc_TypeError, "Keyframe 'mode' must be an integer");
				return nullptr;
			}
			mode = static_cast<anim::HandleMode>(PyLong_AsLong(modeObj));
		}
		
		channel.set_keyframe_value(static_cast<size_t>(index), value);
		channel.set_keyframe_handle_mode(static_cast<size_t>(index), mode);
		}

		me->context->makeNodeDirty();
		Py_RETURN_NONE;
	}
	catch (const std::out_of_range&) {
		PyErr_SetString(PyExc_RuntimeError, "Channel not found");
		return nullptr;
	}
}

static PyObject*
py_setKeyframesAtTime(PyObject* self, PyObject* args)
{
    PY_Struct* me = (PY_Struct*)self;
    const char* name;

	// a list of dicts 
	// {'time': 0.0, 'value': 1.0, 'mode': 0, 
	// 'in_tangent_time': 0.0, 'in_tangent_value': 0.0, 
	// 'out_tangent_time': 0.0, 'out_tangent_value': 0.0}
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
	anim::Channel* channel = nullptr;
	try { channel = &inst->animation().channel(name); } catch (const std::out_of_range&) { channel = nullptr; }
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
		
		if (!timeObj || !valueObj)
		{
			PyErr_SetString(PyExc_TypeError, "Keyframe must have 'time' and 'value' keys");
			return nullptr;
		}
		auto time = PyFloat_AsDouble(timeObj);
		auto value = PyFloat_AsDouble(valueObj);		anim::HandleMode mode = anim::HandleMode::smooth;
		if (modeObj) {
			if (!PyLong_Check(modeObj)) {
				PyErr_SetString(PyExc_TypeError, "Keyframe 'mode' must be an integer");
				return nullptr;
			}
			mode = static_cast<anim::HandleMode>(PyLong_AsLong(modeObj));
		}
		
		// TODO: Update with proper Function objects when API is clarified
		anim::Function in_handle = anim::Function::bezier;  // Default function
		anim::Function out_handle = anim::Function::bezier; // Default function
		
		channel->create_keyframe(time, value, anim::Point(), anim::Point(), in_handle, mode);
	}

	me->context->makeNodeDirty();

	Py_RETURN_NONE;
}

static PyObject*
py_removeKeyframesAtTime(PyObject* self, PyObject* args)
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
      try {
    		auto& channel = inst->animation().channel(name);
        int removed_count = 0;
        
        // Remove keyframes at specified times (iterate backwards to maintain indices)
        for (auto time_it = times.rbegin(); time_it != times.rend(); ++time_it) {
            double time = *time_it;
            
            // Find keyframe at this time
            for (size_t i = 0; i < channel.num_keyframes(); ++i) {
                if (std::abs(channel.keyframe(i).position.time - time) < 1e-9) {
                    channel.delete_keyframe(i);
                    removed_count++;
                    break;
                }
            }
        }
        
        me->context->makeNodeDirty();
        return PyLong_FromLong(removed_count);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    }
}

static PyObject*
py_debugChannel(PyObject* self, PyObject* args)
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

	try {
		auto& channel = inst->animation().channel(name);
		
		// Create a debug info dictionary
		PyObject* debug_info = PyDict_New();
		
		// Add keyframe count
		PyDict_SetItemString(debug_info, "keyframe_count", PyLong_FromSize_t(channel.num_keyframes()));
		
		// Add channel name
		PyDict_SetItemString(debug_info, "name", PyUnicode_FromString(channel.name().c_str()));
		
		// Check if channel is empty
		PyDict_SetItemString(debug_info, "is_empty", PyBool_FromLong(channel.empty()));
		
		// Get start and end times if available
		if (!channel.empty()) {
			PyDict_SetItemString(debug_info, "start_time", PyFloat_FromDouble(channel.start_time()));
			PyDict_SetItemString(debug_info, "end_time", PyFloat_FromDouble(channel.end_time()));
			PyDict_SetItemString(debug_info, "length", PyFloat_FromDouble(channel.length()));
		} else {
			Py_INCREF(Py_None);
			PyDict_SetItemString(debug_info, "start_time", Py_None);
			Py_INCREF(Py_None);
			PyDict_SetItemString(debug_info, "end_time", Py_None);
			Py_INCREF(Py_None);
			PyDict_SetItemString(debug_info, "length", Py_None);
		}

		return debug_info;
	} catch (const std::exception& e) {
		PyErr_SetString(PyExc_RuntimeError, e.what());
		return nullptr;
	}
}


