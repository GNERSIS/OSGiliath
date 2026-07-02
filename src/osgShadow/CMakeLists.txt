
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
    ${HEADER_PATH}/Export.hpp
    ${HEADER_PATH}/ShadowMap.hpp
    ${HEADER_PATH}/ShadowTechnique.hpp
    ${HEADER_PATH}/ShadowTexture.hpp
    ${HEADER_PATH}/ShadowedScene.hpp
    ${HEADER_PATH}/ShadowSettings.hpp
    ${HEADER_PATH}/SoftShadowMap.hpp
    ${HEADER_PATH}/ParallelSplitShadowMap.hpp
    ${HEADER_PATH}/Version.hpp
    ${HEADER_PATH}/ConvexPolyhedron.hpp
    ${HEADER_PATH}/DebugShadowMap.hpp
    ${HEADER_PATH}/LightSpacePerspectiveShadowMap.hpp
    ${HEADER_PATH}/MinimalCullBoundsShadowMap.hpp
    ${HEADER_PATH}/MinimalDrawBoundsShadowMap.hpp
    ${HEADER_PATH}/MinimalShadowMap.hpp
    ${HEADER_PATH}/ProjectionShadowMap.hpp
    ${HEADER_PATH}/StandardShadowMap.hpp
    ${HEADER_PATH}/ViewDependentShadowTechnique.hpp
    ${HEADER_PATH}/ViewDependentShadowMap.hpp
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

