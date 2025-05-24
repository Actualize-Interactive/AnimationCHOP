#pragma once

#include <anim/bezier_utils.hpp>
#include "py_bezier_handle.h"

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <Python/Python.h>
    #include <Python/structmember.h>
#endif

// Module function definitions for bezier_utils namespace

// Function to evaluate a cubic Bézier curve at a specific parameter value
static PyObject* py_evaluate_cubic_bezier(PyObject* self, PyObject* args) {
    PyObject* p0_obj = NULL;
    PyObject* p1_obj = NULL;
    PyObject* p2_obj = NULL;
    PyObject* p3_obj = NULL;
    double t = 0.0;
    
    if (!PyArg_ParseTuple(args, "OOOOd", &p0_obj, &p1_obj, &p2_obj, &p3_obj, &t))
        return NULL;
    
    // Extract BezierHandle objects directly
    anim::BezierHandle p0, p1, p2, p3;
    
    if (!PyObjectToBezierHandle(p0_obj, p0) || !PyObjectToBezierHandle(p1_obj, p1) || 
        !PyObjectToBezierHandle(p2_obj, p2) || !PyObjectToBezierHandle(p3_obj, p3)) {
        return NULL;
    }
    
    try {
        anim::BezierHandle result = anim::bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
        return BezierHandleToPyObject(result);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// Function to find the parameter t that corresponds to a specific time value
static PyObject* py_find_parameter_for_time(PyObject* self, PyObject* args, PyObject* kwargs) {
    PyObject* p0_obj = NULL;
    PyObject* p1_obj = NULL;
    PyObject* p2_obj = NULL;
    PyObject* p3_obj = NULL;
    double target_time = 0.0;
    double precision = 1e-6;
    int max_iterations = 30;
    
    static char* kwlist[] = {
        (char*)"p0", (char*)"p1", (char*)"p2", (char*)"p3", 
        (char*)"target_time", (char*)"precision", (char*)"max_iterations", NULL
    };
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOOOd|di", kwlist, 
                                   &p0_obj, &p1_obj, &p2_obj, &p3_obj, 
                                   &target_time, &precision, &max_iterations))
        return NULL;
    
    // Extract BezierHandle objects directly
    anim::BezierHandle p0, p1, p2, p3;
    
    if (!PyObjectToBezierHandle(p0_obj, p0) || !PyObjectToBezierHandle(p1_obj, p1) || 
        !PyObjectToBezierHandle(p2_obj, p2) || !PyObjectToBezierHandle(p3_obj, p3)) {
        return NULL;
    }
    
    try {
        double parameter = anim::bezier_utils::find_parameter_for_time(
            p0, p1, p2, p3, target_time, precision, max_iterations);
            
        return PyFloat_FromDouble(parameter);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// Function to create control points for a linear Bézier curve
static PyObject* py_create_linear_bezier_handles(PyObject* self, PyObject* args) {
    PyObject* p0_obj = NULL;
    PyObject* p3_obj = NULL;
    
    if (!PyArg_ParseTuple(args, "OO", &p0_obj, &p3_obj))
        return NULL;
    
    // Extract BezierHandle objects directly
    anim::BezierHandle p0, p3;
    
    if (!PyObjectToBezierHandle(p0_obj, p0) || !PyObjectToBezierHandle(p3_obj, p3)) {
        return NULL;
    }
    
    try {
        anim::BezierHandle p1, p2;
        anim::bezier_utils::create_linear_bezier_handles(p0, p3, p1, p2);
        
        PyObject* p1_py = BezierHandleToPyObject(p1);
        if (p1_py == NULL) {
            return NULL;
        }
        
        PyObject* p2_py = BezierHandleToPyObject(p2);
        if (p2_py == NULL) {
            Py_DECREF(p1_py);
            return NULL;
        }
        
        PyObject* result_tuple = PyTuple_Pack(2, p1_py, p2_py);
        Py_DECREF(p1_py);
        Py_DECREF(p2_py);
        
        return result_tuple;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// Function to create flat tangent handles for a keyframe
static PyObject* py_create_flat_bezier_handles(PyObject* self, PyObject* args) {
    PyObject* keyframe_point_obj = NULL;
    double time_offset = 0.1; // Default value, can be overridden
    
    // Allow time_offset to be optional
    if (!PyArg_ParseTuple(args, "O|d", &keyframe_point_obj, &time_offset))
        return NULL;
    
    // Extract BezierHandle object directly
    anim::BezierHandle keyframe_point;
    
    if (!PyObjectToBezierHandle(keyframe_point_obj, keyframe_point)) {
        return NULL;
    }
    
    try {
        anim::BezierHandle in_handle, out_handle;
        anim::bezier_utils::create_flat_bezier_handles(keyframe_point, time_offset, in_handle, out_handle);

        PyObject* in_handle_py = BezierHandleToPyObject(in_handle);
        if (in_handle_py == NULL) {
            return NULL;
        }
        
        PyObject* out_handle_py = BezierHandleToPyObject(out_handle);
        if (out_handle_py == NULL) {
            Py_DECREF(in_handle_py);
            return NULL;
        }
        
        PyObject* result_tuple = PyTuple_Pack(2, in_handle_py, out_handle_py);
        Py_DECREF(in_handle_py);
        Py_DECREF(out_handle_py);
        
        return result_tuple;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}
// Method definitions for BezierUtils namespace
static PyMethodDef py_bezier_utils_methods[] = {
    {"evaluate_cubic_bezier", (PyCFunction)py_evaluate_cubic_bezier, METH_VARARGS,
     "Evaluate a cubic Bézier curve at a specific parameter value"},
    {"find_parameter_for_time", (PyCFunction)py_find_parameter_for_time, METH_VARARGS | METH_KEYWORDS,
     "Find the parameter t that corresponds to a specific time value on a Bézier curve"},
    {"create_linear_bezier_handles", (PyCFunction)py_create_linear_bezier_handles, METH_VARARGS,
     "Create control points for a linear Bézier curve"},
    {"create_flat_bezier_handles", (PyCFunction)py_create_flat_bezier_handles, METH_VARARGS,
     "Create flat tangent handles for a keyframe"},
    {NULL, NULL, 0, NULL}  // Sentinel
};
