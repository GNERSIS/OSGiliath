/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Specifies a named viewpoint within the scene graph.
 * Provides position and orientation for camera presets.
 */
#include <osg/nodes/CameraView.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>

using namespace osg;

CameraView::CameraView() :
    _fieldOfView( 60.0 ),
    _fieldOfViewMode( VERTICAL ),
    _focalLength( 0.0 )
{
}

bool
CameraView::computeLocalToWorldMatrix( dmat4& matrix,
                                       NodeVisitor* ) const
{
    if( _referenceFrame == RELATIVE_RF )
    {
        matrix = matrix *
                 osg::translate( dvec3( _position ) ) *
                 osg::rotate( dquat( _attitude ) );
    }
    else    // absolute
    {
        matrix =
            osg::translate( dvec3( _position ) ) * osg::rotate( dquat( _attitude ) );
    }
    return true;
}

bool
CameraView::computeWorldToLocalMatrix( dmat4& matrix,
                                       NodeVisitor* ) const
{
    if( _referenceFrame == RELATIVE_RF )
    {
        matrix = osg::rotate( osg::inverse( dquat( _attitude ) ) ) *
                 osg::translate( -dvec3( _position ) ) *
                 matrix;
    }
    else    // absolute
    {
        matrix = osg::rotate( osg::inverse( dquat( _attitude ) ) ) *
                 osg::translate( -dvec3( _position ) );
    }
    return true;
}
