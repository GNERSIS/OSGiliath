#!/usr/bin/env bash
# register-runner.sh — register the local machine as a self-hosted GitLab
# runner for the OSGiliath project. Adapted from l0's register-runner.sh.
#
# Run once per machine. Registers an additional [[runners]] entry in
# /etc/gitlab-runner/config.toml with Docker executor, the locally-built
# CI image (pull_policy if-not-present — no registry push needed), and a
# persistent host ccache bind-mount.
#
# Prerequisites:
# - Linux, sudo access, Docker installed
# - gitlab-runner installed (l0's setup already provides it on legion)
# - The CI image built locally:
#     docker build -f .gitlab/ci-image.Dockerfile \
#                  -t registry.gitlab.com/a1z3n/osgiliath/ci:clang-23 .
# - A runner registration token from the GitLab project:
#     https://gitlab.com/a1z3n/osgiliath/-/settings/ci_cd
#       → Runners → "New project runner"
#       → tick "Run untagged jobs"
#       → "Create runner" → copy the glrt-... token
#
# Usage:
#   .gitlab/scripts/register-runner.sh            # interactive — prompts for token
#   GITLAB_RUNNER_TOKEN=glrt-... .gitlab/scripts/register-runner.sh

set -euo pipefail

IMAGE="registry.gitlab.com/a1z3n/osgiliath/ci:clang-23"
CCACHE_HOST_DIR="/var/cache/gitlab-runner/osgiliath/ccache"

if [[ -t 2 ]]; then
    C_RED=$'\033[31m' C_GREEN=$'\033[32m' C_YELLOW=$'\033[33m' C_BLUE=$'\033[34m' C_RESET=$'\033[0m'
else
    C_RED=''; C_GREEN=''; C_YELLOW=''; C_BLUE=''; C_RESET=''
fi
info() { printf '%s[register-runner]%s %s\n' "$C_BLUE"  "$C_RESET" "$*" >&2; }
ok()   { printf '%s[register-runner]%s %s\n' "$C_GREEN" "$C_RESET" "$*" >&2; }
warn() { printf '%s[register-runner]%s %s\n' "$C_YELLOW" "$C_RESET" "$*" >&2; }
fail() { printf '%s[register-runner] FAIL%s %s\n' "$C_RED" "$C_RESET" "$*" >&2; exit 1; }

[[ "$EUID" -ne 0 ]] || fail "do not run as root — sudo is used internally"
command -v docker >/dev/null 2>&1 || fail "docker not installed"
command -v gitlab-runner >/dev/null 2>&1 || fail "gitlab-runner not installed (run l0's register-runner.sh first or apt install gitlab-runner)"

docker image inspect "$IMAGE" >/dev/null 2>&1 \
    || fail "CI image not built locally — run: docker build -f .gitlab/ci-image.Dockerfile -t $IMAGE ."

if [[ -z "${GITLAB_RUNNER_TOKEN:-}" ]]; then
    cat >&2 <<'EOF'
Open: https://gitlab.com/a1z3n/osgiliath/-/settings/ci_cd
  → expand "Runners" → "New project runner"
  → tick "Run untagged jobs" → "Create runner"
  → copy the glrt-... token shown once
EOF
    printf 'Paste runner registration token: '
    read -r -s GITLAB_RUNNER_TOKEN
    printf '\n'
fi
[[ "${GITLAB_RUNNER_TOKEN}" =~ ^glrt- ]] \
    || warn "token doesn't start with 'glrt-' — a runner authentication token is expected"

sudo mkdir -p "$CCACHE_HOST_DIR"

info "registering runner (docker executor, image: $IMAGE)"
sudo gitlab-runner register \
    --non-interactive \
    --url "https://gitlab.com" \
    --token "$GITLAB_RUNNER_TOKEN" \
    --executor "docker" \
    --docker-image "$IMAGE" \
    --docker-pull-policy "if-not-present" \
    --docker-volumes "/cache" \
    --docker-volumes "${CCACHE_HOST_DIR}:/ccache:rw" \
    --description "${HOSTNAME:-$(hostname)} (osgiliath self-hosted)"
# With the glrt- auth-token flow, attributes like --locked/--run-untagged
# are server-side (set in the GitLab UI when creating the runner).

ok "runner registered — restarting service"
sudo systemctl restart gitlab-runner
ok "done. Push to gitlab.com/a1z3n/osgiliath to trigger the pipeline."
