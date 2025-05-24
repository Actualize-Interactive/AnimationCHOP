#pragma once

#include <anim/bezier_handle.hpp>

#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
#else
	#include <Python/Python.h>
	#include <Python/structmember.h>
#endif

typedef struct {
    PyObject_HEAD
    anim::BezierHandle point;  // Using BezierHandle directly
} PyBezierHandle;


extern PyTypeObject PyBezierHandleType;

// Utility functions for conversion
[[maybe_unused]] PyBezierHandle* BezierHandleToPyBezierHandle(const anim::BezierHandle& handle);
[[maybe_unused]] PyObject* BezierHandleToPyObject(const anim::BezierHandle& handle);
[[maybe_unused]] bool PyObjectToBezierHandle(PyObject* obj, anim::BezierHandle& handle);

// Type getter for external use
[[maybe_unused]] PyObject* get_bezier_handle_type(PyObject* self, void* closure);