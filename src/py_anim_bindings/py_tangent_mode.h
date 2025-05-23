#pragma once

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <Python/Python.h>
    #include <Python/structmember.h>
#endif


// Getter function for the TangentMode enum
PyObject* get_tangent_mode_enum([[maybe_unused]] PyObject* self, [[maybe_unused]] void* closure);