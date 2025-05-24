#pragma once

#include <anim/point2d.hpp>

#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
#else
	#include <Python/Python.h>
	#include <Python/structmember.h>
#endif

typedef struct {
    PyObject_HEAD
    anim::Point2D point;  // The actual C++ object
} PyPoint2D;


extern PyTypeObject PyPoint2DType;

// Utility functions for conversion
[[maybe_unused]] PyPoint2D* Point2DToPyPoint2D(const anim::Point2D& point);
[[maybe_unused]] PyObject* Point2DToPyObject(const anim::Point2D& point);
[[maybe_unused]] bool PyObjectToPoint2D(PyObject* obj, anim::Point2D& point);

// Type getter for external use
[[maybe_unused]] PyObject* get_point2d_type(PyObject* self, void* closure);