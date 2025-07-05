#include "py_extend.h"
#include <anim/extend.hpp>

static PyObject* extend_enum_singleton = NULL;

static PyObject* create_extend_enum() {
    // If we already have a singleton instance, return that
    if (extend_enum_singleton) {
        Py_INCREF(extend_enum_singleton);
        return extend_enum_singleton;
    }
    
    PyObject* enum_module = NULL;
    PyObject* int_enum_class = NULL;
    PyObject* members_dict = NULL;
    PyObject* extend_enum_name = NULL;
    PyObject* extend_enum_type = NULL; // The created enum type

    enum_module = PyImport_ImportModule("enum");
    if (!enum_module) {
        // PyImport_ImportModule sets an error
        return NULL;
    }

    int_enum_class = PyObject_GetAttrString(enum_module, "IntEnum");
    if (!int_enum_class) {
        Py_DECREF(enum_module);
        return NULL; 
    }

    members_dict = PyDict_New();
    if (!members_dict) {
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL; 
    }

    // Helper lambda to add members and check for errors
    auto add_member = [&](const char* name, anim::Extend val) -> bool {
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

    if (!add_member("HOLD", anim::Extend::Hold) ||
        !add_member("REPEAT", anim::Extend::Repeat) ||
        !add_member("MIRROR", anim::Extend::Mirror)) {
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module); 
        return NULL; 
    }

    extend_enum_name = PyUnicode_FromString("Extend");
    if (!extend_enum_name) {
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL; 
    }

    // Create the IntEnum type: IntEnum("Extend", {"HOLD": 0, ...})
    extend_enum_type = PyObject_CallFunctionObjArgs(int_enum_class, extend_enum_name, members_dict, NULL);
    if (!extend_enum_type) {
        Py_DECREF(extend_enum_name);
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL; 
    }    

    // Success path: Clean up intermediate objects
    Py_DECREF(extend_enum_name);
    Py_DECREF(members_dict);
    Py_DECREF(int_enum_class);
    Py_DECREF(enum_module);
    
    // Store the created enum as our singleton
    extend_enum_singleton = extend_enum_type;
    Py_INCREF(extend_enum_singleton); // Add extra reference to keep it alive
    return extend_enum_type; // Return new reference to the created enum type
}

// Getter function for the Extend enum
PyObject* get_extend_enum(PyObject* self, void* closure) {
    return create_extend_enum();
}

// Cleanup function for module shutdown
void cleanup_extend_enum() {
    if (extend_enum_singleton) {
        Py_DECREF(extend_enum_singleton);
        extend_enum_singleton = NULL;
    }
}

// Extend enum string representations
static const char* extend_names[] = {
    "HOLD",
    "REPEAT",
    "MIRROR"
};

static const char* extend_full_names[] = {
    "Extend.HOLD",
    "Extend.REPEAT",
    "Extend.MIRROR"
};

// Helper functions for state serialization
const char* extend_to_string(anim::Extend extend) {
    int idx = static_cast<int>(extend);
    if (idx >= 0 && idx < 3) {
        return extend_full_names[idx];
    }
    return "Extend.HOLD";
}

anim::Extend string_to_extend(const char* str) {
    for (int i = 0; i < 3; ++i) {
        if (strcmp(str, extend_full_names[i]) == 0 || strcmp(str, extend_names[i]) == 0) {
            return static_cast<anim::Extend>(i);
        }
    }
    return anim::Extend::Hold; // Default fallback
}
