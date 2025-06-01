#pragma once

#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
	#include <modsupport.h>
#else
	#include <Python.h>
	#include <structmember.h>
#endif

#include <anim/handle_mode.hpp>

// Getter function for the HandleMode enum
PyObject* get_handle_mode_enum(PyObject* self, void* closure);

// Cleanup function for module shutdown
void cleanup_handle_mode_enum();

const char* handle_mode_to_string(anim::HandleMode mode);
anim::HandleMode string_to_handle_mode(const char* str);