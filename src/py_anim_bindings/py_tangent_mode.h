#pragma once

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <python3.12/Python.h>
    #include <python3.12/structmember.h>
#endif


// Getter function for the TangentMode enum
PyObject* get_tangent_mode_enum([[maybe_unused]] PyObject* self, [[maybe_unused]] void* closure);
