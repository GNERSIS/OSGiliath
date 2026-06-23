# clang-format gate (`clang-format --dry-run --Werror`).
#
# The file set MIRRORS cmake/Format.cmake's GLOB_RECURSE exactly
# (include/*.hpp, src|examples|applications|tests/*.{cpp,hpp}) so this
# gate can never flag a file the build's `format ALL` target doesn't
# auto-format. Vendored C (src/osgPlugins/lua/lua-5.2.3 — .c/.h) is
# naturally excluded by the extension filter, same as in Format.cmake.

cmd_format() {
    require_tool clang-format
    header "format check (clang-format --dry-run --Werror)"

    local total
    cd "$REPO_ROOT"
    mapfile -t files < <(
        find include -type f -name '*.hpp' 2>/dev/null
        find src examples applications tests \
             -type f \( -name '*.cpp' -o -name '*.hpp' \) \
             -not -path '*/build*/*' \
             2>/dev/null
    )
    total=${#files[@]}

    # Zero discovered files is NOT a pass. In this repo include/src/examples/
    # applications/tests always hold source, so an empty set can only mean a wrong
    # CWD or a moved/renamed tree — a misconfiguration that must fail loud, never
    # a silent always-green gate.
    if (( total == 0 )); then
        fail "no source files discovered under include/src/examples/applications/tests \
— the format gate cannot run (wrong CWD or a moved/renamed tree?)"
    fi

    info "running clang-format on $total files (parallel: $(nproc_value))"

    # Each xargs batch writes to its own temp file; concatenated at the
    # end. clang-format uses C++ iostreams, so concurrent writes to a
    # shared pipe interleave mid-line — per-batch files are the only
    # reliable capture. (Same rationale as l0's format.sh.)
    local log_file="$REPO_ROOT/format.log"
    local tmpdir
    tmpdir="$(mktemp -d "$REPO_ROOT/.format-tmp.XXXXXX")"
    : > "$log_file"

    local xargs_rc=0
    printf '%s\n' "${files[@]}" \
        | xargs -P"$(nproc_value)" -n50 bash -c '
            out=$(mktemp -p "$0")
            clang-format --dry-run --Werror --style=file "$@" > "$out" 2>&1
        ' "$tmpdir" \
        || xargs_rc=$?

    if compgen -G "$tmpdir/*" >/dev/null; then
        cat "$tmpdir"/* >> "$log_file"
    fi
    rm -rf "$tmpdir"

    if (( xargs_rc != 0 )); then
        local flagged
        grep -oE '^[^:]+\.(cpp|hpp)' "$log_file" | sort -u > "$REPO_ROOT/format-files.txt" || true
        flagged=$(wc -l < "$REPO_ROOT/format-files.txt")
        warn "clang-format flagged $flagged file(s):"
        head -20 "$REPO_ROOT/format-files.txt" >&2 || true
        fail "format check failed — full diagnostics in format.log"
    fi

    rm -f "$REPO_ROOT/format-files.txt"
    ok "format check green ($total files clean)"
}
