SET(TARGET_SRC
    ReaderWriterNormals.cpp 
    Normals.cpp
)
SET(TARGET_H Normals.hpp )

#### end var setup  ###
SETUP_PLUGIN(normals)

# Legacy code — suppress conversion warnings until full migration
target_compile_options(${TARGET_TARGETNAME} PRIVATE
    -Wno-error=implicit-float-conversion
    -Wno-error=sign-conversion
    -Wno-error=shorten-64-to-32
)
