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

#include <iostream>


#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
	#include <modsupport.h>

#else
	#include <Python/Python.h>
	#include <Python/structmember.h>
#endif

// static PyObject* py_chop_animationFromDict(PyObject* self, PyObject* args);
static PyObject* py_chop_create_channel(PyObject* self, PyObject* args);
static PyObject* py_chop_emplace_channel(PyObject* self, PyObject* args);
static PyObject* py_chop_insert_channel(PyObject* self, PyObject* args);
static PyObject* py_chop_channel(PyObject* self, PyObject* args);
static PyObject* py_chop_remove_channel(PyObject* self, PyObject* args);
static PyObject* py_chop_has_channel(PyObject* self, PyObject* args);
static PyObject* py_chop_get_channel(PyObject *self, PyObject *key);
static PyObject* py_chop_clear(PyObject* self, PyObject* args);
static PyObject* py_chop_get_state_method(PyObject* self, PyObject* args);
static PyObject* py_chop_set_state_method(PyObject* self, PyObject* args);

// --- Python method table for AnimationCHOP ---
static PyMethodDef methods[] = {
    {"create_channel", (PyCFunction)py_chop_create_channel, METH_VARARGS, "Create a new channel."},
    {"emplace_channel", (PyCFunction)py_chop_emplace_channel, METH_VARARGS, "Emplace a new channel."},
    {"insert_channel", (PyCFunction)py_chop_insert_channel, METH_VARARGS, "Insert a channel at a given index."},
    {"remove_channel", (PyCFunction)py_chop_remove_channel, METH_VARARGS, "Remove a channel by name or index."},
    {"has_channel", (PyCFunction)py_chop_has_channel, METH_VARARGS, "Check if a channel exists."},
	{"get_channel", (PyCFunction)py_chop_get_channel, METH_VARARGS, "Get a channel by name or index."},
    {"clear", (PyCFunction)py_chop_clear, METH_NOARGS, "Clear all channels."},
    {"get_state", (PyCFunction)py_chop_get_state_method, METH_VARARGS, "Get the state of the AnimationCHOP."},
    {"set_state", (PyCFunction)py_chop_set_state_method, METH_VARARGS | METH_KEYWORDS, "Set the state of the AnimationCHOP."},
    
    {nullptr, nullptr, 0, nullptr}
};

static PyObject* py_chop_get_channels(PyObject* self, void* closure);
static PyObject* py_chop_get_channel_names(PyObject* self, void* closure);
static PyObject* py_chop_get_num_channels(PyObject* self, void* closure);
static PyObject* py_chop_get_start_time(PyObject* self, void* closure);
static int py_chop_set_start_time(PyObject* self, PyObject* value, void* closure);
static PyObject* py_chop_get_end_time(PyObject* self, void* closure);
static int py_chop_set_end_time(PyObject* self, PyObject* value, void* closure);
static PyObject* py_chop_get_length(PyObject* self, void* closure);
static int py_chop_set_length(PyObject* self, PyObject* value, void* closure);
static PyObject* py_chop_get_num_samples(PyObject* self, void* closure);
static PyObject* py_chop_get_state(PyObject* self, void* closure);
static int py_chop_set_state(PyObject* self, PyObject* value, void* closure);

// This struct lists the different getters and/or settings the Custom Operator will expose.
static PyGetSetDef getSets[] =
{
    {"Point", get_point_type, nullptr, "Point type for representing time-value pairs.", nullptr},
    {"HandleMode", get_handle_mode_enum, nullptr, "HandleMode enum for keyframe handle behavior.", nullptr},
    {"Function", get_function_enum, nullptr, "Function enum for keyframe interpolation type.", nullptr},
    {"Keyframe", get_keyframe_type, nullptr, "Keyframe type for animation curves.", nullptr},
    {"Channel", get_channel_type, nullptr, "Channel type for animation data.", nullptr},
    {"channels", py_chop_get_channels, nullptr, "Get all channels.", nullptr}, 
    {"channel_names", py_chop_get_channel_names, nullptr, "Get all channel names.", nullptr},
    {"num_channels",py_chop_get_num_channels, nullptr, "Get the number of channels.", nullptr},
	{"start_time", py_chop_get_start_time, py_chop_set_start_time, "Get or set the start time of the animation.", nullptr},
	{"end_time", py_chop_get_end_time, py_chop_set_end_time, "Get or set the end time of the animation.", nullptr},
	{"length", py_chop_get_length, py_chop_set_length, "Get or set the length of the animation.", nullptr},
	{"num_samples", py_chop_get_num_samples, nullptr, "Get the number of samples in the animation.", nullptr},
    {"state", py_chop_get_state, py_chop_set_state, "Get or set the serializable state of the animation.", nullptr},
    // {"animation", (getter)py_chop_animation, nullptr, "The PyObject interface for this AnimationCHOP.", nullptr},
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
	, m_animation(std::make_unique<anim::Animation>())
{
}

AnimationCHOP::~AnimationCHOP()
{
	cleanup_handle_mode_enum();
    cleanup_function_enum();
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
    auto output_mode = inputs->getParString("Outputmode");
    if (strcmp(output_mode, "sequence") == 0 && inputs->getParInt("Timeslice") == 1) {
        ginfo->timeslice = true;
    } 
}

bool
AnimationCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
    info->numChannels = static_cast<int32_t>(m_animation->num_channels());
    std::cout << "AnimationCHOP::getOutputInfo: numChannels = " << info->numChannels << std::endl;
    info->sampleRate = static_cast<float>(inputs->getParDouble("Samplerate"));
    applyOutputMode(inputs);


    // switch(m_outputMode) {
    // case OutputMode::input: {
    //     const OP_CHOPInput* input_chop = inputs->getInputCHOP(0);
    //     if (input_chop) {
    //         info->startIndex = input_chop->startIndex;
    //         info->numSamples = input_chop->numSamples;
    //         info->sampleRate = input_chop->sampleRate;
            
    //     } else {
    //         info->startIndex = 0;
    //         info->numSamples = 1;
    //         m_error = "An CHOP with at least one channel must be connected when using 'Input' mode.";
    //     }
    //     break;
    // } case OutputMode::sequence: {
    //     info->startIndex = static_cast<uint32_t>(inputs->getParDouble("Sequence"));

    //     // overridden if CHOP_GeneralInfo::timeslice == true
    //     info->numSamples = 1;
    //     break;
    // } case OutputMode::range: {
    //     auto start_time = inputs->getParDouble("Range", 0);
    //     auto end_time = inputs->getParDouble("Range", 1);
    //     m_animation->set_start_time(start_time);
    //     m_animation->set_end_time(end_time);
    //     info->startIndex = static_cast<uint32_t>(start_time * info->sampleRate);
    //     info->numSamples = static_cast<int32_t>(end_time * info->sampleRate) - info->startIndex;
    //     if (info->numSamples < 1) {
    //         info->numSamples = 1;
    //     }
    //     break;
    // } case OutputMode::autoRange: default: {
    //     double max_length = 0.0;
    //     for (size_t i = 0; i < m_animation->num_channels(); ++i) {
    //         const auto& channel = m_animation->channel(i);
    //         max_length = std::max(max_length, channel.length());
    //     }
    //     info->numSamples = static_cast<int32_t>(std::ceil(max_length * info->sampleRate));
    //     m_animation->set_start_time(0.0);
    //     m_animation->set_end_time(max_length);
    //     break;
    // }
    // }
    return true;
}

void
AnimationCHOP::getChannelName(int32_t index, OP_String *name, const OP_Inputs* inputs, void* reserved1)
{
	// If we have animation channels, use their names
	if (index < static_cast<int32_t>(m_animation->num_channels())) {
		// try {
			const auto& channel = m_animation->channel(index);
			name->setString(channel.name().c_str());
		// } catch (const std::out_of_range&) {
        //     std::cout << "Warning: Channel index " << index << " out of range for AnimationCHOP." << std::endl;
		// 	// Default fallback name
		// 	name->setString("chan1");
		// }
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

    // switch(m_outputMode) {
    // case OutputMode::input: {
    //     const OP_CHOPInput* input_chop = inputs->getInputCHOP(0);
    //     if (!input_chop) {
    //         m_error = "No input CHOP connected.";
    //         return;
    //     } else if (input_chop->numChannels < 1) {
    //         m_error = "Input CHOP has no channels.";
    //         return;
    //     } else if (input_chop->numSamples != output->numSamples) {
    //         m_error = "Input CHOP and output CHOP have different number of samples.";
    //         return;
    //     } else if (input_chop->sampleRate != output->sampleRate) {
    //         m_error = "Input CHOP and output CHOP have different sample rates.";
    //         return;
    //     }

    //     auto index_unit = inputs->getParString("Indexunit");
    //     std::vector<double> eval_times;
    //     eval_times.reserve(input_chop->numSamples);
    //     if (strcmp(index_unit, "seconds") == 0) {
    //         for (size_t i = 0; i < input_chop->numSamples; ++i) {
    //             eval_times.push_back(input_chop->getChannelData(0)[i]);
    //         }
    //     } else if (strcmp(index_unit, "samples") == 0) {
    //         for (size_t i = 0; i < input_chop->numSamples; ++i) {
    //             eval_times.push_back(i / input_chop->sampleRate);
    //         }
    //     } else if (strcmp(index_unit, "frames") == 0) {
    //         for (size_t i = 0; i < input_chop->numSamples; ++i) {
    //             eval_times.push_back((i + 1.0f) / input_chop->sampleRate);
    //         }
    //     } 
    //     for (size_t i = 0; i < output->numChannels; ++i) {
    //         if (i < static_cast<int32_t>(input_chop->numChannels) && i < m_animation->num_channels()) {
    //             for (int j = 0; j < output->numSamples; ++j) {
    //                 output->channels[i][j] = static_cast<float>(m_animation->channel(i).evaluate(eval_times[j]));
    //             }
    //         } 
    //     }
    //     return;
    // } case OutputMode::sequence: {
    //     auto index_unit = inputs->getParString("Indexunit");
    //     auto eval_time = inputs->getParDouble("Sequence");
    //     if (strcmp(index_unit, "samples") == 0) {
    //         eval_time /= output->sampleRate; // Convert samples to seconds
    //     } else if (strcmp(index_unit, "frames") == 0) {
    //         eval_time = (eval_time - 1.0f) / output->sampleRate; // Convert frames to seconds
    //     }
            
    //     for (size_t i = 0; i < output->numChannels; ++i) {
    //         if (i < m_animation->num_channels()) {
    //             output->channels[i][0] = static_cast<float>(m_animation->channel(i).evaluate(eval_time));
    //         }
    //     }
    //     return;
    // } case OutputMode::range: case OutputMode::autoRange: default: {
    //     size_t num_anim_channels = m_animation->num_channels();
    //     for (int i = 0; i < output->numChannels; i++) {
    //         if (i < static_cast<int>(num_anim_channels)) {
    //             auto samples = m_animation->channel(i).evaluate_range(
    //                 m_animation->start_time(),
    //                 m_animation->end_time(),
    //                 output->numSamples
    //             );
    //             std::copy(samples.begin(), samples.end(), output->channels[i]);
    //         }
    //     }
    //     return;
    // }
    // }
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
	{
		OP_NumericParameter	np;
		np.name = "Range";
		np.label = "Range";
		np.defaultValues[0] = 0.0;
        np.minValues[0] = 0.0;
        np.clampMins[0] = true;
        np.defaultValues[1] = 30.0;
		
		OP_ParAppendResult res = manager->appendFloat(np, 2);
		assert(res == OP_ParAppendResult::Success);
	} {
		OP_StringParameter	sp;
		sp.name = "Outputmode";
		sp.label = "Output Mode";
		sp.defaultValue = "fullrange";
		const char *names[] = { "range", "autorange", "input", "sequence" };
		const char *labels[] = { "Range", "Auto Range", "Input Index (first channel)", "Sequence Index" };

		OP_ParAppendResult res = manager->appendMenu(sp, 2, names, labels);
		assert(res == OP_ParAppendResult::Success);
	} {
        OP_StringParameter	sp;
		sp.name = "Indexunit";
        sp.label = "Index Unit";
        sp.defaultValue = "seconds";
        const char *names[] = { "seconds", "samples", "frames" };
        const char *labels[] = { "Seconds", "Samples", "Frames" };

        OP_ParAppendResult res = manager->appendMenu(sp, 3, names, labels);
        assert(res == OP_ParAppendResult::Success);
    } {
        OP_NumericParameter	np;
        np.name = "Sequence";
        np.label = "Sequence Index";
		np.defaultValues[0] = 0.0;
        np.minSliders[0] = 0.0;
        np.maxSliders[0] = 30.0;
        OP_ParAppendResult res = manager->appendFloat(np);
        assert(res == OP_ParAppendResult::Success);
    } {
        OP_StringParameter	sp;
        sp.name = "Extendleft";
        sp.label = "Extend Left";
        sp.defaultValue = "hold";
        const char *names[] = { "hold", "repeat"};
        const char *labels[] = { "Hold", "Repeat" };

        OP_ParAppendResult res = manager->appendMenu(sp, 2, names, labels);
        assert(res == OP_ParAppendResult::Success);
    } {
        OP_StringParameter	sp;
        sp.name = "Extendright";
        sp.label = "Extend Right";
        sp.defaultValue = "repeat";
        const char *names[] = { "hold", "repeat"};
        const char *labels[] = { "Hold", "Repeat" };

        OP_ParAppendResult res = manager->appendMenu(sp, 2, names, labels);
        assert(res == OP_ParAppendResult::Success);
    } {
		OP_NumericParameter	np;
		np.name = "Samplerate";
		np.label = "Sample Rate";
		np.defaultValues[0] = 60.0;
		np.minSliders[0] = 120.0;
        np.minValues[0] = 1.0;
		np.maxSliders[0] =  30.0;
        np.clampMins[0] = true;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	} {
		OP_NumericParameter	np;
        np.name = "Timeslice";
        np.label = "Timeslice";
        np.defaultValues[0] = 0.0;

        OP_ParAppendResult res = manager->appendToggle(np);
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

void
AnimationCHOP::applyOutputMode(const OP_Inputs *inputs)
{
    const char* output_mode = inputs->getParString("Outputmode");
    if (strcmp(output_mode, "range") == 0) {
        m_outputMode = OutputMode::range;
    } else if (strcmp(output_mode, "autorange") == 0) {
        m_outputMode = OutputMode::autoRange;
    } else if (strcmp(output_mode, "input") == 0) {
        m_outputMode = OutputMode::input;
    } else if (strcmp(output_mode, "sequence") == 0) {
        m_outputMode = OutputMode::sequence;
    } else {
        m_outputMode = OutputMode::autoRange;
    }
}

// Python bindings for animation methods

// --- Channel creation and insertion ---
static PyObject* py_chop_create_channel(PyObject *self, PyObject *args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get CHOP instance for create_channel"); // Set an error
        return NULL;
    }
    auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }

    const char* name;
    Py_ssize_t index;
    Py_ssize_t nargs = PyTuple_Size(args);
    
    if (nargs == 1) {
        if (!PyArg_ParseTuple(args, "s", &name))
            return NULL;
        try {
            anim::Channel& channel = animation->create_channel(std::string(name));
            me->context->makeNodeDirty();
            return ChannelToPyObject(&channel, (PyObject*)self);
        } catch (const std::exception& e) {
            PyErr_SetString(PyExc_RuntimeError, e.what());
            return NULL;
        }
    } else if (nargs == 2) {
        if (!PyArg_ParseTuple(args, "sn", &name, &index))
            return NULL;
        try {
            if (index < 0) {
                PyErr_SetString(PyExc_IndexError, "Channel index cannot be negative");
                return NULL;
            }
            if (static_cast<size_t>(index) > animation->num_channels()) {
				PyErr_Format(PyExc_IndexError, "Channel index out of range: %zd (max %zd)", index, animation->num_channels());
				return NULL;
                // PyErr_SetString(PyExc_IndexError, "Channel index out of range");
                // return NULL;
            }
            anim::Channel& channel = animation->create_channel(std::string(name), static_cast<size_t>(index));
            me->context->makeNodeDirty();
            return ChannelToPyObject(&channel, (PyObject*)self);
        } catch (const std::exception& e) {
            PyErr_SetString(PyExc_RuntimeError, e.what());
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "create_channel expects (name) or (name, index)");
        return NULL;
    }
}

static PyObject* py_chop_emplace_channel(PyObject *self, PyObject *args) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }

    PyObject* py_chop_channel;
    if (!PyArg_ParseTuple(args, "O", &py_chop_channel))
        return NULL;
    anim::Channel* channel_ptr = nullptr;
    if (!PyObjectToChannel(py_chop_channel, channel_ptr))
        return NULL;
    try {
        anim::Channel& channel = animation->emplace_channel(std::move(*channel_ptr));
		me->context->makeNodeDirty();
        return ChannelToPyObject(&channel, (PyObject*)self);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static PyObject* py_chop_insert_channel(PyObject *self, PyObject *args) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }

    Py_ssize_t index;
    PyObject* py_chop_channel;
    if (!PyArg_ParseTuple(args, "nO", &index, &py_chop_channel))
        return NULL;
    anim::Channel* channel_ptr = nullptr;
    if (!PyObjectToChannel(py_chop_channel, channel_ptr))
        return NULL;
    try {
        anim::Channel& channel = animation->insert_channel(static_cast<size_t>(index), *channel_ptr);
		me->context->makeNodeDirty();
        return ChannelToPyObject(&channel, (PyObject*)self);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// --- Channel access ---
static PyObject* py_chop_get_channel(PyObject *self, PyObject *args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
    auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }

    PyObject* key;
    if (!PyArg_ParseTuple(args, "O", &key))
        return NULL;

    if (PyLong_Check(key)) {
        size_t index = PyLong_AsSize_t(key);
        if (PyErr_Occurred()) {
            if (PyErr_ExceptionMatches(PyExc_OverflowError)) {
                PyErr_SetString(PyExc_IndexError, "Channel index out of range");
            }
            return NULL;
        }
        try {
            anim::Channel& channel = animation->channel(index);
            return ChannelToPyObject(&channel, (PyObject*)self);
        } catch (const std::exception& e) {
            PyErr_SetString(PyExc_IndexError, e.what());
            return NULL;
        }
    } else if (PyUnicode_Check(key)) {
        const char* name_cstr = PyUnicode_AsUTF8(key);
        if (!name_cstr) {
            return NULL;  // PyUnicode_AsUTF8 failed and set an exception
        }
        std::string name = name_cstr;
        try {
            anim::Channel& channel = animation->channel(name);
            return ChannelToPyObject(&channel, (PyObject*)self);
        } catch (const std::exception& e) {
            PyErr_SetString(PyExc_KeyError, e.what());
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Channel key must be int or str");
        return NULL;
    }
}

static PyObject* py_chop_has_channel(PyObject *self, PyObject *args) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return NULL;
    }
    auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    const char* name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    
    // Direct conversion - PyBool_FromLong handles 0/1 automatically
    bool has_ch = animation->has_channel(std::string(name));
    return PyBool_FromLong(has_ch);
}


// --- Channel removal ---
static PyObject* py_chop_remove_channel(PyObject *self, PyObject *args) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    Py_ssize_t nargs = PyTuple_Size(args);
    if (nargs == 1) {
        PyObject* arg = PyTuple_GetItem(args, 0);
        if (PyLong_Check(arg)) {
            auto index = static_cast<size_t>(PyLong_AsSize_t(arg));
            try {
                animation->remove_channel(index);
				me->context->makeNodeDirty();
                Py_RETURN_NONE;
            } catch (const std::exception& e) {
                PyErr_SetString(PyExc_IndexError, e.what());
                return NULL;
            }
        } else if (PyUnicode_Check(arg)) {
            std::string name = PyUnicode_AsUTF8(arg);
            try {
                animation->remove_channel(name);
				me->context->makeNodeDirty();
                Py_RETURN_NONE;
            } catch (const std::exception& e) {
                PyErr_SetString(PyExc_KeyError, e.what());
                return NULL;
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "remove_channel expects int (index) or str (name)");
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "remove_channel expects one argument");
        return NULL;
    }
}


static PyObject* py_chop_clear(PyObject *self, PyObject *args) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    animation->clear();
	me->context->makeNodeDirty();
    Py_RETURN_NONE;
}


// -------------------------------------------------------------------------------------
// --- Properties ---

static PyObject* py_chop_get_channels(PyObject *self, void* closure) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    const auto& chans = animation->channels();
    PyObject* list = PyList_New(chans.size());
    if (!list) return NULL;
    for (size_t i = 0; i < chans.size(); ++i) {
        PyObject* py_chop_ch = ChannelToPyObject(const_cast<anim::Channel*>(&chans[i]), (PyObject*)self);
        if (!py_chop_ch) {
            Py_DECREF(list);
            return NULL;
        }
        PyList_SET_ITEM(list, i, py_chop_ch);
    }
    return list;
}

static PyObject* py_chop_get_channel_names(PyObject *self, void* closure) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    std::vector<std::string> names = animation->channel_names();
    PyObject* names_list = PyList_New(names.size());
    if (!names_list) {
        return NULL;
    }
    for (size_t i = 0; i < names.size(); ++i) {
        PyObject* py_chop_name = PyUnicode_FromString(names[i].c_str());
        if (!py_chop_name) {
            Py_DECREF(names_list);
            return NULL;
        }
        PyList_SET_ITEM(names_list, i, py_chop_name);
    }
    return names_list;
}

static PyObject* py_chop_get_num_channels(PyObject *self, void* closure) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    size_t count = animation->num_channels();
    return PyLong_FromSize_t(count);
}

// --- Properties and other methods ---
static PyObject* py_chop_get_start_time(PyObject *self, void* closure) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    return PyFloat_FromDouble(animation->start_time());
}

static int py_chop_set_start_time(PyObject *self, PyObject *value, void* closure) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return -1;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return -1;
    }

	PyObject* arg = PyNumber_Float(value);
	if (!arg || !PyFloat_Check(arg)) {
		PyErr_SetString(PyExc_TypeError, "start_time must be a float");
		return -1;
	}
	auto start_time = PyFloat_AsDouble(arg);
	if (start_time > animation->end_time()) {
		animation->set_end_time(start_time);
	}

    animation->set_start_time(start_time);
	me->context->makeNodeDirty();
    return 0;
}

static PyObject* py_chop_get_end_time(PyObject *self, void* closure) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    return PyFloat_FromDouble(animation->end_time());
}

static int py_chop_set_end_time(PyObject *self, PyObject *value, void* closure) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return -1;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return -1;
    }

	PyObject* arg = PyNumber_Float(value);
	if (!arg || !PyFloat_Check(arg)) {
		PyErr_SetString(PyExc_TypeError, "end_time must be a float");
		return -1;
	}
	auto end_time = PyFloat_AsDouble(arg);
	if (end_time < animation->start_time()) {
		animation->set_start_time(end_time);
	}

    animation->set_end_time(end_time);
	me->context->makeNodeDirty();
    return 0;
}

static PyObject* py_chop_get_length(PyObject *self, void* closure) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
    
    try {
        return PyFloat_FromDouble(animation->length());
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

static int py_chop_set_length(PyObject *self, PyObject *value, void* closure) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return -1;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return -1;
    }
    
	PyObject* arg = PyNumber_Float(value);
	if (!arg || !PyFloat_Check(arg)) {
		PyErr_SetString(PyExc_TypeError, "length must be a float");
		return -1;
	}
	auto length = PyFloat_AsDouble(arg);
	if (length < 0.0) {
		PyErr_SetString(PyExc_ValueError, "length must be non-negative");
		return -1;
	}

	animation->set_length(length);
	me->context->makeNodeDirty();
	return 0;

}

static PyObject* py_chop_get_num_samples(PyObject *self, void* closure) {
	PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
	auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }
 
    try {
        int samples = animation->num_samples(static_cast<double>(inst->sampleRate()));
        return PyLong_FromLong(samples);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_ValueError, e.what());
        return NULL;
    }
}

static PyObject* py_chop_get_state(PyObject *self, void* closure) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return nullptr;
    }
    auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return NULL;
    }

    // Create main animation dict
    PyObject* anim_dict = PyDict_New();
    if (!anim_dict) return NULL;
    
    try {
        // Add animation-level properties
        PyDict_SetItemString(anim_dict, "start_time", PyFloat_FromDouble(animation->start_time()));
        PyDict_SetItemString(anim_dict, "end_time", PyFloat_FromDouble(animation->end_time()));
        PyDict_SetItemString(anim_dict, "length", PyFloat_FromDouble(animation->length()));
        PyDict_SetItemString(anim_dict, "num_channels", PyLong_FromSize_t(animation->num_channels()));
        
        // Create channels list using channel state
        PyObject* channels_list = PyList_New(animation->num_channels());
        if (!channels_list) {
            Py_DECREF(anim_dict);
            return NULL;
        }
        
        // Process each channel using their state methods
        for (size_t ch_idx = 0; ch_idx < animation->num_channels(); ++ch_idx) {
            try {
                anim::Channel& channel = animation->channel(ch_idx);
                PyChannel* py_channel = (PyChannel*)ChannelToPyObject(&channel, (PyObject*)self);
                if (!py_channel) {
                    Py_DECREF(channels_list);
                    Py_DECREF(anim_dict);
                    return NULL;
                }
                
                PyObject* channel_state = PyChannel_get_state(py_channel, NULL);
                Py_DECREF(py_channel); // We only needed it for the state
                
                if (!channel_state) {
                    Py_DECREF(channels_list);
                    Py_DECREF(anim_dict);
                    return NULL;
                }
                
                PyList_SET_ITEM(channels_list, ch_idx, channel_state);
                
            } catch (const std::exception& e) {
                PyErr_Format(PyExc_RuntimeError, "Error processing channel %zu: %s", ch_idx, e.what());
                Py_DECREF(channels_list);
                Py_DECREF(anim_dict);
                return NULL;
            }
        }
        
        // Add channels list to animation dict
        PyDict_SetItemString(anim_dict, "channels", channels_list);
        
        return anim_dict;
        
    } catch (const std::exception& e) {
        PyErr_Format(PyExc_RuntimeError, "Error serializing animation state: %s", e.what());
        Py_DECREF(anim_dict);
        return NULL;
    }
}

static PyObject* py_chop_get_state_method(PyObject *self, PyObject* args) {
    return py_chop_get_state(self, NULL);
}

static int py_chop_set_state(PyObject *self, PyObject *value, void* closure) {
    PY_Struct* me = (PY_Struct*)self;
    PY_GetInfo info;
    info.autoCook = false;
    AnimationCHOP* inst = (AnimationCHOP*)me->context->getNodeInstance(info);
    if (!inst) {
        return -1;
    }
    auto animation = inst->animation();
    if (!animation) {
        PyErr_SetString(PyExc_RuntimeError, "Animation is not valid");
        return -1;
    }
    
    if (!PyDict_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "Animation state must be a dictionary");
        return -1;
    }
    
    try {
        // Clear existing animation
        animation->clear();
        
        // Get animation properties
        PyObject* start_time_obj = PyDict_GetItemString(value, "start_time");
        PyObject* end_time_obj = PyDict_GetItemString(value, "end_time");
        PyObject* channels_obj = PyDict_GetItemString(value, "channels");
        
        if (!channels_obj || !PyList_Check(channels_obj)) {
            PyErr_SetString(PyExc_ValueError, "Animation state must contain a 'channels' list");
            return -1;
        }
        
        // Set animation timing if provided
        if (start_time_obj && PyFloat_Check(start_time_obj)) {
            animation->set_start_time(PyFloat_AsDouble(start_time_obj));
        }
        if (end_time_obj && PyFloat_Check(end_time_obj)) {
            animation->set_end_time(PyFloat_AsDouble(end_time_obj));
        }
        
        // Process channels using their state methods
        Py_ssize_t num_channels = PyList_Size(channels_obj);
        for (Py_ssize_t ch_idx = 0; ch_idx < num_channels; ++ch_idx) {
            PyObject* channel_state = PyList_GetItem(channels_obj, ch_idx);
            if (!PyDict_Check(channel_state)) {
                PyErr_Format(PyExc_ValueError, "Channel %zd state must be a dictionary", ch_idx);
                return -1;
            }
            
            // Get channel name
            PyObject* name_obj = PyDict_GetItemString(channel_state, "name");
            if (!name_obj || !PyUnicode_Check(name_obj)) {
                PyErr_Format(PyExc_ValueError, "Channel %zd must have a 'name' string", ch_idx);
                return -1;
            }
            
            // Create channel
            std::string channel_name = PyUnicode_AsUTF8(name_obj);
            anim::Channel& channel = animation->create_channel(channel_name);
            
            // Create PyChannel wrapper and set its state
            PyChannel* py_channel = (PyChannel*)ChannelToPyObject(&channel, (PyObject*)self);
            if (!py_channel) {
                return -1;
            }
            
            if (PyChannel_set_state(py_channel, channel_state, NULL) < 0) {
                Py_DECREF(py_channel);
                return -1;
            }
            
            Py_DECREF(py_channel);
        }
        
        me->context->makeNodeDirty();
        return 0;
        
    } catch (const std::exception& e) {
        PyErr_Format(PyExc_RuntimeError, "Error deserializing animation state: %s", e.what());
        return -1;
    }
}

static PyObject* py_chop_set_state_method(PyObject *self, PyObject *args) {
    PyObject* value;
    if (!PyArg_ParseTuple(args, "O", &value)) {
        return NULL;
    }
    
    if (py_chop_set_state(self, value, NULL) < 0) {
        return NULL;
    }
    
    Py_RETURN_NONE;
}
