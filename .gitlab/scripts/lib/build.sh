# Build + 0-warning clang-tidy gate + ctest.
#
# Unlike l0 (whose `build` wave is Release/no-sanitizers), OSGiliath's
# toolchain keeps ASan+UBSan and --coverage ALWAYS on (cmake/Sanitizers.cmake,
# Coverage.cmake have no toggles), so this single job covers l0's `build`
# AND `asan` kinds: compile under -Werror, clang-tidy on every TU, then
# ctest runs the GTest suite under ASan+UBSan.

cmd_build() {
    require_tool cmake
    require_tool ninja
    setup_ccache

    header "build  (default preset: clang, ASan+UBSan, tidy on every TU, ctest)"

    local build_dir="$REPO_ROOT/build"
    local log_file="$REPO_ROOT/build.log"
    local junit_file="$build_dir/ctest-results.xml"

    mkdir -p "$build_dir"
    : > "$log_file"

    # BUILD_OSG_EXAMPLES=ON comes from the preset — examples are part of
    # the tidy surface by design (l0: "ALL means ALL").
    info "configure (default preset, BUILD_TESTING=ON)"
    if ! ( cd "$REPO_ROOT" && cmake --preset default -DBUILD_TESTING=ON ) \
            > "$log_file" 2>&1; then
        warn "configure failed — last 50 lines of $log_file:"
        tail -50 "$log_file" >&2
        _write_junit_stub "$junit_file" "cmake-configure" \
            "cmake --preset default failed"
        fail "cmake configure failed"
    fi

    info "build (-j$(nproc_value))"
    # Log to file only — keeps GitLab's 4 MB job-log cap from filling with
    # compile commands + tidy notes. Full log is in the artifact.
    if ! ( cd "$REPO_ROOT" && cmake --build build -j"$(nproc_value)" ) \
            >> "$log_file" 2>&1; then
        warn "build failed — last 100 lines of $log_file:"
        tail -100 "$log_file" >&2
        _write_junit_stub "$junit_file" "cmake-build" \
            "cmake --build failed"
        fail "cmake build failed"
    fi
    info "build succeeded ($(wc -l < "$log_file") lines logged to $log_file)"

    # ctest BEFORE the tidy gate (deliberate reorder vs l0): the tidy
    # gate starts deep red on this codebase (~880k warnings at adoption,
    # hard-gate policy accepted), and running tests first means the
    # JUnit report still lands in GitLab while the cleanup campaign runs.
    info "ctest (--parallel $(nproc_value))"
    if ! ( cd "$REPO_ROOT" && ctest --test-dir build \
                                    --output-on-failure \
                                    --parallel "$(nproc_value)" \
                                    --output-junit ctest-results.xml ); then
        # ctest emits its own ctest-results.xml even on failure.
        fail "ctest failed"
    fi

    info "tidy gate (project policy: 0 warnings)"
    # Anchored to clang-tidy's diagnostic format:
    #   path/to/file.cpp:LINE:COL: warning: <msg> [check-name]
    # Compile warnings (no [check-name] suffix) are caught earlier by
    # -Werror in the build phase, so this gate's job is strictly tidy.
    local tidy_re='^[^:]+:[0-9]+:[0-9]+: warning: .* \[[a-z][a-z0-9.-]*\]$'
    # build.log is multi-GB on this codebase (tidy notes per TU) — never
    # ship it as an artifact. Ship instead: the DEDUPED warning list
    # (headers re-warn once per including TU; unique list is ~30x smaller),
    # a per-check summary, and a tail for general triage.
    tail -500 "$log_file" > "$build_dir/build-tail.log" || true
    if grep -qE "$tidy_re" "$log_file"; then
        local n n_unique
        n=$(grep -cE "$tidy_re" "$log_file" || true)
        grep -hE "$tidy_re" "$log_file" | sort -u \
            > "$build_dir/tidy-warnings-unique.log" || true
        n_unique=$(wc -l < "$build_dir/tidy-warnings-unique.log")
        grep -hE "$tidy_re" "$log_file" \
            | sed -E 's/.*\[([a-z][a-z0-9.-]*)\]$/\1/' \
            | sort | uniq -c | sort -rn | head -15 \
            > "$build_dir/tidy-warning-summary.txt" || true
        warn "clang-tidy emitted $n warning(s) ($n_unique unique); first 20:"
        # `grep | head` SIGPIPEs grep when head closes early; under
        # pipefail+errexit that would kill the script before fail() runs.
        grep -E "$tidy_re" "$log_file" | head -20 >&2 || true
        warn "artifacts: tidy-warnings-unique.log + tidy-warning-summary.txt"
        fail "clang-tidy emitted $n warning(s) ($n_unique unique)"
    fi
    ok "tidy gate green (0 warnings)"

    show_ccache_stats
    ok "build green"
}
