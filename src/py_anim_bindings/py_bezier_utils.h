#pragma once

#include <anim/bezier.hpp>

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <Python/Python.h>
    #include <Python/structmember.h>
#endif

// Simple bezier utility functions for Python
static PyObject* py_evaluate_bezier(PyObject* self, PyObject* args) {
    double p0, p1, p2, p3, t;
    
    if (!PyArg_ParseTuple(args, "ddddd", &p0, &p1, &p2, &p3, &t)) {
        return NULL;
    }
    
    double result = anim::bezier::evaluate(p0, p1, p2, p3, t);
    return PyFloat_FromDouble(result);
}

static PyObject* py_evaluate_bezier2d(PyObject* self, PyObject* args) {
    double p0_x, p0_y, p1_x, p1_y, p2_x, p2_y, p3_x, p3_y, t;
    
    if (!PyArg_ParseTuple(args, "ddddddddd", &p0_x, &p0_y, &p1_x, &p1_y, &p2_x, &p2_y, &p3_x, &p3_y, &t)) {
        return NULL;
    }
    
    anim::Point2D p0(p0_x, p0_y);
    anim::Point2D p1(p1_x, p1_y);
    anim::Point2D p2(p2_x, p2_y);
    anim::Point2D p3(p3_x, p3_y);
    
    anim::Point2D result = anim::bezier::evaluate(p0, p1, p2, p3, t);
    
    // Return a tuple (x, y)
    return Py_BuildValue("(dd)", result.time, result.value);
}