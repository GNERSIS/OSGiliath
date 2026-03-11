IF(WIN32)
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT")
ENDIF()

INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIRS} )

IF(APPLE)
    IF(NOT OSG_BUILD_PLATFORM_IPHONE)
        SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
    ENDIF()
ENDIF()

SET(TARGET_SRC
    FreeTypeFont.cpp
    FreeTypeLibrary.cpp
    ReaderWriterFreeType.cpp
)

SET(TARGET_H
    FreeTypeFont.h
    FreeTypeLibrary.h
)

SET(TARGET_ADDED_LIBRARIES osgText )

SET(TARGET_EXTERNAL_LIBRARIES ${FREETYPE_LIBRARIES} )

# Freetype may be built with PNG support (for color bitmap fonts),
# which requires linking libpng and zlib.
IF(PNG_FOUND)
    LIST(APPEND TARGET_EXTERNAL_LIBRARIES ${PNG_LIBRARIES})
ENDIF()
IF(ZLIB_FOUND)
    LIST(APPEND TARGET_EXTERNAL_LIBRARIES ${ZLIB_LIBRARIES})
ENDIF()

#### end var setup  ###
SETUP_PLUGIN(freetype)
