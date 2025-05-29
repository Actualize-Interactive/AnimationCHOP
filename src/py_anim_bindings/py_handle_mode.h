#pragma once

#ifdef _WIN32
    #include <Python.h>
    #include <structmember.h>
#else
    #include <Python/Python.h>
    #include <Python/structmember.h>
#endif


// Getter function for the HandleMode enum
PyObject* get_handle_mode_enum(PyObject* self, void* closure);
