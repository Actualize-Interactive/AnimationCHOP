#pragma once

#ifdef _WIN32
	#include <Python.h>
	#include <structmember.h>
#else
	#include <Python/Python.h>
	#include <Python/structmember.h>
#endif

#include "CPlusPlus_Common.h"
#include "animation_chop.h"

TD::PY_Struct* get_td_node_struct(PyObject* node_obj, AnimationCHOP* node_inst, bool autoCook = false) {
    TD::PY_Struct* node_struct = (TD::PY_Struct*)node_obj;
    TD::PY_GetInfo info;
    info.autoCook = false;
    node_inst = (AnimationCHOP*)node_struct->context->getNodeInstance(info);
    return node_struct;
}