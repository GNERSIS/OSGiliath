
SET(TARGET_SRC 
    ReaderWriterTXF.cpp
    TXFFont.cpp
)
SET(TARGET_H
    TXFFont.hpp
)
SET(TARGET_ADDED_LIBRARIES osgText )

#### end var setup  ###
SETUP_PLUGIN(txf)
