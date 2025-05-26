#pragma once

#include <anim/channel.hpp>
#include "py_keyframe.h"
#include "py_bezier_handle.h"
// py_tangent_mode.h is not strictly needed here if TangentMode is only used in .cpp
// but keeping it for now in case anim::TangentMode is used directly in other headers that include this.
#include "py_tangent_mode.h" 

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <Python/Python.h>
    #include <Python/structmember.h>
#endif

typedef struct {
    PyObject_HEAD
    anim::Channel channel;  // The actual C++ object
} PyChannel;

extern PyTypeObject PyChannelType;

// Type getter for external use
PyObject* get_channel_type(PyObject* self, void* closure);

// Helper functions for conversion between C++ and Python
PyObject* ChannelToPyObject(const anim::Channel& channel);
bool PyObjectToChannel(PyObject* obj, anim::Channel& channel);
