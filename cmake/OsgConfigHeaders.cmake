# ── OSG Generated Headers ────────────────────────────────────
# Configure headers from .in templates.
# ─────────────────────────────────────────────────────────────

# Version variables expected by Config.in / Version.in
set(OSGILIATH_MAJOR_VERSION ${PROJECT_VERSION_MAJOR})
set(OSGILIATH_MINOR_VERSION ${PROJECT_VERSION_MINOR})
set(OSGILIATH_PATCH_VERSION ${PROJECT_VERSION_PATCH})
set(OSGILIATH_VERSION       ${PROJECT_VERSION})

set(OSGILIATH_CONFIG_HEADER "${PROJECT_BINARY_DIR}/include/osg/Config")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/osg/Config.in"
    "${OSGILIATH_CONFIG_HEADER}"
)

set(OSGILIATH_OPENGL_HEADER "${PROJECT_BINARY_DIR}/include/osg/GL")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/osg/GL.in"
    "${OSGILIATH_OPENGL_HEADER}"
)

set(OSGILIATH_VERSION_HEADER "${PROJECT_BINARY_DIR}/include/osg/Version")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/osg/Version.in"
    "${OSGILIATH_VERSION_HEADER}"
)

# Make generated headers visible
include_directories("${PROJECT_BINARY_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories(SYSTEM "${OPENGL_INCLUDE_DIR}")
