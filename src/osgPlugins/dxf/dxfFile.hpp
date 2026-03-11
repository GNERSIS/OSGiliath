/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * dxfFile class.
 * Provides: _fileName, _isNewSection, parseFile, dxf2osg, findBlock, getVariable.
 */
#pragma once

#include "codeValue.hpp"
#include "dxfReader.hpp"
#include "dxfSection.hpp"
#include "dxfSectionBase.hpp"
#include "scene.hpp"

#include <iostream>
#include <osg/nodes/Group.hpp>
#include <string>

class dxfFile
{
    public:

        dxfFile( std::string fileName ) :
            _fileName( fileName ),
            _isNewSection( false )
        {
        }

        bool
        parseFile();
        osg::Group*
        dxf2osg();
        dxfBlock*
        findBlock( std::string name );
        VariableList
        getVariable( std::string var );

    protected:

        short
                                  assign( codeValue& cv );
        std::string               _fileName;
        bool                      _isNewSection;
        osg::ref_ptr<dxfReader>   _reader;
        osg::ref_ptr<dxfSection>  _current;
        osg::ref_ptr<dxfHeader>   _header;
        osg::ref_ptr<dxfTables>   _tables;
        osg::ref_ptr<dxfBlocks>   _blocks;
        osg::ref_ptr<dxfEntities> _entities;
        osg::ref_ptr<dxfSection>  _unknown;
        osg::ref_ptr<scene>       _scene;
};
