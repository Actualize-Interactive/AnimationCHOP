#pragma once

#include <anim/keyframe.hpp>
#include <anim/tangent_mode.hpp>
#include "py_bezier_handle.h"

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
[[maybe_unused]] PyObject* get_keyframe_type(PyObject* self, void* closure);

// Utility functions for conversion
[[maybe_unused]] PyKeyframe* KeyframeToPyKeyframe(const anim::Keyframe& keyframe);
[[maybe_unused]] PyObject* KeyframeToPyObject(const anim::Keyframe& keyframe);
[[maybe_unused]] bool PyKeyframeToKeyframe(PyKeyframe* py_keyframe, anim::Keyframe& keyframe);
[[maybe_unused]] bool PyObjectToKeyframe(PyObject* obj, anim::Keyframe& keyframe);
