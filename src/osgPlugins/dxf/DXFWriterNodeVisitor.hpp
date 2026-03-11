// -*-c++-*-

/*
 * Wavefront DXF loader for Open Scene Graph
 *
 * Copyright (C) 2001 Ulrich Hertlein <u.hertlein@web.de>
 *
 * Modified by Robert Osfield to support per Drawable coord, normal and
 * texture coord arrays, bug fixes, and support for texture mapping.
 *
 * Writing support added 2007 by Stephan Huber, http://digitalmind.de,
 * some ideas taken from the dae-plugin
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for
 * real-time rendering of large 3D photo-realistic models.
 * The OSG homepage is http://www.openscenegraph.org/
 */

#pragma once

#include <iostream>
#include <map>
#include <osg/core/Notify.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <set>
#include <sstream>
#include <stack>
#include <string>

struct Layer
{
    public:

        Layer( const std::string name  = "",
               unsigned int      color = 7 ) :
            _name( name ),
            _color( color )
        {
        }

        std::string  _name;
        unsigned int _color;
};

// reuse aci class for autocad colors, see
// http://bitsy.sub-atomic.com/~moses/acadcolors.html for samples
#include "aci.hpp"

class AcadColor
{
    public:

        AcadColor()
        {
            int index = 10;
            for( int ii = 10 * 3; ii < 256 * 3; )
            {
                // find RGB for each Autocad index colour
                unsigned int red   = ( int )floor( aci::table[ii++] * 255.0F );
                unsigned int green = ( int )floor( aci::table[ii++] * 255.0F );
                unsigned int blue  = ( int )floor( aci::table[ii++] * 255.0F );
                unsigned int rgb   = ( red << 16 ) + ( green << 8 ) + blue;
                _indexColors[rgb]  = index++;
            }
            //
        }

        // returns Autocad index color for supplied RGB.
        // if no exact match is found returns nearest color based on hue
        //  also adds match to cache for future lookups.
        int
        findColor( unsigned int rgb )
        {
            int                      aci = 255;
            ColorMap::const_iterator itr = _indexColors.find( rgb );
            if( itr != _indexColors.end() )
            {
                aci = itr->second;
            }
            else
            {
                // not found - match based on hue
                aci = nearestColor( rgb );

                // add matching colour to list to cache
                _indexColors[rgb] = aci;
            }
            return aci;
        }

    protected:

        // returns hue as an angle in range 0-360, saturation and value as 0-1
        void
        hsv( unsigned int rgb,
             float&       hue,
             float&       sat,
             float&       value )
        {
            int red   = rgb >> 16;
            int green = ( 0X00'00'FF'00 & rgb ) >> 8;
            int blue  = 0X00'00'00'FF & rgb;
            int H     = std::max( std::max( red, green ), blue );
            int L     = std::min( std::min( red, green ), blue );

            value     = ( float )H / 255.0F;    // note hsv and hsl define v differently!
            sat       = ( float )( H - L ) / ( float )H;

            if( H == L )
            {
                hue = 0.0;
            }
            else if( H == red )
            {
                hue = 360.0 + ( 60.0 * ( float )( green - blue ) / ( float )( H - L ) );
                if( hue > 360 )
                {
                    hue -= 360;
                }
            }
            else if( H == green )
            {
                hue = 120.0 + ( 60.0 * ( float )( blue - red ) / ( float )( H - L ) );
            }
            else if( H == blue )
            {
                hue = 240.0 + ( 60.0 * ( float )( red - green ) / ( float )( H - L ) );
            }
            else
            {
                hue = 0.0;
            }
        }

        int
        nearestColor( unsigned int rgb )
        {
            //- match based on hue
            float h;
            float s;
            float v;
            hsv( rgb, h, s, v );

            // aci index format is
            // last digit odd = 50% sat, even=100%
            // last digit 0,1 = 100% value, 2,3=80%, 4,5=60% 6,7=50%, 8,9=30%
            //  first two sigits are hue angle /1.5 but count starts at 10, first 9
            //  values are dummy named colours
            int aci  = 10 + ( int )( h / 1.5 );
            aci     -= ( aci % 10 );    // ensure last digit is zero

            if( v < 0.3 )
            {
                aci += 9;
            }
            else if( v < 0.5 )
            {
                aci += 6;
            }
            else if( v < 0.6 )
            {
                aci += 4;
            }
            else if( v < 0.8 )
            {
                aci += 2;
            }
            else
            {
                // last digit=0;
            }

            if( s < 0.5 )
            {
                aci += 1;
            }

            return aci;
        }

    protected:

        typedef std::map<unsigned int, unsigned char> ColorMap;
        ColorMap _indexColors;    // maps RGB to autocad index colour
        ColorMap _hueColors;      // maps hue angle to autocad index colour
};

class DXFWriterNodeVisitor : public osg::DualModeVisitor
{

    public:

        DXFWriterNodeVisitor( std::ostream& fout ) :
            osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
            _fout( fout ),
            _currentStateSet( new osg::StateSet() ),
            _count( 0 ),
            _firstPass( true ),
            _writeTriangleAs3DFace( true )
        {
        }

        static unsigned int
        getNodeRGB( osg::Geometry* geo,
                    unsigned int   index = 0 )
        {
            osg::Vec4Array* data = static_cast<osg::Vec4Array*>( geo->getColorArray() );
            if( data && index < data->size() )
            {
                return ( osg::asABGR( data->at( index ) ) ) >> 8;
            }
            return 0;
        }

        bool
        writeHeader(
            const osg::sphere& bound
        );    // call after first pass to trigger draw pass
        void
        writeFooter();

        void
        buildColorMap();

        virtual void
        apply( osg::Geode& node );

        virtual void
        apply( osg::Group& node )
        {
            osg::NodeVisitor::traverse( node );
        }

        void
        traverse( osg::Node& node )
        {
            pushStateSet( node.getStateSet() );

            osg::NodeVisitor::traverse( node );

            popStateSet( node.getStateSet() );
        }

        void
        pushStateSet( osg::StateSet* ss )
        {
            if( NULL != ss )
            {
                // Save our current stateset
                _stateSetStack.push( _currentStateSet.get() );

                // merge with node stateset
                _currentStateSet = static_cast<osg::StateSet*>(
                    _currentStateSet->clone( osg::CopyOp::SHALLOW_COPY )
                );
                _currentStateSet->merge( *ss );
            }
        }

        void
        popStateSet( osg::StateSet* ss )
        {
            if( NULL != ss )
            {
                // restore the previous stateset
                _currentStateSet = _stateSetStack.top();
                _stateSetStack.pop();
            }
        }

        int
        getNodeAcadColor( osg::Geometry* /*geo*/,
                          int /*index*/ = 0 )
        {
            return 0;
        }

    protected:

        struct CompareStateSet
        {
                bool
                operator()( const osg::ref_ptr<osg::StateSet>& ss1,
                            const osg::ref_ptr<osg::StateSet>& ss2 ) const
                {
                    return ss1->compare( *ss2, true ) < 0;
                }
        };

    private:

        DXFWriterNodeVisitor&
        operator=( const DXFWriterNodeVisitor& )
        {
            return *this;
        }

        // first pass get layer names and draw types
        void
        makeGeometryLayer( osg::Geometry* geo );

        // second pass - output data
        void
        processGeometry( osg::Geometry* geo,
                         osg::dmat4&    m );

        void
        processArray( osg::Array*       array,
                      const Layer&      layer,
                      const osg::dmat4& m = osg::dmat4() );

        void
        processStateSet( osg::StateSet* stateset );

        std::string
        getLayerName( const std::string& defaultValue = "" );

        typedef std::stack<osg::ref_ptr<osg::StateSet>> StateSetStack;

        std::ostream&                                   _fout;
        std::list<std::string>                          _nameStack;
        StateSetStack                                   _stateSetStack;
        osg::ref_ptr<osg::StateSet>                     _currentStateSet;

        unsigned int                                    _count;
        std::vector<Layer>                              _layers;
        bool                                            _firstPass;
        Layer                                           _layer;

        bool                                            _writeTriangleAs3DFace;

        AcadColor                                       _acadColor;
};
