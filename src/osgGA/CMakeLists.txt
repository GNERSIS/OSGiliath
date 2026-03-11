
IF   (DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGGA_LIBRARY)
ELSE (DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OSGILIATH)

SET(LIB_NAME osgGA)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    # Top-level
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Version

    # events/
    ${HEADER_PATH}/events/Device.hpp
    ${HEADER_PATH}/events/Event.hpp
    ${HEADER_PATH}/events/EventHandler.hpp
    ${HEADER_PATH}/events/EventQueue.hpp
    ${HEADER_PATH}/events/EventVisitor.hpp
    ${HEADER_PATH}/events/GUIActionAdapter.hpp
    ${HEADER_PATH}/events/GUIEventAdapter.hpp
    ${HEADER_PATH}/events/GUIEventHandler.hpp

    # manipulators/
    ${HEADER_PATH}/manipulators/AnimationPathManipulator.hpp
    ${HEADER_PATH}/manipulators/CameraManipulator.hpp
    ${HEADER_PATH}/manipulators/CameraViewSwitchManipulator.hpp
    ${HEADER_PATH}/manipulators/DriveManipulator.hpp
    ${HEADER_PATH}/manipulators/FirstPersonManipulator.hpp
    ${HEADER_PATH}/manipulators/FlightManipulator.hpp
    ${HEADER_PATH}/manipulators/KeySwitchMatrixManipulator.hpp
    ${HEADER_PATH}/manipulators/MultiTouchTrackballManipulator.hpp
    ${HEADER_PATH}/manipulators/NodeTrackerManipulator.hpp
    ${HEADER_PATH}/manipulators/OrbitManipulator.hpp
    ${HEADER_PATH}/manipulators/SphericalManipulator.hpp
    ${HEADER_PATH}/manipulators/StandardManipulator.hpp
    ${HEADER_PATH}/manipulators/TerrainManipulator.hpp
    ${HEADER_PATH}/manipulators/TrackballManipulator.hpp
    ${HEADER_PATH}/manipulators/UFOManipulator.hpp

    # handlers/
    ${HEADER_PATH}/handlers/StateSetManipulator.hpp
    ${HEADER_PATH}/handlers/Widget.hpp
)

SET(TARGET_SRC
    # events/
    events/Device.cpp
    events/Event.cpp
    events/EventHandler.cpp
    events/EventQueue.cpp
    events/EventVisitor.cpp
    events/GUIEventAdapter.cpp
    events/GUIEventHandler.cpp

    # manipulators/
    manipulators/AnimationPathManipulator.cpp
    manipulators/CameraManipulator.cpp
    manipulators/CameraViewSwitchManipulator.cpp
    manipulators/DriveManipulator.cpp
    manipulators/FirstPersonManipulator.cpp
    manipulators/FlightManipulator.cpp
    manipulators/KeySwitchMatrixManipulator.cpp
    manipulators/MultiTouchTrackballManipulator.cpp
    manipulators/NodeTrackerManipulator.cpp
    manipulators/OrbitManipulator.cpp
    manipulators/SphericalManipulator.cpp
    manipulators/StandardManipulator.cpp
    manipulators/TerrainManipulator.cpp
    manipulators/TrackballManipulator.cpp
    manipulators/UFOManipulator.cpp

    # handlers/
    handlers/StateSetManipulator.cpp
    handlers/Widget.cpp

    # top-level
    Version.cpp
    ${OSGILIATH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgDB
    osgUtil
    osg
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "GUI event library for Openscenegraph")

SETUP_LIBRARY(${LIB_NAME})
