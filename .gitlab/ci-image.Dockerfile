# OSGiliath CI image — Ubuntu 24.04 + Clang 23 (apt.llvm.org snapshot) +
# OpenGL/X11/EGL toolchain + headless software-GL runtime (Xvfb +
# Mesa llvmpipe) + MSan-instrumented libc++ (/opt/llvm-msan) +
# include-what-you-use (built from source).
#
# Adapted from l0's .gitlab/ci-image.Dockerfile (same clang-23 snapshot
# toolchain, MSan-libc++ build, and IWYU phases). l0's BoringSSL phase is
# dropped (OSGiliath does not link BoringSSL). The MSan-libc++ phase is
# present because this pipeline now adds an MSan sanitizer stage that runs
# the non-GL test subset against an instrumented libc++; the headless
# software-GL runtime backs the Xvfb+llvmpipe example-render smoke.
#
# Build locally (the runner uses pull_policy = if-not-present, so a
# local image never needs a registry push):
#
#   docker build -f .gitlab/ci-image.Dockerfile \
#                -t registry.gitlab.com/a1z3n/osgiliath/ci:clang-23 .
#
# This image is NEVER built by the pipeline itself.

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# ── Phase 1: bootstrap apt + add LLVM snapshot repo ─────────────────
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates curl gnupg lsb-release \
 && curl -sSL https://apt.llvm.org/llvm-snapshot.gpg.key \
        | gpg --dearmor -o /etc/apt/trusted.gpg.d/llvm.gpg \
 && echo "deb http://apt.llvm.org/noble/ llvm-toolchain-noble main" \
        > /etc/apt/sources.list.d/llvm.list \
 && rm -rf /var/lib/apt/lists/*

# ── Phase 2: Clang 23 toolchain + OSGiliath build deps ──────────────
# Required by lib/CMakeLists.txt: Threads, OpenGL, GLEW, X11, EGL, libm.
# Optional (gate plugins): zlib, curl, gif, jpeg, png, tiff, freetype,
# fontconfig. SDL/FFmpeg deliberately absent (ffmpeg plugin needs the
# pre-5.0 API; SDL examples are not CI targets).
#
# Headless software-GL RUNTIME (xvfb, libgl1-mesa-dri, libegl-mesa0,
# mesa-utils): the *-dev packages above only ship headers + the loader.
# The example-render smoke runs `headlessCapture()` which takes the
# X11/GLX path (Xvfb) backed by Mesa's swrast/llvmpipe DRI driver. Without
# the runtime DRI driver there is no software GL and the smoke can't
# render. `mesa-utils` provides glxinfo for the Phase-A GL-version spike.
# `imagemagick` provides `identify`, which lib/smoke.sh uses for the
# blank-frame check (`identify -format '%[standard-deviation]'` ⇒ a
# zero-std-deviation render is flat/blank). Without it the smoke degrades
# to a size + PNG-signature check only (still hard, just coarser).
RUN apt-get update && apt-get install -y --no-install-recommends \
        clang-23 lld-23 llvm-23 \
        clang-format-23 clang-tidy-23 clang-tools-23 \
        libclang-rt-23-dev \
        cmake ninja-build ccache pkg-config \
        libgl1-mesa-dev libglew-dev libegl1-mesa-dev \
        xvfb libgl1-mesa-dri libegl-mesa0 mesa-utils imagemagick \
        libx11-dev libxext-dev libxrandr-dev libxinerama-dev \
        zlib1g-dev libcurl4-openssl-dev \
        libgif-dev libjpeg-turbo8-dev libpng-dev libtiff-dev \
        libfreetype-dev libfontconfig-dev \
        nlohmann-json3-dev \
        jq git bash python3 build-essential \
 && update-alternatives --install /usr/bin/clang        clang        /usr/bin/clang-23        100 \
 && update-alternatives --install /usr/bin/clang++      clang++      /usr/bin/clang++-23      100 \
 && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-23 100 \
 && update-alternatives --install /usr/bin/clang-tidy   clang-tidy   /usr/bin/clang-tidy-23   100 \
 && update-alternatives --install /usr/bin/lld          lld          /usr/bin/lld-23          100 \
 && update-alternatives --install /usr/bin/ld.lld       ld.lld       /usr/bin/ld.lld-23       100 \
 && rm -rf /var/lib/apt/lists/*

# ── Phase 3: build MSan-instrumented libc++ at /opt/llvm-msan ───────
# apt.llvm.org doesn't ship an MSan-instrumented libc++; we build it
# ourselves. Source is llvm-project main (branch tip) — the snapshot
# clang-23 from apt is also built off main, so the API/ABI mismatch
# is small. When LLVM 23.x.x stable is tagged, switch to that tag for
# full reproducibility. The msan CMake preset links against this prefix
# (-isystem /opt/llvm-msan/include/c++/v1, -L/opt/llvm-msan/lib) so the
# non-GL MSan test runtime is built on instrumented standard-library code.
RUN git clone --depth 1 --branch main \
        https://github.com/llvm/llvm-project.git /tmp/llvm \
 && cmake -G Ninja -S /tmp/llvm/runtimes -B /tmp/llvm/build-msan \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=/usr/bin/clang \
        -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
        -DLLVM_USE_SANITIZER=MemoryWithOrigins \
        -DLLVM_ENABLE_RUNTIMES='libcxx;libcxxabi;libunwind' \
        -DLLVM_INCLUDE_TESTS=OFF \
        -DLIBCXX_INCLUDE_TESTS=OFF \
        -DLIBCXX_INCLUDE_BENCHMARKS=OFF \
        -DLIBCXXABI_INCLUDE_TESTS=OFF \
        -DLIBUNWIND_INCLUDE_TESTS=OFF \
        -DCMAKE_INSTALL_PREFIX=/opt/llvm-msan \
 && cmake --build /tmp/llvm/build-msan -j"$(nproc)" \
        --target install-cxx install-cxxabi install-unwind \
 && rm -rf /tmp/llvm

# ── Phase 4: build include-what-you-use ─────────────────────────────
# Same rationale as l0: the `iwyu` apt package tracks Ubuntu's default
# Clang, not the apt.llvm.org snapshot; build master against clang-23.
# Installed to /usr/local/bin so cmake/IWYU.cmake's find_program
# resolves it (its build-from-source fallback must never fire in CI).
RUN apt-get update && apt-get install -y --no-install-recommends \
        llvm-23-dev libclang-23-dev \
 && git clone --depth 1 --branch master \
        https://github.com/include-what-you-use/include-what-you-use.git /tmp/iwyu \
 && cmake -G Ninja -S /tmp/iwyu -B /tmp/iwyu/build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=/usr/bin/clang \
        -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
        -DCMAKE_PREFIX_PATH=/usr/lib/llvm-23 \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
 && cmake --build /tmp/iwyu/build -j"$(nproc)" \
 && cmake --install /tmp/iwyu/build \
 && rm -rf /tmp/iwyu \
 && rm -rf /var/lib/apt/lists/*

# ── Phase 5: ccache defaults — overridden per-job by .gitlab-ci.yml ─
ENV CCACHE_DIR=/cache/ccache \
    CCACHE_MAXSIZE=10G \
    CCACHE_COMPRESS=true \
    CCACHE_COMPRESSLEVEL=6 \
    CCACHE_COMPILERCHECK=content \
    CC=clang \
    CXX=clang++

WORKDIR /workspace

# ── Phase 6: sanity check — fail the build if any tool is missing ───
RUN clang --version \
 && clang-format --version \
 && clang-tidy --version \
 && ld.lld --version \
 && cmake --version \
 && ninja --version \
 && ccache --version \
 && jq --version \
 && pkg-config --exists egl \
 && test -f /usr/include/GL/glew.h \
 && test -d /opt/llvm-msan/include/c++/v1 \
 && test -f /opt/llvm-msan/lib/libc++.so.1 \
 && test -e /usr/lib/x86_64-linux-gnu/dri/swrast_dri.so \
 && identify --version \
 && include-what-you-use --version

CMD ["bash"]
