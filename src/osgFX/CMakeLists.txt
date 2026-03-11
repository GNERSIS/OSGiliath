
IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGFX_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgFX)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/AnisotropicLighting
    ${HEADER_PATH}/BumpMapping
    ${HEADER_PATH}/Cartoon
    ${HEADER_PATH}/Effect
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/MultiTextureControl
    ${HEADER_PATH}/Outline
    ${HEADER_PATH}/Registry
    ${HEADER_PATH}/Scribe
    ${HEADER_PATH}/SpecularHighlights
    ${HEADER_PATH}/Technique
    ${HEADER_PATH}/Validator
    ${HEADER_PATH}/Version
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
