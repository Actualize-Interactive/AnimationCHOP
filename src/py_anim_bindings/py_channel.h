#pragma once

#include <anim/channel.hpp>

#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
#else
	#include <Python/Python.h>
	#include <Python/structmember.h>
#endif

typedef struct {
    PyObject_HEAD
    anim::Channel* channel;  // Pointer to the actual C++ object
    bool ownsChannel;       // Whether this object owns the channel pointer
} PyChannel;


extern PyTypeObject PyChannelType;

// Utility functions for conversion
[[maybe_unused]] PyChannel* ChannelToPyChannel(anim::Channel* channel, bool ownsChannel = false);
[[maybe_unused]] PyObject* ChannelToPyObject(anim::Channel* channel, bool ownsChannel = false);
[[maybe_unused]] bool PyObjectToChannel(PyObject* obj, anim::Channel*& channel);

// Type getter for external use
[[maybe_unused]] PyObject* get_channel_type(PyObject* self, void* closure);