#include "py_function.h"
#include <anim/function.hpp>

static PyObject* function_enum_singleton = NULL;

static PyObject* create_function_enum() {
    // If we already have a singleton instance, return that
    if (function_enum_singleton) {
        Py_INCREF(function_enum_singleton);
        return function_enum_singleton;
    }
    
    PyObject* enum_module = NULL;
    PyObject* int_enum_class = NULL;
    PyObject* members_dict = NULL;
    PyObject* function_enum_name = NULL;
    PyObject* function_enum_type = NULL; // The created enum type

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
    auto add_member = [&](const char* name, anim::Function val) -> bool {
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

    if (!add_member("CONSTANT", anim::Function::Constant) ||
        !add_member("LINEAR", anim::Function::Linear) ||
        !add_member("BEZIER", anim::Function::Bezier)) {
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module); 
        return NULL; 
    }

    function_enum_name = PyUnicode_FromString("Function");
    if (!function_enum_name) {
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL; 
    }

    // Create the IntEnum type: IntEnum("Function", {"CONSTANT": 0, ...})
    function_enum_type = PyObject_CallFunctionObjArgs(int_enum_class, function_enum_name, members_dict, NULL);
    if (!function_enum_type) {
        Py_DECREF(function_enum_name);
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL; 
    }    

    // Success path: Clean up intermediate objects
    Py_DECREF(function_enum_name);
    Py_DECREF(members_dict);
    Py_DECREF(int_enum_class);
    Py_DECREF(enum_module);
    
    // Store the created enum as our singleton
    function_enum_singleton = function_enum_type;
    Py_INCREF(function_enum_singleton); // Add extra reference to keep it alive
    return function_enum_type; // Return new reference to the created enum type
}

// Getter function for the Function enum
PyObject* get_function_enum(PyObject* self, void* closure) {
    return create_function_enum();
}

// Cleanup function for module shutdown
void cleanup_function_enum() {
    if (function_enum_singleton) {
        Py_DECREF(function_enum_singleton);
        function_enum_singleton = NULL;
    }
}

// Function enum string representations
static const char* function_names[] = {
    "CONSTANT",
    "LINEAR", 
    "BEZIER"
};

static const char* function_full_names[] = {
    "Function.CONSTANT",
    "Function.LINEAR",
    "Function.BEZIER"
};

// Helper functions for state serialization
const char* function_to_string(anim::Function func) {
    int idx = static_cast<int>(func);
    if (idx >= 0 && idx < static_cast<int>(anim::Function::Count)) {
        return function_full_names[idx];
    }
    return "Function.UNKNOWN";
}

anim::Function string_to_function(const char* str) {
    for (int i = 0; i < static_cast<int>(anim::Function::Count); ++i) {
        if (strcmp(str, function_full_names[i]) == 0 || strcmp(str, function_names[i]) == 0) {
            return static_cast<anim::Function>(i);
        }
    }
    return anim::Function::Bezier; // Default fallback
}

