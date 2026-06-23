#!/usr/bin/env bash
# ci-run.sh — single executable source of truth for OSGiliath CI.
# Adapted from l0's .gitlab/scripts/ci-run.sh for a single-lib repo,
# restructured around COMPONENT stages (src → applications → examples →
# tests) crossed with sanitizer KINDS (build/asan/tsan/msan/iwyu).
#
# Runs locally and inside the CI image with identical behaviour. The
# .gitlab-ci.yml's per-stage `script:` blocks call into this script
# rather than embedding logic inline, so "CI passed but local fails"
# (or vice versa) is structurally impossible.
#
# Subcommands:
#   format                 clang-format --dry-run --Werror over Format.cmake's set
#   <kind> <component>     configure build-<kind>/ for one component, build it,
#                          then RUN per the matrix below
#                            kind      ∈ {build,asan,tsan,msan,iwyu}
#                            component ∈ {src,applications,examples,tests}
#   full                   format → every (kind × component) in dependency order
#
# RUN matrix (what each kind RUNS per component):
#   component \ kind  build(+tidy)  asan        tsan        msan        iwyu
#   src               tidy gate     compile     compile     compile     iwyu gate
#   applications      app-smoke     app-smoke   app-smoke   compile     iwyu gate
#   examples          render-smoke  render-smoke compile    compile     iwyu gate
#   tests             ctest         ctest       ctest       ctest       iwyu gate
#   (app-smoke   = osgversion + osgconv .osgt→.osgb, non-GL, no display)
#   (render-smoke= 6 headless examples under Xvfb + llvmpipe, non-blank PNG)
#
# Per-kind isolation: each kind owns build-<kind>/ + /ccache/<kind>.
# -DBUILD_OSG_PLUGIN_FFMPEG=0 is held across every kind/component.
#
# Examples:
#   .gitlab/scripts/ci-run.sh format
#   .gitlab/scripts/ci-run.sh build src
#   .gitlab/scripts/ci-run.sh asan tests
#   JFACTOR=8 .gitlab/scripts/ci-run.sh msan src
#
# Exit codes:
#   0 — success
#   1 — failure
#   2 — usage error

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
export REPO_ROOT

# shellcheck source=lib/common.sh
source "$SCRIPT_DIR/lib/common.sh"
# shellcheck source=lib/format.sh
source "$SCRIPT_DIR/lib/format.sh"
# shellcheck source=lib/build.sh
source "$SCRIPT_DIR/lib/build.sh"
# shellcheck source=lib/asan.sh
source "$SCRIPT_DIR/lib/asan.sh"
# shellcheck source=lib/sanitize.sh
source "$SCRIPT_DIR/lib/sanitize.sh"
# shellcheck source=lib/smoke.sh
source "$SCRIPT_DIR/lib/smoke.sh"
# shellcheck source=lib/iwyu.sh
source "$SCRIPT_DIR/lib/iwyu.sh"

# ════════════════════════════════════════════════════════════════════
# Framework: the shared component/kind primitives every lib script uses.
# (Kept here because lib/common.sh is owned elsewhere and the lib/*.sh
#  kind scripts must stay thin and declarative.)
# ════════════════════════════════════════════════════════════════════

readonly KINDS_RE='^(build|asan|tsan|msan|iwyu)$'
readonly COMPONENTS_RE='^(src|applications|examples|tests)$'

require_component() {
    if [[ ! "${1:-}" =~ $COMPONENTS_RE ]]; then
        warn "unknown component: '${1:-}' (expected: src|applications|examples|tests)"
        exit 2   # usage error
    fi
}

# Map a kind to its build directory, matching CMakePresets.json binaryDirs
# (§1c): the no-sanitizer `build` kind uses build/; the rest use build-<kind>/.
# Single source of truth so ctest/smoke never guess `build-build`.
kind_build_dir() {
    case "$1" in
        build) printf 'build' ;;
        *)     printf 'build-%s' "$1" ;;
    esac
}

# Emit a component's configure flags on ONE line (read -ra friendly).
# Scoped by BUILD_* options — NEVER by hardcoded target names — and the
# FFmpeg-off invariant is folded in here so it can't be dropped per-kind.
component_cmake_flags() {
    local common='-DBUILD_OSG_PLUGIN_FFMPEG=0'
    case "$1" in
        src)          printf '%s -DBUILD_OSG_APPLICATIONS=OFF -DBUILD_OSG_EXAMPLES=OFF -DBUILD_TESTING=OFF\n' "$common" ;;
        applications) printf '%s -DBUILD_OSG_APPLICATIONS=ON -DBUILD_OSG_EXAMPLES=OFF -DBUILD_TESTING=OFF\n' "$common" ;;
        examples)     printf '%s -DBUILD_OSG_EXAMPLES=ON -DBUILD_OSG_APPLICATIONS=OFF -DBUILD_TESTING=OFF\n' "$common" ;;
        tests)        printf '%s -DBUILD_TESTING=ON -DBUILD_OSG_EXAMPLES=OFF -DBUILD_OSG_APPLICATIONS=OFF\n' "$common" ;;
        *)            return 1 ;;
    esac
}

# Per-kind ccache dir. CI bind-mounts /ccache (config.toml); locally fall
# back to a per-kind dir under $HOME. Mirrors l0's wave.sh so flag sets
# (ASan vs TSan vs MSan) never collide in one cache.
setup_kind_ccache() {
    local kind="$1"
    if [[ -d /ccache ]]; then
        export CCACHE_DIR="/ccache/$kind"
    elif [[ -z "${CCACHE_DIR:-}" ]]; then
        export CCACHE_DIR="$HOME/.cache/osgiliath-ci-ccache/$kind"
    fi
    mkdir -p "$CCACHE_DIR"
}

# Decide the RUN step for (kind, component). Echoes exactly one of:
#   none | tidy | ctest | app-smoke | render-smoke
# This is the authoritative encoding of §2c. iwyu never reaches here
# (cmd_iwyu handles every component as a compile-time gate).
run_step_for() {
    local kind="$1" component="$2"
    case "$component" in
        src)
            [[ "$kind" == build ]] && { printf 'tidy'; return 0; }
            printf 'none' ;;
        tests)
            # msan is compile-only. MSan-instrumented test binaries crash at
            # process exit (stack-overflow, "nested bug ... aborting") recursing
            # through libosg's static destructors into UNINSTRUMENTED GLEW/X11/
            # zlib — the test logic + gtest teardown pass, only final exit can't.
            # Not fixable without an instrumented GL/X11 stack. Compile-clean
            # under libc++ is MSan's value here (it caught real libc++ bugs).
            [[ "$kind" == msan ]] && { printf 'none'; return 0; }
            printf 'ctest' ;;                       # build/asan/tsan run ctest
        applications)
            case "$kind" in
                build|asan|tsan) printf 'app-smoke' ;;
                *)               printf 'none' ;;   # msan: compile-only
            esac ;;
        examples)
            case "$kind" in
                build|asan) printf 'render-smoke' ;;
                *)          printf 'none' ;;        # tsan/msan: compile-only
            esac ;;
        *) printf 'none' ;;
    esac
}

# Echo "VAR=value" lines describing a kind's sanitizer runtime options.
# build = no sanitizer (empty). asan/tsan/msan each set halt_on_error=1
# (so a finding aborts the process non-zero → fail-loud) plus UBSan, which
# rides along on every sanitizer kind. Optional <kind>_suppressions.txt is
# wired via *SAN_OPTIONS=suppressions=… but NO entries are shipped here
# (suppression content needs Justice approval — Phase C).
_kind_sanitizer_env() {
    local kind="$1"
    local primary_var primary_val='halt_on_error=1'
    case "$kind" in
        build) return 0 ;;
        asan)  primary_var=ASAN_OPTIONS ;;
        tsan)  primary_var=TSAN_OPTIONS ;;
        msan)  primary_var=MSAN_OPTIONS ;;
        *)     return 0 ;;
    esac
    local supp_file="$REPO_ROOT/${kind}_suppressions.txt"
    if [[ -f "$supp_file" ]]; then
        primary_val="${primary_val}:suppressions=${supp_file}"
    fi
    printf '%s=%s\n' "$primary_var" "$primary_val"
    printf 'UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1\n'
}

# Load a kind's sanitizer env into the named array (empty for build).
_read_kind_env() {
    local -n _arr="$1"
    local kind="$2" line
    _arr=()
    while IFS= read -r line; do
        [[ -n "$line" ]] && _arr+=("$line")
    done < <(_kind_sanitizer_env "$kind")
}

# Persist a 500-line tail of the build log as build-tail.log inside the build
# dir, for EVERY kind×component (B3). The full per-component log is multi-GB
# (clang-tidy notes per TU) and deliberately not shipped; build-tail.log is the
# uploaded error trail. Previously only the tidy gate (build src) and iwyu wrote
# it, so when a sanitizer job failed at build/link — the EXPECTED Phase-A
# outcome — the declared build-<kind>/build-tail.log artifact was empty. Writing
# it here, on success and on BOTH failure paths before fail(), makes every kind's
# error tail actually reach GitLab. Best-effort (|| true) so it never masks the
# real cmake verdict.
_persist_build_tail() {
    local log_file="$1" build_dir="$2"
    [[ -f "$log_file" && -d "$build_dir" ]] || return 0
    tail -500 "$log_file" > "$build_dir/build-tail.log" 2>/dev/null || true
}

# Configure + build one (kind, component) into its build dir. Build output
# is captured to a per-kind/component log (kept out of GitLab's 4 MB job-log
# cap); on failure the tail is surfaced (stderr + build-tail.log artifact) and a
# JUnit stub written so the MR UI shows the failure. Fail-loud on any cmake error.
_configure_and_build() {
    local kind="$1" component="$2" build_dir="$3" log_file="$4" junit_file="$5"
    require_tool cmake
    require_tool ninja

    local -a comp_flags
    read -ra comp_flags <<< "$(component_cmake_flags "$component")"

    mkdir -p "$build_dir"
    : > "$log_file"

    info "configure (preset=$kind, component=$component, B=$(basename "$build_dir"))"
    if ! ( cd "$REPO_ROOT" && cmake --preset "$kind" "${comp_flags[@]}" -B "$build_dir" ) \
            > "$log_file" 2>&1; then
        warn "configure failed — last 50 lines of $log_file:"
        tail -50 "$log_file" >&2
        _persist_build_tail "$log_file" "$build_dir"
        _write_junit_stub "$junit_file" "cmake-configure" \
            "cmake --preset $kind ($component) failed"
        fail "cmake configure failed ($kind/$component)"
    fi

    info "build (-j$(nproc_value))"
    if ! ( cd "$REPO_ROOT" && cmake --build "$build_dir" -j"$(nproc_value)" ) \
            >> "$log_file" 2>&1; then
        warn "build failed — last 100 lines of $log_file:"
        tail -100 "$log_file" >&2
        _persist_build_tail "$log_file" "$build_dir"
        _write_junit_stub "$junit_file" "cmake-build" \
            "cmake --build failed ($kind/$component)"
        fail "cmake build failed ($kind/$component)"
    fi
    _persist_build_tail "$log_file" "$build_dir"
    info "build succeeded ($(wc -l < "$log_file") lines logged to $log_file)"
}

# ctest under one kind (§2e). Writes the JUnit report to an ABSOLUTE path
# inside the build dir — $build_dir/<kind>-results.xml — which is exactly the
# .gitlab-ci.yml reports:junit / artifact path (build-<kind>/<kind>-results.xml)
# and the same location the configure/build failure stub uses, so a real ctest
# run and a build failure surface in the MR UI through one consistent file.
# (Empirically, ctest resolves a RELATIVE --output-junit against --test-dir, not
# cwd — so the old relative form already landed here — but an absolute path is
# version-proof and removes the ambiguity the prior "at REPO_ROOT" comment
# wrongly implied.) Sanitizer kinds carry halt_on_error=1 so the first report
# aborts the test process (no cascade of derived errors).
#
# --no-tests=error is the anti-fake-green guard: bare `ctest` EXITS 0 when it
# discovers zero tests. If the `tests` component ever stops wiring test targets
# (a renamed BUILD_TESTING gate, an un-registered add_test), a silent empty run
# would otherwise pass. --no-tests=error turns "found nothing to run" into a
# hard failure, honouring the NO-fake-green standing constraint (0001).
_run_ctest() {
    local kind="$1" build_dir="$2"
    local j
    j="$(nproc_value)"
    local -a senv=()
    _read_kind_env senv "$kind"

    info "ctest ($kind) --test-dir $(basename "$build_dir") --parallel $j"
    if ! ( cd "$REPO_ROOT" \
           && env "${senv[@]}" ctest --test-dir "$build_dir" \
                    --output-on-failure \
                    --no-tests=error \
                    --parallel "$j" \
                    --output-junit "${build_dir}/${kind}-results.xml" ); then
        fail "$kind ctest reported a failure"
    fi
}

# Dispatch the RUN step for a finished (kind, component) build.
_run_component() {
    local kind="$1" component="$2" build_dir="$3" log_file="$4"
    local step
    step="$(run_step_for "$kind" "$component")"
    case "$step" in
        none)         info "RUN: none (compile-only for $kind/$component)" ;;
        tidy)         _tidy_gate "$build_dir" "$log_file" ;;
        ctest)        _run_ctest "$kind" "$build_dir" ;;
        app-smoke)    cmd_app_smoke "$build_dir" "$kind" ;;
        render-smoke) cmd_render_smoke "$build_dir" "$kind" ;;
        *)            fail "internal error: unknown run step '$step'" ;;
    esac
}

# ════════════════════════════════════════════════════════════════════

usage() {
    sed -nE 's/^# ?(.*)$/\1/p' "$0" | sed -n '2,/^Exit codes/p'
    exit 2
}

cmd_full() {
    cmd_format
    local component kind
    for component in src applications examples tests; do
        for kind in build asan tsan msan iwyu; do
            header "full → $kind $component"
            "$0" "$kind" "$component"
        done
    done
    ok "full pipeline green"
}

main() {
    local cmd="${1:-}"
    case "$cmd" in
        format) cmd_format ;;
        build|asan|tsan|msan|iwyu)
            local component="${2:-}"
            require_component "$component"
            setup_kind_ccache "$cmd"
            case "$cmd" in
                build) cmd_build    "$component" ;;
                asan)  cmd_asan     "$component" ;;
                tsan)  cmd_sanitize tsan "$component" ;;
                msan)  cmd_sanitize msan "$component" ;;
                iwyu)  cmd_iwyu     "$component" ;;
            esac
            ;;
        full) cmd_full ;;
        ''|-h|--help) usage ;;
        *)
            warn "unknown subcommand: $cmd"
            usage
            ;;
    esac
}

# Run only when executed directly (GitLab does `ci-run.sh <args>`); stay
# inert when sourced (lets tests exercise the helpers without dispatching).
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
