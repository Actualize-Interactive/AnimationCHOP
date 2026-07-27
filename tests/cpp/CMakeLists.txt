# C++ unit tests (Catch2). No Python or TouchDesigner required.
#
# The codec is the only part of this repository that is pure C++ -- everything
# else is either anim (which has its own suite) or binding glue that needs a
# Python interpreter. That makes it exactly what a C++ suite is good for, and
# it matters more than most: the bytes it writes go into users' .toe files.
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(animationchop_cpp_tests
  test_animation_codec.cpp
  ${CMAKE_SOURCE_DIR}/src/animation_codec.cpp
)
target_include_directories(animationchop_cpp_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(animationchop_cpp_tests PRIVATE
  Catch2::Catch2WithMain
  anim_static
)
target_compile_features(animationchop_cpp_tests PRIVATE cxx_std_20)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(animationchop_cpp_tests)
