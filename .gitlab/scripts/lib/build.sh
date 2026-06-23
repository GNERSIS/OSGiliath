# build kind — the no-sanitizer baseline compile, per component.
#
# The `build` preset (CMakePresets.json) is ENABLE_SANITIZERS=OFF +
# ENABLE_COVERAGE=OFF: an optimized compile under -Werror with clang-tidy
# running on every TU. This is l0's `build` kind (the dedicated `asan`
# kind owns the ASan/UBSan RUN now — see lib/asan.sh).
#
# Per the RUN matrix (§2c) the `build` kind RUNs:
#   src           → tidy gate (the ONLY place the 0-new-warning gate fires)
#   applications  → app-smoke (osgversion + osgconv, non-GL)
#   examples      → render-smoke (6 headless examples, Xvfb + llvmpipe)
#   tests         → ctest
# All of that is dispatched by _run_component; this file owns the build
# configure/compile and the tidy gate itself.

cmd_build() {
    local component="$1"
    require_component "$component"
    setup_ccache

    header "build  $component  (no-sanitizer baseline, clang-tidy on every TU)"

    local build_dir="$REPO_ROOT/$(kind_build_dir build)"
    local log_file="$REPO_ROOT/build-$component.log"
    local junit_file="$build_dir/build-results.xml"

    # Wipe build/ at job start — SAME rationale as iwyu.sh's rm -rf (read its
    # comment). clang-tidy runs via CMAKE_CXX_CLANG_TIDY ONLY when ninja fires a
    # compile rule for a TU. On an incremental re-run ninja skips up-to-date TUs,
    # so clang-tidy is never re-invoked on a dirty-but-unchanged file → the tidy
    # gate greps an empty log and reports a green that proves nothing. CI clones
    # fresh per job (build/ always absent there), so wiping locally is what makes
    # the local tidy gate CI-faithful and forces EVERY TU back through clang-tidy
    # each invocation. ccache keeps the recompile cheap; clang-tidy itself still
    # re-runs, which is exactly the point of the wipe.
    rm -rf "$build_dir"

    _configure_and_build build "$component" "$build_dir" "$log_file" "$junit_file"
    _run_component build "$component" "$build_dir" "$log_file"

    show_ccache_stats
    ok "build  $component  green"
}

# 0-new-warning clang-tidy gate. Tidy.cmake runs clang-tidy during the
# build (warnings only — compiler -Werror failures are caught earlier in
# _configure_and_build), so the gate is a post-build scan of the build log.
# Fires ONLY on `build src` (§2c): src is the lib's own TUs; apps/examples/
# tests run their RUN steps instead. New warnings (vs cmake/tidy-baseline.txt)
# fail the gate; baseline warnings pass.
_tidy_gate() {
    local build_dir="$1" log_file="$2"

    info "tidy gate (project policy: 0 new warnings)"
    # clang-tidy diagnostic shape:
    #   path/file.cpp:LINE:COL: warning: <msg> [check-name]
    # Compiler diagnostics surfaced by clang-tidy use clang-diagnostic-* and
    # are already caught by -Werror in the build, so the gate is strictly
    # tidy checks.
    local tidy_re='^[^:]+:[0-9]+:[0-9]+: warning: .* \[[[:alpha:]][^]]*\]$'
    local tidy_ignore_re=' \[clang-diagnostic-[^]]*\]$'

    # build-tail.log is written authoritatively by _configure_and_build
    # (_persist_build_tail) for every kind×component, so the tidy gate no longer
    # needs to emit it here.
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
}
