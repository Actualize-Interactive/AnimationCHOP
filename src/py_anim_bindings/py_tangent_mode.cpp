#include "py_tangent_mode.h"
#include <anim/tangent_mode.hpp>

// Module will be created once and stored here
static PyObject* cached_tangent_mode_enum = NULL;

PyObject* get_tangent_mode_enum([[maybe_unused]] PyObject* self, [[maybe_unused]] void* closure) {
    // Return cached enum if we already created it
    if (cached_tangent_mode_enum) {
        Py_INCREF(cached_tangent_mode_enum);
        return cached_tangent_mode_enum;
    }
    
    // Otherwise create the enum module and enum
    PyObject* enum_module = PyImport_ImportModule("enum");
    if (!enum_module) {
        return NULL;
    }

    PyObject* int_enum_class = PyObject_GetAttrString(enum_module, "IntEnum");
    if (!int_enum_class) {
        Py_DECREF(enum_module);
        return NULL;
    }

    PyObject* members_dict = PyDict_New();
    if (!members_dict) {
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL;
    }

    // Helper lambda to add members and check for errors
    auto add_member = [&](const char* name, anim::TangentMode val) -> bool {
        PyObject* py_val = PyLong_FromLong(static_cast<long>(val));
        if (!py_val) {
            return false;
        }
        
        int result = PyDict_SetItemString(members_dict, name, py_val);
        Py_DECREF(py_val);
        
        if (result < 0) {
            return false;
        }
        
        return true;
    };

    // Add all tangent mode enum values
    if (!add_member("LINEAR", anim::TangentMode::linear) ||
        !add_member("FLAT", anim::TangentMode::flat) ||
        !add_member("SMOOTH_MANUAL", anim::TangentMode::smoothManual) ||
        !add_member("SMOOTH_AUTO", anim::TangentMode::smoothAuto) ||
        !add_member("STEPPED", anim::TangentMode::stepped)) {
        
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL;
    }

    PyObject* tangent_mode_enum_name = PyUnicode_FromString("TangentMode");
    if (!tangent_mode_enum_name) {
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL;
    }

    // Create the IntEnum type: IntEnum("TangentMode", {"LINEAR": 0, ...})
    PyObject* tangent_mode_enum_type = PyObject_CallFunctionObjArgs(
        int_enum_class, 
        tangent_mode_enum_name, 
        members_dict, 
        NULL
    );
    
    Py_DECREF(tangent_mode_enum_name);
    Py_DECREF(members_dict);
    Py_DECREF(int_enum_class);
    Py_DECREF(enum_module);
    
    if (!tangent_mode_enum_type) {
        return NULL;
    }

    // Store in our cached module
    cached_tangent_mode_enum = tangent_mode_enum_type;
    Py_INCREF(cached_tangent_mode_enum); 
    
    return tangent_mode_enum_type;
}