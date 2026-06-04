# Include-What-You-Use gate. Adapted from l0's iwyu.sh (single-lib paths).
#
# Runs IWYU as a compiler proxy via CMAKE_CXX_INCLUDE_WHAT_YOU_USE (set
# by the `iwyu` preset → WITH_IWYU=ON → cmake/IWYU.cmake). The gate
# fails iff at least one TU has an actionable recommendation (an
# #include line inside an "add" block or a `- #include` line inside a
# "remove" block).

cmd_iwyu() {
    require_tool cmake
    require_tool ninja
    require_tool include-what-you-use
    setup_ccache

    header "iwyu"

    local build_subdir="build-iwyu"
    local build_dir="$REPO_ROOT/$build_subdir"
    local log_file="$REPO_ROOT/${build_subdir}-build.log"
    local junit_file="$build_dir/iwyu-results.xml"
    local recs_file="$build_dir/iwyu-recommendations.txt"

    # ninja skips up-to-date .o files — and skipping the build action
    # skips IWYU with it. A stale build-iwyu/ from a prior run silently
    # produces a fake-green gate. Wipe at job start to force every TU
    # through IWYU. (Same rationale as l0's iwyu.sh.)
    rm -rf "$build_dir"
    mkdir -p "$build_dir"
    : > "$log_file"

    info "configure (iwyu preset, B=$build_subdir)"
    if ! ( cd "$REPO_ROOT" && cmake --preset iwyu -DBUILD_TESTING=ON -B "$build_subdir" ) \
            > "$log_file" 2>&1; then
        warn "configure failed — last 50 lines of $log_file:"
        tail -50 "$log_file" >&2
        _write_junit_stub "$junit_file" "cmake-configure" \
            "cmake --preset iwyu failed"
        fail "cmake configure failed (iwyu)"
    fi

    info "build (-j$(nproc_value)) — IWYU runs as compiler proxy"
    if ! ( cd "$REPO_ROOT" && cmake --build "$build_subdir" -j"$(nproc_value)" ) \
            >> "$log_file" 2>&1; then
        warn "build failed — last 100 lines of $log_file:"
        tail -100 "$log_file" >&2
        _write_junit_stub "$junit_file" "cmake-build" \
            "cmake --build failed under IWYU"
        fail "cmake build failed (iwyu)"
    fi
    info "build succeeded ($(wc -l < "$log_file") lines logged to $log_file)"

    # The full log carries clang-tidy noise per TU (Tidy.cmake is
    # unconditional) — multi-GB. Ship only a tail + the recommendations.
    tail -500 "$log_file" > "$build_dir/build-tail.log" || true

    info "scanning for IWYU recommendations"
    if awk '
        /should add these lines:$/ {
            path=$0; sub(/ should add these lines:$/, "", path)
            mode="add"; next
        }
        /should remove these lines:$/ {
            path=$0; sub(/ should remove these lines:$/, "", path)
            mode="remove"; next
        }
        /^The full include-list for / {
            mode=""; next
        }
        mode=="add"    && /^#include/    { print "ADD    " path "  " $0; dirty=1; next }
        mode=="remove" && /^- #include/ { print "REMOVE " path "  " $0; dirty=1; next }
        END { exit dirty ? 1 : 0 }
    ' "$log_file" > "$recs_file"; then
        rm -f "$recs_file"
        ok "iwyu green (no include-cleanliness issues)"
    else
        local n
        n=$(wc -l < "$recs_file")
        warn "iwyu emitted $n recommendation(s); first 40:"
        head -40 "$recs_file" >&2
        if (( n > 40 )); then warn "... and $((n - 40)) more in $recs_file"; fi
        _write_junit_stub "$junit_file" "iwyu" \
            "IWYU reported $n include-cleanliness issue(s)"
        fail "iwyu reported include-cleanliness issues"
    fi

    show_ccache_stats
}
