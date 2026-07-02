
IF   (DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGUI_LIBRARY)
ELSE (DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OSGILIATH)

SET(LIB_NAME osgUI)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Export.hpp
    ${HEADER_PATH}/ColorPalette.hpp
    ${HEADER_PATH}/Widget.hpp
    ${HEADER_PATH}/Label.hpp
    ${HEADER_PATH}/LineEdit.hpp
    ${HEADER_PATH}/Dialog.hpp
    ${HEADER_PATH}/Popup.hpp
    ${HEADER_PATH}/PushButton.hpp
    ${HEADER_PATH}/ComboBox.hpp
    ${HEADER_PATH}/Callbacks.hpp
    ${HEADER_PATH}/Style.hpp
    ${HEADER_PATH}/AlignmentSettings.hpp
    ${HEADER_PATH}/FrameSettings.hpp
    ${HEADER_PATH}/TabWidget.hpp
    ${HEADER_PATH}/TextSettings.hpp
    ${HEADER_PATH}/Validator.hpp
)

SET(TARGET_SRC
    Widget.cpp
    ColorPalette.cpp
    Label.cpp
    LineEdit.cpp
    Dialog.cpp
    Popup.cpp
    PushButton.cpp
    ComboBox.cpp
    Callbacks.cpp
    Style.cpp
    AlignmentSettings.cpp
    FrameSettings.cpp
    TabWidget.cpp
    TextSettings.cpp
    Validator.cpp
    ${OSGILIATH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgViewer
    osgText
    osgGA
    osgDB
    osgUtil
    osg
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "Library for creating user interfaces in OpenSceneGraph")

SETUP_LIBRARY(${LIB_NAME})

