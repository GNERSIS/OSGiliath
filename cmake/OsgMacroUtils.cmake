# ── OSG Build Macros ─────────────────────────────────────────
# Macros used by src/, examples/, applications/ CMakeLists.txt
# to set up libraries, plugins, examples, and applications.
# ─────────────────────────────────────────────────────────────

# ── LINK_WITH_VARIABLES ──────────────────────────────────────
# Link a target against libraries specified by variable names.
# Handles debug/release variants when _DEBUG/_RELEASE suffixes exist.

macro(LINK_WITH_VARIABLES TRGTNAME)
    foreach(varname ${ARGN})
        if(${varname}_DEBUG)
            if(${varname}_RELEASE)
                target_link_libraries(${TRGTNAME} optimized "${${varname}_RELEASE}" debug "${${varname}_DEBUG}")
            else()
                target_link_libraries(${TRGTNAME} optimized "${${varname}}" debug "${${varname}_DEBUG}")
            endif()
        else()
            target_link_libraries(${TRGTNAME} ${${varname}})
        endif()
    endforeach()
endmacro()

# ── LINK_INTERNAL ────────────────────────────────────────────
# Link a target against internal project libraries.

macro(LINK_INTERNAL TRGTNAME)
    target_link_libraries(${TRGTNAME} ${ARGN})
endmacro()

# ── LINK_EXTERNAL ────────────────────────────────────────────
# Link a target against external libraries.

macro(LINK_EXTERNAL TRGTNAME)
    foreach(LINKLIB ${ARGN})
        target_link_libraries(${TRGTNAME} "${LINKLIB}")
    endforeach()
endmacro()

# ── LINK_CORELIB_DEFAULT ─────────────────────────────────────
# Link OpenGL and OpenThreads, set SOVERSION.

macro(LINK_CORELIB_DEFAULT CORELIB_NAME)
    LINK_EXTERNAL(${CORELIB_NAME} ${OPENGL_gl_LIBRARY})
    LINK_WITH_VARIABLES(${CORELIB_NAME} OPENTHREADS_LIBRARY)
    if(OSGILIATH_SONAMES)
        set_target_properties(${CORELIB_NAME} PROPERTIES
            VERSION   ${PROJECT_VERSION}
            SOVERSION ${OSGILIATH_SOVERSION}
        )
    endif()
endmacro()

# ── SETUP_LINK_LIBRARIES ────────────────────────────────────
# Common link setup for plugins, examples, and applications.
# Expects TARGET_TARGETNAME, TARGET_COMMON_LIBRARIES,
# TARGET_ADDED_LIBRARIES, TARGET_EXTERNAL_LIBRARIES,
# TARGET_LIBRARIES_VARS to be set.

macro(SETUP_LINK_LIBRARIES)
    set(TARGET_LIBRARIES ${TARGET_COMMON_LIBRARIES})

    foreach(LINKLIB ${TARGET_ADDED_LIBRARIES})
        if(NOT "${LINKLIB}" IN_LIST TARGET_LIBRARIES)
            list(APPEND TARGET_LIBRARIES ${LINKLIB})
        endif()
    endforeach()

    LINK_INTERNAL(${TARGET_TARGETNAME} ${TARGET_LIBRARIES})
    target_link_libraries(${TARGET_TARGETNAME} ${TARGET_EXTERNAL_LIBRARIES})

    # EGL for headless capture
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(EGL QUIET egl)
        if(EGL_FOUND)
            target_link_libraries(${TARGET_TARGETNAME} ${EGL_LIBRARIES})
        endif()
    endif()

    if(TARGET_LIBRARIES_VARS)
        LINK_WITH_VARIABLES(${TARGET_TARGETNAME} ${TARGET_LIBRARIES_VARS})
    endif()
endmacro()

# ── SETUP_LIBRARY ────────────────────────────────────────────
# Set up a core OSG library target.
# Expects LIB_NAME, TARGET_SRC, TARGET_H, TARGET_H_NO_MODULE_INSTALL,
# TARGET_LIBRARIES, TARGET_EXTERNAL_LIBRARIES, TARGET_LIBRARIES_VARS.

macro(SETUP_LIBRARY LIB_NAME)
    set(TARGET_NAME ${LIB_NAME})
    set(TARGET_TARGETNAME ${LIB_NAME})

    add_library(${LIB_NAME}
        ${OSGILIATH_LINK_TYPE}
        ${TARGET_H}
        ${TARGET_H_NO_MODULE_INSTALL}
        ${TARGET_SRC}
    )

    target_include_directories(${LIB_NAME} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${INSTALL_INCDIR}>
    )

    set_target_properties(${LIB_NAME} PROPERTIES FOLDER "OSG Core")

    if(TARGET_LIBRARIES)
        LINK_INTERNAL(${LIB_NAME} ${TARGET_LIBRARIES})
    endif()
    if(TARGET_EXTERNAL_LIBRARIES)
        LINK_EXTERNAL(${LIB_NAME} ${TARGET_EXTERNAL_LIBRARIES})
    endif()
    if(TARGET_LIBRARIES_VARS)
        LINK_WITH_VARIABLES(${LIB_NAME} ${TARGET_LIBRARIES_VARS})
    endif()
    LINK_CORELIB_DEFAULT(${LIB_NAME})

    include(ModuleInstall OPTIONAL)
endmacro()

# ── SETUP_PLUGIN ─────────────────────────────────────────────
# Set up a plugin (MODULE or STATIC library).
# Expects TARGET_SRC, TARGET_H, TARGET_DEFAULT_PREFIX,
# TARGET_DEFAULT_LABEL_PREFIX.

macro(SETUP_PLUGIN PLUGIN_NAME)
    set(TARGET_NAME ${PLUGIN_NAME})

    if(NOT TARGET_TARGETNAME)
        set(TARGET_TARGETNAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")
    endif()
    if(NOT TARGET_LABEL)
        set(TARGET_LABEL "${TARGET_DEFAULT_LABEL_PREFIX} ${TARGET_NAME}")
    endif()

    if(DYNAMIC_OSGILIATH)
        add_library(${TARGET_TARGETNAME} MODULE ${TARGET_SRC} ${TARGET_H})
    else()
        add_library(${TARGET_TARGETNAME} STATIC ${TARGET_SRC} ${TARGET_H})
    endif()

    set_target_properties(${TARGET_TARGETNAME} PROPERTIES
        PROJECT_LABEL "${TARGET_LABEL}"
        FOLDER        "Plugins"
        LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_LIBDIR}/${OSG_PLUGINS}"
        ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_LIBDIR}/${OSG_PLUGINS}"
    )

    SETUP_LINK_LIBRARIES()

    install(TARGETS ${TARGET_TARGETNAME}
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION ${OSG_INSTALL_LIBDIR}/${OSG_PLUGINS}
        LIBRARY DESTINATION ${OSG_INSTALL_LIBDIR}/${OSG_PLUGINS}
    )
endmacro()

# ── SETUP_EXE ────────────────────────────────────────────────
# Common executable setup for examples and applications.

macro(SETUP_EXE IS_COMMANDLINE_APP)
    if(NOT TARGET_TARGETNAME)
        set(TARGET_TARGETNAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")
    endif()
    if(NOT TARGET_LABEL)
        set(TARGET_LABEL "${TARGET_DEFAULT_LABEL_PREFIX} ${TARGET_NAME}")
    endif()

    add_executable(${TARGET_TARGETNAME} ${TARGET_SRC} ${TARGET_H})

    set_target_properties(${TARGET_TARGETNAME} PROPERTIES
        PROJECT_LABEL    "${TARGET_LABEL}"
        OUTPUT_NAME      "${TARGET_NAME}"
        DEBUG_OUTPUT_NAME          "${TARGET_NAME}${CMAKE_DEBUG_POSTFIX}"
        RELEASE_OUTPUT_NAME        "${TARGET_NAME}${CMAKE_RELEASE_POSTFIX}"
        RELWITHDEBINFO_OUTPUT_NAME "${TARGET_NAME}${CMAKE_RELWITHDEBINFO_POSTFIX}"
        MINSIZEREL_OUTPUT_NAME     "${TARGET_NAME}${CMAKE_MINSIZEREL_POSTFIX}"
    )

    SETUP_LINK_LIBRARIES()
endmacro()

# ── SETUP_APPLICATION ────────────────────────────────────────

macro(SETUP_APPLICATION APPLICATION_NAME)
    set(TARGET_NAME ${APPLICATION_NAME})

    if(${ARGC} GREATER 1)
        set(IS_COMMANDLINE_APP ${ARGV1})
    else()
        set(IS_COMMANDLINE_APP 0)
    endif()

    SETUP_EXE(${IS_COMMANDLINE_APP})

    set_target_properties(${TARGET_TARGETNAME} PROPERTIES FOLDER "Applications")

    install(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION bin)
endmacro()

macro(SETUP_COMMANDLINE_APPLICATION APPLICATION_NAME)
    SETUP_APPLICATION(${APPLICATION_NAME} 1)
endmacro()

# ── SETUP_EXAMPLE ────────────────────────────────────────────

macro(SETUP_EXAMPLE EXAMPLE_NAME)
    set(TARGET_NAME ${EXAMPLE_NAME})

    if(${ARGC} GREATER 1)
        set(IS_COMMANDLINE_APP ${ARGV1})
    else()
        set(IS_COMMANDLINE_APP 0)
    endif()

    SETUP_EXE(${IS_COMMANDLINE_APP})

    set_target_properties(${TARGET_TARGETNAME} PROPERTIES FOLDER "Examples")

    install(TARGETS ${TARGET_TARGETNAME}
        RUNTIME DESTINATION share/OSGiliath/bin
    )
endmacro()

macro(SETUP_COMMANDLINE_EXAMPLE EXAMPLE_NAME)
    SETUP_EXAMPLE(${EXAMPLE_NAME} 1)
endmacro()

# ── REMOVE_CXX_FLAG ─────────────────────────────────────────
# Remove a flag from CMAKE_CXX_FLAGS (used by plugins that
# need to suppress specific warnings for third-party code).

macro(REMOVE_CXX_FLAG flag)
    string(REPLACE "${flag}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endmacro()
