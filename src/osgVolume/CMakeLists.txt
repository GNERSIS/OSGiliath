
IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGVOLUME_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgVolume)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Export.hpp
    ${HEADER_PATH}/Layer.hpp
    ${HEADER_PATH}/Locator.hpp
    ${HEADER_PATH}/MultipassTechnique.hpp
    ${HEADER_PATH}/Property.hpp
    ${HEADER_PATH}/RayTracedTechnique.hpp
    ${HEADER_PATH}/Version.hpp
    ${HEADER_PATH}/Volume.hpp
    ${HEADER_PATH}/VolumeScene.hpp
    ${HEADER_PATH}/VolumeSettings.hpp
    ${HEADER_PATH}/VolumeTechnique.hpp
    ${HEADER_PATH}/VolumeTile.hpp
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    Layer.cpp
    Locator.cpp
    MultipassTechnique.cpp
    Property.cpp
    RayTracedTechnique.cpp
    Version.cpp
    Volume.cpp
    VolumeScene.cpp
    VolumeSettings.cpp
    VolumeTechnique.cpp
    VolumeTile.cpp
    ${OSGILIATH_VERSIONINFO_RC}
)


SET(TARGET_LIBRARIES
    osgGA
    osgDB
    osgUtil
    osg
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "Volume library for Openscenegraph")

SETUP_LIBRARY(${LIB_NAME})


