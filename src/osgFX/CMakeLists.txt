
IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGFX_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgFX)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/AnisotropicLighting.hpp
    ${HEADER_PATH}/BumpMapping.hpp
    ${HEADER_PATH}/Cartoon.hpp
    ${HEADER_PATH}/Effect.hpp
    ${HEADER_PATH}/Export.hpp
    ${HEADER_PATH}/MultiTextureControl.hpp
    ${HEADER_PATH}/Outline.hpp
    ${HEADER_PATH}/Registry.hpp
    ${HEADER_PATH}/Scribe.hpp
    ${HEADER_PATH}/SpecularHighlights.hpp
    ${HEADER_PATH}/Technique.hpp
    ${HEADER_PATH}/Validator.hpp
    ${HEADER_PATH}/Version.hpp
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    AnisotropicLighting.cpp
    BumpMapping.cpp
    Cartoon.cpp
    Effect.cpp
    MultiTextureControl.cpp
    Outline.cpp
    Registry.cpp
    Scribe.cpp
    SpecularHighlights.cpp
    Technique.cpp
    Validator.cpp
    Version.cpp
    ${OSGILIATH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgDB
    osgUtil
    osg
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "Special effects library for Openscenegraph")

SETUP_LIBRARY(${LIB_NAME})
