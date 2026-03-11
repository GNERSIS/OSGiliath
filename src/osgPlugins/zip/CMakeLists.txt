SET(TARGET_SRC
    unzip.cpp
    ZipArchive.cpp
    ReaderWriterZIP.cpp
)

SET(TARGET_H
    unzip.hpp
    ZipArchive.hpp
)

ADD_DEFINITIONS(-DZIP_STD)

IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-implicit-fallthrough")
ENDIF()

SETUP_PLUGIN(zip)
