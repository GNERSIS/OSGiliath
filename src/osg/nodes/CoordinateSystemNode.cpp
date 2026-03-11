/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Node defining a geographic coordinate system with ellipsoid
 * model. Used for planetary rendering and geo-referenced scenes.
 */
#include <osg/nodes/CoordinateSystemNode.hpp>

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>

using namespace osg;

CoordinateSystemNode::CoordinateSystemNode()
{
}

CoordinateSystemNode::CoordinateSystemNode( const std::string& format,
                                            const std::string& cs ) :
    _format( format ),
    _cs( cs )
{
}

CoordinateSystemNode::CoordinateSystemNode( const CoordinateSystemNode& csn,
                                            const osg::CopyOp&          copyop ) :
    Inherit( csn,
             copyop ),
    _format( csn._format ),
    _cs( csn._cs ),
    _ellipsoidModel( csn._ellipsoidModel )
{
}

void
CoordinateSystemNode::set( const CoordinateSystemNode& csn )
{
    _format         = csn._format;
    _cs             = csn._cs;
    _ellipsoidModel = csn._ellipsoidModel;
}

CoordinateFrame
CoordinateSystemNode::computeLocalCoordinateFrame( const dvec3& position ) const
{
    if( _ellipsoidModel.valid() )
    {
        dmat4  localToWorld;

        double latitude, longitude, height;
        _ellipsoidModel->convertXYZToLatLongHeight( position.x,
                                                    position.y,
                                                    position.z,
                                                    latitude,
                                                    longitude,
                                                    height );
        _ellipsoidModel->computeLocalToWorldTransformFromLatLongHeight( latitude,
                                                                        longitude,
                                                                        0.0F,
                                                                        localToWorld );

        return localToWorld;
    }
    else
    {
        return osg::translate( position.x, position.y, 0.0 );
    }
}

osg::dvec3
CoordinateSystemNode::computeLocalUpVector( const dvec3& position ) const
{
    if( _ellipsoidModel.valid() )
    {
        return _ellipsoidModel->computeLocalUpVector( position.x,
                                                      position.y,
                                                      position.z );
    }
    else
    {
        return osg::dvec3( 0.0F, 0.0F, 1.0F );
    }
}
