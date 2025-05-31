#pragma once

#include <anim/point.hpp>

#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
#else
	#include <Python/Python.h>
	#include <Python/structmember.h>
#endif

typedef struct {
    PyObject_HEAD
    anim::Point point;  // Using Point directly
} PY_Point;


extern PyTypeObject PY_PointType;

// Utility functions for conversion
PY_Point* PointToPY_Point(const anim::Point& point);
PyObject* PointToPY_Object(const anim::Point& point);
bool PY_ObjectToPoint(PyObject* obj, anim::Point& point);

// Type getter for external use
PyObject* get_point_type(PyObject* self, void* closure);
