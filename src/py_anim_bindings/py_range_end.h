#ifndef PY_RANGE_END_H
#define PY_RANGE_END_H

#include <Python.h>
#include <anim/range_end.hpp>

// Getter function for the RangeEnd enum
PyObject* get_range_end_enum(PyObject* self, void* closure);

// Cleanup function for module shutdown
void cleanup_range_end_enum();

// Parses an optional range_end argument accepted by the sampling methods.
//
// Returns false with a Python error set if the object is not a valid RangeEnd.
// A null object leaves `range_end` alone, so callers seed it with the default
// and can pass an omitted argument straight through.
bool PY_ObjectToRangeEnd(PyObject* obj, anim::RangeEnd& range_end);

#endif // PY_RANGE_END_H
