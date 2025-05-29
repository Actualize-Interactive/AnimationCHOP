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
} PyPoint;


extern PyTypeObject PyPointType;

// Utility functions for conversion
PyPoint* PointToPyPoint(const anim::Point& point);
PyObject* PointToPyObject(const anim::Point& point);
bool PyObjectToPoint(PyObject* obj, anim::Point& point);

// Type getter for external use
PyObject* get_point_type(PyObject* self, void* closure);
