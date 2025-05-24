#pragma once

#include <anim/bezier_utils.hpp>
#include <anim/point2d.hpp>
#include <anim/bezier_handle.hpp>
#include "py_point.h"

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
    
    // Extract Point2D objects
    anim::Point2D p0, p1, p2, p3;
    
    // Helper function to extract a Point2D from a Python object
    auto extract_point = [](PyObject* obj, anim::Point2D& point) -> bool {
        if (!PyObject_HasAttrString(obj, "time") || !PyObject_HasAttrString(obj, "value")) {
            PyErr_SetString(PyExc_TypeError, "Expected Point2D object");
            return false;
        }
        
        PyObject* py_time = PyObject_GetAttrString(obj, "time");
        PyObject* py_value = PyObject_GetAttrString(obj, "value");
        
        if (!PyFloat_Check(py_time) || !PyFloat_Check(py_value)) {
            Py_XDECREF(py_time);
            Py_XDECREF(py_value);
            PyErr_SetString(PyExc_TypeError, "Point2D attributes must be floats");
            return false;
        }
        
        point.time = PyFloat_AsDouble(py_time);
        point.value = PyFloat_AsDouble(py_value);
        
        Py_DECREF(py_time);
        Py_DECREF(py_value);
        return true;
    };
    
    if (!extract_point(p0_obj, p0) || !extract_point(p1_obj, p1) || 
        !extract_point(p2_obj, p2) || !extract_point(p3_obj, p3)) {
        return NULL;
    }
    
    try {
        anim::Point2D result = anim::bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
          // Create a Point2D object directly
        PyPoint2D* py_point = PyObject_New(PyPoint2D, &PyPoint2DType);
        if (py_point == NULL) {
            return NULL;
        }
        
        py_point->point = result;
        return (PyObject*)py_point;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
    return NULL;
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
    
    // Extract Point2D objects
    anim::Point2D p0, p1, p2, p3;
    
    // Helper function to extract a Point2D from a Python object
    auto extract_point = [](PyObject* obj, anim::Point2D& point) -> bool {
        if (!PyObject_HasAttrString(obj, "time") || !PyObject_HasAttrString(obj, "value")) {
            PyErr_SetString(PyExc_TypeError, "Expected Point2D object");
            return false;
        }
        
        PyObject* py_time = PyObject_GetAttrString(obj, "time");
        PyObject* py_value = PyObject_GetAttrString(obj, "value");
        
        if (!PyFloat_Check(py_time) || !PyFloat_Check(py_value)) {
            Py_XDECREF(py_time);
            Py_XDECREF(py_value);
            PyErr_SetString(PyExc_TypeError, "Point2D attributes must be floats");
            return false;
        }
        
        point.time = PyFloat_AsDouble(py_time);
        point.value = PyFloat_AsDouble(py_value);
        
        Py_DECREF(py_time);
        Py_DECREF(py_value);
        return true;
    };
    
    if (!extract_point(p0_obj, p0) || !extract_point(p1_obj, p1) || 
        !extract_point(p2_obj, p2) || !extract_point(p3_obj, p3)) {
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
    
    // Extract Point2D objects
    anim::Point2D p0, p3;
    
    // Helper function to extract a Point2D from a Python object
    auto extract_point = [](PyObject* obj, anim::Point2D& point) -> bool {
        if (!PyObject_HasAttrString(obj, "time") || !PyObject_HasAttrString(obj, "value")) {
            PyErr_SetString(PyExc_TypeError, "Expected Point2D object");
            return false;
        }
        
        PyObject* py_time = PyObject_GetAttrString(obj, "time");
        PyObject* py_value = PyObject_GetAttrString(obj, "value");
        
        if (!PyFloat_Check(py_time) || !PyFloat_Check(py_value)) {
            Py_XDECREF(py_time);
            Py_XDECREF(py_value);
            PyErr_SetString(PyExc_TypeError, "Point2D attributes must be floats");
            return false;
        }
        
        point.time = PyFloat_AsDouble(py_time);
        point.value = PyFloat_AsDouble(py_value);
        
        Py_DECREF(py_time);
        Py_DECREF(py_value);
        return true;
    };
    
    if (!extract_point(p0_obj, p0) || !extract_point(p3_obj, p3)) {
        return NULL;
    }
    
    try {
        anim::BezierHandle p1, p2;
        anim::bezier_utils::create_linear_bezier_handles(p0, p3, p1, p2);
          // Create Point2D objects directly
        PyPoint2D* p1_point = PyObject_New(PyPoint2D, &PyPoint2DType);
        if (p1_point == NULL) {
            return NULL;
        }
        p1_point->point.time = p1.time;
        p1_point->point.value = p1.value;
        
        PyPoint2D* p2_point = PyObject_New(PyPoint2D, &PyPoint2DType);
        if (p2_point == NULL) {
            Py_DECREF(p1_point);
            return NULL;
        }
        p2_point->point.time = p2.time;
        p2_point->point.value = p2.value;
        
        PyObject* result_tuple = PyTuple_Pack(2, p1_point, p2_point);
        Py_DECREF(p1_point);
        Py_DECREF(p2_point);
        
        return result_tuple;
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return NULL;
    }
}

// Function to create flat handle points for a keyframe
static PyObject* py_create_flat_bezier_handles(PyObject* self, PyObject* args) {
    PyObject* keyframe_point_obj = NULL;
    double time_offset = 0.1;
    
    if (!PyArg_ParseTuple(args, "O|d", &keyframe_point_obj, &time_offset))
        return NULL;
    
    // Extract Point2D object
    anim::Point2D keyframe_point;
    
    // Helper function to extract a Point2D from a Python object
    auto extract_point = [](PyObject* obj, anim::Point2D& point) -> bool {
        if (!PyObject_HasAttrString(obj, "time") || !PyObject_HasAttrString(obj, "value")) {
            PyErr_SetString(PyExc_TypeError, "Expected Point2D object");
            return false;
        }
        
        PyObject* py_time = PyObject_GetAttrString(obj, "time");
        PyObject* py_value = PyObject_GetAttrString(obj, "value");
        
        if (!PyFloat_Check(py_time) || !PyFloat_Check(py_value)) {
            Py_XDECREF(py_time);
            Py_XDECREF(py_value);
            PyErr_SetString(PyExc_TypeError, "Point2D attributes must be floats");
            return false;
        }
        
        point.time = PyFloat_AsDouble(py_time);
        point.value = PyFloat_AsDouble(py_value);
        
        Py_DECREF(py_time);
        Py_DECREF(py_value);
        return true;
    };
    
    if (!extract_point(keyframe_point_obj, keyframe_point)) {
        return NULL;
    }
    
    try {
        anim::BezierHandle in_handle, out_handle;
        anim::bezier_utils::create_flat_bezier_handles(keyframe_point, time_offset, in_handle, out_handle);
        // Create Point2D objects for Python compatibility
        PyPoint2D* in_handle_point = PyObject_New(PyPoint2D, &PyPoint2DType);
        if (in_handle_point == NULL) {
            return NULL;
        }
        in_handle_point->point.time = in_handle.time;
        in_handle_point->point.value = in_handle.value;
        
        PyPoint2D* out_handle_point = PyObject_New(PyPoint2D, &PyPoint2DType);
        if (out_handle_point == NULL) {
            Py_DECREF(in_handle_point);
            return NULL;
        }
        out_handle_point->point.time = out_handle.time;
        out_handle_point->point.value = out_handle.value;
        
        PyObject* result_tuple = PyTuple_Pack(2, in_handle_point, out_handle_point);
        Py_DECREF(in_handle_point);
        Py_DECREF(out_handle_point);
        
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
     "Create flat handle points for a keyframe"},
    {NULL, NULL, 0, NULL}  // Sentinel
};
