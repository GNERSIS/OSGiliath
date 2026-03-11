
IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGSIM_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgSim)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/BlinkSequence
    ${HEADER_PATH}/ColorRange
    ${HEADER_PATH}/DOFTransform
    ${HEADER_PATH}/ElevationSlice
    ${HEADER_PATH}/HeightAboveTerrain
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/GeographicLocation
    ${HEADER_PATH}/Impostor
    ${HEADER_PATH}/ImpostorSprite
    ${HEADER_PATH}/InsertImpostorsVisitor
    ${HEADER_PATH}/LightPoint
    ${HEADER_PATH}/LightPointNode
    ${HEADER_PATH}/LightPointSystem
    ${HEADER_PATH}/LineOfSight
    ${HEADER_PATH}/MultiSwitch
    ${HEADER_PATH}/OverlayNode
    ${HEADER_PATH}/ObjectRecordData
    ${HEADER_PATH}/ScalarBar
    ${HEADER_PATH}/ScalarsToColors
    ${HEADER_PATH}/Sector
    ${HEADER_PATH}/ShapeAttribute
    ${HEADER_PATH}/SphereSegment
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/VisibilityGroup
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    BlinkSequence.cpp
    ColorRange.cpp
    DOFTransform.cpp
    ElevationSlice.cpp
    HeightAboveTerrain.cpp
    Impostor.cpp
    ImpostorSprite.cpp
    InsertImpostorsVisitor.cpp
    LightPoint.cpp
    LightPointDrawable.cpp
    LightPointDrawable.hpp
    LightPointNode.cpp
    LightPointSpriteDrawable.cpp
    LightPointSpriteDrawable.hpp
    LineOfSight.cpp
    MultiSwitch.cpp
    OverlayNode.cpp
    ScalarBar.cpp
    ScalarsToColors.cpp
    Sector.cpp
    ShapeAttribute.cpp
    SphereSegment.cpp
    Version.cpp
    VisibilityGroup.cpp
    ${OSGILIATH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgText
    osgDB
    osgUtil
    osg
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "Simulator utility library for Openscenegraph")

SETUP_LIBRARY(${LIB_NAME})

# Legacy code — suppress conversion warnings until full migration
target_compile_options(${LIB_NAME} PRIVATE
    -Wno-error=implicit-float-conversion
    -Wno-error=implicit-int-float-conversion
    -Wno-error=sign-conversion
    -Wno-error=shorten-64-to-32
    -Wno-error=overloaded-virtual
    -Wno-error=inconsistent-missing-override
    -Wno-error=deprecated-copy-with-user-provided-copy
    -Wno-error=implicit-int-conversion
)

