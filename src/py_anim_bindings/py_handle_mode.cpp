#include "py_handle_mode.h"
#include <anim/handle_mode.hpp>

static PyObject* handle_mode_enum_singleton = NULL;

static PyObject* create_handle_mode_enum() {
    // If we already have a singleton instance, return that
    if (handle_mode_enum_singleton) {
        Py_INCREF(handle_mode_enum_singleton);
        return handle_mode_enum_singleton;
    }
    
    PyObject* enum_module = NULL;
    PyObject* int_enum_class = NULL;
    PyObject* members_dict = NULL;
    PyObject* handle_mode_enum_name = NULL;
    PyObject* handle_mode_enum_type = NULL; // The created enum type

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
    auto add_member = [&](const char* name, anim::HandleMode val) -> bool {
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

    if (!add_member("FLAT", anim::HandleMode::flat) ||
        !add_member("SMOOTH", anim::HandleMode::smooth) ||
        !add_member("ALIGNED", anim::HandleMode::aligned) ||
        !add_member("FREE", anim::HandleMode::free) ||
        !add_member("ALIGN_STRICT", anim::HandleMode::alignStrict) ||
        !add_member("ALIGN_FLEX", anim::HandleMode::alignFlex) ||
        !add_member("ALIGN_ADJUSTABLE", anim::HandleMode::alignAdjustable)) {
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module); 
        return NULL; 
    }

    handle_mode_enum_name = PyUnicode_FromString("HandleMode");
    if (!handle_mode_enum_name) {
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL; 
    }

    // Create the IntEnum type: IntEnum("HandleMode", {"FLAT": 0, ...})
    handle_mode_enum_type = PyObject_CallFunctionObjArgs(int_enum_class, handle_mode_enum_name, members_dict, NULL);
    if (!handle_mode_enum_type) {
        Py_DECREF(handle_mode_enum_name);
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL; 
    }    

    // Success path: Clean up intermediate objects
    Py_DECREF(handle_mode_enum_name);
    Py_DECREF(members_dict);
    Py_DECREF(int_enum_class);
    Py_DECREF(enum_module);
    
    // Store the created enum as our singleton
    handle_mode_enum_singleton = handle_mode_enum_type;
    return handle_mode_enum_type; // Return new reference to the created enum type
}

// Getter function for the HandleMode enum
PyObject* get_handle_mode_enum(PyObject* self, void* closure) {
    return create_handle_mode_enum();
}
