#!/usr/bin/env bash
# ci-run.sh — single executable source of truth for OSGiliath CI.
# Adapted from l0's .gitlab/scripts/ci-run.sh for a single-lib repo.
#
# Runs locally and inside the CI image with identical behaviour. The
# .gitlab-ci.yml's per-stage `script:` blocks call into this script
# rather than embedding logic inline, so "CI passed but local fails"
# (or vice versa) is structurally impossible.
#
# Subcommands:
#   format    clang-format --dry-run --Werror over the Format.cmake file set
#   build     cmake --preset default -DBUILD_TESTING=ON; build everything
#             (lib + plugins + apps + examples + tests) under -Werror with
#             ASan+UBSan always-on; 0-warning clang-tidy gate; ctest
#   iwyu      cmake --preset iwyu into build-iwyu/ (wiped first); fails on
#             any actionable add/remove include recommendation
#   full      format → build → iwyu
#
# Examples:
#   .gitlab/scripts/ci-run.sh format
#   .gitlab/scripts/ci-run.sh build
#   JFACTOR=8 .gitlab/scripts/ci-run.sh iwyu
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
# shellcheck source=lib/iwyu.sh
source "$SCRIPT_DIR/lib/iwyu.sh"

usage() {
    sed -nE 's/^# ?(.*)$/\1/p' "$0" | sed -n '2,/^Exit codes/p'
    exit 2
}

main() {
    local cmd="${1:-}"
    case "$cmd" in
        format) cmd_format ;;
        build)  cmd_build ;;
        iwyu)   cmd_iwyu ;;
        full)
            cmd_format
            cmd_build
            cmd_iwyu
            ok "full pipeline green"
            ;;
        ''|-h|--help) usage ;;
        *)
            warn "unknown subcommand: $cmd"
            usage
            ;;
    esac
}

main "$@"
