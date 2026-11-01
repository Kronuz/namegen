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

option(NAMEGEN_BUILD_BENCH "Build the benchmark (fetches the C port as a comparator)" OFF)
option(NAMEGEN_FUZZ "Build the libFuzzer target (Clang)" OFF)

# Only build the tests, demo and benchmark when this repo is the top-level project,
# so consumers (e.g. via FetchContent) don't build them.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  enable_testing()

  add_executable(namegen_test test/test.cc)
  target_link_libraries(namegen_test PRIVATE namegen)
  add_test(NAME namegen COMMAND namegen_test)

  # The library against the frozen original (test/frozen) and the reference
  # interpreter (test/reference.h), engines seeded alike, in every available locale.
  add_executable(namegen_diff_test test/diff_test.cc)
  target_link_libraries(namegen_diff_test PRIVATE namegen)
  add_test(NAME namegen_diff COMMAND namegen_diff_test)

  # One generator shared by eight threads (run under ThreadSanitizer in CI), and
  # seeded engines.
  find_package(Threads REQUIRED)
  add_executable(namegen_thread_test test/thread_test.cc)
  target_link_libraries(namegen_thread_test PRIVATE namegen Threads::Threads)
  add_test(NAME namegen_thread COMMAND namegen_thread_test)

  # The fuzz target's own driver: random patterns and mutations through the same
  # differential check (`namegen_fuzz [iterations] [files...]`).
  add_executable(namegen_fuzz fuzz/fuzz_pattern.cc)
  target_link_libraries(namegen_fuzz PRIVATE namegen)
  target_compile_definitions(namegen_fuzz PRIVATE NAMEGEN_FUZZ_DRIVER)
  add_test(NAME namegen_fuzz COMMAND namegen_fuzz 5000)

  # The libFuzzer target: cmake -DNAMEGEN_FUZZ=ON with Clang, and sanitizers in
  # CMAKE_CXX_FLAGS (e.g. -fsanitize=address,undefined).
  if(NAMEGEN_FUZZ)
    add_executable(namegen_libfuzzer fuzz/fuzz_pattern.cc)
    target_link_libraries(namegen_libfuzzer PRIVATE namegen)
    target_compile_options(namegen_libfuzzer PRIVATE -fsanitize=fuzzer)
    target_link_options(namegen_libfuzzer PRIVATE -fsanitize=fuzzer)
  endif()

  # The benchmark (-DNAMEGEN_BUILD_BENCH=ON): compile and generate against the frozen
  # original and the C port. The C port comes from skeeto/fantasyname (c/namegen.h,
  # public domain) at a pinned commit, fetched here only; the library never uses it.
  if(NAMEGEN_BUILD_BENCH)
    enable_language(C)
    include(FetchContent)
    FetchContent_Declare(fantasyname_c
      GIT_REPOSITORY https://github.com/skeeto/fantasyname.git
      GIT_TAG        e5a475287902457dac09284fb681c13dfdd96715
      SOURCE_SUBDIR  c)  # no CMakeLists.txt there: populated, not added
    FetchContent_MakeAvailable(fantasyname_c)
    add_executable(namegen_bench bench/bench.cc bench/cport.c)
    # By path: the C port's header is also named namegen.h.
    set_source_files_properties(bench/cport.c PROPERTIES
      COMPILE_DEFINITIONS "NAMEGEN_C_HEADER=\"${fantasyname_c_SOURCE_DIR}/c/namegen.h\"")
    target_link_libraries(namegen_bench PRIVATE namegen)
    add_test(NAME namegen_bench_check COMMAND namegen_bench --check)
  endif()

  # A runnable tour of the library (not a test): ./build/namegen_demo [pattern [n]]
  add_executable(namegen_demo examples/demo.cc)
  target_link_libraries(namegen_demo PRIVATE namegen)
endif()
