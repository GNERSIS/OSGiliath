
IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGMANIPULATOR_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgManipulator)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/AntiSquish.hpp
    ${HEADER_PATH}/Command.hpp
    ${HEADER_PATH}/CommandManager.hpp
    ${HEADER_PATH}/Constraint.hpp
    ${HEADER_PATH}/Dragger.hpp
    ${HEADER_PATH}/Export.hpp
    ${HEADER_PATH}/Projector.hpp
    ${HEADER_PATH}/RotateCylinderDragger.hpp
    ${HEADER_PATH}/RotateSphereDragger.hpp
    ${HEADER_PATH}/Scale1DDragger.hpp
    ${HEADER_PATH}/Scale2DDragger.hpp
    ${HEADER_PATH}/ScaleAxisDragger.hpp
    ${HEADER_PATH}/Selection.hpp
    ${HEADER_PATH}/TabBoxDragger.hpp
    ${HEADER_PATH}/TabBoxTrackballDragger.hpp
    ${HEADER_PATH}/TabPlaneDragger.hpp
    ${HEADER_PATH}/TabPlaneTrackballDragger.hpp
    ${HEADER_PATH}/TrackballDragger.hpp
    ${HEADER_PATH}/Translate1DDragger.hpp
    ${HEADER_PATH}/Translate2DDragger.hpp
    ${HEADER_PATH}/TranslateAxisDragger.hpp
    ${HEADER_PATH}/TranslatePlaneDragger.hpp
    ${HEADER_PATH}/Version.hpp
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    AntiSquish.cpp
    Command.cpp
    Constraint.cpp
    Dragger.cpp
    Projector.cpp
    RotateCylinderDragger.cpp
    RotateSphereDragger.cpp
    Scale1DDragger.cpp
    Scale2DDragger.cpp
    ScaleAxisDragger.cpp
    TabBoxDragger.cpp
    TabBoxTrackballDragger.cpp
    TabPlaneDragger.cpp
    TabPlaneTrackballDragger.cpp
    TrackballDragger.cpp
    Translate1DDragger.cpp
    Translate2DDragger.cpp
    TranslateAxisDragger.cpp
    TranslatePlaneDragger.cpp
    Version.cpp
    ${OSGILIATH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgViewer
    osgGA
    osgUtil
    osg
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "Manipulator library for Openscenegraph")

SETUP_LIBRARY(${LIB_NAME})
