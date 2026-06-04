# OSGiliath CI image — Ubuntu 24.04 + Clang 23 (apt.llvm.org snapshot) +
# OpenGL/X11/EGL toolchain + include-what-you-use (built from source).
#
# Adapted from l0's .gitlab/ci-image.Dockerfile (same clang-23 snapshot
# toolchain and IWYU phases; l0's MSan-libc++ and BoringSSL phases are
# dropped — this pipeline runs format/build+tidy/iwyu, not MSan).
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
RUN apt-get update && apt-get install -y --no-install-recommends \
        clang-23 lld-23 llvm-23 \
        clang-format-23 clang-tidy-23 clang-tools-23 \
        libclang-rt-23-dev \
        cmake ninja-build ccache pkg-config \
        libgl1-mesa-dev libglew-dev libegl1-mesa-dev \
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

# ── Phase 3: build include-what-you-use ─────────────────────────────
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

# ── Phase 4: ccache defaults — overridden per-job by .gitlab-ci.yml ─
ENV CCACHE_DIR=/cache/ccache \
    CCACHE_MAXSIZE=10G \
    CCACHE_COMPRESS=true \
    CCACHE_COMPRESSLEVEL=6 \
    CCACHE_COMPILERCHECK=content \
    CC=clang \
    CXX=clang++

WORKDIR /workspace

# ── Phase 5: sanity check — fail the build if any tool is missing ───
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
 && include-what-you-use --version

CMD ["bash"]
