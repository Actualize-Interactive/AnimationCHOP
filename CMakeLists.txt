cmake_minimum_required(VERSION 3.14)
project(AnimationCHOP VERSION 1.0.0 LANGUAGES CXX)

set(VERBOSE_STATUS ON CACHE BOOL "Verbose status messages" FORCE)
set(SUPPRESS_NOT_REFERENCED_WARNINGS ON CACHE BOOL "Suppress not referenced warnings" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PY_ANIM_BINDINGS_SRCS
    ${SRC}/py_anim_bindings/py_channel.cpp
    ${SRC}/py_anim_bindings/py_function.cpp
    ${SRC}/py_anim_bindings/py_handle_mode.cpp
    ${SRC}/py_anim_bindings/py_keyframe.cpp
    ${SRC}/py_anim_bindings/py_point.cpp
)

# Explicitly define sources for AnimationCHOP
set(ANIMATION_CHOP_SOURCES
    ${PY_ANIM_BINDINGS_SRCS}
    ${SRC}/animation_chop.cpp
)

# Explicitly define sources for AnimationSelectCHOP
set(ANIMATION_SELECT_CHOP_SOURCES
    ${SRC}/animation_select_chop.cpp
)

if(VERBOSE_STATUS)
    foreach(SOURCE ${ANIMATION_CHOP_SOURCES})
        message(STATUS "Found AnimationCHOP source file: ${SOURCE}")
    endforeach()
    foreach(SOURCE ${ANIMATION_SELECT_CHOP_SOURCES})
        message(STATUS "Found AnimationSelectCHOP source file: ${SOURCE}")
    endforeach()
endif()

# Explicitly disable anim tests and examples when building as submodule
set(ANIM_BUILD_TESTS OFF CACHE BOOL "Build anim tests" FORCE)
set(ANIM_BUILD_EXAMPLES OFF CACHE BOOL "Build anim examples" FORCE)

# Add subdirectory for anim library to build it
add_subdirectory(ext/anim)

# Create the animation_chop target
add_library(animation_chop SHARED ${ANIMATION_CHOP_SOURCES})

# Create the animation_select_chop target
add_library(animation_select_chop SHARED ${ANIMATION_SELECT_CHOP_SOURCES})

# Include directories for both targets
foreach(TARGET animation_chop animation_select_chop)
    target_include_directories(${TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/td/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/Python/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/anim/include
    )
endforeach()

if(WIN32)
    foreach(TARGET animation_chop animation_select_chop)
        target_include_directories(${TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/ext/Python/Include/PC
        )

        target_link_directories(${TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/ext/Python/lib/x64
        )

        # Link against the anim library (AnimationSelectCHOP only needs anim, not full Python bindings)
        if(${TARGET} STREQUAL "animation_chop")
            target_link_libraries(${TARGET} PRIVATE 
                python311
                anim
            )
        else()
            target_link_libraries(${TARGET} PRIVATE 
                anim
            )
        endif()
    endforeach()
elseif(APPLE)
    # On macOS, use the system Python or the one set up by GitHub Actions
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    foreach(TARGET animation_chop animation_select_chop)
        if(${TARGET} STREQUAL "animation_chop")
            target_link_libraries(${TARGET} PRIVATE 
                Python3::Python
                anim
            )
        else()
            target_link_libraries(${TARGET} PRIVATE 
                anim
            )
        endif()
        target_compile_definitions(${TARGET} PRIVATE __cdecl=)
    endforeach()
else()
    # On other platforms, use pkg-config to find Python
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PYTHON REQUIRED python3)
    foreach(TARGET animation_chop animation_select_chop)
        target_include_directories(${TARGET} PRIVATE ${PYTHON_INCLUDE_DIRS})
        if(${TARGET} STREQUAL "animation_chop")
            target_link_libraries(${TARGET} PRIVATE 
                ${PYTHON_LIBRARIES}
                anim
            )
        else()
            target_link_libraries(${TARGET} PRIVATE 
                anim
            )
        endif()
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

set_target_properties(animation_select_chop PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "AnimationSelectCHOP"
    PREFIX ""
)

if(SUPPRESS_NOT_REFERENCED_WARNINGS)
    foreach(TARGET animation_chop animation_select_chop)
        if(MSVC)
            target_compile_options(${TARGET} PRIVATE /wd4100 /wd4189)
            target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS)
        endif()
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${TARGET} PRIVATE -Wno-unused-parameter -Wno-unused-variable)  
        endif()
    endforeach()
endif()

# Add post-build commands to copy both DLLs to the Plugins folder
foreach(TARGET animation_chop animation_select_chop)
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/td/Plugins"
        COMMAND ${CMAKE_COMMAND} -E copy 
                "$<TARGET_FILE:${TARGET}>" 
                "${CMAKE_SOURCE_DIR}/td/Plugins/"
        COMMENT "Copying ${TARGET} to Plugins directory"
    )
endforeach()

# Add platform-specific settings for both targets
foreach(TARGET animation_chop animation_select_chop)
    if(WIN32)
        target_compile_definitions(${TARGET} PRIVATE WIN32 _WINDOWS _USRDLL)
        # Windows-specific settings
        if(MSVC)
            # MSVC compiler flags
            target_compile_options(${TARGET} PRIVATE /W4)
        endif()
    elseif(APPLE)
        # macOS-specific settings
        set_target_properties(${TARGET} PROPERTIES
            BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER com.actualize.${TARGET}
        )
    elseif(UNIX)
        # Linux-specific settings
        target_compile_options(${TARGET} PRIVATE -Wall -Wextra)
    endif()
endforeach()

# Install targets
install(TARGETS animation_chop animation_select_chop
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
