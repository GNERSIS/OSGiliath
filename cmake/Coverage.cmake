# ── Coverage ─────────────────────────────────────────────────
# Always enabled. Generates gcov-compatible coverage data.
# ─────────────────────────────────────────────────────────────

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(--coverage)
    add_link_options(--coverage)
    message(STATUS "Coverage: enabled (--coverage)")
else()
    message(STATUS "Coverage: skipped (MSVC)")
endif()
