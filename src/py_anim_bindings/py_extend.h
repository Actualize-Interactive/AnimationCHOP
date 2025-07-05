#ifndef PY_EXTEND_H
#define PY_EXTEND_H

#include <Python.h>
#include <anim/extend.hpp>

// Getter function for the Extend enum
PyObject* get_extend_enum(PyObject* self, void* closure);

// Cleanup function for module shutdown
void cleanup_extend_enum();

// Helper functions for state serialization
const char* extend_to_string(anim::Extend extend);
anim::Extend string_to_extend(const char* str);

#endif // PY_EXTEND_H
