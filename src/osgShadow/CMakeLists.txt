
IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGSHADOW_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()


################################################################################
## Quieten warnings that a due to optional code paths

IF(CMAKE_COMPILER_IS_GNUCXX)
    # Remove -Wextra flag as it barfs on ffmoeg headers
   STRING(REGEX REPLACE "-Wextra" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF()

SET(LIB_NAME osgShadow)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/ShadowMap
    ${HEADER_PATH}/ShadowTechnique
    ${HEADER_PATH}/ShadowTexture
    ${HEADER_PATH}/ShadowedScene
    ${HEADER_PATH}/ShadowSettings
    ${HEADER_PATH}/SoftShadowMap
    ${HEADER_PATH}/ParallelSplitShadowMap
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/ConvexPolyhedron
    ${HEADER_PATH}/DebugShadowMap
    ${HEADER_PATH}/LightSpacePerspectiveShadowMap
    ${HEADER_PATH}/MinimalCullBoundsShadowMap
    ${HEADER_PATH}/MinimalDrawBoundsShadowMap
    ${HEADER_PATH}/MinimalShadowMap
    ${HEADER_PATH}/ProjectionShadowMap
    ${HEADER_PATH}/StandardShadowMap
    ${HEADER_PATH}/ViewDependentShadowTechnique
    ${HEADER_PATH}/ViewDependentShadowMap
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    ShadowMap.cpp
    ShadowTechnique.cpp
    ShadowTexture.cpp
    ShadowedScene.cpp
    ShadowSettings.cpp
    SoftShadowMap.cpp
    ParallelSplitShadowMap.cpp
    Version.cpp
    ConvexPolyhedron.cpp
    DebugShadowMap.cpp
    LightSpacePerspectiveShadowMap.cpp
    MinimalCullBoundsShadowMap.cpp
    MinimalDrawBoundsShadowMap.cpp
    MinimalShadowMap.cpp
    StandardShadowMap.cpp
    ViewDependentShadowTechnique.cpp
    ViewDependentShadowMap.cpp
    ${OSGILIATH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgDB
    osgUtil
    osg
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "Shadow techniques library for Openscenegraph")

SETUP_LIBRARY(${LIB_NAME})

