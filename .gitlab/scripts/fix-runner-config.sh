#!/usr/bin/env bash
# fix-runner-config.sh — one-time repair of /etc/gitlab-runner/config.toml
# for the osgiliath runner registration:
#   1. pull_policy = "if-not-present" on any [runners.docker] section
#      missing one (so the locally-built CI image is used, never pulled)
#   2. add the persistent osgiliath ccache bind-mount to a bare default
#      volumes = ["/cache"] line (the l0 runner's multi-volume line
#      doesn't match the pattern and is left untouched)
# Idempotent; backs up config.toml first.
#
# Run:  sudo ./fix-runner-config.sh

set -euo pipefail

CONFIG=/etc/gitlab-runner/config.toml
CCACHE_DIR=/var/cache/gitlab-runner/osgiliath/ccache

if [[ "$EUID" -ne 0 ]]; then
    exec sudo "$0" "$@"
fi

cp -a "$CONFIG" "${CONFIG}.bak.$(date +%s)"
mkdir -p "$CCACHE_DIR"

python3 - "$CONFIG" <<'EOF'
import sys

path = sys.argv[1]
s = open(path).read()

# 1. pull_policy into [runners.docker] sections that lack one
parts = s.split('[runners.docker]')
out = [parts[0]]
for chunk in parts[1:]:
    head = chunk.split('[[runners]]')[0]
    if 'pull_policy' not in head:
        chunk = '\n    pull_policy = "if-not-present"' + chunk
    out.append(chunk)
s = '[runners.docker]'.join(out)

# 2. ccache volume onto the bare default volumes line
s = s.replace(
    'volumes = ["/cache"]',
    'volumes = ["/cache", "/var/cache/gitlab-runner/osgiliath/ccache:/ccache:rw"]',
)

open(path, 'w').write(s)
print('pull_policy entries:', s.count('pull_policy'))
EOF

systemctl restart gitlab-runner
echo "--- resulting docker sections ---"
grep -E 'pull_policy|volumes' "$CONFIG"
echo "--- runner restarted ---"
systemctl is-active gitlab-runner
