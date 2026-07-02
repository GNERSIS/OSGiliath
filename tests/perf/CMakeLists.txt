# ── Performance suite ────────────────────────────────────────
# Micro-benchmarks (Google Benchmark) + the EGL headless frame
# stress harness. Names deliberately do NOT end in _test so the
# CI ctest sweep (-R '_test$') never picks them up; run manually:
#
#   cmake --preset profile && ninja -C build-profile perf_all
#   ./build-profile/bin/bench_object_model
#   ./build-profile/bin/perf_frames --scene grid --count 10000
# ─────────────────────────────────────────────────────────────

include(AddGBench)

set(PERF_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)

# ── Micro-benchmarks ──
set(PERF_BENCHES
    bench_object_model
    bench_traversal
    bench_renderbackend
    bench_maths
)

foreach(_bench IN LISTS PERF_BENCHES)
    add_executable(${_bench} ${_bench}.cpp)
    target_link_libraries(${_bench} PRIVATE osg osgUtil benchmark::benchmark_main)
    target_include_directories(${_bench} PRIVATE ${PERF_INCLUDE_DIRS})
    # The BENCHMARK() registration macro expands __COUNTER__ in our TUs,
    # which clang's -Wpedantic flags as a C2y extension.
    target_compile_options(${_bench} PRIVATE -Wno-c2y-extensions)
endforeach()

# ── Frame stress harness (EGL headless, whole frame loop) ──
add_executable(perf_frames perf_frames.cpp)
target_link_libraries(perf_frames PRIVATE osg osgDB osgGA osgUtil osgViewer)
target_include_directories(perf_frames PRIVATE ${PERF_INCLUDE_DIRS})

add_custom_target(perf_all DEPENDS ${PERF_BENCHES} perf_frames)
