# ── Google Benchmark ─────────────────────────────────────────
# FetchContent google/benchmark v1.8.3 for the perf suite.
# Mirrors AddGTest.cmake: suspends -Werror / clang-tidy / IWYU so
# the third-party library compiles clean under project strictness.
# ─────────────────────────────────────────────────────────────

include(FetchContent)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.8.3
)

# ── Suspend project strictness for benchmark ──
set(_saved_tidy "${CMAKE_CXX_CLANG_TIDY}")
set(CMAKE_CXX_CLANG_TIDY "")
set(_saved_iwyu "${CMAKE_CXX_INCLUDE_WHAT_YOU_USE}")
set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "")

get_directory_property(_saved_opts COMPILE_OPTIONS)
set(_clean_opts "${_saved_opts}")
list(FILTER _clean_opts EXCLUDE REGEX "-Werror|-Wpedantic")
set_directory_properties(PROPERTIES COMPILE_OPTIONS "${_clean_opts}")

FetchContent_MakeAvailable(googlebenchmark)

# ── Silence benchmark's own warnings ──
foreach(_gb benchmark benchmark_main)
    if(TARGET ${_gb})
        target_compile_options(${_gb} PRIVATE -w)
    endif()
endforeach()

# ── Restore project strictness ──
set(CMAKE_CXX_CLANG_TIDY "${_saved_tidy}")
set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${_saved_iwyu}")
set_directory_properties(PROPERTIES COMPILE_OPTIONS "${_saved_opts}")
