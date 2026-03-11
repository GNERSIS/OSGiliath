
IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGPRESENTATION_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgPresentation)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/Cursor
    ${HEADER_PATH}/AnimationMaterial
    ${HEADER_PATH}/CompileSlideCallback
    ${HEADER_PATH}/PickEventHandler
    ${HEADER_PATH}/PropertyManager
    ${HEADER_PATH}/KeyEventHandler
    ${HEADER_PATH}/SlideEventHandler
    ${HEADER_PATH}/SlideShowConstructor
    ${HEADER_PATH}/Timeout
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    Cursor.cpp
    AnimationMaterial.cpp
    CompileSlideCallback.cpp
    PickEventHandler.cpp
    PropertyManager.cpp
    KeyEventHandler.cpp
    SlideEventHandler.cpp
    SlideShowConstructor.cpp
    Timeout.cpp
    ${OSGILIATH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgViewer
    osgUI
    osgWidget
    osgManipulator
    osgVolume
    osgFX
    osgText
    osgGA
    osgDB
    osgUtil
    osg
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "A library for creating slide show presentations with OpenSceneGraph")

SETUP_LIBRARY(${LIB_NAME})

# Legacy code — suppress conversion warnings until full migration
target_compile_options(${LIB_NAME} PRIVATE
    -Wno-error=implicit-float-conversion
    -Wno-error=implicit-int-float-conversion
    -Wno-error=sign-conversion
    -Wno-error=shorten-64-to-32
    -Wno-error=overloaded-virtual
    -Wno-error=inconsistent-missing-override
    -Wno-error=unused-variable
)

