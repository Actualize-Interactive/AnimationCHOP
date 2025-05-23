#pragma once

#include <anim/keyframe.hpp>

#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
#else
	#include <Python/Python.h>
	#include <Python/structmember.h>
#endif

typedef struct {
    PyObject_HEAD
    anim::Keyframe* keyframe;  // Pointer to the actual C++ object
} PyKeyframe;


extern PyTypeObject PyKeyframeType;

// Utility functions for conversion
[[maybe_unused]] PyKeyframe* KeyframeToPyKeyframe(anim::Keyframe* keyframe);
[[maybe_unused]] PyObject* KeyframeToPyObject(anim::Keyframe* keyframe);
[[maybe_unused]] bool PyObjectToKeyframe(PyObject* obj, anim::Keyframe*& keyframe);

// Type getter for external use
[[maybe_unused]] PyObject* get_keyframe_type(PyObject* self, void* closure);