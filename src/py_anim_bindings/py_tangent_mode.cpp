#include "py_tangent_mode.h"
#include <anim/tangent_mode.hpp>

static PyObject* tangent_mode_enum_singleton = NULL;

static PyObject* create_tangent_mode_enum() {
    // If we already have a singleton instance, return that
    if (tangent_mode_enum_singleton) {
        Py_INCREF(tangent_mode_enum_singleton);
        return tangent_mode_enum_singleton;
    }
    
    PyObject* enum_module = NULL;
    PyObject* int_enum_class = NULL;
    PyObject* members_dict = NULL;
    PyObject* tangent_mode_enum_name = NULL;
    PyObject* tangent_mode_enum_type = NULL; // The created enum type

    enum_module = PyImport_ImportModule("enum");
    if (!enum_module) {
        // PyImport_ImportModule sets an error
        return NULL;
    }

    int_enum_class = PyObject_GetAttrString(enum_module, "IntEnum");
    if (!int_enum_class) {
        Py_DECREF(enum_module); // enum_module is guaranteed non-NULL if int_enum_class was attempted
        return NULL; 
    }

    members_dict = PyDict_New();
    if (!members_dict) {
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module); // enum_module is guaranteed non-NULL if int_enum_class was attempted
        return NULL; 
    }

    // Helper lambda to add members and check for errors
    auto add_member = [&](const char* name, anim::TangentMode val) -> bool {
        PyObject* py_val = PyLong_FromLong(static_cast<long>(val));
        if (!py_val) { // PyLong_FromLong failed, error is set
            return false;
        }
        int result = PyDict_SetItemString(members_dict, name, py_val);
        Py_DECREF(py_val); // PyDict_SetItemString INCREFs on success, so we DECREF our ref.
        if (result < 0) { // PyDict_SetItemString failed, error is set
            return false;
        }
        return true;
    };

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

    tangent_mode_enum_name = PyUnicode_FromString("TangentMode");
    if (!tangent_mode_enum_name) {
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL; 
    }

    // Create the IntEnum type: IntEnum("TangentMode", {"LINEAR": 0, ...})
    tangent_mode_enum_type = PyObject_CallFunctionObjArgs(int_enum_class, tangent_mode_enum_name, members_dict, NULL);
    if (!tangent_mode_enum_type) {
        Py_DECREF(tangent_mode_enum_name);
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL; 
    }    

    // Success path: Clean up intermediate objects
    Py_DECREF(tangent_mode_enum_name);
    Py_DECREF(members_dict);
    Py_DECREF(int_enum_class);
    Py_DECREF(enum_module);
    
    // Store the created enum as our singleton
    tangent_mode_enum_singleton = tangent_mode_enum_type;
    return tangent_mode_enum_type; // Return new reference to the created enum type
}

// Getter function for the TangentMode enum
PyObject* get_tangent_mode_enum([[maybe_unused]] PyObject* self, [[maybe_unused]] void* closure) {
    return create_tangent_mode_enum();
}