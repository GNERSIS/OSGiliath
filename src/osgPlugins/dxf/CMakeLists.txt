SET(TARGET_SRC
    ReaderWriterDXF.cpp
    DXFWriterNodeVisitor.cpp
    aci.cpp
    dxfBlock.cpp
    dxfEntity.cpp
    dxfFile.cpp
    dxfReader.cpp
    dxfSection.cpp
    dxfTable.cpp
    scene.cpp
)

SET(TARGET_H
    aci.hpp
    codeValue.hpp
    dxfBlock.hpp
    dxfDataTypes.hpp
    dxfEntity.hpp
    dxfFile.hpp
    dxfReader.hpp
    dxfSection.hpp
    dxfSectionBase.hpp
    dxfTable.hpp
    scene.hpp
    DXFWriterNodeVisitor.hpp
)

SET(TARGET_ADDED_LIBRARIES osgText )

#### end var setup  ###
SETUP_PLUGIN(dxf)
