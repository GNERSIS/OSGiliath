/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Convex planar polygon used for occlusion culling.
 * Defines an occluder in world space with optional holes.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/geometry/ConvexPlanarPolygon.hpp>

namespace osg
{

    class OccluderVolume;

    /** A class for representing convex clipping volumes made up of several
     * ConvexPlanarPolygon. */
    class OSG_EXPORT ConvexPlanarOccluder
        : public osg::Inherit<Object, ConvexPlanarOccluder>
    {

        public:

            ConvexPlanarOccluder()
            {
            }

            ConvexPlanarOccluder( const ConvexPlanarOccluder& cpo,
                                  const CopyOp& copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( cpo,
                         copyop ),
                _occluder( cpo._occluder ),
                _holeList( cpo._holeList )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               ConvexPlanarOccluder )

            void
            setOccluder( const ConvexPlanarPolygon& cpp )
            {
                _occluder = cpp;
            }

            ConvexPlanarPolygon&
            getOccluder()
            {
                return _occluder;
            }

            const ConvexPlanarPolygon&
            getOccluder() const
            {
                return _occluder;
            }

            typedef std::vector<ConvexPlanarPolygon> HoleList;

            void
            addHole( const ConvexPlanarPolygon& cpp )
            {
                _holeList.push_back( cpp );
            }

            void
            setHoleList( const HoleList& holeList )
            {
                _holeList = holeList;
            }

            HoleList&
            getHoleList()
            {
                return _holeList;
            }

            const HoleList&
            getHoleList() const
            {
                return _holeList;
            }

        protected:

            ~ConvexPlanarOccluder();    // {}

            ConvexPlanarPolygon _occluder;
            HoleList            _holeList;
    };

}    // end of namespace
