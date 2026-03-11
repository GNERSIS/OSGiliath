/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: _coord.
 */
//
// OpenFlight� loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include "Vertex.hpp"

using namespace flt;

Vertex::Vertex() :
    _coord( 0,
            0,
            0 ),
    _color( 1,
            1,
            1,
            1 ),
    _normal( 0,
             0,
             1 ),
    _validColor( false ),
    _validNormal( false )
{
    for( int layer = 0; layer < MAX_LAYERS; layer++ )
    {
        _validUV[layer] = false;
    }
}

Vertex::Vertex( const Vertex& vertex ) :
    _coord( vertex._coord ),
    _color( vertex._color ),
    _normal( vertex._normal ),
    _validColor( vertex._validColor ),
    _validNormal( vertex._validNormal )
{
    for( int layer = 0; layer < MAX_LAYERS; layer++ )
    {
        _uv[layer]      = vertex._uv[layer];
        _validUV[layer] = vertex._validUV[layer];
    }
}

void
Vertex::setCoord( const osg::vec3& coord )
{
    _coord = coord;
}

void
Vertex::setColor( const osg::vec4& color )
{
    _color      = color;
    _validColor = true;
}

void
Vertex::setNormal( const osg::vec3& normal )
{
    _normal      = normal;
    _validNormal = true;
}

void
Vertex::setUV( int              layer,
               const osg::vec2& uv )
{
    if( layer >= 0 && layer < MAX_LAYERS )
    {
        _uv[layer]      = uv;
        _validUV[layer] = true;
    }
}
