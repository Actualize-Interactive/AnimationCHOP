cmake_minimum_required(VERSION 3.14)
project(AnimationCHOP VERSION 1.0.0 LANGUAGES CXX)

set(VERBOSE_STATUS OFF CACHE BOOL "Verbose status messages" FORCE)
set(SUPPRESS_NOT_REFERENCED_WARNINGS ON CACHE BOOL "Suppress not referenced warnings" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# recursively get all .cpp files in the src directory
file(GLOB_RECURSE SOURCES
    src/*.cpp
)

if(VERBOSE_STATUS)
    foreach(SOURCE ${SOURCES})
        message(STATUS "Found source file: ${SOURCE}")
    endforeach()
endif()

# Explicitly disable anim tests and examples when building as submodule
set(ANIM_BUILD_TESTS OFF CACHE BOOL "Build anim tests" FORCE)
set(ANIM_BUILD_EXAMPLES OFF CACHE BOOL "Build anim examples" FORCE)

# Add subdirectory for anim library to build it
add_subdirectory(ext/anim)

# Create the animation_chop target
add_library(animation_chop SHARED ${SOURCES})

# Include directories
target_include_directories(animation_chop PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/td/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/Python/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/anim/include
)

if(WIN32)
    target_include_directories(animation_chop PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/Python/Include/PC
    )

    target_link_directories(animation_chop PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/Python/lib/x64
    )

    # Link against the anim library
    target_link_libraries(animation_chop PRIVATE 
        python311
        anim
    )
elseif(APPLE)
    # On macOS, use the system Python or the one set up by GitHub Actions
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    target_link_libraries(animation_chop PRIVATE 
        Python3::Python
        anim
    )
    target_compile_definitions(animation_chop PRIVATE __cdecl=)
else()
    # On other platforms, use pkg-config to find Python
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PYTHON REQUIRED python3)
    target_include_directories(animation_chop PRIVATE ${PYTHON_INCLUDE_DIRS})
    target_link_libraries(animation_chop PRIVATE 
        ${PYTHON_LIBRARIES}
        anim
    )
    target_compile_definitions(animation_chop PRIVATE __cdecl=)
endif()


# Define output directories
set_target_properties(animation_chop PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "AnimationCHOP"
    PREFIX ""
)

if(SUPPRESS_NOT_REFERENCED_WARNINGS)
    if(MSVC)
        target_compile_options(animation_chop PRIVATE /wd4100 /wd4189)
        target_compile_definitions(animation_chop PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(animation_chop PRIVATE -Wno-unused-parameter -Wno-unused-variable)  
    endif()
endif()

# Add a post-build command to copy the DLL to the Plugins folder
add_custom_command(TARGET animation_chop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/test/Plugins"
    COMMAND ${CMAKE_COMMAND} -E copy 
            "$<TARGET_FILE:animation_chop>" 
            "${CMAKE_SOURCE_DIR}/test/Plugins/"
    COMMENT "Copying animation_chop.dll to Plugins directory"
)

# Add platform-specific settings
if(WIN32)
    target_compile_definitions(animation_chop PRIVATE WIN32 _WINDOWS _USRDLL)
    # Windows-specific settings
    if(MSVC)
        # MSVC compiler flags
        target_compile_options(animation_chop PRIVATE /W4)
    endif()
elseif(APPLE)
    # macOS-specific settings
    set_target_properties(animation_chop PROPERTIES
        BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.actualize.animationchop
    )
elseif(UNIX)
    # Linux-specific settings
    target_compile_options(animation_chop PRIVATE -Wall -Wextra)
endif()

# Install targets
install(TARGETS animation_chop
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
