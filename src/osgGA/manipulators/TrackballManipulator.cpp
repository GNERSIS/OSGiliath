/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Orbiting camera manipulator. Provides rotate, zoom, and pan via
 * mouse interaction around a center point.
 */
#include <osgGA/manipulators/TrackballManipulator.hpp>

using namespace osg;
using namespace osgGA;

/// Constructor.
TrackballManipulator::TrackballManipulator( int flags ) :
    Inherit( flags )
{
    setVerticalAxisFixed( false );
}

/// Constructor.
TrackballManipulator::TrackballManipulator( const TrackballManipulator& tm,
                                            const CopyOp&               copyOp ) :
    osg::Object( tm,
                 copyOp ),
    osg::Callback( tm,
                   copyOp ),
    Inherit( tm,
             copyOp )
{
}
