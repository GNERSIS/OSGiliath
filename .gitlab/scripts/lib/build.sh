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
    if ! ( cd "$REPO_ROOT" && cmake --preset default \
                                      -DBUILD_TESTING=ON \
                                      -DBUILD_OSG_PLUGIN_FFMPEG=0 ) \
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

    info "tidy gate (project policy: 0 new warnings)"
    # Anchored to clang-tidy's diagnostic format:
    #   path/to/file.cpp:LINE:COL: warning: <msg> [check-name]
    #   path/to/file.cpp:LINE:COL: warning: <msg> [check-a,check-b]
    # Compiler diagnostics surfaced by clang-tidy use clang-diagnostic-* and
    # are caught earlier by -Werror in the build phase, so this gate's job is
    # strictly tidy checks.
    local tidy_re='^[^:]+:[0-9]+:[0-9]+: warning: .* \[[[:alpha:]][^]]*\]$'
    local tidy_ignore_re=' \[clang-diagnostic-[^]]*\]$'
    # build.log can be large on this codebase. Ship instead: normalized,
    # deduped new warnings, a per-check summary, and a tail for triage.
    tail -500 "$log_file" > "$build_dir/build-tail.log" || true
    local tidy_baseline="$REPO_ROOT/cmake/tidy-baseline.txt"
    local tidy_warnings_all="$build_dir/tidy-warnings-all.log"
    local tidy_warnings="$build_dir/tidy-warnings-new.log"
    grep -hE "$tidy_re" "$log_file" \
        | grep -Ev "$tidy_ignore_re" \
        | awk -v repo="$REPO_ROOT" '{
            sub("^" repo "/", "")
            sub("^/workspace/", "")
            print
        }' \
        | sort -u > "$tidy_warnings_all" || true

    if [[ -s "$tidy_warnings_all" ]]; then
        if [[ -f "$tidy_baseline" ]]; then
            local tidy_baseline_sorted="$build_dir/tidy-baseline-sorted.log"
            sort -u "$tidy_baseline" > "$tidy_baseline_sorted"
            comm -13 "$tidy_baseline_sorted" "$tidy_warnings_all" > "$tidy_warnings"
        else
            cp "$tidy_warnings_all" "$tidy_warnings"
        fi
    else
        rm -f "$tidy_warnings"
    fi

    if [[ -s "$tidy_warnings" ]]; then
        local n
        n=$(wc -l < "$tidy_warnings")
        cp "$tidy_warnings" "$build_dir/tidy-warnings-unique.log"
        sed -E 's/.*\[([^]]+)\]$/\1/' "$tidy_warnings" \
            | tr ',' '\n' | sort | uniq -c | sort -rn | head -15 \
            > "$build_dir/tidy-warning-summary.txt" || true
        warn "clang-tidy emitted $n new warning(s); first 20:"
        head -20 "$tidy_warnings" >&2 || true
        warn "artifacts: tidy-warnings-unique.log + tidy-warning-summary.txt"
        fail "clang-tidy emitted $n new warning(s)"
    fi

    if [[ -s "$tidy_warnings_all" ]]; then
        local baseline_count
        baseline_count=$(wc -l < "$tidy_warnings_all")
        ok "tidy gate green ($baseline_count baseline warning(s), 0 new)"
    else
        ok "tidy gate green (0 warnings)"
    fi
    rm -f "$tidy_warnings"

    show_ccache_stats
    ok "build green"
}
