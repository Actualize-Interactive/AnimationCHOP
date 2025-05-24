#pragma once

#include <anim/bezier_utils.hpp>
#include <anim/bezier_handle.hpp>
#include "point2d.hpp"
#include "py_point.h"
#include "py_bezier_handle.h"

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <python3.12/Python.h>
    #include <python3.12/structmember.h>
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
    
    // Extract BezierHandle objects
    anim::BezierHandle p0, p1, p2, p3;
    
    // Try to convert the input parameters to BezierHandle
    if (!PyObjectToBezierHandle(p0_obj, p0) || !PyObjectToBezierHandle(p1_obj, p1) || 
        !PyObjectToBezierHandle(p2_obj, p2) || !PyObjectToBezierHandle(p3_obj, p3)) {
        // If direct conversion fails, try with Point2D for backward compatibility
        anim::Point2D temp_p0, temp_p1, temp_p2, temp_p3;
        
        auto extract_point = [](PyObject* obj, anim::Point2D& point) -> bool {
            if (!PyObject_HasAttrString(obj, "time") || !PyObject_HasAttrString(obj, "value")) {
                PyErr_SetString(PyExc_TypeError, "Expected BezierHandle or Point2D object");
                return false;
            }
            
            PyObject* py_time = PyObject_GetAttrString(obj, "time");
            PyObject* py_value = PyObject_GetAttrString(obj, "value");
            
            if (!PyFloat_Check(py_time) || !PyFloat_Check(py_value)) {
                Py_XDECREF(py_time);
                Py_XDECREF(py_value);
                PyErr_SetString(PyExc_TypeError, "Time and value attributes must be floats");
                return false;
            }
            
            point.time = PyFloat_AsDouble(py_time);
            point.value = PyFloat_AsDouble(py_value);
            
            Py_DECREF(py_time);
            Py_DECREF(py_value);
            return true;
        };
        
        if (!extract_point(p0_obj, temp_p0) || !extract_point(p1_obj, temp_p1) || 
            !extract_point(p2_obj, temp_p2) || !extract_point(p3_obj, temp_p3)) {
            return NULL;
        }
        
        // Convert Point2D to BezierHandle
        p0 = anim::BezierHandle(temp_p0.time, temp_p0.value);
        p1 = anim::BezierHandle(temp_p1.time, temp_p1.value);
        p2 = anim::BezierHandle(temp_p2.time, temp_p2.value);
        p3 = anim::BezierHandle(temp_p3.time, temp_p3.value);
    }
    
    try {
        anim::BezierHandle result = anim::bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
        // Return a BezierHandle object
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
    
    // Extract BezierHandle objects
    anim::BezierHandle p0, p1, p2, p3;
    
    // Try to convert the input parameters to BezierHandle
    if (!PyObjectToBezierHandle(p0_obj, p0) || !PyObjectToBezierHandle(p1_obj, p1) || 
        !PyObjectToBezierHandle(p2_obj, p2) || !PyObjectToBezierHandle(p3_obj, p3)) {
        // If direct conversion fails, try with Point2D for backward compatibility
        anim::Point2D temp_p0, temp_p1, temp_p2, temp_p3;
        
        auto extract_point = [](PyObject* obj, anim::Point2D& point) -> bool {
            if (!PyObject_HasAttrString(obj, "time") || !PyObject_HasAttrString(obj, "value")) {
                PyErr_SetString(PyExc_TypeError, "Expected BezierHandle or Point2D object");
                return false;
            }
            
            PyObject* py_time = PyObject_GetAttrString(obj, "time");
            PyObject* py_value = PyObject_GetAttrString(obj, "value");
            
            if (!PyFloat_Check(py_time) || !PyFloat_Check(py_value)) {
                Py_XDECREF(py_time);
                Py_XDECREF(py_value);
                PyErr_SetString(PyExc_TypeError, "Time and value attributes must be floats");
                return false;
            }
            
            point.time = PyFloat_AsDouble(py_time);
            point.value = PyFloat_AsDouble(py_value);
            
            Py_DECREF(py_time);
            Py_DECREF(py_value);
            return true;
        };
        
        if (!extract_point(p0_obj, temp_p0) || !extract_point(p1_obj, temp_p1) || 
            !extract_point(p2_obj, temp_p2) || !extract_point(p3_obj, temp_p3)) {
            return NULL;
        }
        
        // Convert Point2D to BezierHandle
        p0 = anim::BezierHandle(temp_p0.time, temp_p0.value);
        p1 = anim::BezierHandle(temp_p1.time, temp_p1.value);
        p2 = anim::BezierHandle(temp_p2.time, temp_p2.value);
        p3 = anim::BezierHandle(temp_p3.time, temp_p3.value);
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
    
    // Extract BezierHandle objects
    anim::BezierHandle p0, p3;
    
    // Try to convert the input parameters to BezierHandle
    if (!PyObjectToBezierHandle(p0_obj, p0) || !PyObjectToBezierHandle(p3_obj, p3)) {
        // If direct conversion fails, try with Point2D for backward compatibility
        anim::Point2D temp_p0, temp_p3;
        
        auto extract_point = [](PyObject* obj, anim::Point2D& point) -> bool {
            if (!PyObject_HasAttrString(obj, "time") || !PyObject_HasAttrString(obj, "value")) {
                PyErr_SetString(PyExc_TypeError, "Expected BezierHandle or Point2D object");
                return false;
            }
            
            PyObject* py_time = PyObject_GetAttrString(obj, "time");
            PyObject* py_value = PyObject_GetAttrString(obj, "value");
            
            if (!PyFloat_Check(py_time) || !PyFloat_Check(py_value)) {
                Py_XDECREF(py_time);
                Py_XDECREF(py_value);
                PyErr_SetString(PyExc_TypeError, "Time and value attributes must be floats");
                return false;
            }
            
            point.time = PyFloat_AsDouble(py_time);
            point.value = PyFloat_AsDouble(py_value);
            
            Py_DECREF(py_time);
            Py_DECREF(py_value);
            return true;
        };
        
        if (!extract_point(p0_obj, temp_p0) || !extract_point(p3_obj, temp_p3)) {
            return NULL;
        }
        
        // Convert Point2D to BezierHandle
        p0 = anim::BezierHandle(temp_p0.time, temp_p0.value);
        p3 = anim::BezierHandle(temp_p3.time, temp_p3.value);
    }
    
    try {
        anim::BezierHandle p1, p2;
        anim::bezier_utils::create_linear_bezier_handles(p0, p3, p1, p2);
        
        // Create BezierHandle objects
        PyBezierHandle* p1_handle = BezierHandleToPyBezierHandle(p1);
        if (p1_handle == NULL) {
            return NULL;
        }
        
        PyBezierHandle* p2_handle = BezierHandleToPyBezierHandle(p2);
        if (p2_handle == NULL) {
            Py_DECREF(p1_handle);
            return NULL;
        }
        
        PyObject* result_tuple = PyTuple_Pack(2, p1_handle, p2_handle);
        Py_DECREF(p1_handle);
        Py_DECREF(p2_handle);
        
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
    
    // Extract BezierHandle object
    anim::BezierHandle keyframe_point;
    
    // Try to convert the input parameter to BezierHandle
    if (!PyObjectToBezierHandle(keyframe_point_obj, keyframe_point)) {
        // If direct conversion fails, try with Point2D for backward compatibility
        anim::Point2D temp_point;
        
        auto extract_point = [](PyObject* obj, anim::Point2D& point) -> bool {
            if (!PyObject_HasAttrString(obj, "time") || !PyObject_HasAttrString(obj, "value")) {
                PyErr_SetString(PyExc_TypeError, "Expected BezierHandle or Point2D object");
                return false;
            }
            
            PyObject* py_time = PyObject_GetAttrString(obj, "time");
            PyObject* py_value = PyObject_GetAttrString(obj, "value");
            
            if (!PyFloat_Check(py_time) || !PyFloat_Check(py_value)) {
                Py_XDECREF(py_time);
                Py_XDECREF(py_value);
                PyErr_SetString(PyExc_TypeError, "Time and value attributes must be floats");
                return false;
            }
            
            point.time = PyFloat_AsDouble(py_time);
            point.value = PyFloat_AsDouble(py_value);
            
            Py_DECREF(py_time);
            Py_DECREF(py_value);
            return true;
        };
        
        if (!extract_point(keyframe_point_obj, temp_point)) {
            return NULL;
        }
        
        // Convert Point2D to BezierHandle
        keyframe_point = anim::BezierHandle(temp_point.time, temp_point.value);
    }
    
    try {
        anim::BezierHandle in_handle, out_handle;
        anim::bezier_utils::create_flat_bezier_handles(keyframe_point, time_offset, in_handle, out_handle);
        
        // Create BezierHandle objects
        PyBezierHandle* in_handle_obj = BezierHandleToPyBezierHandle(in_handle);
        if (in_handle_obj == NULL) {
            return NULL;
        }
        
        PyBezierHandle* out_handle_obj = BezierHandleToPyBezierHandle(out_handle);
        if (out_handle_obj == NULL) {
            Py_DECREF(in_handle_obj);
            return NULL;
        }
        
        PyObject* result_tuple = PyTuple_Pack(2, in_handle_obj, out_handle_obj);
        Py_DECREF(in_handle_obj);
        Py_DECREF(out_handle_obj);
        
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
