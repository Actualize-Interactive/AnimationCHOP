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

// Cleanup function for module shutdown
void cleanup_function_enum();

// Helper functions
bool PY_ObjectToFunction(PyObject* obj, anim::Function& function_enum);
PyObject* FunctionToPY_Object(anim::Function function_enum);


const char* function_to_string(anim::Function func);
anim::Function string_to_function(const char* str);