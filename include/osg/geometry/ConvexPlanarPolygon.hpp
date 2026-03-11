/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convex polygon defined by a list of vertices in a single plane.
 * Used as a building block for occluders and clip regions.
 */
#pragma once

#include <osg/core/Export.hpp>
#include <osg/maths/plane.hpp>
#include <vector>

namespace osg
{

    /** A class for representing components of convex clipping volumes. */
    class OSG_EXPORT ConvexPlanarPolygon
    {

        public:

            ConvexPlanarPolygon();

            typedef std::vector<osg::vec3> VertexList;

            void
            add( const vec3& v )
            {
                _vertexList.push_back( v );
            }

            void
            setVertexList( const VertexList& vertexList )
            {
                _vertexList = vertexList;
            }

            VertexList&
            getVertexList()
            {
                return _vertexList;
            }

            const VertexList&
            getVertexList() const
            {
                return _vertexList;
            }

        protected:

            VertexList _vertexList;
    };

}    // end of namespace
