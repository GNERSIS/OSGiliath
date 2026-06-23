# ── Coverage ─────────────────────────────────────────────────
# Default ON (preserves current behaviour). Sanitizer presets turn it
# OFF: the gcov runtime is uninstrumented (→ MSan false positives) and
# the instrumentation roughly halves TSan/MSan throughput.
# ─────────────────────────────────────────────────────────────

option(ENABLE_COVERAGE "Enable --coverage instrumentation" ON)
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(--coverage)
    add_link_options(--coverage)
    message(STATUS "Coverage: enabled (--coverage)")
else()
    message(STATUS "Coverage: disabled")
endif()
