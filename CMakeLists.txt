cmake_minimum_required(VERSION 3.14)

# macOS: TouchDesigner 2025 runs on macOS 13 and later, on Apple silicon and
# Intel. Without these the plugin inherits the build machine's OS release as its
# minimum and its CPU as its only architecture -- the CI runner is macOS 26 on
# arm64, so v0.4.0 would only ever have loaded on a Mac newer than most users'.
# 13.3 rather than 13.0 because the bindings format floats with std::format,
# which libc++ only provides from 13.3. They must be set before project(), and
# are ignored on other platforms.
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.3" CACHE STRING "Minimum macOS version the plugins load on")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures to build the plugins for")

# Stays on 0.x until the API is stable; see CHANGELOG.md.
project(AnimationCHOP VERSION 0.4.1 LANGUAGES CXX)

set(VERBOSE_STATUS ON CACHE BOOL "Verbose status messages" FORCE)
set(SUPPRESS_NOT_REFERENCED_WARNINGS ON CACHE BOOL "Suppress not referenced warnings" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PY_ANIM_BINDINGS_SRCS
    ${SRC}/py_anim_bindings/py_channel.cpp
    ${SRC}/py_anim_bindings/py_extend.cpp
    ${SRC}/py_anim_bindings/py_function.cpp
    ${SRC}/py_anim_bindings/py_handle_mode.cpp
    ${SRC}/py_anim_bindings/py_keyframe.cpp
    ${SRC}/py_anim_bindings/py_point.cpp
    ${SRC}/py_anim_bindings/py_range_end.cpp
)

# Explicitly define sources for AnimationCHOP
set(ANIMATION_CHOP_SOURCES
    ${PY_ANIM_BINDINGS_SRCS}
    ${SRC}/animation_chop.cpp
    ${SRC}/animation_codec.cpp
)

# Explicitly define sources for AnimationViewCHOP
set(ANIMATION_VIEW_CHOP_SOURCES
    ${SRC}/animation_view_chop.cpp
)

if(VERBOSE_STATUS)
    foreach(SOURCE ${ANIMATION_CHOP_SOURCES})
        message(STATUS "Found AnimationCHOP source file: ${SOURCE}")
    endforeach()
    foreach(SOURCE ${ANIMATION_VIEW_CHOP_SOURCES})
        message(STATUS "Found AnimationViewCHOP source file: ${SOURCE}")
    endforeach()
endif()

# Explicitly disable anim tests and examples when building as submodule
set(ANIM_BUILD_TESTS OFF CACHE BOOL "Build anim tests" FORCE)
set(ANIM_BUILD_EXAMPLES OFF CACHE BOOL "Build anim examples" FORCE)

# Add subdirectory for anim library to build it
add_subdirectory(ext/anim)

# Create the animation_chop target
if(APPLE)
    add_library(animation_chop MODULE ${ANIMATION_CHOP_SOURCES})
else()
    add_library(animation_chop SHARED ${ANIMATION_CHOP_SOURCES})
endif()

# Create the animation_view_chop target
if(APPLE)
    add_library(animation_view_chop MODULE ${ANIMATION_VIEW_CHOP_SOURCES})
else()
    add_library(animation_view_chop SHARED ${ANIMATION_VIEW_CHOP_SOURCES})
endif()

# Include directories for both targets
foreach(TARGET animation_chop animation_view_chop)
    target_include_directories(${TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/td/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/Python/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/anim/include
    )
endforeach()

if(WIN32)
    foreach(TARGET animation_chop animation_view_chop)
        target_include_directories(${TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/ext/Python/Include/PC
        )

        target_link_directories(${TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/ext/Python/lib/x64
        )

        # Both targets now need Python for their bindings
        target_link_libraries(${TARGET} PRIVATE 
            python311
            anim
        )
    endforeach()
elseif(APPLE)
    # TouchDesigner embeds CPython 3.11 and loads the plugin into its own
    # process, so the plugin must not link a libpython of its own: whatever it
    # linked would have to exist, at that exact path, on every user's Mac.
    # (v0.4.0 was built on a runner without TouchDesigner, linked the runner's
    # Homebrew Python 3.14, and failed to load everywhere.) Instead, as every
    # CPython extension module does, leave the Python symbols undefined and let
    # dyld resolve them from the host process at load time. All the build needs
    # from Python is the 3.11 headers.
    #
    # TouchDesigner's own copy is preferred, because it is exactly the ABI the
    # plugin runs against. Without a TouchDesigner install (CI), any CPython
    # 3.11 supplies identical headers. EXACT matters: FindPython otherwise
    # returns the newest interpreter it can find, and headers from another
    # minor version would silently build against the wrong ABI.
    set(ANIMATIONCHOP_TD_APP "/Applications/TouchDesigner.app"
        CACHE PATH "TouchDesigner install to take the Python 3.11 headers from")
    set(TD_PYTHON_INCLUDE_DIR
        "${ANIMATIONCHOP_TD_APP}/Contents/Frameworks/Python.framework/Versions/3.11/include/python3.11")

    if(EXISTS "${TD_PYTHON_INCLUDE_DIR}/Python.h")
        message(STATUS "Using TouchDesigner's Python 3.11 headers: ${TD_PYTHON_INCLUDE_DIR}")
        set(ANIMATIONCHOP_MACOS_PYTHON_INCLUDE_DIRS "${TD_PYTHON_INCLUDE_DIR}")
    else()
        set(Python3_FIND_FRAMEWORK LAST)
        find_package(Python3 3.11 EXACT COMPONENTS Interpreter Development.Module REQUIRED)
        message(STATUS "TouchDesigner not found at ${ANIMATIONCHOP_TD_APP}; "
                       "using Python ${Python3_VERSION} headers: ${Python3_INCLUDE_DIRS}")
        set(ANIMATIONCHOP_MACOS_PYTHON_INCLUDE_DIRS ${Python3_INCLUDE_DIRS})
    endif()

    foreach(TARGET animation_chop animation_view_chop)
        target_include_directories(${TARGET} PRIVATE ${ANIMATIONCHOP_MACOS_PYTHON_INCLUDE_DIRS})
        target_link_libraries(${TARGET} PRIVATE anim)
        target_link_options(${TARGET} PRIVATE "LINKER:-undefined,dynamic_lookup")
        target_compile_definitions(${TARGET} PRIVATE __cdecl=)

        # The linker's implicit ad-hoc signature covers only the Mach-O, not the
        # bundle, so `codesign --verify` rejects the .plugin and Gatekeeper
        # reports a downloaded copy as damaged. Sign the whole bundle (still
        # ad-hoc: no certificate needed). Runs before the copies below, which
        # are added later in this file, so every copy carries the signature.
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND codesign --force --sign - "$<TARGET_BUNDLE_DIR:${TARGET}>"
            COMMENT "Ad-hoc signing $<TARGET_PROPERTY:${TARGET},OUTPUT_NAME>.plugin"
        )
    endforeach()
else()
    # On other platforms, use pkg-config to find Python
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PYTHON REQUIRED python3)
    foreach(TARGET animation_chop animation_view_chop)
        target_include_directories(${TARGET} PRIVATE ${PYTHON_INCLUDE_DIRS})
        target_link_libraries(${TARGET} PRIVATE 
            ${PYTHON_LIBRARIES}
            anim
        )
        target_compile_definitions(${TARGET} PRIVATE __cdecl=)
    endforeach()
endif()

# Define output directories for both targets
set_target_properties(animation_chop PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "AnimationCHOP"
    PREFIX ""
)

set_target_properties(animation_view_chop PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "AnimationViewCHOP"
    PREFIX ""
)

if(SUPPRESS_NOT_REFERENCED_WARNINGS)
    foreach(TARGET animation_chop animation_view_chop)
        if(MSVC)
            target_compile_options(${TARGET} PRIVATE /wd4100 /wd4189)
            target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS)
        endif()
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${TARGET} PRIVATE -Wno-unused-parameter -Wno-unused-variable)  
        endif()
    endforeach()
endif()

# Add post-build commands to copy both DLLs to the Plugins folders.
#
# TouchDesigner only loads Custom Operators from a Plugins/ folder beside the
# .toe, with no way to point it at a build directory, so every project that
# needs the operators gets its own copy: td/ for the example project, and
# tests/td/ for the integration-test project (see TESTING.md).
set(ANIMATIONCHOP_PLUGIN_DIRS
    "${CMAKE_SOURCE_DIR}/td/Plugins"
    "${CMAKE_SOURCE_DIR}/tests/td/Plugins"
)

foreach(TARGET animation_chop animation_view_chop)
    foreach(PLUGIN_DIR ${ANIMATIONCHOP_PLUGIN_DIRS})
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PLUGIN_DIR}"
            COMMENT "Creating Plugins directory: ${PLUGIN_DIR}"
        )

        if(APPLE)
            # On macOS, copy the entire .plugin bundle
            add_custom_command(TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "$<TARGET_BUNDLE_DIR:${TARGET}>"
                        "${PLUGIN_DIR}/$<TARGET_PROPERTY:${TARGET},OUTPUT_NAME>.plugin"
                COMMENT "Copying ${TARGET}.plugin bundle to ${PLUGIN_DIR}"
            )
        else()
            # On other platforms, copy the library file
            add_custom_command(TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                        "$<TARGET_FILE:${TARGET}>"
                        "${PLUGIN_DIR}/"
                COMMENT "Copying ${TARGET} to ${PLUGIN_DIR}"
            )
        endif()
    endforeach()
endforeach()

# Add platform-specific settings for both targets
foreach(TARGET animation_chop animation_view_chop)
    if(WIN32)
        target_compile_definitions(${TARGET} PRIVATE WIN32 _WINDOWS _USRDLL)
        # Windows-specific settings
        if(MSVC)
            # MSVC compiler flags
            target_compile_options(${TARGET} PRIVATE /W4)
        endif()
    elseif(APPLE)
        # macOS-specific settings for TouchDesigner modules
        string(REPLACE "_" "-" BUNDLE_ID_TARGET ${TARGET})
        set_target_properties(${TARGET} PROPERTIES
            BUNDLE TRUE
            BUNDLE_EXTENSION "plugin"
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.actualize.${BUNDLE_ID_TARGET}"
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.actualize.${BUNDLE_ID_TARGET}"
        )
    elseif(UNIX)
        # Linux-specific settings
        target_compile_options(${TARGET} PRIVATE -Wall -Wextra)
    endif()
endforeach()

# Install targets
install(TARGETS animation_chop animation_view_chop
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# ---------------------------------------------------------------------------
# Tests
#
# tests/python runs pytest against an extension that compiles the real operator
# sources against a fake PY_Context. No TouchDesigner needed, so it runs in CI.
#
# tests/cpp is Catch2 over the .toe persistence codec -- the one piece here that
# is pure C++, and the one whose output lands in users' project files. Curve
# behaviour is anim's to test; it carries its own suite.
#
# The TouchDesigner integration test is separate and local-only (TouchDesigner
# needs a licence and a GPU, so it cannot run on a cloud runner); see
# run_td_tests.ps1 and TESTING.md.
#
# Configure with -DANIMATIONCHOP_BUILD_TESTS=ON, or use the `dev` preset.
# ---------------------------------------------------------------------------
option(ANIMATIONCHOP_BUILD_TESTS "Build the Catch2 and pytest unit tests" OFF)

if(ANIMATIONCHOP_BUILD_TESTS)
    enable_testing()

    # TouchDesigner embeds CPython 3.11, so the extension must be built and
    # imported by 3.11. uv is preferred: it can supply both the interpreter and
    # pytest without anything being installed globally.
    find_program(UV_EXECUTABLE uv)
    if(UV_EXECUTABLE)
        message(STATUS "AnimationCHOP: using uv at ${UV_EXECUTABLE} to run pytest")
    else()
        find_package(Python3 3.11 COMPONENTS Interpreter REQUIRED)
        set(ANIMATIONCHOP_PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
        message(STATUS "AnimationCHOP: uv not found; running pytest with ${Python3_EXECUTABLE}")
    endif()

    # A CPython extension is .pyd on Windows and .so everywhere else, including
    # macOS -- CMake's default MODULE suffix there is .so already, but .dylib
    # would not be importable, so be explicit.
    if(WIN32)
        set(ANIMATIONCHOP_PYEXT_SUFFIX ".pyd")
        # Windows builds against the vendored headers/import library, matching
        # the operator targets above.
        set(ANIMATIONCHOP_PYTHON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/ext/Python/Include)
    elseif(APPLE)
        # Same headers and undefined-symbol linking as the operators above; the
        # importing interpreter supplies the symbols.
        set(ANIMATIONCHOP_PYEXT_SUFFIX ".so")
        set(ANIMATIONCHOP_PYTHON_INCLUDE_DIRS ${ANIMATIONCHOP_MACOS_PYTHON_INCLUDE_DIRS})
    else()
        set(ANIMATIONCHOP_PYEXT_SUFFIX ".so")
        find_package(Python3 3.11 EXACT COMPONENTS Development.Module REQUIRED)
        set(ANIMATIONCHOP_PYTHON_INCLUDE_DIRS ${Python3_INCLUDE_DIRS})
        set(ANIMATIONCHOP_PYTHON_LIBRARIES Python3::Module)
    endif()

    add_subdirectory(tests/cpp)
    add_subdirectory(tests/python)
endif()
