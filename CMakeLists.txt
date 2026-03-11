#
# OSGiliath CMake build file
#
cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(OSGiliath VERSION 3.7.0)

set(OSGILIATH_SOVERSION 202)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ── Toolchain modules ────────────────────────────────────────
include(Cache)
include(Warnings)
include(Sanitizers)
include(Coverage)
include(Format)
include(Tidy)
include(IWYU)
include(Profile)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ── Dependencies ─────────────────────────────────────────────
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(X11 REQUIRED)
find_library(MATH_LIBRARY m REQUIRED)
find_library(DL_LIBRARY dl)
if(NOT DL_LIBRARY)
    set(DL_LIBRARY "")
endif()
find_library(RT_LIBRARY rt)

find_package(ZLIB)
find_package(CURL)
find_package(FFmpeg)
find_package(SDL2)
find_package(SDL)
find_package(GIFLIB)
find_package(JPEG)
find_package(PNG)
find_package(TIFF)

option(OSG_USE_LOCAL_LUA_SOURCE "Use local Lua source for the lua plugin" ON)
if(NOT OSG_USE_LOCAL_LUA_SOURCE)
    find_package(Lua52)
    if(NOT LUA_LIBRARIES OR NOT LUA_INCLUDE_DIR)
        find_package(Lua51)
    endif()
endif()

add_subdirectory(thirdparty)

# ── Options ──────────────────────────────────────────────────
include(OsgOptions)

# ── Generated headers ────────────────────────────────────────
include(OsgConfigHeaders)

# ── Output directories ───────────────────────────────────────
include(OsgOutputDirs)

# ── Build macros ─────────────────────────────────────────────
include(OsgMacroUtils)

# ── Build targets ────────────────────────────────────────────
option(BUILD_OSG_APPLICATIONS "Build OSG applications (osgviewer, etc.)" ON)
option(BUILD_OSG_EXAMPLES "Build OSG examples" OFF)
option(BUILD_OSG_PLUGINS "Build OSG plugins" ON)
mark_as_advanced(BUILD_OSG_PLUGINS)

add_subdirectory(src)

if(BUILD_OSG_APPLICATIONS)
    add_subdirectory(applications)
endif()

if(BUILD_OSG_EXAMPLES)
    add_subdirectory(examples)
endif()

# ── Uninstall target ─────────────────────────────────────────
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    @ONLY
)
add_custom_target(uninstall
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
)

include(FeatureSummary)
feature_summary(WHAT ALL)
