#pragma once

#include <anim/animation.hpp>
#include "py_channel.h" // For PyChannelType and ChannelToPyObject

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <Python/Python.h>
    #include <Python/structmember.h>
#endif

typedef struct {
    PyObject_HEAD
    anim::Animation* animation; // Pointer to the animation in AnimationCHOP
    PyObject* parent;           // Reference to the parent AnimationCHOP to keep it alive
} PyAnimation;

extern PyTypeObject PyAnimationType;

// Type getter for external use
PyObject* get_animation_type(PyObject* self, void* closure);

// Helper functions for conversion between C++ and Python
PyObject* AnimationToPyObject(anim::Animation* animation, PyObject* parent);
bool PyObjectToAnimation(PyObject* obj, anim::Animation*& animation);
