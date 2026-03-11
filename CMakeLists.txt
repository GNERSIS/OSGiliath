#
# OpenSceneGraph CMake build file
#
SET(OPENSCENEGRAPH_MAJOR_VERSION 3)
SET(OPENSCENEGRAPH_MINOR_VERSION 7)
SET(OPENSCENEGRAPH_PATCH_VERSION 0)
SET(OPENSCENEGRAPH_SOVERSION 202)


# set to 0 when not a release candidate, non zero means that any generated
# git tags will be treated as release candidates of given number
SET(OPENSCENEGRAPH_RELEASE_CANDIDATE 1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

PROJECT(OpenSceneGraph)

IF(WIN32 OR MINGW)
    SET(OSG_WINDOWING_SYSTEM "Win32" CACHE STRING "Windowing system type for graphics window creation.")
ELSE()
    SET(OSG_WINDOWING_SYSTEM "X11" CACHE STRING "Windowing system type for graphics window creation.")
ENDIF()

SET(OPENSCENEGRAPH_VERSION ${OPENSCENEGRAPH_MAJOR_VERSION}.${OPENSCENEGRAPH_MINOR_VERSION}.${OPENSCENEGRAPH_PATCH_VERSION})

SET(OSG_PLUGINS osgPlugins-${OPENSCENEGRAPH_VERSION})

SET(OSG_PLUGIN_PREFIX "")

IF (CYGWIN)
    SET(OSG_PLUGIN_PREFIX "cygwin_")
ENDIF()

IF(MINGW)
    SET(OSG_PLUGIN_PREFIX "mingw_")
ENDIF()


# We want to build SONAMES shared librariess
SET(OPENSCENEGRAPH_SONAMES TRUE)
SET(OPENTHREADS_SONAMES TRUE)

SET(OpenThreads_SOURCE_DIR ${OpenSceneGraph_SOURCE_DIR})

# We have some custom .cmake scripts not in the official distribution.
# Maybe this can be used override existing behavior if needed?
SET(CMAKE_MODULE_PATH "${OpenSceneGraph_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

# Change the default build type to Release
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)


# Okay, here's the problem: On some platforms, linking against OpenThreads
# is not enough and explicit linking to the underlying thread library
# is also required (e.g. FreeBSD). But OpenThreads may be built with different
# backends (Pthreads, Sproc, Windows) so we don't know what the underlying
# thread library is because some platforms support multiple backends (e.g.
# IRIX supports Sproc and Pthreads). Linking all libraries won't work
# because the libraries may be incompatible.
# So the current solution is to attempt best guess linking and exempt certain
# cases. With IRIX, we're going to hope explicit linking to the underlying
# library is not necessary. We currently don't case for pthreads on Windows
# which might be an issue on things like Cygwin. This may need to be fixed.
FIND_PACKAGE(Threads)
IF(CMAKE_SYSTEM MATCHES IRIX)
    # Erase CMAKE_THREAD_LIBS_INIT and hope it works
    SET(CMAKE_THREAD_LIBS_INIT "" CACHE INTERNAL "")
ENDIF()

OPTION(OSG_MAINTAINER "Enable OpenSceneGraph maintainer build methods, such as making git branches, tags, updating ChangeLog." OFF)
IF (OSG_MAINTAINER)

    SET(OPENSCENEGRAPH_BRANCH OpenSceneGraph-${OPENSCENEGRAPH_MAJOR_VERSION}.${OPENSCENEGRAPH_MINOR_VERSION})

    #
    # Provide target for tagging a release
    #
    SET(GITCOMMAND git)

    IF   (OPENSCENEGRAPH_RELEASE_CANDIDATE EQUAL 0)
        SET(RELEASE_NAME OpenSceneGraph-${OPENSCENEGRAPH_VERSION})
    ELSE()
        SET(RELEASE_NAME OpenSceneGraph-${OPENSCENEGRAPH_VERSION}-rc${OPENSCENEGRAPH_RELEASE_CANDIDATE})
    ENDIF()

    SET(RELEASE_MESSAGE "Release ${RELEASE_NAME}")
    SET(BRANCH_MESSAGE "Branch ${OPENSCENEGRAPH_BRANCH}")


    ADD_CUSTOM_TARGET(tag-test
        COMMAND echo ${GITCOMMAND} tag -a ${RELEASE_NAME} -m ${RELEASE_MESSAGE}
        COMMAND echo ${GITCOMMAND} push origin ${RELEASE_NAME}
    )

    ADD_CUSTOM_TARGET(tag-run
        COMMAND ${GITCOMMAND} tag -a ${RELEASE_NAME} -m ${RELEASE_MESSAGE}
        COMMAND ${GITCOMMAND} push origin ${RELEASE_NAME}
    )

    ADD_CUSTOM_TARGET(branch-test
        COMMAND echo ${GITCOMMAND} branch ${OPENSCENEGRAPH_BRANCH}
        COMMAND echo ${GITCOMMAND} push origin ${OPENSCENEGRAPH_BRANCH}
    )

    ADD_CUSTOM_TARGET(branch-run
        COMMAND ${GITCOMMAND} branch ${OPENSCENEGRAPH_BRANCH}
        COMMAND ${GITCOMMAND} push origin ${OPENSCENEGRAPH_BRANCH}
    )

    #
    # Provide target for generating ChangeLog
    #
    SET(GITLOGFORMAT "%aD%nAuthor : %an%n%s%b%n")
    SET(GENERATELOGS git log --pretty=format:${GITLOGFORMAT})

    ADD_CUSTOM_TARGET(ChangeLog
        COMMAND ${GENERATELOGS} > ChangeLog
    )

ENDIF()

# Find OpenGL
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)


IF(UNIX)
    # Not sure what this will do on Cygwin and Msys
    FIND_PACKAGE(X11)
    # Some Unicies need explicit linkage to the Math library or the build fails.
    FIND_LIBRARY(MATH_LIBRARY m)

    FIND_LIBRARY(DL_LIBRARY dl)
    IF(NOT DL_LIBRARY)
        SET(DL_LIBRARY "") # change from NOTFOUND to empty when passed to linker
    ENDIF()

    IF( CMAKE_SYSTEM MATCHES "Linux" )
        FIND_LIBRARY( RT_LIBRARY rt )
    ENDIF( CMAKE_SYSTEM MATCHES "Linux" )

ENDIF()


# Make the headers visible to everything
IF(NOT ${PROJECT_BINARY_DIR} EQUAL ${PROJECT_SOURCE_DIR})
   INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/include)
ENDIF()

INCLUDE_DIRECTORIES(${OpenSceneGraph_SOURCE_DIR}/include/)

INCLUDE_DIRECTORIES(SYSTEM ${OPENGL_INCLUDE_DIR})

# Common global definitions
#ADD_DEFINITIONS(-D)
# Platform specific definitions


IF(WIN32)
    OPTION(OSG_DETERMINE_WIN_VERSION "Wheter to automatically determine current Windows version" ON)

    IF(OSG_DETERMINE_WIN_VERSION)
        # Check window version
        INCLUDE (OsgDetermineWinVersion)
        get_WIN32_WINNT(WIN_VERSION)
        ADD_DEFINITIONS(-D_WIN32_WINNT=${WIN_VERSION})
    ENDIF(OSG_DETERMINE_WIN_VERSION)

    IF(MSVC)
        # This option is to enable the /MP switch for Visual Studio 2005 and above compilers
        OPTION(WIN32_USE_MP "Set to ON to build OpenSceneGraph with the /MP option (Visual Studio 2005 and above)." OFF)
        MARK_AS_ADVANCED(WIN32_USE_MP)
        IF(WIN32_USE_MP)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        ENDIF(WIN32_USE_MP)

        # turn off various warnings
        # foreach(warning 4244 4251 4267 4275 4290 4786 4305 4996)
        #     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd${warning}")
        # endforeach(warning)

        # This option is to enable the /DYNAMICBASE switch
        # It is used to workaround a bug in Windows 7 when linking in release, which results in corrupt
        # binaries. See this page for details: http://www.wintellect.com/CS/blogs/jrobbins/archive/2009/01/24/the-case-of-the-corrupt-pe-binaries.aspx
        OPTION(WIN32_USE_DYNAMICBASE "Set to ON to build OpenSceneGraph with the /DYNAMICBASE option to work around a bug when linking release executables on Windows 7." OFF)
        MARK_AS_ADVANCED(WIN32_USE_DYNAMICBASE)
        IF(WIN32_USE_DYNAMICBASE)
            SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DYNAMICBASE")
        ENDIF(WIN32_USE_DYNAMICBASE)

        # More MSVC specific compilation flags
        ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)

        OPTION(MSVC_DISABLE_CHECKED_ITERATORS "Set to ON to disable Visual C++ checked iterators. If you do this you must ensure that every other project in your solution and all dependencies are compiled with _SECURE_SCL=0." OFF)
        MARK_AS_ADVANCED(MSVC_DISABLE_CHECKED_ITERATORS)
        IF(MSVC_DISABLE_CHECKED_ITERATORS)
            ADD_DEFINITIONS(-D_ITERATOR_DEBUG_LEVEL=0) # this supercedes _SECURE_SCL and _HAS_ITERATOR_DEBUGGING in VS2010 and forward
            ADD_DEFINITIONS(-D_SECURE_SCL=0)
            ADD_DEFINITIONS(-D_HAS_ITERATOR_DEBUGGING=0)
        ENDIF(MSVC_DISABLE_CHECKED_ITERATORS)

        OPTION(MSVC_USE_DEFAULT_STACK_SIZE "Set to ON to use the default Visual C++ stack size. CMake forces a high stack size by default, which can cause problems for applications with large number of threads." OFF)
        MARK_AS_ADVANCED(MSVC_USE_DEFAULT_STACK_SIZE)
        IF(MSVC_USE_DEFAULT_STACK_SIZE)
            STRING(REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
            STRING(REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
            STRING(REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")
        ENDIF(MSVC_USE_DEFAULT_STACK_SIZE)
        get_filename_component( CMAKE_MAKE_PROGRAM_NAME ${CMAKE_MAKE_PROGRAM} NAME)
        IF (CMAKE_MAKE_PROGRAM_NAME STREQUAL "VCExpress.exe")
            OPTION(MSVC_BUILD_USE_SOLUTION_FOLDERS "Enable project grouping in VS - VCExpress detected, not supported in VCExpress )" OFF)
        ELSE()
            OPTION(MSVC_BUILD_USE_SOLUTION_FOLDERS "Enable project grouping in VS" ON)
        ENDIF()
        SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ${MSVC_BUILD_USE_SOLUTION_FOLDERS})
    ENDIF()

    #needed for net plugin
    SET (OSG_SOCKET_LIBS wsock32)
    # Both Cygwin and Msys need -DNOMINMAX ???
    IF(UNIX)
        ADD_DEFINITIONS(-DNOMINMAX)
    ENDIF()

########################################################################################################
# the following options are MSVC specific,
# the first OSG_MSVC_VERSIONED_DLL activate a custom build-time layout that should allow to run examples and application
# fron bin folder without requiring installation step.
# it also prepend "osg${OPENSCENEGRAPH_SOVERSION}-" to only .dll files, leaving .lib files untouched in lib
# it also use a hack to get rid of Debug and Release folder in MSVC projects
# all the .dll and .pdb are in bin and all the .lib and .exp are in lib
#
# the second option disable incremental linking in debug build , that is enabled by default by CMake
##########################################################################################################

    IF(MSVC)
        OPTION(OSG_MSVC_VERSIONED_DLL "Set to ON to build OpenSceneGraph with versioned dll names" ON)
        MARK_AS_ADVANCED(OSG_MSVC_VERSIONED_DLL)
        OPTION(OSG_MSVC_DEBUG_INCREMENTAL_LINK "Set to OFF to build OpenSceneGraph without incremental linking in debug (release is off by default)" ON)
        MARK_AS_ADVANCED(OSG_MSVC_DEBUG_INCREMENTAL_LINK)
        IF(NOT OSG_MSVC_DEBUG_INCREMENTAL_LINK)
            SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
            SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
            SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
        ENDIF(NOT OSG_MSVC_DEBUG_INCREMENTAL_LINK)
    ENDIF(MSVC)
ENDIF(WIN32)

########################################################################################################
##### these were settings located in SetupCommon.cmake used in Luigi builds.... find out what are useful
########################################################################################################
#luigi#SET(CMAKE_VERBOSE_MAKEFILE TRUE)
#luigi#SET(CMAKE_SKIP_RPATH TRUE)
#luigi#SET(CMAKE_SKIP_RULE_DEPENDENCY TRUE)
#luigi#IF(UNIX)
#luigi#    LIST_CONTAINS(contains "g++" ${CMAKE_CXX_COMPILER_LIST})
#luigi#    IF (contains)
#luigi#        MESSAGE(${MY_MESSAGE_DEFAULT} "${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} setting  CMAKE_CXX_COMPILER to g++")
#luigi#        SET(CMAKE_CXX_COMPILER "g++")
#luigi#        SET(CMAKE_CXX_COMPILER_LOADED 2)
#luigi#        SET(CMAKE_CXX_COMPILER_WORKS 2)
#luigi#    ENDIF (contains)
#luigi#    SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
#luigi#    SET(CMAKE_CXX_FLAGS_DEBUG "-ggdb -gstabs")
#luigi#ENDIF(UNIX)
########################################################################################################

OPTION(OSG_NOTIFY_DISABLED "Set to ON to build OpenSceneGraph with the notify() disabled." OFF)

OPTION(OSG_USE_DEPRECATED_API "Set to ON to build OpenSceneGraph with the OSG_USE_DEPREFATED_API #define enabled to allow access to deprecated APIs ." ON)

OPTION(OSG_USE_FLOAT_MATRIX "Set to ON to build OpenSceneGraph with float Matrix instead of double." OFF)
MARK_AS_ADVANCED(OSG_USE_FLOAT_MATRIX)

OPTION(OSG_USE_FLOAT_PLANE "Set to ON to build OpenSceneGraph with float Plane instead of double." OFF)
MARK_AS_ADVANCED(OSG_USE_FLOAT_PLANE)

OPTION(OSG_USE_FLOAT_BOUNDINGSPHERE "Set to ON to build OpenSceneGraph with float BoundingSphere instead of double." ON)
MARK_AS_ADVANCED(OSG_USE_FLOAT_BOUNDINGSPHERE)

OPTION(OSG_USE_FLOAT_BOUNDINGBOX "Set to ON to build OpenSceneGraph with float BoundingBox instead of double." ON)
MARK_AS_ADVANCED(OSG_USE_FLOAT_BOUNDINGBOX)

OPTION(OSG_USE_FLOAT_QUAT "Set to ON to build OpenSceneGraph with float Quat instead of double." OFF)
MARK_AS_ADVANCED(OSG_USE_FLOAT_QUAT)

IF (WIN32)
    OPTION(OSG_USE_UTF8_FILENAME "Set to ON to use a UTF8 locale for filenames instead of the default locale." OFF)
    MARK_AS_ADVANCED(OSG_USE_UTF8_FILENAME)
ENDIF()

OPTION(OSG_DISABLE_MSVC_WARNINGS "Set to OFF to not disable MSVC warnings generated by OSG headers." ON)
MARK_AS_ADVANCED(OSG_DISABLE_MSVC_WARNINGS)

OPTION(OSG_PROVIDE_READFILE "Set to ON for include/osgDB/ReadFile to provide the osgDB::read*File(() methods. " ON)
OPTION(OSG_USE_REF_PTR_IMPLICIT_OUTPUT_CONVERSION "Set to ON to use the ref_ptr<> T* operator() output conversion. " ON)
OPTION(OSG_USE_REF_PTR_SAFE_DEREFERENCE "Set to ON to throw an exception whenever ref_ptr<> is dereferenced or called. " OFF)

OPTION(OSG_ENVVAR_SUPPORTED "Set to ON to build OpenSceneGraph with the #define OSG_ENVVAR_SUPPORTED, found in include/osg/Config, to enable use of getenv() related functions." ON)



# OpenGL 4.6 Core Profile — the only supported profile
SET(OSG_GL_LIBRARY_STATIC OFF)

# C++ exceptions are available
SET(OSG_CPP_EXCEPTIONS_AVAILABLE ON)

# FontConfig
OPTION(OSG_TEXT_USE_FONTCONFIG "Set to OFF to disable attempt to use FontConfig in osgText." ON)

# GL Context version
SET(OSG_GL_CONTEXT_VERSION "4.6" CACHE STRING "GL Context version string")

# GL header — GLEW handles everything
SET(OPENGL_HEADER1 "#include <GL/glew.h>" CACHE STRING "OpenGL Header" FORCE)
SET(OPENGL_HEADER2 "" CACHE STRING "Additional OpenGL header" FORCE)

# Feature flags for GL.in
SET(OSG_GL3_FEATURES 1)

################################################################################
# Set Config header file

SET(OPENSCENEGRAPH_CONFIG_HEADER "${PROJECT_BINARY_DIR}/include/osg/Config")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/src/osg/Config.in"
               "${OPENSCENEGRAPH_CONFIG_HEADER}")


################################################################################
# Set OpenGL header file

SET(OPENSCENEGRAPH_OPENGL_HEADER "${PROJECT_BINARY_DIR}/include/osg/GL")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/src/osg/GL.in"
               "${OPENSCENEGRAPH_OPENGL_HEADER}")


################################################################################
# Set Version header file

SET(OPENSCENEGRAPH_VERSION_HEADER "${PROJECT_BINARY_DIR}/include/osg/Version")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/src/osg/Version.in"
               "${OPENSCENEGRAPH_VERSION_HEADER}")

################################################################################
# Set Version Info resource file


IF(MSVC)
    SET(OPENSCENEGRAPH_VERSIONINFO_RC "${PROJECT_BINARY_DIR}/PlatformSpecifics/Windows/OpenSceneGraphVersionInfo.rc")
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/PlatformSpecifics/Windows/OpenSceneGraphVersionInfo.rc.in"
                   "${OPENSCENEGRAPH_VERSIONINFO_RC}")
ENDIF()

################################################################################
# Optional build components

# OSG Applications
OPTION(BUILD_OSG_APPLICATIONS "Enable to build OSG Applications (e.g. osgviewer)" ON)

# OSG Examples
OPTION(BUILD_OSG_EXAMPLES "Enable to build OSG Examples" OFF)

# OSG Plugins
OPTION(BUILD_OSG_PLUGINS "Build OSG Plugins - Disable for compile testing examples on a time limit" ON)
mark_as_advanced(BUILD_OSG_PLUGINS)
################################################################################
# 3rd Party Dependency Stuff
OPTION(OSG_FIND_3RD_PARTY_DEPS "Enable to search for Windows dependencies in ./3rdparty" ON)

IF(WIN32 AND OSG_FIND_3RD_PARTY_DEPS)
    INCLUDE(Find3rdPartyDependencies)
ENDIF()

OPTION(OSG_USE_LOCAL_LUA_SOURCE "Enable to use local Lua source when building the lua plugin" ON)

#
# If you want to prevent CMake from picking up on any of the following optional 3rd Party dependencies in CMake 2.8 onwards
# you can use the following style of command line option when invoking Cmake (here illustrating ignoring PythonLibs) :
# cmake -DCMAKE_DISABLE_FIND_PACKAGE_PythonLibs=1 .
#
# Build FreeType from source (thirdparty)
add_subdirectory(thirdparty)
# FIND_PACKAGE(Freetype)  # replaced by thirdparty/CMakeLists.txt
FIND_PACKAGE(ilmbase)
FIND_PACKAGE(Inventor)
FIND_PACKAGE(Jasper)
FIND_PACKAGE(OpenEXR)
FIND_PACKAGE(OpenCascade)
FIND_PACKAGE(COLLADA)
FIND_PACKAGE(FBX)
FIND_PACKAGE(ZLIB)
FIND_PACKAGE(GDAL)
FIND_PACKAGE(GTA)
FIND_PACKAGE(CURL)
FIND_PACKAGE(LibVNCServer)
FIND_PACKAGE(DCMTK)
FIND_PACKAGE(FFmpeg)
FIND_PACKAGE(GStreamer COMPONENTS app pbutils)
FIND_PACKAGE(GLIB COMPONENTS gobject)
FIND_PACKAGE(DirectShow)
FIND_PACKAGE(SDL2)
FIND_PACKAGE(SDL)
FIND_PACKAGE(Poppler-glib)
FIND_PACKAGE(RSVG)
FIND_PACKAGE(GtkGl)
FIND_PACKAGE(DirectInput)
FIND_PACKAGE(NVTT)
IF (NOT WIN32)
    FIND_PACKAGE(Asio)
ENDIF()
FIND_PACKAGE(ZeroConf)

FIND_PACKAGE(LIBLAS)

IF (NOT(OSG_USE_LOCAL_LUA_SOURCE))
    FIND_PACKAGE(Lua52)
    IF (NOT (LUA_LIBRARIES AND LUA_INCLUDE_DIR))
        FIND_PACKAGE(Lua51)
    ENDIF()
ENDIF()

# V8 and Python plugins are tests for linking against these libraries but aren't functionality beyond this.
# FIND_PACKAGE(V8)
# FIND_PACKAGE(PythonLibs)

# Include macro utilities here
INCLUDE(OsgMacroUtils)


#optional example related dependencies
IF(BUILD_OSG_EXAMPLES)

    FIND_PACKAGE(FLTK)
    FIND_PACKAGE(FOX)
    FIND_PACKAGE(wxWidgets COMPONENTS base core gl net)

ENDIF()


# Image readers/writers depend on 3rd party libraries.
FIND_PACKAGE(GIFLIB)
FIND_PACKAGE(JPEG)
FIND_PACKAGE(PNG)
FIND_PACKAGE(TIFF)

################################################################################
# Create bin and lib directories if required

IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/lib ${CMAKE_BINARY_DIR}/lib/${OSG_PLUGINS})
ENDIF()


################################################################################
# Installation stuff

SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "add a postfix, usually empty on windows")
SET(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "add a postfix, usually empty on windows")

# Correct any incorrect case usage in CMAKE_BUILD_TYPE
IF     (CMAKE_BUILD_TYPE MATCHES "release" OR CMAKE_BUILD_TYPE MATCHES "RELEASE")
    SET(CMAKE_BUILD_TYPE "Release")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "minsizerel" OR CMAKE_BUILD_TYPE MATCHES "MINSIZEREL")
    SET(CMAKE_BUILD_TYPE "MinSizeRel")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "relwithdebinfo" OR CMAKE_BUILD_TYPE MATCHES "RELWITHDEBINFO")
    SET(CMAKE_BUILD_TYPE "RelWithDebInfo")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "debug" OR CMAKE_BUILD_TYPE MATCHES "DEBUG")
    SET(CMAKE_BUILD_TYPE "Debug")
ENDIF()

# Set the build postfix extension according to what configuration is being built.
IF (CMAKE_BUILD_TYPE MATCHES "Release")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_RELEASE_POSTFIX}")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_MINSIZEREL_POSTFIX}")
ELSEIF(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_RELWITHDEBINFO_POSTFIX}")
ELSEIF(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
ELSE()
    SET(CMAKE_BUILD_POSTFIX "")
ENDIF()

IF(UNIX AND NOT WIN32)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
ENDIF()

IF(CYGWIN)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
ENDIF()

# Set OSG_INSTALL_LIBDIR to specify the installation directories of object code libraries
IF(DEFINED LIB_POSTFIX)
  # Use LIB_POSTFIX if defined
  SET(OSG_INSTALL_LIBDIR lib${LIB_POSTFIX})
ELSE()
  # Use the GNU standard installation directories
  INCLUDE(GNUInstallDirs)
  SET(OSG_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
ENDIF()

# Set installation variables
SET(INSTALL_INCDIR include)
SET(INSTALL_BINDIR bin)
IF(WIN32)
    SET(INSTALL_LIBDIR bin)
    SET(INSTALL_ARCHIVEDIR lib)
ELSE()
    SET(INSTALL_LIBDIR ${OSG_INSTALL_LIBDIR})
    SET(INSTALL_ARCHIVEDIR ${OSG_INSTALL_LIBDIR})
ENDIF()

# Tell the project that we will be exporting targets
SET(OSG_EXPORT_TARGETS TRUE CACHE BOOL "Install a config-file and export targets")

# Set the package namespace for exporting
SET(PKG_NAMESPACE osg${OPENSCENEGRAPH_MAJOR_VERSION})

IF(WIN32)
    SET(INSTALL_CONFIGDIR cmake)
ELSE()
    SET(INSTALL_CONFIGDIR ${OSG_INSTALL_LIBDIR}/cmake/OpenSceneGraph)
ENDIF()

INCLUDE(CMakePackageConfigHelpers)
CONFIGURE_PACKAGE_CONFIG_FILE(
    ${PROJECT_SOURCE_DIR}/packaging/cmake/OpenSceneGraphConfig.cmake.in
    configure/OpenSceneGraphConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
    PATH_VARS INSTALL_INCDIR
)

WRITE_BASIC_PACKAGE_VERSION_FILE(
    configure/OpenSceneGraphConfigVersion.cmake
    VERSION ${OPENSCENEGRAPH_VERSION}
    COMPATIBILITY SameMajorVersion
)

INSTALL(
    FILES
        ${PROJECT_BINARY_DIR}/configure/OpenSceneGraphConfig.cmake
        ${PROJECT_BINARY_DIR}/configure/OpenSceneGraphConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
    COMPONENT libopenscenegraph-dev
)

SET(INSTALL_PKGCONFIGDIR ${OSG_INSTALL_LIBDIR}/pkgconfig)

# Here we apparently do some funky stuff with making the bin/ and lib/
# folders which is probably needed to work around a very old CMake bug?

#SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin/${CMAKE_SYSTEM_NAME})
SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})
IF(MSVC AND NOT MSVC_IDE)
    MAKE_DIRECTORY(${OUTPUT_BINDIR}/${OSG_PLUGINS})
ENDIF(MSVC AND NOT MSVC_IDE)

#SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib/${CMAKE_SYSTEM_NAME})
SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})
IF(NOT MSVC OR MSVC_IDE)
    MAKE_DIRECTORY(${OUTPUT_LIBDIR}/${OSG_PLUGINS})
ENDIF(NOT MSVC OR MSVC_IDE)

# Set per-configuration directories so binaries aren't generated in /bin/Debug and /bin/Release subdirs with MSVC.
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
IF(WIN32)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
ELSE(WIN32)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
ENDIF(WIN32)

# Set per-configuration output directories
FOREACH(CONF ${CMAKE_CONFIGURATION_TYPES})        # For each configuration (Debug, Release, MinSizeRel... and/or anything the user chooses)
    STRING(TOUPPER "${CONF}" CONF)                # Go uppercase (DEBUG, RELEASE...)
    SET("CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}")
    SET("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_BINDIR}")
    IF(WIN32)
        SET("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_BINDIR}")
    ELSE()
        SET("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF}" "${OUTPUT_LIBDIR}")
    ENDIF()
ENDFOREACH()

#SET(INSTALL_BINDIR OpenSceneGraph/bin)
#SET(INSTALL_INCDIR OpenSceneGraph/include)
#SET(INSTALL_LIBDIR OpenSceneGraph/lib)
#SET(INSTALL_DOCDIR OpenSceneGraph/doc)

################################################################################
# User Options


# Expose CMAKE_INCLUDE_PATH and CMAKE_LIBARY_PATH to the GUI so users
# may set these values without needing to manipulate the environment.
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} CACHE STRING "You may add additional search paths here. Use ; to separate multiple paths.")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} CACHE STRING "You may add additional search paths here. Use ; to separate multiple paths.")
# We are proposing that a new variable called CMAKE_PREFIX_PATH be introduced
# to CMake to compliment CMAKE_INCLUDE_PATH and CMAKE_LIBRARY_PATH.
# A formal feature request has been submited to CMake, Bug #4947.
# It is intended for those users who have common prefixes for their INCLUDE
# and LIBRARY locations. So if users have headers in /usr/local/include
# and libraries in /usr/local/lib, the common prefix is /usr/local.
# It should also cover the case where headers and libraries are
# in the same directory.
# Our proposal expects that FIND_* commands will automatically search for
# CMAKE_PREFIX_PATH right after CMAKE_INCLUDE_PATH or CMAKE_LIBRARY_PATH.
# Obviously, since CMake does not currently support this, we must write
# our Find*.cmake modules to explicitly support this. Otherwise, this variable
# will have no impact.
# This is unofficial so this may be removed or changed at anytime.
SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} CACHE STRING "(EXPERIMENTAL) You may add additional search paths here. Use ; to separate multiple paths.")


# This is for an advanced option to give aggressive warnings
# under different compilers. If yours is not implemented, this option
# will not be made available.
IF(CMAKE_COMPILER_IS_GNUCXX)
    # To be complete, we might also do GNUCC flags,
    # but everything here is C++ code.
    # -Wshadow and -Woverloaded-virtual are also interesting flags, but OSG
    # returns too many hits.
    # FYI, if we do implement GNUCC, then -Wmissing-prototypes in another
    # interesting C-specific flag.
    # Also, there is a bug in gcc 4.0. Under C++, -pedantic will create
    # errors instead of warnings for certain issues, including superfluous
    # semicolons and commas, and the use of long long. -fpermissive seems
    # to be the workaround.
    SET(OSG_AGGRESSIVE_WARNING_FLAGS -Wall -Wparentheses -Wno-long-long -Wno-import -pedantic -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wunused)

    MESSAGE( STATUS "g++ version ${CMAKE_CXX_COMPILER_VERSION} ")
    IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.6)
        SET(OSG_AGGRESSIVE_WARNING_FLAGS ${OSG_AGGRESSIVE_WARNING_FLAGS} -Wmaybe-uninitialized -Wextra)
        IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
            SET(OSG_AGGRESSIVE_WARNING_FLAGS ${OSG_AGGRESSIVE_WARNING_FLAGS} -Wshadow)
            IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
                # -Wmisleading-indentation (in -Wall) generates less warnings when interpreting tab as 4 spaces instead of the default of 8
                # SET(OSG_AGGRESSIVE_WARNING_FLAGS ${OSG_AGGRESSIVE_WARNING_FLAGS} -ftabstop=4)
                SET(OSG_AGGRESSIVE_WARNING_FLAGS ${OSG_AGGRESSIVE_WARNING_FLAGS} -Wno-misleading-indentation)
            ENDIF()
        ENDIF()
    ENDIF()

    # Previous included -Wformat=2 in OSG_AGGRESSIVE_WARNING_FLAGS but had to remove it due to standard library errors

ELSEIF(MSVC)
    #disable specific warning level 4 warnings:
    #C4100 'identifier' : unreferenced formal parameter
    #C4127 Error Message conditional expression is constant
    #C4706 assignment within conditional expression
    #C4589: Constructor of abstract class 'osgGA::CameraManipulator' ignores initializer for virtual base class 'osg::Object'
    SET(OSG_AGGRESSIVE_WARNING_FLAGS /W4 /wd4589 /wd4706 /wd4127 /wd4100)
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    SET(OSG_AGGRESSIVE_WARNING_FLAGS  -Wall -Wparentheses -Wno-long-long -Wno-import -pedantic -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wunused -Wno-overloaded-virtual)
ENDIF()

# This part is for the CMake menu option to toggle the warnings on/off.
# This will only be made available if we set values for OSG_AGGRESSIVE_WARNING_FLAGS.
IF(OSG_AGGRESSIVE_WARNING_FLAGS)

    SET(OSG_AGGRESSIVE_WARNING_FLAGS ${OSG_AGGRESSIVE_WARNING_FLAGS} CACHE STRING "Compiler flags to use when OSG_AGGRESSIVE_WARNINGS is enabled." FORCE)

    SET(DEFAULT_USE_AGGRESSIVE_WARNINGS ON)

    OPTION(OSG_AGGRESSIVE_WARNINGS "Enable to activate aggressive warnings" ${DEFAULT_USE_AGGRESSIVE_WARNINGS})

    IF(OSG_AGGRESSIVE_WARNINGS)
        # Add flags defined by OSG_AGGRESSIVE_WARNING_FLAGS if they aren't already there
        FOREACH(flag ${OSG_AGGRESSIVE_WARNING_FLAGS})
            IF(NOT CMAKE_CXX_FLAGS MATCHES "${flag}")
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
            ENDIF()
        ENDFOREACH()
    ELSE()
        # Remove all flags considered aggressive
        FOREACH(flag ${OSG_AGGRESSIVE_WARNING_FLAGS})
            STRING(REGEX REPLACE "${flag}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        ENDFOREACH()
    ENDIF()
ENDIF()


# Dynamic vs Static Linking
OPTION(DYNAMIC_OPENSCENEGRAPH "Set to ON to build OpenSceneGraph for dynamic linking.  Use OFF for static." ON)
IF   (DYNAMIC_OPENSCENEGRAPH)
    SET(OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC "SHARED")
ELSE ()
    SET(OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC "STATIC")
ENDIF()


# OSG Core
ADD_SUBDIRECTORY(src)

IF(BUILD_OSG_APPLICATIONS)
    ADD_SUBDIRECTORY(applications)
ENDIF()

IF   (BUILD_OSG_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF()


# For Doxygen
find_package(Doxygen QUIET)
OPTION(BUILD_DOCUMENTATION "Build OpenSceneGraph reference documentation using doxygen (use: make doc_openscenegraph doc_openthreads)" OFF)
MARK_AS_ADVANCED(CLEAR BUILD_DOCUMENTATION)
# To build the documention, you will have to enable it
# and then do the equivalent of "make doc_openscenegraph doc_openthreads".
IF(BUILD_DOCUMENTATION)

    OPTION(BUILD_REF_DOCS_SEARCHENGINE "Enable doxygen's search engine (requires that documentation to be installed on a php enabled web server)" OFF)
    IF(BUILD_REF_DOCS_SEARCHENGINE)
        SET(SEARCHENGINE YES)
    ELSE()
        SET(SEARCHENGINE NO)
    ENDIF()

    OPTION(BUILD_REF_DOCS_TAGFILE "Generate a tag file named osg.tag on the documentation web server" OFF)
    IF(BUILD_REF_DOCS_TAGFILE)
        SET(GENERATE_TAGFILE "${OpenSceneGraph_BINARY_DIR}/doc/OpenSceneGraphReferenceDocs/osg.tag")
    ELSE()
        SET(GENERATE_TAGFILE "")
    ENDIF()

    IF(DOT)
        SET(HAVE_DOT YES)
    ELSE()
        SET(HAVE_DOT NO)
    ENDIF()

    # If html help generation was requested. DOCUMENTATION_HTML_HELP is defined by Documentation.cmake
    SET(GENERATE_HTMLHELP "NO")
    IF(DOCUMENTATION_HTML_HELP)
        # on windows Documentation.cmake finds the html help workshop if it exists. On u*ix we might have it with wine but no way to point it out
        IF(NOT WIN32)
            SET(HTML_HELP_COMPILER "" CACHE FILEPATH "Enter location of the HTML help compiler to let doxygen compile html")
            MARK_AS_ADVANCED(HTML_HELP_COMPILER)
        ENDIF()
        # this var sets a proper value in .doxygen files when configuring them below
        SET(GENERATE_HTMLHELP "YES")
    endif()

    # This processes our doxyfile.cmake and substitutes paths to generate
    # a final Doxyfile
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfiles/doxyfile.cmake
        ${PROJECT_BINARY_DIR}/doc/openscenegraph.doxyfile
    )
    # copy the osg logo to documentations target folder
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/PlatformSpecifics/Windows/icons/src/osg32-32.png
        ${PROJECT_BINARY_DIR}/doc/OpenSceneGraphReferenceDocs/osg32-32.png COPYONLY
    )
    #INSTALL(FILES ${PROJECT_BINARY_DIR}/doc/${PROJECT_NAME}ReferenceDocs-${OPENSCENEGRAPH_VERSION}.chm DESTINATION doc OPTIONAL COMPONENT openscenegraph-doc)
    INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/doc/OpenSceneGraphReferenceDocs DESTINATION doc COMPONENT openscenegraph-doc)

    # now set up openthreads documentation generation
    IF(BUILD_REF_DOCS_TAGFILE)
        SET(GENERATE_TAGFILE "${OpenSceneGraph_BINARY_DIR}/doc/OpenThreadsReferenceDocs/ot.tag")
    ENDIF()

    # This processes our openthreads.doxyfile.cmake and generate a final doxyfile
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfiles/openthreads.doxyfile.cmake
        ${PROJECT_BINARY_DIR}/doc/openthreads.doxyfile
    )
    # copy the osg logo to documentations target folder
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/PlatformSpecifics/Windows/icons/src/osg32-32.png
        ${PROJECT_BINARY_DIR}/doc/OpenThreadsReferenceDocs/osg32-32.png COPYONLY
    )
    #INSTALL(FILES ${PROJECT_BINARY_DIR}/doc/${PROJECT_NAME}ReferenceDocs-${OPENSCENEGRAPH_VERSION}.chm DESTINATION doc OPTIONAL COMPONENT openscenegraph-doc)
    INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/doc/OpenThreadsReferenceDocs DESTINATION doc COMPONENT openthreads-doc)

    # Process our other doxyfiles but don't create targets for these
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfiles/all_Doxyfile
        ${PROJECT_BINARY_DIR}/doc/all_Doxyfile)
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfiles/auto_Doxyfile
        ${PROJECT_BINARY_DIR}/doc/auto_Doxyfile)
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfiles/core_Doxyfile
        ${PROJECT_BINARY_DIR}/doc/core_Doxyfile)

    # This creates a new target to build documentation.
    # It runs ${DOXYGEN} which is the full path and executable to
    # Doxygen on your system, set by the FindDoxygen.cmake module
    # (called by FindDocumentation.cmake).
    # It runs the final generated Doxyfile against it.
    # The DOT_PATH is substituted into the Doxyfile.
    ADD_CUSTOM_TARGET(doc_openscenegraph ${DOXYGEN}
        ${PROJECT_BINARY_DIR}/doc/openscenegraph.doxyfile
    )
    SET_TARGET_PROPERTIES(doc_openscenegraph PROPERTIES FOLDER "Documentation")

    ADD_CUSTOM_TARGET(doc_openthreads ${DOXYGEN}
        ${PROJECT_BINARY_DIR}/doc/openthreads.doxyfile
    )
    SET_TARGET_PROPERTIES(doc_openthreads PROPERTIES FOLDER "Documentation")
ENDIF(BUILD_DOCUMENTATION)

OPTION(BUILD_DASHBOARD_REPORTS "Set to ON to activate reporting of OpenSceneGraph builds here http://cdash.openscenegraph.org/index.php?project=OpenSceneGraph" OFF)
IF(BUILD_DASHBOARD_REPORTS)
# The following are required to uses Dart and the Cdash dashboard
# viewable here : http://cdash.openscenegraph.org/index.php?project=OpenSceneGraph
    INCLUDE(Dart)
ENDIF()

# present the packaging option only if we have the cpack command defined (effectively >= 2.6.0)
IF(CMAKE_CPACK_COMMAND)
    OPTION(BUILD_OSG_PACKAGES "Set to ON to generate CPack configuration files and packaging targets" OFF)
    IF(BUILD_OSG_PACKAGES)
      INCLUDE(OsgCPack)
    ENDIF()
ENDIF()


# Run this as late as possible so users can easier spot the message
IF (NOT DEFINED REQUIRES_LIBPATH_MESSAGE AND ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local")
    SET(REQUIRES_LIBPATH_MESSAGE ON)
ENDIF()

IF(REQUIRES_LIBPATH_MESSAGE)
    IF (NOT OSG_LIBPATH_MESSAGE_HAS_BEEN_RUN_BEFORE)
        SET(OSG_LIBPATH_MESSAGE_HAS_BEEN_RUN_BEFORE 1 CACHE INTERNAL "Flag to track whether the libpath message has been reported before")

        MESSAGE("\nThe build system is configured to install libraries to ${CMAKE_INSTALL_PREFIX}/${OSG_INSTALL_LIBDIR}\n"
            "Your applications may not be able to find your installed libraries unless you:\n"
            "    set your LD_LIBRARY_PATH (user specific) or\n"
            "    update your ld.so configuration (system wide)")
        IF(IS_DIRECTORY /etc/ld.so.conf.d)
            MESSAGE("You have an ld.so.conf.d directory on your system, so if you wish to ensure that\n"
                "applications find the installed osg libraries, system wide, you could install an\n"
                "OpenSceneGraph specific ld.so configuration with:\n"
                "    sudo make install_ld_conf\n")
            CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/packaging/ld.so.conf.d/openscenegraph.conf.in
                ${PROJECT_BINARY_DIR}/packaging/ld.so.conf.d/openscenegraph.conf
            )
            ADD_CUSTOM_TARGET(install_ld_conf ${CMAKE_COMMAND} -E copy_if_different
                ${PROJECT_BINARY_DIR}/packaging/ld.so.conf.d/openscenegraph.conf
                /etc/ld.so.conf.d/openscenegraph.conf
                COMMAND ldconfig
                COMMENT "Copying openscenegraph.conf to /etc/ld.so.conf.d and running ldconfig"
            )
        ELSE()
            IF(EXISTS /etc/ld.so.conf)
                MESSAGE("You have an ld.so.conf file in /etc, if you wish to ensure \n"
                "that applications find the installed osg libraries, system wide, you\n"
                "could add ${CMAKE_INSTALL_PREFIX}/${OSG_INSTALL_LIBDIR} to it.")
            ENDIF()
        ENDIF()

        # emit a message during installation.
        INSTALL(CODE "MESSAGE(\"Libraries were installed to ${CMAKE_INSTALL_PREFIX}${OSG_INSTALL_LIBDIR}.\\nYou may need to update your ld.so configuration. \")")

    ENDIF(NOT OSG_LIBPATH_MESSAGE_HAS_BEEN_RUN_BEFORE)

ELSE()
    SET(OSG_LIBPATH_MESSAGE_HAS_BEEN_RUN_BEFORE 0 CACHE INTERNAL "Flag to track whether the libpath message has been reported before")
ENDIF()


# This needs to be run very last so other parts of the scripts can take
# advantage of this.
IF(NOT OSG_CONFIG_HAS_BEEN_RUN_BEFORE)
    SET(OSG_CONFIG_HAS_BEEN_RUN_BEFORE 1 CACHE INTERNAL "Flag to track whether this is the first time running CMake or if CMake has been configured before")
ENDIF()

#-----------------------------------------------------------------------------
# add clobber build target to clear all the non git registered files/directories
#-----------------------------------------------------------------------------
add_custom_target(clobber
    COMMAND git clean -d -f -x
)

#-----------------------------------------------------------------------------
### uninstall target
#-----------------------------------------------------------------------------
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

#
include(FeatureSummary)
feature_summary(WHAT ALL)
