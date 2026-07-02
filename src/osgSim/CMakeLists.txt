
IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGSIM_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgSim)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/BlinkSequence.hpp
    ${HEADER_PATH}/ColorRange.hpp
    ${HEADER_PATH}/DOFTransform.hpp
    ${HEADER_PATH}/ElevationSlice.hpp
    ${HEADER_PATH}/HeightAboveTerrain.hpp
    ${HEADER_PATH}/Export.hpp
    ${HEADER_PATH}/GeographicLocation.hpp
    ${HEADER_PATH}/Impostor.hpp
    ${HEADER_PATH}/ImpostorSprite.hpp
    ${HEADER_PATH}/InsertImpostorsVisitor.hpp
    ${HEADER_PATH}/LightPoint.hpp
    ${HEADER_PATH}/LightPointNode.hpp
    ${HEADER_PATH}/LightPointSystem.hpp
    ${HEADER_PATH}/LineOfSight.hpp
    ${HEADER_PATH}/MultiSwitch.hpp
    ${HEADER_PATH}/OverlayNode.hpp
    ${HEADER_PATH}/ObjectRecordData.hpp
    ${HEADER_PATH}/ScalarBar.hpp
    ${HEADER_PATH}/ScalarsToColors.hpp
    ${HEADER_PATH}/Sector.hpp
    ${HEADER_PATH}/ShapeAttribute.hpp
    ${HEADER_PATH}/SphereSegment.hpp
    ${HEADER_PATH}/Version.hpp
    ${HEADER_PATH}/VisibilityGroup.hpp
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

