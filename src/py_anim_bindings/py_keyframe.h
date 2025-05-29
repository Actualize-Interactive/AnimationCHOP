#pragma once

#include <anim/keyframe.hpp>
#include <anim/handle_mode.hpp>
#include <anim/function.hpp>
#include "py_point.h"

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <Python/Python.h>
    #include <Python/structmember.h>
#endif

typedef struct {
    PyObject_HEAD
    anim::Keyframe keyframe;
} PyKeyframe;

extern PyTypeObject PyKeyframeType;

// Type getter for external use
PyObject* get_keyframe_type(PyObject* self, void* closure);

// Utility functions for conversion
PyKeyframe* KeyframeToPyKeyframe(const anim::Keyframe& keyframe);
PyObject* KeyframeToPyObject(const anim::Keyframe& keyframe);
bool PyKeyframeToKeyframe(PyKeyframe* py_keyframe, anim::Keyframe& keyframe);
bool PyObjectToKeyframe(PyObject* obj, anim::Keyframe& keyframe);
