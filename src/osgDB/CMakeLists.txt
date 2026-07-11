
IF   (DYNAMIC_OSGILIATH)

    OPTION(OSG_PLUGIN_SEARCH_INSTALL_DIR_FOR_PLUGINS "Set to ON to have OSGiliath search the configured install directory for plugins." ON)

    ADD_DEFINITIONS(-DOSGDB_LIBRARY)

    IF(OSG_PLUGIN_SEARCH_INSTALL_DIR_FOR_PLUGINS)
        # Add a default plugin search path component
        ADD_DEFINITIONS(-DOSG_DEFAULT_LIBRARY_PATH=\"${CMAKE_INSTALL_PREFIX}/${OSG_INSTALL_LIBDIR}/${OSG_PLUGINS}\")
    ENDIF()

    # Set the library extension according to what configuration is being built.
    # If the string is empty, don't set the define.
    IF(CMAKE_DEBUG_POSTFIX)
        SET(CMAKE_CXX_FLAGS_DEBUG
            "${CMAKE_CXX_FLAGS_DEBUG} -DOSG_LIBRARY_POSTFIX=${CMAKE_DEBUG_POSTFIX}")
    ENDIF()
    IF(CMAKE_RELEASE_POSTFIX)
        SET(CMAKE_CXX_FLAGS_RELEASE
            "${CMAKE_CXX_FLAGS_RELEASE} -DOSG_LIBRARY_POSTFIX=${CMAKE_RELEASE_POSTFIX}")
    ENDIF()
    IF(CMAKE_RELWITHDEBINFO_POSTFIX)
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO
            "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DOSG_LIBRARY_POSTFIX=${CMAKE_RELWITHDEBINFO_POSTFIX}")
    ENDIF()
    IF(CMAKE_MINSIZEREL_POSTFIX)
        SET(CMAKE_CXX_FLAGS_MINSIZEREL
            "${CMAKE_CXX_FLAGS_MINSIZEREL} -DOSG_LIBRARY_POSTFIX=${CMAKE_MINSIZEREL_POSTFIX}")
    ENDIF()
ELSE ()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

IF(APPLE AND NOT ANDROID)
     # compile FileUtils.cpp as objective-c++
     SET_SOURCE_FILES_PROPERTIES(io/FileUtils.cpp
       PROPERTIES COMPILE_FLAGS "-x objective-c++"
     )
ENDIF()


SET(LIB_NAME osgDB)
SET(HEADER_PATH ${OSGiliath_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    # top-level
    ${HEADER_PATH}/Export.hpp

    # registry/
    ${HEADER_PATH}/registry/Archive.hpp
    ${HEADER_PATH}/registry/Callbacks.hpp
    ${HEADER_PATH}/registry/DynamicLibrary.hpp
    ${HEADER_PATH}/registry/ImageProcessor.hpp
    ${HEADER_PATH}/registry/Options.hpp
    ${HEADER_PATH}/registry/PluginQuery.hpp
    ${HEADER_PATH}/registry/ReaderWriter.hpp
    ${HEADER_PATH}/registry/Registry.hpp
    ${HEADER_PATH}/registry/Version.hpp

    # serialization/
    ${HEADER_PATH}/serialization/Archive.hpp
    ${HEADER_PATH}/serialization/BinaryArchive.hpp
    ${HEADER_PATH}/serialization/FlexBufferArchive.hpp
    ${HEADER_PATH}/serialization/ClassInterface.hpp
    ${HEADER_PATH}/serialization/DataTypes.hpp
    ${HEADER_PATH}/serialization/DotOsgWrapper.hpp
    ${HEADER_PATH}/serialization/Input.hpp
    ${HEADER_PATH}/serialization/InputStream.hpp
    ${HEADER_PATH}/serialization/ObjectSerializer.hpp
    ${HEADER_PATH}/serialization/ObjectWrapper.hpp
    ${HEADER_PATH}/serialization/Output.hpp
    ${HEADER_PATH}/serialization/OutputStream.hpp
    ${HEADER_PATH}/serialization/ParameterOutput.hpp
    ${HEADER_PATH}/serialization/SceneCook.hpp
    ${HEADER_PATH}/serialization/Serializer.hpp
    ${HEADER_PATH}/serialization/StreamOperator.hpp
    ${HEADER_PATH}/serialization/TextureCompression.hpp

    # io/
    ${HEADER_PATH}/io/ConvertBase64.hpp
    ${HEADER_PATH}/io/ConvertUTF.hpp
    ${HEADER_PATH}/io/ExternalFileWriter.hpp
    ${HEADER_PATH}/io/FileCache.hpp
    ${HEADER_PATH}/io/FileNameUtils.hpp
    ${HEADER_PATH}/io/FileUtils.hpp
    ${HEADER_PATH}/io/fstream.hpp
    ${HEADER_PATH}/io/ImageOptions.hpp
    ${HEADER_PATH}/io/ReadFile.hpp
    ${HEADER_PATH}/io/WriteFile.hpp
    ${HEADER_PATH}/io/XmlParser.hpp

    # cache/
    ${HEADER_PATH}/cache/AuthenticationMap.hpp
    ${HEADER_PATH}/cache/DatabasePager.hpp
    ${HEADER_PATH}/cache/DatabaseRevisions.hpp
    ${HEADER_PATH}/cache/ImagePager.hpp
    ${HEADER_PATH}/cache/ObjectCache.hpp
    ${HEADER_PATH}/cache/SharedStateManager.hpp
)

SET(TARGET_SRC
    # registry/
    registry/Archive.cpp
    registry/Callbacks.cpp
    registry/DynamicLibrary.cpp
    registry/MimeTypes.cpp
    registry/Options.cpp
    registry/PluginQuery.cpp
    registry/ReaderWriter.cpp
    registry/Registry.cpp
    registry/Version.cpp

    # serialization/
    serialization/BinaryArchive.cpp
    serialization/ClassInterface.cpp
    serialization/Compressors.cpp
    serialization/DotOsgWrapper.cpp
    serialization/Field.cpp
    serialization/FieldReader.cpp
    serialization/FieldReaderIterator.cpp
    serialization/FlexBufferArchive.cpp
    serialization/Input.cpp
    serialization/InputStream.cpp
    serialization/ObjectSerializer.cpp
    serialization/ObjectWrapper.cpp
    serialization/Output.cpp
    serialization/OutputStream.cpp
    serialization/SceneCook.cpp
    serialization/StreamOperator.cpp
    serialization/TextureCompression.cpp
    serialization/serializers/CoreSerializers.cpp
    serialization/serializers/GeometrySerializers.cpp
    serialization/serializers/StateSerializers.cpp
    serialization/serializers/TextureSerializers.cpp

    # io/
    io/ConvertBase64.cpp
    io/ConvertUTF.cpp
    io/ExternalFileWriter.cpp
    io/FileCache.cpp
    io/FileNameUtils.cpp
    io/FileUtils.cpp
    io/fstream.cpp
    io/ImageOptions.cpp
    io/ReadFile.cpp
    io/WriteFile.cpp
    io/XmlParser.cpp

    # cache/
    cache/AuthenticationMap.cpp
    cache/DatabasePager.cpp
    cache/DatabaseRevisions.cpp
    cache/ImagePager.cpp
    cache/ObjectCache.cpp
    cache/SharedStateManager.cpp

    ${OSGILIATH_VERSIONINFO_RC}
)

IF(APPLE AND NOT ANDROID)
    IF(NOT OSG_BUILD_PLATFORM_IPHONE)
        IF(OSG_OSX_VERSION VERSION_GREATER_EQUAL 10.14)
            # Needs CoreFoundation
            SET(OSGDB_PLATFORM_SPECIFIC_LIBRARIES ${COCOA_LIBRARY})
        ELSE()
            # Needs CoreFoundation calls and a Carbon function
            SET(OSGDB_PLATFORM_SPECIFIC_LIBRARIES ${CARBON_LIBRARY} ${COCOA_LIBRARY})
        ENDIF()
    ENDIF()

    IF(OSG_DEFAULT_IMAGE_PLUGIN_FOR_OSX STREQUAL "quicktime")
        ADD_DEFINITIONS(-DDARWIN_QUICKTIME)
    ELSE()
        ADD_DEFINITIONS(-DDARWIN_IMAGEIO)
    ENDIF()
ENDIF()
IF(QTKIT_FOUND)
    ADD_DEFINITIONS(-DUSE_QTKIT)
ENDIF()
IF(QUICKTIME_FOUND)
    ADD_DEFINITIONS(-DUSE_QUICKTIME)
ENDIF()
IF(AV_FOUNDATION_FOUND)
    ADD_DEFINITIONS(-DUSE_AV_FOUNDATION)
ENDIF()

IF(FFMPEG_FOUND)
    ADD_DEFINITIONS(-DUSE_FFMPEG)
ENDIF()

IF(INVENTOR_FOUND)
    ADD_DEFINITIONS(-DUSE_INVENTOR)
ENDIF()

IF(OPENVRML_FOUND)
    ADD_DEFINITIONS(-DUSE_VRML)
ENDIF()

IF( ZLIB_FOUND )
    ADD_DEFINITIONS( -DUSE_ZLIB )
    INCLUDE_DIRECTORIES( ${ZLIB_INCLUDE_DIR} )
    SET(COMPRESSION_LIBRARIES ZLIB_LIBRARIES)
ENDIF()

################################################################################
## Quieten warnings that a due to optional code paths

IF(CMAKE_COMPILER_IS_GNUCXX)
    # Remove -Wextra flag as it barfs on ffmoeg headers
   STRING(REGEX REPLACE "-Wextra" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
ENDIF()

ADD_DEFINITIONS(-DOSG_PLUGIN_EXTENSION=${CMAKE_SHARED_MODULE_SUFFIX})

SET(TARGET_LIBRARIES
    osg
    osgUtil
)
SET(TARGET_EXTERNAL_LIBRARIES ${OSGDB_PLATFORM_SPECIFIC_LIBRARIES} ${DL_LIBRARY})
SET(TARGET_LIBRARIES_VARS  ${COMPRESSION_LIBRARIES})

SET(COMPONENT_PKGCONFIG_DESCRIPTION "File format reading library for Openscenegraph")

SETUP_LIBRARY(${LIB_NAME})

target_include_directories(osgDB SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/thirdparty/flatbuffers/include
    ${CMAKE_SOURCE_DIR}/thirdparty/stb
)
target_compile_definitions(osgDB PRIVATE FLATBUFFERS_LOCALE_INDEPENDENT=0)
