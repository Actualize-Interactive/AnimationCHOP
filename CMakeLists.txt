cmake_minimum_required(VERSION 3.14)
project(AnimationCHOP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# recursively get all .cpp files in the src directory
file(GLOB_RECURSE SOURCES
    src/*.cpp
)
set(VERBOSE_STATUS OFF CACHE BOOL "Verbose status messages" FORCE)
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
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/Python/Include/PC
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/anim/include
)

target_link_directories(animation_chop PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/Python/lib/x64
)

# Link against the anim library
target_link_libraries(animation_chop PRIVATE 
    python311
    anim
)

# Define output directories
set_target_properties(animation_chop PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Add a post-build command to copy the DLL to the Plugins folder
add_custom_command(TARGET animation_chop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/Plugins"
    COMMAND ${CMAKE_COMMAND} -E copy 
            "$<TARGET_FILE:animation_chop>" 
            "${CMAKE_SOURCE_DIR}/Plugins/"
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
