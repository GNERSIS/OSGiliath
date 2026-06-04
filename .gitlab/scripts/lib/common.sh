# Shared helpers: logging, ccache setup, parallelism, tool checks.
# Adapted from l0's .gitlab/scripts/lib/common.sh for the single-lib
# OSGiliath repo (REPO_ROOT is the lib itself — no <tier>/<lib> paths).

# ── Logging ─────────────────────────────────────────────────────────
# All output goes to stderr so stdout stays clean for subcommands that
# pipe into other tools.

if [[ -t 2 ]]; then
    readonly C_RED=$'\033[31m' C_YELLOW=$'\033[33m' C_GREEN=$'\033[32m'
    readonly C_BLUE=$'\033[34m' C_DIM=$'\033[2m' C_RESET=$'\033[0m'
else
    readonly C_RED='' C_YELLOW='' C_GREEN='' C_BLUE='' C_DIM='' C_RESET=''
fi

info()    { printf '%s[ci-run]%s %s\n'         "$C_BLUE"   "$C_RESET" "$*" >&2; }
ok()      { printf '%s[ci-run]%s %s\n'         "$C_GREEN"  "$C_RESET" "$*" >&2; }
warn()    { printf '%s[ci-run]%s %s\n'         "$C_YELLOW" "$C_RESET" "$*" >&2; }
fail()    { printf '%s[ci-run] FAIL%s %s\n'    "$C_RED"    "$C_RESET" "$*" >&2; exit 1; }
header()  { printf '\n%s━━ %s ━━%s\n\n'        "$C_BLUE"   "$*"      "$C_RESET" >&2; }

# ── ccache ──────────────────────────────────────────────────────────
# Honour CCACHE_DIR if already set (CI sets it per-kind); otherwise
# default to a per-user cache outside the project tree.

setup_ccache() {
    : "${CCACHE_DIR:=$HOME/.cache/osgiliath-ci-ccache}"
    : "${CCACHE_MAXSIZE:=10G}"
    : "${CCACHE_COMPRESS:=true}"
    : "${CCACHE_COMPRESSLEVEL:=6}"
    : "${CCACHE_COMPILERCHECK:=content}"
    export CCACHE_DIR CCACHE_MAXSIZE CCACHE_COMPRESS CCACHE_COMPRESSLEVEL CCACHE_COMPILERCHECK
    mkdir -p "$CCACHE_DIR"
    if command -v ccache >/dev/null 2>&1; then
        ccache --max-size="$CCACHE_MAXSIZE" >/dev/null
        ccache --zero-stats >/dev/null
    fi
}

show_ccache_stats() {
    if command -v ccache >/dev/null 2>&1; then
        ccache --show-stats 2>/dev/null | head -20 >&2 || true
    fi
}

# ── Parallelism ─────────────────────────────────────────────────────
# JFACTOR, when set, overrides nproc (used when multiple jobs share the
# runner; matches `concurrent` in /etc/gitlab-runner/config.toml).

nproc_value() {
    if [[ -n "${JFACTOR:-}" ]]; then
        printf '%s' "$JFACTOR"
        return 0
    fi
    if command -v nproc >/dev/null 2>&1; then
        nproc
    else
        getconf _NPROCESSORS_ONLN 2>/dev/null || printf '4'
    fi
}

# ── Required tools ──────────────────────────────────────────────────

require_tool() {
    local tool="$1"
    command -v "$tool" >/dev/null 2>&1 \
        || fail "required tool not found in PATH: $tool"
}

# ── JUnit stub ──────────────────────────────────────────────────────
# Write a minimal JUnit XML at $1 describing a failed phase $2 with
# message $3. GitLab's `reports.junit` ingests it and surfaces the
# failure in the MR UI even when ctest never ran.

_write_junit_stub() {
    local out="$1" phase="$2" message="$3"
    mkdir -p "$(dirname "$out")"
    cat > "$out" <<XML
<?xml version="1.0" encoding="UTF-8"?>
<testsuites>
  <testsuite name="$(basename "$(dirname "$out")")" tests="1" failures="1" errors="0" time="0">
    <testcase classname="ci-run.sh" name="$phase" time="0">
      <failure message="$message">$message — see the build log artifact for the full error.</failure>
    </testcase>
  </testsuite>
</testsuites>
XML
}
