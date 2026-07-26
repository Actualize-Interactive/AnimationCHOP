# Python extension + pytest suite. Exposes the operator bindings without
# TouchDesigner by compiling the real operator sources against a fake
# PY_Context (see extension/animationchop_module.cpp).

add_library(animationchop_testext MODULE
    extension/animationchop_module.cpp
    ${CMAKE_SOURCE_DIR}/src/animation_chop.cpp
    ${CMAKE_SOURCE_DIR}/src/animation_codec.cpp
    ${CMAKE_SOURCE_DIR}/src/py_anim_bindings/py_channel.cpp
    ${CMAKE_SOURCE_DIR}/src/py_anim_bindings/py_extend.cpp
    ${CMAKE_SOURCE_DIR}/src/py_anim_bindings/py_function.cpp
    ${CMAKE_SOURCE_DIR}/src/py_anim_bindings/py_handle_mode.cpp
    ${CMAKE_SOURCE_DIR}/src/py_anim_bindings/py_keyframe.cpp
    ${CMAKE_SOURCE_DIR}/src/py_anim_bindings/py_point.cpp
)

# A CPython extension is a .pyd on Windows and a .so elsewhere -- never .dylib.
set_target_properties(animationchop_testext PROPERTIES
    OUTPUT_NAME "animationchop"
    PREFIX ""
    SUFFIX "${ANIMATIONCHOP_PYEXT_SUFFIX}"
)

target_include_directories(animationchop_testext PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/ext/td/include
    ${CMAKE_SOURCE_DIR}/ext/anim/include
    ${ANIMATIONCHOP_PYTHON_INCLUDE_DIRS}
)

target_link_libraries(animationchop_testext PRIVATE anim)
target_compile_features(animationchop_testext PRIVATE cxx_std_20)

if(WIN32)
    target_include_directories(animationchop_testext PRIVATE
        ${CMAKE_SOURCE_DIR}/ext/Python/Include/PC)
    target_link_directories(animationchop_testext PRIVATE
        ${CMAKE_SOURCE_DIR}/ext/Python/lib/x64)
    target_link_libraries(animationchop_testext PRIVATE python311)
    target_compile_options(animationchop_testext PRIVATE /wd4100 /wd4189)
    target_compile_definitions(animationchop_testext PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    # Extension modules leave Python's symbols undefined; the host interpreter
    # supplies them at import.
    target_link_libraries(animationchop_testext PRIVATE ${ANIMATIONCHOP_PYTHON_LIBRARIES})
    target_compile_definitions(animationchop_testext PRIVATE __cdecl=)
    target_compile_options(animationchop_testext PRIVATE
        -Wno-unused-parameter -Wno-unused-variable)
endif()

# Drop the module next to the pytest files so `import animationchop` resolves
# regardless of the per-configuration build subdirectory.
add_custom_command(TARGET animationchop_testext POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:animationchop_testext>"
            "${CMAKE_CURRENT_SOURCE_DIR}/"
    COMMENT "Copying animationchop test extension to tests/python/"
)

# Run pytest as a ctest test. Prefer `uv run`, which supplies pytest
# ephemerally so nothing has to be installed first; otherwise call pytest in
# the discovered interpreter (see requirements.txt).
if(UV_EXECUTABLE)
    add_test(
        NAME python_bindings
        COMMAND ${UV_EXECUTABLE} run --no-project --python 3.11 --with pytest
                pytest -q "${CMAKE_CURRENT_SOURCE_DIR}"
    )
else()
    add_test(
        NAME python_bindings
        COMMAND ${ANIMATIONCHOP_PYTHON_EXECUTABLE} -m pytest -q "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()
set_tests_properties(python_bindings PROPERTIES DEPENDS animationchop_testext)
