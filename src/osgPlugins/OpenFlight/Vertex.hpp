/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Vertex class.
 * Provides: setCoord, setColor, setNormal, setUV, validColor, validNormal.
 */
//
// OpenFlight� loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#pragma once

#include <osg/core/Referenced.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <vector>

namespace flt
{

    class Vertex
    {
        public:

            Vertex();
            Vertex( const Vertex& vertex );

            void
            setCoord( const osg::vec3& coord );
            void
            setColor( const osg::vec4& color );
            void
            setNormal( const osg::vec3& normal );
            void
            setUV( int              layer,
                   const osg::vec2& uv );

            bool
            validColor() const
            {
                return _validColor;
            }

            bool
            validNormal() const
            {
                return _validNormal;
            }

            bool
            validUV( int layer ) const
            {
                return layer >= 0 && layer < MAX_LAYERS && _validUV[layer];
            }

            static const int MAX_LAYERS = 8;

            osg::vec3        _coord;
            osg::vec4        _color;
            osg::vec3        _normal;
            osg::vec2        _uv[MAX_LAYERS];

            bool             _validColor;
            bool             _validNormal;
            bool             _validUV[MAX_LAYERS];
    };

    class VertexList : public osg::Referenced,
                       public std::vector<Vertex>
    {
        public:

            VertexList()
            {
            }

            explicit VertexList( int size ) :
                std::vector<Vertex>( size )
            {
            }

        protected:

            virtual ~VertexList()
            {
            }
    };

}    // end namespace
