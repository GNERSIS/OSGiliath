IF(OSG_TEXT_USE_FONTCONFIG)
    find_package(Fontconfig MODULE)
ENDIF()

IF(DYNAMIC_OSGILIATH)
    ADD_DEFINITIONS(-DOSGTEXT_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgText)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/Export.hpp
    ${HEADER_PATH}/Font.hpp
    ${HEADER_PATH}/Font3D.hpp
    ${HEADER_PATH}/FadeText.hpp
    ${HEADER_PATH}/Glyph.hpp
    ${HEADER_PATH}/KerningType.hpp
    ${HEADER_PATH}/String.hpp
    ${HEADER_PATH}/Style.hpp
    ${HEADER_PATH}/TextBase.hpp
    ${HEADER_PATH}/Text.hpp
    ${HEADER_PATH}/Text3D.hpp
    ${HEADER_PATH}/Version.hpp
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    DefaultFont.cpp
    DefaultFont.hpp
    GlyphGeometry.hpp
    GlyphGeometry.cpp
    Font.cpp
    FadeText.cpp
    Glyph.cpp
    String.cpp
    Style.cpp
    TextBase.cpp
    Text.cpp
    Text3D.cpp
    Version.cpp
    ${OSGILIATH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES
    osgDB
    osgUtil
    osg
)

SET(COMPONENT_PKGCONFIG_DESCRIPTION "Text rendering library for OpenSceneGraph")

if(OSG_TEXT_USE_FONTCONFIG AND Fontconfig_FOUND)
    MESSAGE(STATUS "osgText will be linked with FontConfig library")
    LIST(APPEND TARGET_EXTERNAL_LIBRARIES Fontconfig::Fontconfig)
    ADD_DEFINITIONS(-DWITH_FONTCONFIG)
else()
    MESSAGE(STATUS "osgText will not be linked with FontConfig library")
endif()

SETUP_LIBRARY(${LIB_NAME})

