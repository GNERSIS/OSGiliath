IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGWIDGET_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

################################################################################
## Quieten warnings that a due to optional code paths

IF(CMAKE_COMPILER_IS_GNUCXX)
    # Remove -Wextra flag as it barfs on ffmoeg headers
   STRING(REGEX REPLACE "-Wextra" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF()

SET(LIB_NAME osgWidget)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Export.hpp
    ${HEADER_PATH}/Box.hpp
    ${HEADER_PATH}/Browser.hpp
    ${HEADER_PATH}/PdfReader.hpp
    ${HEADER_PATH}/VncClient.hpp
    ${HEADER_PATH}/Canvas.hpp
    ${HEADER_PATH}/EventInterface.hpp
    ${HEADER_PATH}/Frame.hpp
    ${HEADER_PATH}/Input.hpp
    ${HEADER_PATH}/Label.hpp
    ${HEADER_PATH}/Lua.hpp
    ${HEADER_PATH}/Python.hpp
    ${HEADER_PATH}/ScriptEngine.hpp
    ${HEADER_PATH}/StyleInterface.hpp
    ${HEADER_PATH}/StyleManager.hpp
    ${HEADER_PATH}/Table.hpp
    ${HEADER_PATH}/Types.hpp
    ${HEADER_PATH}/UIObjectParent.hpp
    ${HEADER_PATH}/Util.hpp
    ${HEADER_PATH}/Version.hpp
    ${HEADER_PATH}/ViewerEventHandlers.hpp
    ${HEADER_PATH}/Widget.hpp
    ${HEADER_PATH}/Window.hpp
    ${HEADER_PATH}/WindowManager.hpp
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    Box.cpp
    Browser.cpp
    PdfReader.cpp
    VncClient.cpp
    Canvas.cpp
    Frame.cpp
    Input.cpp
    Label.cpp
    Lua.cpp
    Python.cpp
    StyleManager.cpp
    Table.cpp
    Util.cpp
    Version.cpp
    ViewerEventHandlers.cpp
    Widget.cpp
    Window.cpp
    WindowManager.cpp
    ${OSGILIATH_VERSIONINFO_RC}
)


SET(TARGET_LIBRARIES
    osgViewer
    osgGA
    osgText
    osgDB
    osg
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "Windowing toolkit library for Openscenegraph")

SETUP_LIBRARY(${LIB_NAME})

# Legacy code — suppress conversion warnings until full migration
target_compile_options(${LIB_NAME} PRIVATE
    -Wno-error=implicit-float-conversion
    -Wno-error=implicit-int-float-conversion
    -Wno-error=sign-conversion
    -Wno-error=shorten-64-to-32
    -Wno-error=float-conversion
    -Wno-error=unused-parameter
)

