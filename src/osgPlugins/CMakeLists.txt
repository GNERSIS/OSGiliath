#---------------------------------------------------
# OSG CMAKE SUPPORT
# (C) by Michael Wagner, mtw@shared-reality.com 2005
# (C) Eric Wing, Luigi Calori and Robert Osfield 2006-2007
#---------------------------------------------------

PROJECT(OSG_PLUGINS_MASTER)

IF(NOT DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

IF(NOT MSVC)
    SET(LIBRARY_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH}/${OSG_PLUGINS}")
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${OSG_PLUGINS}")
ENDIF()

SET(CMAKE_SHARED_MODULE_PREFIX ${OSG_PLUGIN_PREFIX})

IF(MSVC80 OR MSVC90)
    IF(NOT OSG_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS)
        SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
    ENDIF()
ENDIF()

SET(TARGET_DEFAULT_PREFIX "osgdb_")
SET(TARGET_DEFAULT_LABEL_PREFIX "Plugins")
SET(TARGET_COMMON_LIBRARIES
    osg
    osgDB
    osgUtil
)

# Legacy plugin code — suppress conversion warnings until full migration
add_compile_options(
    -Wno-error=implicit-float-conversion
    -Wno-error=implicit-int-float-conversion
    -Wno-error=implicit-int-conversion
    -Wno-error=sign-conversion
    -Wno-error=shorten-64-to-32
    -Wno-error=overloaded-virtual
    -Wno-error=float-conversion
    -Wno-error=unused-parameter
    -Wno-error=unused-but-set-variable
    -Wno-error=unused-variable
    -Wno-error=deprecated-declarations
    -Wno-error=deprecated-copy-with-user-provided-copy
)

OPTION(BUILD_OSG_PLUGINS_BY_DEFAULT "If set, all available plugins are built by default and each plugin can be disabled from the build by setting BUILD_OSG_PLUGIN_<PLUGINNAME> to 0. If not set, no plugins are built by default and each desired plugin must be explicitly enabled by setting BUILD_OSG_PLUGIN_<PLUGINNAME> to 1." ON)

# Macro used for adding plugins below. By default, all plugins are built
# but can be disabled by setting the CMake Variable BUILD_OSG_PLUGIN_<PLUGINNAME> to 0.
macro(ADD_PLUGIN_DIRECTORY DIRECTORY)
    STRING(TOUPPER "${DIRECTORY}" DIRECTORY_UPPER)
    SET(VARNAME "BUILD_OSG_PLUGIN_${DIRECTORY_UPPER}")
    if (NOT DEFINED ${VARNAME})
        if (BUILD_OSG_PLUGINS_BY_DEFAULT)
            SET(${VARNAME} 1)
        else()
            SET(${VARNAME} 0)
        endif()
    endif()

    if ("${${VARNAME}}")
        ADD_SUBDIRECTORY("${DIRECTORY}")
    endif()
endmacro()

############################################################
#
#  NodeKit/Psudo loader plugins
#
ADD_PLUGIN_DIRECTORY(rot)
ADD_PLUGIN_DIRECTORY(scale)
ADD_PLUGIN_DIRECTORY(trans)
ADD_PLUGIN_DIRECTORY(normals)
ADD_PLUGIN_DIRECTORY(revisions)
ADD_PLUGIN_DIRECTORY(view)
ADD_PLUGIN_DIRECTORY(shadow)
ADD_PLUGIN_DIRECTORY(terrain)

############################################################
#
#  Viewer plugins
#
#ADD_PLUGIN_DIRECTORY(cfg)  # disabled: FlexLexer pragma once conflict

############################################################
#
#  Shader plugins
#
ADD_PLUGIN_DIRECTORY(glsl)

############################################################
#
#  Image plugins
#
ADD_PLUGIN_DIRECTORY(rgb)
ADD_PLUGIN_DIRECTORY(bmp)
ADD_PLUGIN_DIRECTORY(pnm)
ADD_PLUGIN_DIRECTORY(dds)
ADD_PLUGIN_DIRECTORY(tga)
ADD_PLUGIN_DIRECTORY(hdr)
ADD_PLUGIN_DIRECTORY(dot)
ADD_PLUGIN_DIRECTORY(vtf)
ADD_PLUGIN_DIRECTORY(ktx)

IF(JPEG_FOUND)
    ADD_PLUGIN_DIRECTORY(jpeg)
ENDIF()
IF(JASPER_FOUND)
    ADD_PLUGIN_DIRECTORY(jp2)
ENDIF()
IF(OPENEXR_FOUND AND ZLIB_FOUND AND OSG_CPP_EXCEPTIONS_AVAILABLE)
    ADD_PLUGIN_DIRECTORY(exr)
ENDIF()
IF(GIFLIB_FOUND)
    ADD_PLUGIN_DIRECTORY(gif)
ENDIF()
IF(PNG_FOUND AND ZLIB_FOUND)
    ADD_PLUGIN_DIRECTORY(png)
ENDIF()
IF(TIFF_FOUND)
    ADD_PLUGIN_DIRECTORY(tiff)
ENDIF()
IF(GDAL_FOUND)
    ADD_PLUGIN_DIRECTORY(gdal)
    ADD_PLUGIN_DIRECTORY(ogr)
ENDIF()
IF(GTA_FOUND AND OSG_CPP_EXCEPTIONS_AVAILABLE)
    ADD_PLUGIN_DIRECTORY(gta)
ENDIF()


############################################################
#
# 3D Image plugins
#
IF(DCMTK_FOUND AND ZLIB_FOUND)
    ADD_PLUGIN_DIRECTORY(dicom)
ELSE()
    IF(ITK_FOUND)
        ADD_PLUGIN_DIRECTORY(dicom)
    ENDIF()
ENDIF()

############################################################
#
#  3rd party 3d plugins
#

ADD_PLUGIN_DIRECTORY(3dc)

IF(CURL_FOUND)
    ADD_PLUGIN_DIRECTORY(curl)
ENDIF()

IF(ZLIB_FOUND)
    ADD_PLUGIN_DIRECTORY(gz)
ENDIF()

IF(INVENTOR_FOUND)
    ADD_PLUGIN_DIRECTORY(Inventor)
ENDIF()

IF(COLLADA_FOUND AND OSG_CPP_EXCEPTIONS_AVAILABLE)
    ADD_PLUGIN_DIRECTORY(dae)
ENDIF()

IF(FBX_FOUND AND OSG_CPP_EXCEPTIONS_AVAILABLE)
    ADD_PLUGIN_DIRECTORY(fbx)
ENDIF()

IF(OPENCASCADE_FOUND)
    ADD_PLUGIN_DIRECTORY(OpenCASCADE)
ENDIF()

ADD_PLUGIN_DIRECTORY(bvh)
ADD_PLUGIN_DIRECTORY(dxf)
ADD_PLUGIN_DIRECTORY(OpenFlight)

ADD_PLUGIN_DIRECTORY(obj)

ADD_PLUGIN_DIRECTORY(pic)
ADD_PLUGIN_DIRECTORY(stl)

ADD_PLUGIN_DIRECTORY(logo)
ADD_PLUGIN_DIRECTORY(tgz)

ADD_PLUGIN_DIRECTORY(shp)
ADD_PLUGIN_DIRECTORY(txf)

ADD_PLUGIN_DIRECTORY(gltf)

IF(OSG_CPP_EXCEPTIONS_AVAILABLE)
    ADD_PLUGIN_DIRECTORY(ply)
    #ADD_PLUGIN_DIRECTORY(txp)  # disabled: needs header guard restoration
ENDIF()

IF(FFMPEG_FOUND AND OSG_CPP_EXCEPTIONS_AVAILABLE)
    ADD_PLUGIN_DIRECTORY(ffmpeg)
ENDIF()

IF(GSTREAMER_FOUND AND GLIB_FOUND)
    ADD_PLUGIN_DIRECTORY(gstreamer)
ENDIF()

IF(DIRECTSHOW_FOUND)
    ADD_PLUGIN_DIRECTORY(directshow)
ENDIF()

# IF(OPENAL_FOUND)
#    ADD_SUBDIRECTORY(OpenAL)
# ENDIF()

IF(NVTT_FOUND)
    ADD_PLUGIN_DIRECTORY(nvtt)
ENDIF()


IF(FREETYPE_FOUND)
    ADD_PLUGIN_DIRECTORY(freetype)
ENDIF()

IF(ZLIB_FOUND)
    ADD_PLUGIN_DIRECTORY(zip)
ENDIF()

IF(RSVG_FOUND)
    ADD_PLUGIN_DIRECTORY(svg)
ENDIF()

IF(POPPLER_FOUND)
    ADD_PLUGIN_DIRECTORY(pdf)
ENDIF()

IF(LIBVNCSERVER_FOUND)
    ADD_PLUGIN_DIRECTORY(vnc)
ENDIF()

ADD_PLUGIN_DIRECTORY(pvr)

ADD_PLUGIN_DIRECTORY(osc)
ADD_PLUGIN_DIRECTORY(trk)
ADD_PLUGIN_DIRECTORY(tf)

IF(LIBLAS_FOUND)
    ADD_PLUGIN_DIRECTORY(las)
ENDIF()


####################################################
#
# ScriptEngines
#
IF ((LUA_LIBRARIES AND LUA_INCLUDE_DIR) OR (LUA_SOURCE_DIR AND LUA_INCLUDE_DIR) OR (OSG_USE_LOCAL_LUA_SOURCE))
    ADD_PLUGIN_DIRECTORY(lua)
ENDIF()


# V8 and Python plugins are tests for linking against these libraries but aren't functionality beyond this.
# IF (V8_FOUND)
#     ADD_PLUGIN_DIRECTORY(V8)
# ENDIF()
#
# IF (PYTHONLIBS_FOUND)
#     ADD_PLUGIN_DIRECTORY(python)
# ENDIF()

####################################################
#
# Device integration plugins
#
IF   (SDL_FOUND)
    ADD_PLUGIN_DIRECTORY(sdl)
ENDIF(SDL_FOUND)

IF(ASIO_FOUND)
    ADD_PLUGIN_DIRECTORY(RestHttpDevice)
ENDIF(ASIO_FOUND)


IF(ZEROCONF_FOUND)
    ADD_PLUGIN_DIRECTORY(ZeroConfDevice)
ENDIF()

##########to get all the variables of Cmake
#GET_CMAKE_PROPERTY(MYVARS VARIABLES)
#FOREACH(myvar ${MYVARS})
#    FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/AllVariables.txt
#        "${myvar} -->${${myvar}}<-\n"
#    )
#ENDFOREACH(myvar)


