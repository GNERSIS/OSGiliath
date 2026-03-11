PROJECT(OSG_APPLICATIONS)

SET(OSGCORE_BUNDLED TRUE)

IF(NOT OSGCORE_BUNDLED)
    FIND_PACKAGE(OSGCORE)
    IF(OSGCORE_FOUND)
        INCLUDE(${OSGCORE_USE_FILE})
    ELSE()
        MESSAGE(ERROR "OSGCORE neeeded but NOT FOUND")
    ENDIF()
    SET(CMAKE_MODULE_PATH  ${PROJECT_SOURCE_DIR}/../../Macros)
ENDIF()
SET(OSGILIATH_APPLICATION_DIR ${PROJECT_SOURCE_DIR})


SET(TARGET_DEFAULT_PREFIX "application_")
SET(TARGET_DEFAULT_LABEL_PREFIX "Applications")

# Legacy application code — suppress conversion warnings until full migration
add_compile_options(
    -Wno-error=implicit-float-conversion
    -Wno-error=implicit-int-float-conversion
    -Wno-error=sign-conversion
    -Wno-error=shorten-64-to-32
    -Wno-error=overloaded-virtual
)
SET(TARGET_COMMON_LIBRARIES 
	osg 
	osgDB 
	osgUtil
	osgGA
	osgViewer 
	osgText 
)

IF(DYNAMIC_OSGILIATH)
    ADD_SUBDIRECTORY(osgviewer)
    ADD_SUBDIRECTORY(osgarchive)
    ADD_SUBDIRECTORY(osgconv)
    ADD_SUBDIRECTORY(osgfilecache)
    ADD_SUBDIRECTORY(osgversion)
    ADD_SUBDIRECTORY(present3D)
ELSE()
    # need to define this on win32 or linker cries about _declspecs
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)

    ADD_SUBDIRECTORY(osgversion)
    ADD_SUBDIRECTORY(present3D)
ENDIF()

