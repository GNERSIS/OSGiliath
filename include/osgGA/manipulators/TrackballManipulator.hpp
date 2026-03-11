/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Orbiting camera manipulator. Provides rotate, zoom, and pan via
 * mouse interaction around a center point.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgGA/manipulators/OrbitManipulator.hpp>

namespace osgGA
{

    class OSGGA_EXPORT TrackballManipulator
        : public osg::Inherit<OrbitManipulator, TrackballManipulator>
    {
            typedef OrbitManipulator inherited;

        public:

            TrackballManipulator( int flags = DEFAULT_SETTINGS );
            TrackballManipulator( const TrackballManipulator& tm,
                                  const osg::CopyOp&          copyOp =
                                      osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgGA,
                               TrackballManipulator )
    };

}
