/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * dxfTable, derived from Referenced.
 * Provides: assign, dxfLayer, assign, getName, setName, getFrozen.
 */
#pragma once

#include <map>
#include <osg/core/ref_ptr.hpp>
#include <osg/core/Referenced.hpp>
#include <string>

class dxfFile;
class codeValue;

// special case: the layer table

class dxfTable : public osg::Referenced
{
    public:

        dxfTable()
        {
        }

        virtual ~dxfTable()
        {
        }

        virtual void
        assign( dxfFile*,
                codeValue& )
        {
        }
};

class dxfLayer : public osg::Referenced
{
    public:

        dxfLayer( std::string name = "0" ) :
            _name( name ),
            _color( 7 ),
            _frozen( false ),
            _lineWidth( -1 ),
            _lineThickness( -1 )
        {
        }

        virtual ~dxfLayer()
        {
        }

        virtual void
        assign( dxfFile*   dxf,
                codeValue& cv );

        virtual const std::string&
        getName() const
        {
            return _name;
        }

        virtual const unsigned short&
        getColor() const
        {
            return _color;
        }

        virtual void
        setName( const std::string& name )
        {
            _name = name;
        }

        const bool&
        getFrozen() const
        {
            return _frozen;
        }

        virtual const double&
        getLineWidth() const
        {
            return _lineWidth;
        }

        virtual const double&
        getLineThickness() const
        {
            return _lineThickness;
        }

    protected:

        std::string    _name;
        unsigned short _color;
        bool           _frozen;
        double         _lineWidth;
        double         _lineThickness;
};

class dxfLayerTable : public dxfTable
{
    public:

        dxfLayerTable()
        {
        }

        virtual ~dxfLayerTable()
        {
        }

        virtual void
        assign( dxfFile*   dxf,
                codeValue& cv );

        dxfLayer*
        findOrCreateLayer( std::string name )
        {
            if( name == "" )
            {
                name = "0";    // nowhere it is said "" is invalid, but...
            }
            dxfLayer* layer = _layers[name].get();
            if( !layer )
            {
                layer         = new dxfLayer;
                _layers[name] = layer;
            }
            return layer;
        }

    protected:

        std::map<std::string, osg::ref_ptr<dxfLayer>> _layers;
        osg::ref_ptr<dxfLayer>                        _currentLayer;
};
