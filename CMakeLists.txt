cmake_minimum_required(VERSION 3.14)
project(fantasyname LANGUAGES CXX)

# This is skeeto's fantasy name generator (https://github.com/skeeto/fantasyname),
# a multi-language repo. This CMakeLists builds only the C++ implementation under
# c++/, so the library can be consumed via FetchContent. namegen.cc compiles into a
# small static library; consumers get c++/ on their include path for "namegen.h".
# cxx_std_14 is requested so the C++14 std::make_unique path is taken (see namegen.cc).
add_library(fantasyname STATIC c++/namegen.cc)
target_include_directories(fantasyname PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/c++")
target_compile_features(fantasyname PUBLIC cxx_std_14)
# Namespaced alias so consumers link `fantasyname::fantasyname` whether they add
# this via FetchContent or add_subdirectory.
add_library(fantasyname::fantasyname ALIAS fantasyname)

# Only build the example when this repo is the top-level project, so consumers
# (e.g. via FetchContent) don't build it.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  add_executable(fantasyname_example c++/example.cc)
  target_link_libraries(fantasyname_example PRIVATE fantasyname)
endif()
