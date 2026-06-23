# asan kind — dedicated ASan + UBSan RUN, per component.
#
# The `asan` preset (CMakePresets.json) routes through Sanitizers.cmake's
# default branch (-fsanitize=address,undefined + -fno-omit-frame-pointer)
# with ENABLE_COVERAGE=OFF. Binaries trip ASan on heap-overflow / UAF /
# double-free / leaks and UBSan on signed overflow / null / misaligned /
# invalid-enum / vptr.
#
# Per the RUN matrix (§2c) the `asan` kind RUNs everywhere a runtime check
# pays off:
#   src           → compile only
#   applications  → app-smoke (osgversion + osgconv, non-GL)
#   examples      → render-smoke (6 headless examples, Xvfb + llvmpipe)
#   tests         → ctest
# (No tidy gate — that fires only under the `build` kind on src.)

cmd_asan() {
    local component="$1"
    require_component "$component"
    setup_ccache

    header "asan  $component  (ASan + UBSan)"

    local build_dir="$REPO_ROOT/$(kind_build_dir asan)"
    local log_file="$REPO_ROOT/asan-$component.log"
    local junit_file="$build_dir/asan-results.xml"

    _configure_and_build asan "$component" "$build_dir" "$log_file" "$junit_file"
    _run_component asan "$component" "$build_dir" "$log_file"

    show_ccache_stats
    ok "asan  $component  green"
}
