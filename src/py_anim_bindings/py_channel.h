#pragma once

#include <anim/id.hpp>
#include <anim/channel.hpp>
#include "py_keyframe.h"
#include "py_point.h"
#include "py_handle_mode.h"
#include "py_function.h"

#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
	#include <modsupport.h>
#else
	#include <Python.h>
	#include <structmember.h>
#endif

// Forward declaration
class AnimationCHOP;
namespace TD { struct PY_Struct; }

typedef struct {
    PyObject_HEAD
    // Unique identifier for the channel. Id holds a const member and its raw
    // constructor is private to the library, so this is placement-new copied
    // from Channel::id() rather than assigned.
    anim::Id channel_id;
    PyObject* parent; // Reference to the parent AnimationCHOP to keep it alive
} PY_Channel;

struct ChannelData {
    anim::Channel* channel {nullptr}; // Pointer to the anim::Channel object
    AnimationCHOP* inst {nullptr}; // Pointer to the AnimationCHOP instance for context
    TD::PY_Struct* node_struct {nullptr}; // Pointer to the TD node structure for context
};

extern PyTypeObject PY_ChannelType;

// Helper functions for conversion between C++ and Python
PyObject* ChannelToPY_Object(anim::Channel* channel, PyObject* parent);
bool PY_ObjectToChannel(PyObject* obj, anim::Channel*& channel);

// Type getter for external use
PyObject* get_channel_type(PyObject* self, void* closure);

PyObject* PY_Channel_get_state(PY_Channel *self, void *closure);
int PY_Channel_set_state(PY_Channel *self, PyObject *value, void *closure);

ChannelData getChannelData(PY_Channel *self, bool autoCook = false);