#pragma once

#include <anim/channel.hpp>
#include "py_keyframe.h"
#include "py_point.h"
#include "py_handle_mode.h"
#include "py_function.h"

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <Python/Python.h>
    #include <Python/structmember.h>
#endif

typedef struct {
    PyObject_HEAD
    anim::Channel* channel;  // Pointer to the actual channel in AnimationCHOP
    PyObject* parent;        // Reference to the parent AnimationCHOP to keep it alive
} PyChannel;

extern PyTypeObject PyChannelType;

// Type getter for external use
PyObject* get_channel_type(PyObject* self, void* closure);

// Helper functions for conversion between C++ and Python
PyObject* ChannelToPyObject(anim::Channel* channel, PyObject* parent);
bool PyObjectToChannel(PyObject* obj, anim::Channel*& channel);
