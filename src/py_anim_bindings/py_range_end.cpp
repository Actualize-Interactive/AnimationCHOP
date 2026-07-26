#include "py_range_end.h"

static PyObject* range_end_enum_singleton = NULL;

static PyObject* create_range_end_enum() {
    // If we already have a singleton instance, return that
    if (range_end_enum_singleton) {
        Py_INCREF(range_end_enum_singleton);
        return range_end_enum_singleton;
    }

    PyObject* enum_module = NULL;
    PyObject* int_enum_class = NULL;
    PyObject* members_dict = NULL;
    PyObject* range_end_enum_name = NULL;
    PyObject* range_end_enum_type = NULL; // The created enum type

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
    auto add_member = [&](const char* name, anim::RangeEnd val) -> bool {
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

    if (!add_member("EXCLUSIVE", anim::RangeEnd::Exclusive) ||
        !add_member("INCLUSIVE", anim::RangeEnd::Inclusive)) {
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL;
    }

    range_end_enum_name = PyUnicode_FromString("RangeEnd");
    if (!range_end_enum_name) {
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL;
    }

    // Create the IntEnum type: IntEnum("RangeEnd", {"EXCLUSIVE": 0, ...})
    range_end_enum_type = PyObject_CallFunctionObjArgs(int_enum_class, range_end_enum_name, members_dict, NULL);
    if (!range_end_enum_type) {
        Py_DECREF(range_end_enum_name);
        Py_DECREF(members_dict);
        Py_DECREF(int_enum_class);
        Py_DECREF(enum_module);
        return NULL;
    }

    // Success path: Clean up intermediate objects
    Py_DECREF(range_end_enum_name);
    Py_DECREF(members_dict);
    Py_DECREF(int_enum_class);
    Py_DECREF(enum_module);

    // Store the created enum as our singleton
    range_end_enum_singleton = range_end_enum_type;
    Py_INCREF(range_end_enum_singleton); // Add extra reference to keep it alive
    return range_end_enum_type; // Return new reference to the created enum type
}

// Getter function for the RangeEnd enum
PyObject* get_range_end_enum(PyObject* self, void* closure) {
    return create_range_end_enum();
}

// Cleanup function for module shutdown
void cleanup_range_end_enum() {
    if (range_end_enum_singleton) {
        Py_DECREF(range_end_enum_singleton);
        range_end_enum_singleton = NULL;
    }
}

bool PY_ObjectToRangeEnd(PyObject* obj, anim::RangeEnd& range_end) {
    if (!obj || obj == Py_None) {
        return true;  // Argument omitted; leave the caller's default in place.
    }

    // RangeEnd is exposed as an IntEnum, so its members are ints. Accept a
    // plain int too, matching how the other enums are handled here.
    if (!PyLong_Check(obj)) {
        PyErr_SetString(PyExc_TypeError,
                        "range_end must be a RangeEnd (or its integer value)");
        return false;
    }

    long value = PyLong_AsLong(obj);
    if (value == -1 && PyErr_Occurred()) {
        return false;
    }

    if (value != static_cast<long>(anim::RangeEnd::Exclusive) &&
        value != static_cast<long>(anim::RangeEnd::Inclusive)) {
        PyErr_SetString(PyExc_ValueError,
                        "range_end must be RangeEnd.EXCLUSIVE or RangeEnd.INCLUSIVE");
        return false;
    }

    range_end = static_cast<anim::RangeEnd>(value);
    return true;
}
