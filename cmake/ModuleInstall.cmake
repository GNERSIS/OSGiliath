# ── Module Install ───────────────────────────────────────────
# Install and source-group commands for OSG core libraries.
# Expects: LIB_NAME, TARGET_H, TARGET_LIBRARIES
# ─────────────────────────────────────────────────────────────

source_group("Header Files" FILES ${TARGET_H})

if(OSG_EXPORT_TARGETS)
    install(TARGETS ${LIB_NAME}
        EXPORT  ${LIB_NAME}
        RUNTIME DESTINATION ${INSTALL_BINDIR}   COMPONENT libosgiliath
        LIBRARY DESTINATION ${INSTALL_LIBDIR}    COMPONENT libosgiliath
        ARCHIVE DESTINATION ${INSTALL_ARCHIVEDIR} COMPONENT libosgiliath-dev
    )
    install(EXPORT ${LIB_NAME}
        NAMESPACE   ${PKG_NAMESPACE}::
        DESTINATION ${INSTALL_CONFIGDIR}
        FILE        ${LIB_NAME}-targets.cmake
        COMPONENT   libosgiliath-dev
    )
else()
    install(TARGETS ${LIB_NAME}
        RUNTIME DESTINATION ${INSTALL_BINDIR}   COMPONENT libosgiliath
        LIBRARY DESTINATION ${INSTALL_LIBDIR}    COMPONENT libosgiliath
        ARCHIVE DESTINATION ${INSTALL_ARCHIVEDIR} COMPONENT libosgiliath-dev
    )
endif()

install(FILES ${TARGET_H}
    DESTINATION ${INSTALL_INCDIR}/${LIB_NAME}
    COMPONENT libosgiliath-dev
)
