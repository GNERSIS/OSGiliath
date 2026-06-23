# ── Tidy ─────────────────────────────────────────────────────
# Runs clang-tidy on every compiled source via CMAKE_CXX_CLANG_TIDY.
# Gated behind ENABLE_TIDY (default ON). The build/default configs run the
# per-TU tidy that the `build` CI kind gates on; the asan/tsan/msan sanitizer
# presets set ENABLE_TIDY=OFF, because those builds exist to surface sanitizer
# findings — running Kings Tidy on every TU there is 5-10x wasted compile time
# for output that is discarded.
# Hard abort if tidy is requested (ENABLE_TIDY=ON) but not installed.
# Requires CMAKE_EXPORT_COMPILE_COMMANDS=ON in the root CMakeLists.
# ─────────────────────────────────────────────────────────────

option(ENABLE_TIDY "Run clang-tidy on every compiled TU" ON)
if(NOT ENABLE_TIDY)
    message(STATUS "Tidy: disabled (ENABLE_TIDY=OFF)")
    return()
endif()

find_program(CLANG_TIDY clang-tidy)
if(NOT CLANG_TIDY)
    if(WIN32)
        set(_hint "Install LLVM: https://github.com/llvm/llvm-project/releases")
    else()
        set(_hint "Install it: apt install clang-tidy")
    endif()
    message(FATAL_ERROR "clang-tidy not found.\n${_hint}")
endif()
message(STATUS "Tidy: ${CLANG_TIDY}")

# Analyse project headers (include/ and src/), not thirdparty/_deps
set(CMAKE_CXX_CLANG_TIDY
    ${CLANG_TIDY}
    --config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy
    "--header-filter=${CMAKE_CURRENT_SOURCE_DIR}/(include|src)/.*"
)
