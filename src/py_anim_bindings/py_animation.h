#pragma once

#include <anim/animation.hpp>

#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
#else
	#include <Python/Python.h>
	#include <Python/structmember.h>
#endif

typedef struct {
    PyObject_HEAD
    anim::Animation* animation;  // Pointer to the actual C++ object
    bool ownsAnimation;         // Whether this object owns the animation pointer
} PyAnimation;


extern PyTypeObject PyAnimationType;

// Utility functions for conversion
[[maybe_unused]] PyAnimation* AnimationToPyAnimation(anim::Animation* animation, bool ownsAnimation = false);
[[maybe_unused]] PyObject* AnimationToPyObject(anim::Animation* animation, bool ownsAnimation = false);
[[maybe_unused]] bool PyObjectToAnimation(PyObject* obj, anim::Animation*& animation);

// Type getter for external use
[[maybe_unused]] PyObject* get_animation_type(PyObject* self, void* closure);