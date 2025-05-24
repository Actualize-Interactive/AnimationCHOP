#pragma once

#include <anim/animation.hpp>
#include "py_channel.h" // For PyChannelType and ChannelToPyObject
// py_point.h and py_tangent_mode.h are not directly used by PyAnimation declarations
// but might be included if anim::Animation itself exposes types from them.
// For now, let's assume they are not strictly needed in this header.

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <Python/Python.h>
    #include <Python/structmember.h>
#endif

typedef struct {
    PyObject_HEAD
    anim::Animation* animation_ptr; 
    bool is_owner; // Indicates if this object owns the animation
} PyAnimation;

extern PyTypeObject PyAnimationType;

PyObject* PyAnimation_WrapExisting(anim::Animation& animation, bool owned_by_python_wrapper = false);

// Type getter for external use
[[maybe_unused]] PyObject* get_animation_type(PyObject* self, void* closure);

// Utility functions for conversion
[[maybe_unused]] PyAnimation* AnimationToPyAnimation(const anim::Animation& animation);
[[maybe_unused]] PyObject* AnimationToPyObject(const anim::Animation& animation);
[[maybe_unused]] bool PyObjectToAnimation(PyObject* obj, anim::Animation& animation);
