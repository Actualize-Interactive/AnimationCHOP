#pragma once

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <Python/Python.h>
    #include <Python/structmember.h>
#endif

#include <anim/handle_mode.hpp>

// Getter function for the HandleMode enum
PyObject* get_handle_mode_enum(PyObject* self, void* closure);

const char* handle_mode_to_string(anim::HandleMode mode);
anim::HandleMode string_to_handle_mode(const char* str);