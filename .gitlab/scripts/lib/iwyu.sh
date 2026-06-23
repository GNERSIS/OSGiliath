# iwyu kind — Include-What-You-Use gate, per component.
# Adapted from l0's iwyu.sh (single-lib paths).
#
# Runs IWYU as a compiler proxy via CMAKE_CXX_INCLUDE_WHAT_YOU_USE (set
# by the `iwyu` preset → WITH_IWYU=ON → cmake/IWYU.cmake). The gate
# fails iff at least one TU has an actionable recommendation (an
# #include line inside an "add" block or a `- #include` line inside a
# "remove" block).
#
# This is a COMPILE-TIME gate: it has no RUN step. It fires for every
# component (src/applications/examples/tests, §2c) — the component only
# selects which BUILD_* targets enter the IWYU compile surface. The build
# dir (build-iwyu, matching the preset) is wiped per run so ninja can't
# skip a stale TU and fake-green the gate.

cmd_iwyu() {
    local component="$1"
    require_component "$component"

    if ! command -v include-what-you-use >/dev/null 2>&1; then
        for candidate in "$HOME/.local/iwyu/bin" "$HOME/.cache/iwyu_build/bin"; do
            if [[ -x "$candidate/include-what-you-use" ]]; then
                PATH="$candidate:$PATH"
                export PATH
                break
            fi
        done
    fi

    require_tool cmake
    require_tool ninja
    require_tool include-what-you-use
    setup_ccache

    header "iwyu  $component"

    local build_subdir="build-iwyu"
    local build_dir="$REPO_ROOT/$build_subdir"
    local log_file="$REPO_ROOT/${build_subdir}-build.log"
    local junit_file="$build_dir/iwyu-results.xml"
    local recs_file="$build_dir/iwyu-recommendations.txt"
    local all_recs_file="$build_dir/iwyu-recommendations-all.txt"
    local baseline_file="$REPO_ROOT/cmake/iwyu-baseline.txt"

    # ninja skips up-to-date .o files — and skipping the build action
    # skips IWYU with it. A stale build-iwyu/ from a prior run silently
    # produces a fake-green gate. Wipe at job start to force every TU
    # through IWYU. (Same rationale as l0's iwyu.sh.)
    rm -rf "$build_dir"
    mkdir -p "$build_dir"
    : > "$log_file"

    local -a comp_flags
    read -ra comp_flags <<< "$(component_cmake_flags "$component")"

    info "configure (iwyu preset, component=$component, B=$build_subdir)"
    if ! ( cd "$REPO_ROOT" && cmake --preset iwyu \
                                      "${comp_flags[@]}" \
                                      -B "$build_subdir" ) \
            > "$log_file" 2>&1; then
        warn "configure failed — last 50 lines of $log_file:"
        tail -50 "$log_file" >&2
        _persist_build_tail "$log_file" "$build_dir"
        _write_junit_stub "$junit_file" "cmake-configure" \
            "cmake --preset iwyu ($component) failed"
        fail "cmake configure failed (iwyu/$component)"
    fi

    info "build (-j$(nproc_value)) — IWYU runs as compiler proxy"
    if ! ( cd "$REPO_ROOT" && cmake --build "$build_subdir" -j"$(nproc_value)" ) \
            >> "$log_file" 2>&1; then
        warn "build failed — last 100 lines of $log_file:"
        tail -100 "$log_file" >&2
        _persist_build_tail "$log_file" "$build_dir"
        _write_junit_stub "$junit_file" "cmake-build" \
            "cmake --build failed under IWYU"
        fail "cmake build failed (iwyu)"
    fi
    info "build succeeded ($(wc -l < "$log_file") lines logged to $log_file)"

    # The full log carries clang-tidy noise per TU (Tidy.cmake is
    # unconditional) — multi-GB. Ship only a tail + the recommendations.
    # (Same writer the failure paths above use, for a consistent artifact.)
    _persist_build_tail "$log_file" "$build_dir"

    info "scanning for IWYU recommendations"
    awk -v repo="$REPO_ROOT" '
        function normalize_path(path) {
            sub("^" repo "/", "", path)
            sub("^/workspace/", "", path)
            sub("^\\./", "", path)
            return path
        }
        function normalize_include(line) {
            sub(/^[[:space:]]+/, "", line)
            sub(/[[:space:]]+\/\/.*$/, "", line)
            sub(/[[:space:]]+$/, "", line)
            return line
        }
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
        mode=="add" && /^#include/ {
            print "ADD " normalize_path(path) " " normalize_include($0)
            next
        }
        mode=="remove" && /^- #include/ {
            line=$0
            sub(/^-[[:space:]]+/, "", line)
            print "REMOVE " normalize_path(path) " " normalize_include(line)
            next
        }
    ' "$log_file" | sort -u > "$all_recs_file"

    if [[ ! -s "$all_recs_file" ]]; then
        rm -f "$all_recs_file" "$recs_file"
        ok "iwyu green (no include-cleanliness issues)"
    else
        if [[ -f "$baseline_file" ]]; then
            local baseline_sorted="$build_dir/iwyu-baseline-sorted.txt"
            sort -u "$baseline_file" > "$baseline_sorted"
            comm -13 "$baseline_sorted" "$all_recs_file" > "$recs_file"
        else
            cp "$all_recs_file" "$recs_file"
        fi

        if [[ ! -s "$recs_file" ]]; then
            local baseline_count
            baseline_count=$(wc -l < "$all_recs_file")
            rm -f "$recs_file"
            ok "iwyu green (only $baseline_count baseline recommendation(s) remain)"
            show_ccache_stats
            return
        fi

        local n
        n=$(wc -l < "$recs_file")
        warn "iwyu emitted $n new recommendation(s); first 40:"
        head -40 "$recs_file" >&2
        if (( n > 40 )); then warn "... and $((n - 40)) more in $recs_file"; fi
        _write_junit_stub "$junit_file" "iwyu" \
            "IWYU reported $n new include-cleanliness issue(s)"
        fail "iwyu reported new include-cleanliness issues"
    fi

    show_ccache_stats
}
