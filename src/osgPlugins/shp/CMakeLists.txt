SET(TARGET_SRC
    ESRIShape.cpp
    ESRIShapeParser.cpp
    ESRIShapeReaderWriter.cpp
    XBaseParser.cpp)
SET(TARGET_H
    ESRIShape.hpp
    ESRIShapeParser.hpp
    XBaseParser.hpp)
SET(TARGET_ADDED_LIBRARIES
    osgSim
    osgTerrain)
#### end var setup  ###
SETUP_PLUGIN(shp)
