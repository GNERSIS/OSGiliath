# smoke.sh — runtime smoke checks for the applications and examples
# components. Two flavours:
#
#   cmd_app_smoke    non-GL, no display: osgversion (pure CPU) + osgconv
#                    .osgt→.osgb (osgDB serializers + plugin registry).
#                    Runs under build / asan / tsan.
#   cmd_render_smoke headless GL: 6 conservative examples rendered under
#                    Xvfb + Mesa llvmpipe, asserting a non-blank PNG.
#                    Runs under build / asan.
#
# FAIL-LOUD CONTRACT (Risk #1 — software GL is UNPROVEN): every check below
# must exit NON-ZERO on missing binary, non-zero process exit, timeout,
# missing PNG, or a blank/invalid PNG. A smoke step must NEVER pass silently.
# Sanitizer kinds run these binaries with halt_on_error=1 so a sanitizer
# report aborts the process (non-zero) and is caught here.

# Locate a built binary by its OUTPUT_NAME (apps/examples set OUTPUT_NAME to
# the bare name, e.g. osgversion / osggeometry; Debug builds add a 'd'
# postfix). Prefer <build_dir>/bin (CMAKE_RUNTIME_OUTPUT_DIRECTORY); fall
# back to a tree-wide search. Echoes the path; returns 1 if not found.
_locate_bin() {
    local build_dir="$1" name="$2" cand found
    for cand in "$build_dir/bin/$name" "$build_dir/bin/${name}d"; do
        if [[ -x "$cand" ]]; then printf '%s' "$cand"; return 0; fi
    done
    found="$(find "$build_dir" -type f \( -name "$name" -o -name "${name}d" \) \
                 -perm -u+x 2>/dev/null | head -1)"
    if [[ -n "$found" ]]; then printf '%s' "$found"; return 0; fi
    return 1
}

# Assert $1 is a real, non-blank PNG. Hard checks: file exists, size ≥ min
# bytes, valid PNG signature. Best-effort blank check via ImageMagick
# `identify` (zero standard-deviation ⇒ flat image) when available — never a
# hard dependency, but it upgrades "file exists" to "actually rendered".
_assert_nonblank_png() {
    local png="$1" min="${2:-256}" sz sig stddev
    if [[ ! -f "$png" ]]; then
        warn "PNG missing: $png"
        return 1
    fi
    sz="$(stat -c%s "$png" 2>/dev/null || wc -c < "$png")"
    if (( sz < min )); then
        warn "PNG too small ($sz bytes < $min): $png — likely a blank/failed render"
        return 1
    fi
    sig="$(head -c 8 "$png" | od -An -tx1 | tr -d ' \n')"
    if [[ "$sig" != "89504e470d0a1a0a" ]]; then
        warn "not a PNG (bad signature '$sig'): $png"
        return 1
    fi
    if command -v identify >/dev/null 2>&1; then
        stddev="$(identify -format '%[standard-deviation]' "$png" 2>/dev/null || true)"
        if [[ -n "$stddev" ]] && awk -v s="$stddev" 'BEGIN { exit !(s + 0 == 0) }'; then
            warn "PNG appears blank (zero std-deviation): $png"
            return 1
        fi
    fi
    return 0
}

# Ensure a valid native .osgt smoke asset exists at tests/assets/. To avoid
# hand-authoring a version-specific ascii .osgt (unverifiable at Phase A —
# osgconv is not yet built), we write a trivially-valid Wavefront .obj seed
# via heredoc and let osgconv's OWN writer produce the .osgt. The asset is
# thus valid-by-construction; the contract's measured step (.osgt→.osgb) runs
# afterward. A vendored tests/assets/smoke_scene.osgt, if present, is used
# as-is (drop-in for the Phase-A spike). Fail-loud if generation fails.
ensure_smoke_asset() {
    local osgconv="$1" kind="$2"
    local asset_dir="$REPO_ROOT/tests/assets"
    local osgt="$asset_dir/smoke_scene.osgt"
    local obj="$asset_dir/smoke_seed.obj"
    mkdir -p "$asset_dir"

    if [[ -s "$osgt" ]]; then
        info "using existing smoke asset: $osgt"
        return 0
    fi

    if [[ ! -s "$obj" ]]; then
        cat > "$obj" <<'OBJ'
# OSGiliath CI app-smoke seed — minimal triangle (Wavefront OBJ).
# osgconv converts this to a native .osgt at smoke time, so the .osgt is
# valid-by-construction (OSG's own writer) rather than a hand-authored,
# version-fragile guess. Replace tests/assets/smoke_scene.osgt with a
# vendored asset to pin a richer scene.
v -1.0 -1.0 0.0
v  1.0 -1.0 0.0
v  0.0  1.0 0.0
vn 0.0 0.0 1.0
f 1//1 2//1 3//1
OBJ
    fi

    local -a senv=()
    _read_kind_env senv "$kind"
    info "generating native .osgt seed: osgconv $obj → $osgt"
    if ! ( cd "$REPO_ROOT" && env "${senv[@]}" "$osgconv" "$obj" "$osgt" ) >&2; then
        fail "could not generate .osgt seed via osgconv (osgdb_obj plugin \
missing, or a real serializer crash) — vendor a known-good \
tests/assets/smoke_scene.osgt (Phase-A spike)"
    fi
    [[ -s "$osgt" ]] || fail "osgconv ran but did not produce $osgt"
}

cmd_app_smoke() {
    local build_dir="$1" kind="$2"
    header "app-smoke  ($kind, non-GL: osgversion + osgconv)"

    local -a senv=()
    _read_kind_env senv "$kind"

    # 1) osgversion — pure CPU, prints a version string to stdout.
    local osgversion ver
    osgversion="$(_locate_bin "$build_dir" osgversion)" \
        || fail "osgversion binary not found under $build_dir (build incomplete?)"
    info "osgversion: $osgversion"
    if ! ver="$( cd "$REPO_ROOT" && env "${senv[@]}" "$osgversion" )"; then
        fail "osgversion exited non-zero under $kind"
    fi
    [[ -n "$ver" ]] || fail "osgversion produced no output (expected a version string)"
    info "osgversion → $ver"

    # 2) osgconv .osgt→.osgb — exercises osgDB serializers + plugin registry
    #    read/write with no GL and no display.
    local osgconv out sz
    osgconv="$(_locate_bin "$build_dir" osgconv)" \
        || fail "osgconv binary not found under $build_dir (build incomplete?)"
    ensure_smoke_asset "$osgconv" "$kind"
    local osgt="$REPO_ROOT/tests/assets/smoke_scene.osgt"
    out="$(mktemp -t osgiliath-smoke.XXXXXX.osgb)"
    info "osgconv $osgt → $out"
    if ! ( cd "$REPO_ROOT" && env "${senv[@]}" "$osgconv" "$osgt" "$out" ) >&2; then
        rm -f "$out"
        fail "osgconv .osgt→.osgb exited non-zero under $kind"
    fi
    sz="$(stat -c%s "$out" 2>/dev/null || wc -c < "$out")"
    if (( sz <= 0 )); then
        rm -f "$out"
        fail "osgconv produced an empty .osgb ($out) — write path failed silently"
    fi
    info "osgconv produced $out ($sz bytes)"
    rm -f "$out"

    ok "app-smoke  ($kind)  green"
}

cmd_render_smoke() {
    local build_dir="$1" kind="$2"
    header "render-smoke  ($kind, Xvfb + llvmpipe, 6 examples)"

    require_tool xvfb-run
    require_tool timeout

    local -a senv=()
    _read_kind_env senv "$kind"

    # 6 conservative GL paths (§2d) — deliberately AVOID geometry/tessellation/
    # compute-shader examples (llvmpipe gaps): VBO draw, primitives+lighting,
    # texture upload/sample, basic GLSL, font atlas + blend, FBO/RTT.
    local -a examples=(osggeometry osgshape osgtexture2D osgsimpleshaders osgtext osgprerender)

    # Render INTO build-<kind>/smoke/ (not /tmp), the exact path the .gitlab-ci.yml
    # artifact glob captures (build/smoke/*.png, build-asan/smoke/*.png). This is
    # the retained proof for the plan's #1 risk (software GL under llvmpipe): the
    # one PNG that distinguishes "rendered" from "all-black framebuffer" now
    # survives as a CI artifact instead of being discarded in /tmp. The non-blank
    # assertion below is unchanged and still fails the job loud.
    local smoke_dir="$build_dir/smoke"
    mkdir -p "$smoke_dir"

    local name bin png failures=0
    for name in "${examples[@]}"; do
        if ! bin="$(_locate_bin "$build_dir" "$name")"; then
            warn "  $name  binary not found under $build_dir"
            failures=$((failures + 1))
            continue
        fi
        png="$smoke_dir/${name}.png"
        rm -f "$png"
        info "render $name → $png"
        if timeout 120 xvfb-run -a -s "-screen 0 1280x1024x24" \
                env LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=llvmpipe "${senv[@]}" \
                "$bin" --headless "$png"; then
            if _assert_nonblank_png "$png"; then
                ok "  $name  render OK ($(stat -c%s "$png" 2>/dev/null || echo '?') bytes)"
            else
                warn "  $name  produced a blank/invalid PNG"
                failures=$((failures + 1))
            fi
        else
            warn "  $name  exited non-zero / timed out"
            failures=$((failures + 1))
        fi
    done

    if (( failures != 0 )); then
        fail "render-smoke: $failures of ${#examples[@]} example(s) failed under $kind"
    fi
    ok "render-smoke  ($kind)  green (${#examples[@]} examples)"
}
