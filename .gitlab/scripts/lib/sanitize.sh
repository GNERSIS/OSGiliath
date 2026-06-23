# sanitize.sh — shared TSan / MSan runner, per component.
#
# Usage: cmd_sanitize <flavor> <component>     flavor ∈ {tsan,msan}
#
# The `tsan` / `msan` presets (CMakePresets.json) carry THREAD_SANITIZE=ON /
# MEMORY_SANITIZE=ON (+ ENABLE_COVERAGE=OFF); the MSan preset also links the
# instrumented libc++ from /opt/llvm-msan (Sanitizers.cmake), so no extra -D
# flags are needed here — the preset IS the sanitizer selection.
#
# Per the RUN matrix (§2c):
#   tsan: src/applications → app-smoke (non-GL); examples → compile-only
#         (uninstrumented GL-driver threads flood TSan); tests → ctest
#   msan: src/applications/examples → compile-only (apps/examples pull GL +
#         uninstrumented C deps); tests → ctest (all 3 tests are non-GL)
# Compile-only components are STILL real jobs — they build under the
# sanitizer, they just have no RUN step. Never skipped, never allow_failure.

cmd_sanitize() {
    local flavor="$1" component="$2"
    case "$flavor" in
        tsan|msan) ;;
        *) fail "unknown sanitizer flavor: $flavor (expected tsan|msan)" ;;
    esac
    require_component "$component"

    # MSan needs the instrumented libc++ runtime; fail early + clearly if the
    # CI image lacks it rather than dying deep in a link error.
    if [[ "$flavor" == msan && ! -d /opt/llvm-msan/include/c++/v1 ]]; then
        fail "MSan-instrumented libc++ not found at /opt/llvm-msan; \
run inside the CI image, or provide a local /opt/llvm-msan symlink"
    fi

    setup_ccache

    header "$flavor  $component"

    local build_dir="$REPO_ROOT/$(kind_build_dir "$flavor")"
    local log_file="$REPO_ROOT/$flavor-$component.log"
    local junit_file="$build_dir/$flavor-results.xml"

    _configure_and_build "$flavor" "$component" "$build_dir" "$log_file" "$junit_file"
    _run_component "$flavor" "$component" "$build_dir" "$log_file"

    show_ccache_stats
    ok "$flavor  $component  green"
}
