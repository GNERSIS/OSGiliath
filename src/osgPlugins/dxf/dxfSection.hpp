/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * dxfSection, derived from dxfSectionBase.
 * Provides: dxfHeader, assign, getVariable, dxfTables, assign, getOrCreateLayerTable.
 */
/**
    Classes used to parse each section of a DXF file. Not all
    types of section has been defined here, just the ones
    I found of interest, ie HEADER, TABLES, BLOCKS, and ENTITIES.
    Yet to be implemented: CLASSES, OBJECTS, and THUMBNAILIMAGE.
*/
#pragma once

#include "codeValue.hpp"
#include "dxfBlock.hpp"
#include "dxfDataTypes.hpp"
#include "dxfEntity.hpp"
#include "dxfSectionBase.hpp"
#include "dxfTable.hpp"
#include "scene.hpp"

#include <iostream>
#include <map>
#include <string>
#include <vector>

class dxfFile;

class dxfSection : public dxfSectionBase
{
    public:

        dxfSection()
        {
        }

        virtual ~dxfSection()
        {
        }
};

class dxfHeader : public dxfSection
{
    public:

        dxfHeader() :
            _inVariable( false )
        {
        }

        virtual ~dxfHeader()
        {
        }

        virtual void
        assign( dxfFile*   dxf,
                codeValue& cv );

        VariableList&
        getVariable( std::string inVar )
        {
            return _variables[inVar];
        }

    protected:

        std::map<std::string, VariableList> _variables;
        bool                                _inVariable;
        std::string                         _currentVariable;
};

class dxfTables : public dxfSection
{
    public:

        dxfTables() :
            _inLayerTable( false )
        {
        }

        virtual ~dxfTables()
        {
        }

        virtual void
        assign( dxfFile*   dxf,
                codeValue& cv );

        dxfLayerTable*
        getOrCreateLayerTable()
        {
            if( !_layerTable.get() )
            {
                _layerTable = new dxfLayerTable;
            }
            return _layerTable.get();
        }

    protected:

        bool                                _inLayerTable;
        osg::ref_ptr<dxfLayerTable>         _layerTable;
        std::vector<osg::ref_ptr<dxfTable>> _others;
        osg::ref_ptr<dxfTable>              _currentTable;
};

class dxfEntities : public dxfSection
{
    public:

        dxfEntities() :
            _currentEntity( NULL )
        {
        }

        virtual ~dxfEntities()
        {
        }

        virtual void
        assign( dxfFile*   dxf,
                codeValue& cv );
        virtual void
        drawScene( scene* sc );

    protected:

        dxfEntity* _currentEntity;
        EntityList _entityList;
};

class dxfBlocks : public dxfSection
{
    public:

        dxfBlocks() :
            _currentBlock( NULL )
        {
        }

        virtual ~dxfBlocks()
        {
        }

        virtual void
        assign( dxfFile*   dxf,
                codeValue& cv );
        dxfBlock*
        findBlock( std::string s );

    protected:

        dxfBlock*                           _currentBlock;
        std::map<std::string, dxfBlock*>    _blockNameList;
        std::vector<osg::ref_ptr<dxfBlock>> _blockList;
};
