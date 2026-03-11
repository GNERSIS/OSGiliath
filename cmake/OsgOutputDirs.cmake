# ── OSG Output Directories ───────────────────────────────────
# Output paths, install paths, build postfix.
# ─────────────────────────────────────────────────────────────

include(GNUInstallDirs)

set(OSG_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})

set(INSTALL_INCDIR     include)
set(INSTALL_BINDIR     bin)
set(INSTALL_LIBDIR     ${OSG_INSTALL_LIBDIR})
set(INSTALL_ARCHIVEDIR ${OSG_INSTALL_LIBDIR})
set(INSTALL_CONFIGDIR  ${OSG_INSTALL_LIBDIR}/cmake/OSGiliath)

set(OSG_EXPORT_TARGETS TRUE CACHE BOOL "Export cmake targets on install")
set(PKG_NAMESPACE "osg${PROJECT_VERSION_MAJOR}")

set(OUTPUT_BINDIR "${PROJECT_BINARY_DIR}/bin")
set(OUTPUT_LIBDIR "${PROJECT_BINARY_DIR}/lib")

file(MAKE_DIRECTORY "${OUTPUT_BINDIR}")
file(MAKE_DIRECTORY "${OUTPUT_LIBDIR}")
file(MAKE_DIRECTORY "${OUTPUT_LIBDIR}/${OSG_PLUGINS}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_LIBDIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_LIBDIR}")

set(CMAKE_DEBUG_POSTFIX          "d")
set(CMAKE_RELEASE_POSTFIX        "")
set(CMAKE_RELWITHDEBINFO_POSTFIX "rd")
set(CMAKE_MINSIZEREL_POSTFIX     "s")

add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)
