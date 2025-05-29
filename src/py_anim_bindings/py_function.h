#pragma once

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <Python/Python.h>
    #include <Python/structmember.h>
#endif

#include "anim/function.hpp"

// Getter function for the Function enum
PyObject* get_function_enum(PyObject* self, void* closure);

// Helper functions
bool PyObjectToFunction(PyObject* obj, anim::Function& function_enum);
PyObject* FunctionToPyObject(anim::Function function_enum);
