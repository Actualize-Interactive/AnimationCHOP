#pragma once

#include <anim/keyframe.hpp>
#include <anim/handle_mode.hpp>
#include <anim/function.hpp>
#include "py_point.h"

#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
	#include <modsupport.h>
#else
	#include <Python.h>
	#include <structmember.h>
#endif

typedef struct {
    PyObject_HEAD
    anim::Keyframe keyframe;
} PY_Keyframe;

extern PyTypeObject PY_KeyframeType;


// Utility functions for conversion
PY_Keyframe* KeyframeToPY_Keyframe(const anim::Keyframe& keyframe);
PyObject* KeyframeToPY_Object(const anim::Keyframe& keyframe);
bool PY_KeyframeToKeyframe(PY_Keyframe* py_keyframe, anim::Keyframe& keyframe);
bool PY_ObjectToKeyframe(PyObject* obj, anim::Keyframe& keyframe);


// Type getter for external use
PyObject* get_keyframe_type(PyObject* self, void* closure);

// Forward declare Point state functions for use in keyframe
extern PyObject* PY_Point_get_state(PY_Point *self, void *closure);
extern int PY_Point_set_state(PY_Point *self, PyObject *value, void *closure);

// State function declarations
extern PyObject* PY_Keyframe_get_state(PY_Keyframe *self, void *closure);
extern int PY_Keyframe_set_state(PY_Keyframe *self, PyObject *value, void *closure);