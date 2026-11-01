cmake_minimum_required(VERSION 3.14)
project(namegen LANGUAGES CXX)

# --- Library target ---
# The C++ fantasy name generator: namegen.cc compiles into a small static library and
# consumers get the repository root on their include path for "namegen.h". No
# dependencies beyond the standard library. cxx_std_14 is what the code needs
# (std::make_unique; it also builds as C++11 through its own fallback).
add_library(namegen STATIC namegen.cc)
target_include_directories(namegen PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(namegen PUBLIC cxx_std_14)

# Alias so consumers link `namegen::namegen` whether they add this via
# add_subdirectory or FetchContent.
add_library(namegen::namegen ALIAS namegen)
# The target names of the Kronuz/fantasyname fork's CMakeLists, for consumers that
# move from the fork to this repository.
add_library(fantasyname ALIAS namegen)
add_library(fantasyname::fantasyname ALIAS namegen)

# Only build the tests, demo and benchmark when this repo is the top-level project,
# so consumers (e.g. via FetchContent) don't build them.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  enable_testing()

  add_executable(namegen_test test/test.cc)
  target_link_libraries(namegen_test PRIVATE namegen)
  add_test(NAME namegen COMMAND namegen_test)

  # One generator shared by eight threads (run under ThreadSanitizer in CI), and
  # seeded engines.
  find_package(Threads REQUIRED)
  add_executable(namegen_thread_test test/thread_test.cc)
  target_link_libraries(namegen_thread_test PRIVATE namegen Threads::Threads)
  add_test(NAME namegen_thread COMMAND namegen_thread_test)

  # A runnable tour of the library (not a test): ./build/namegen_demo [pattern [n]]
  add_executable(namegen_demo examples/demo.cc)
  target_link_libraries(namegen_demo PRIVATE namegen)
endif()
