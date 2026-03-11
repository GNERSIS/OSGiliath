# ── OSG Options ──────────────────────────────────────────────
# Build-time options that feed into Config.in / generated headers.
# ─────────────────────────────────────────────────────────────

set(OSG_WINDOWING_SYSTEM "X11" CACHE STRING "Windowing system type")
set(OSG_PLUGINS "osgPlugins-${PROJECT_VERSION}" CACHE STRING "Plugin directory name" FORCE)
set(OSG_PLUGIN_PREFIX "")

set(OSGILIATH_SONAMES TRUE)
set(OSGILIATH_OPENTHREADS_SONAMES TRUE)

option(OSG_NOTIFY_DISABLED       "Disable osg::notify()"                     OFF)
option(OSG_USE_DEPRECATED_API    "Enable deprecated API"                     ON)
option(OSG_PROVIDE_READFILE      "Provide osgDB::read*File() methods"        ON)
option(OSG_USE_REF_PTR_IMPLICIT_OUTPUT_CONVERSION "ref_ptr<> T* conversion"  ON)
option(OSG_USE_REF_PTR_SAFE_DEREFERENCE           "Throw on null deref"      OFF)
option(OSG_ENVVAR_SUPPORTED      "Allow getenv() usage"                      ON)
option(OSG_TEXT_USE_FONTCONFIG   "Use FontConfig in osgText"                 ON)

option(OSG_USE_FLOAT_MATRIX         "float Matrix"          OFF)
option(OSG_USE_FLOAT_PLANE          "float Plane"           OFF)
option(OSG_USE_FLOAT_BOUNDINGSPHERE "float BoundingSphere"  ON)
option(OSG_USE_FLOAT_BOUNDINGBOX    "float BoundingBox"     ON)
option(OSG_USE_FLOAT_QUAT           "float Quat"            OFF)
mark_as_advanced(
    OSG_USE_FLOAT_MATRIX
    OSG_USE_FLOAT_PLANE
    OSG_USE_FLOAT_BOUNDINGSPHERE
    OSG_USE_FLOAT_BOUNDINGBOX
    OSG_USE_FLOAT_QUAT
)

# OpenGL 4.6 Core Profile — the only supported profile
set(OSG_GL_LIBRARY_STATIC OFF)
set(OSG_CPP_EXCEPTIONS_AVAILABLE ON)
set(OSG_GL_CONTEXT_VERSION "4.6" CACHE STRING "GL context version")
set(OPENGL_HEADER1 "#include <GL/glew.h>" CACHE STRING "OpenGL header" FORCE)
set(OPENGL_HEADER2 "" CACHE STRING "Additional OpenGL header" FORCE)

option(DYNAMIC_OSGILIATH "Build shared libraries" ON)
if(DYNAMIC_OSGILIATH)
    set(OSGILIATH_LINK_TYPE "SHARED")
else()
    set(OSGILIATH_LINK_TYPE "STATIC")
endif()
