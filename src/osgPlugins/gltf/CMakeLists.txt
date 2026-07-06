SET(TARGET_SRC
    ReaderWriterGLTF.cpp
    GLTFLoader.cpp
)

SET(TARGET_H
    GLTFLoader.hpp
)

SET(TARGET_ADDED_LIBRARIES)

SETUP_PLUGIN(gltf)

# Create osgdb_glb symlink so OSG's plugin loader finds the gltf plugin for .glb files
IF(UNIX)
    SET(GLTF_GLB_PLUGIN_POSTFIX
        "$<$<CONFIG:Debug>:${CMAKE_DEBUG_POSTFIX}>$<$<CONFIG:Release>:${CMAKE_RELEASE_POSTFIX}>$<$<CONFIG:RelWithDebInfo>:${CMAKE_RELWITHDEBINFO_POSTFIX}>$<$<CONFIG:MinSizeRel>:${CMAKE_MINSIZEREL_POSTFIX}>"
    )
    ADD_CUSTOM_COMMAND(TARGET osgdb_gltf POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            $<TARGET_FILE_NAME:osgdb_gltf>
            $<TARGET_FILE_DIR:osgdb_gltf>/osgdb_glb${GLTF_GLB_PLUGIN_POSTFIX}${CMAKE_SHARED_MODULE_SUFFIX}
        COMMENT "Creating osgdb_glb symlink for .glb extension support"
    )
    INSTALL(FILES
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${OSG_PLUGINS}/osgdb_glb${GLTF_GLB_PLUGIN_POSTFIX}${CMAKE_SHARED_MODULE_SUFFIX}
        DESTINATION ${OSG_INSTALL_LIBDIR}/${OSG_PLUGINS}
        COMPONENT libosgiliath
    )
ENDIF()
