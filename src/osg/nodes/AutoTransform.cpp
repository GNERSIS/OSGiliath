/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Transform that automatically orients toward the camera (billboarding)
 * or auto-scales based on screen size. Used for labels, sprites, and
 * annotations that must remain screen-aligned.
 */
#include <osg/nodes/AutoTransform.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>
#include <osg/traversal/CullStack.hpp>

using namespace osg;

AutoTransform::AutoTransform() :
    _autoUpdateEyeMovementTolerance( 0.0 ),
    _autoRotateMode( NO_ROTATION ),
    _autoScaleToScreen( false ),
    _scale( 1.0,
            1.0,
            1.0 ),
    _minimumScale( 0.0 ),
    _maximumScale( DBL_MAX ),
    _autoScaleTransitionWidthRatio( 0.25 ),
    _axis( 0.0F,
           0.0F,
           1.0F ),
    _normal( 0.0F,
             -1.0F,
             0.0F ),
    _cachedMode( NO_ROTATION ),
    _side( 1.0F,
           0.0,
           0.0F )
{
    // setNumChildrenRequiringUpdateTraversal(1);
}

AutoTransform::AutoTransform( const AutoTransform& pat,
                              const CopyOp&        copyop ) :
    Transform( pat,
               copyop ),
    _position( pat._position ),
    _pivotPoint( pat._pivotPoint ),
    _autoUpdateEyeMovementTolerance( pat._autoUpdateEyeMovementTolerance ),
    _autoRotateMode( pat._autoRotateMode ),
    _autoScaleToScreen( pat._autoScaleToScreen ),
    _rotation( pat._rotation ),
    _scale( pat._scale ),
    _minimumScale( pat._minimumScale ),
    _maximumScale( pat._maximumScale ),
    _autoScaleTransitionWidthRatio( pat._autoScaleTransitionWidthRatio ),
    _axis( pat._axis ),
    _normal( pat._normal ),
    _cachedMode( pat._cachedMode ),
    _side( pat._side )
{
    // setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);
}

void
AutoTransform::setAutoScaleToScreen( bool autoScaleToScreen )
{
    _autoScaleToScreen = autoScaleToScreen;
    if( _autoScaleToScreen )
    {
        setCullingActive( false );
    }
}

void
AutoTransform::setAutoRotateMode( AutoRotateMode mode )
{
    _autoRotateMode = mode;
    _cachedMode     = CACHE_DIRTY;
    updateCache();
}

void
AutoTransform::setAxis( const vec3& axis )
{
    _axis = axis;
    _axis = osg::normalize( _axis );
    updateCache();
}

void
AutoTransform::setNormal( const vec3& normal )
{
    _normal = normal;
    _normal = osg::normalize( _normal );
    updateCache();
}

void
AutoTransform::updateCache()
{
    if( _autoRotateMode == ROTATE_TO_AXIS )
    {
        if( _axis == vec3( 1.0F, 0.0, 0.0F ) && _normal == vec3( 0.0F, -1.0, 0.0F ) )
        {
            _cachedMode = AXIAL_ROT_X_AXIS;
        }
        else if( _axis == vec3( 0.0F, 1.0, 0.0F ) && _normal == vec3( 1.0F, 0.0, 0.0F ) )
        {
            _cachedMode = AXIAL_ROT_Y_AXIS;
        }
        else if( _axis ==
                 vec3( 0.0F, 0.0, 1.0F ) &&
                 _normal == vec3( 0.0F, -1.0, 0.0F ) )
        {
            _cachedMode = AXIAL_ROT_Z_AXIS;
        }
        else
        {
            _cachedMode = ROTATE_TO_AXIS;
        }
    }
    else
    {
        _cachedMode = _autoRotateMode;
    }

    _side = osg::cross( _axis, _normal );
    _side = osg::normalize( _side );
}

void
AutoTransform::setScale( const dvec3& scale )
{
    _scale = scale;
    if( _scale.x < _minimumScale )
    {
        _scale.x = _minimumScale;
    }
    if( _scale.y < _minimumScale )
    {
        _scale.y = _minimumScale;
    }
    if( _scale.z < _minimumScale )
    {
        _scale.z = _minimumScale;
    }

    if( _scale.x > _maximumScale )
    {
        _scale.x = _maximumScale;
    }
    if( _scale.y > _maximumScale )
    {
        _scale.y = _maximumScale;
    }
    if( _scale.z > _maximumScale )
    {
        _scale.z = _maximumScale;
    }

    dirtyBound();
}

bool
AutoTransform::computeLocalToWorldMatrix( dmat4&       matrix,
                                          NodeVisitor* nv ) const
{
    if( _referenceFrame == RELATIVE_RF )
    {
        matrix = matrix * computeMatrix( nv );
    }
    else    // absolute
    {
        matrix = computeMatrix( nv );
    }
    return true;
}

bool
AutoTransform::computeWorldToLocalMatrix( dmat4&       matrix,
                                          NodeVisitor* nv ) const
{
    if( _referenceFrame == RELATIVE_RF )
    {
        matrix = osg::inverse( computeMatrix( nv ) ) * matrix;
    }
    else    // absolute
    {
        matrix = osg::inverse( computeMatrix( nv ) );
    }
    return true;
}

osg::dmat4
AutoTransform::computeMatrix( const osg::NodeVisitor* nv ) const
{
    quat             rotation = _rotation;
    osg::dvec3       scale    = _scale;

    const CullStack* cs       = nv ? nv->asCullStack() : 0;
    if( cs )
    {
        osg::dvec3 eyePoint( cs->getEyeLocal() );
        osg::dvec3 localUp( cs->getUpLocal() );

        if( getAutoScaleToScreen() )
        {
            double size = 1.0 / cs->pixelSize( vec3( getPosition() ), 0.48F );

            if( _autoScaleTransitionWidthRatio > 0.0 )
            {
                if( _minimumScale > 0.0 )
                {
                    double j = _minimumScale;
                    double i =
                        ( _maximumScale < DBL_MAX )
                            ? _minimumScale +
                                  ( _maximumScale - _minimumScale ) *
                                  _autoScaleTransitionWidthRatio
                            : _minimumScale * ( 1.0 + _autoScaleTransitionWidthRatio );
                    double c = 1.0 / ( 4.0 * ( i - j ) );
                    double b = 1.0 - 2.0 * c * i;
                    double a = j + b * b / ( 4.0 * c );
                    double k = -b / ( 2.0 * c );

                    if( size < k )
                    {
                        size = _minimumScale;
                    }
                    else if( size < i )
                    {
                        size = a + b * size + c * ( size * size );
                    }
                }

                if( _maximumScale < DBL_MAX )
                {
                    double n = _maximumScale;
                    double m =
                        ( _minimumScale > 0.0 )
                            ? _maximumScale +
                                  ( _minimumScale - _maximumScale ) *
                                  _autoScaleTransitionWidthRatio
                            : _maximumScale * ( 1.0 - _autoScaleTransitionWidthRatio );
                    double c = 1.0 / ( 4.0 * ( m - n ) );
                    double b = 1.0 - 2.0 * c * m;
                    double a = n + b * b / ( 4.0 * c );
                    double p = -b / ( 2.0 * c );

                    if( size > p )
                    {
                        size = _maximumScale;
                    }
                    else if( size > m )
                    {
                        size = a + b * size + c * ( size * size );
                    }
                }
            }
            else
            {
                if( _minimumScale > 0.0 && size < _minimumScale )
                {
                    size = _minimumScale;
                }

                if( _maximumScale < DBL_MAX && size > _maximumScale )
                {
                    size = _maximumScale;
                }
            }

            // TODO setScale(size);
            scale.set( size, size, size );
        }

        if( _autoRotateMode == ROTATE_TO_SCREEN )
        {
            osg::dvec3 mv_translation;
            osg::dvec3 mv_scale;
            osg::dquat mv_rotation;

            osg::decompose( dmat4( *cs->getModelViewMatrix() ),
                            mv_translation,
                            mv_rotation,
                            mv_scale );

            // TODO setRotation(osg::inverse(rotation));
            rotation = quat( osg::inverse( mv_rotation ) );
        }
        else if( _autoRotateMode == ROTATE_TO_CAMERA )
        {
            osg::dvec3 PosToEye = _position - eyePoint;
            osg::dmat4 lookto = osg::lookAt( osg::dvec3( 0, 0, 0 ), PosToEye, localUp );
            dquat      dq;
            // Extract rotation from inverse of lookAt matrix
            dq = osg::getRotate( osg::inverse( lookto ) );
            // TODO setRotation(q);
            rotation = quat( dq );
        }
        else if( _autoRotateMode == ROTATE_TO_AXIS )
        {
            dmat4 matrix;
            vec3  ev( dvec3( eyePoint ) - dvec3( _position ) );

            switch( _cachedMode )
            {
                case( AXIAL_ROT_Z_AXIS ) :
                    {
                        ev.z            = 0.0F;
                        float ev_length = osg::length( ev );
                        if( ev_length > 0.0F )
                        {
                            // float rotation_zrotation_z = atan2f(ev.x,ev.y);
                            // mat = osg::quat(inRadians(rotation_z),
                            // osg::vec3(0.0f,0.0f,1.0f));
                            float inv      = 1.0F / ev_length;
                            float s        = ev.x * inv;
                            float c        = -ev.y * inv;
                            matrix( 0, 0 ) = c;
                            matrix( 1, 0 ) = -s;
                            matrix( 0, 1 ) = s;
                            matrix( 1, 1 ) = c;
                        }
                        break;
                    }
                case( AXIAL_ROT_Y_AXIS ) :
                    {
                        ev.y            = 0.0F;
                        float ev_length = osg::length( ev );
                        if( ev_length > 0.0F )
                        {
                            // float rotation_zrotation_z = atan2f(ev.x,ev.y);
                            // mat = osg::quat(inRadians(rotation_z),
                            // osg::vec3(0.0f,0.0f,1.0f));
                            float inv      = 1.0F / ev_length;
                            float s        = -ev.z * inv;
                            float c        = ev.x * inv;
                            matrix( 0, 0 ) = c;
                            matrix( 2, 0 ) = s;
                            matrix( 0, 2 ) = -s;
                            matrix( 2, 2 ) = c;
                        }
                        break;
                    }
                case( AXIAL_ROT_X_AXIS ) :
                    {
                        ev.x            = 0.0F;
                        float ev_length = osg::length( ev );
                        if( ev_length > 0.0F )
                        {
                            // float rotation_zrotation_z = atan2f(ev.x,ev.y);
                            // mat = osg::quat(inRadians(rotation_z),
                            // osg::vec3(0.0f,0.0f,1.0f));
                            float inv      = 1.0F / ev_length;
                            float s        = -ev.z * inv;
                            float c        = -ev.y * inv;
                            matrix( 1, 1 ) = c;
                            matrix( 2, 1 ) = -s;
                            matrix( 1, 2 ) = s;
                            matrix( 2, 2 ) = c;
                        }
                        break;
                    }
                case( ROTATE_TO_AXIS ) :    // need to implement
                    {
                        float ev_side   = osg::dot( ev, _side );
                        float ev_normal = osg::dot( ev, _normal );
                        float angle     = atan2f( ev_side, ev_normal );
                        matrix          = osg::rotate( ( double )angle, dvec3( _axis ) );
                        break;
                    }
            }
            quat q = quat( osg::getRotate( matrix ) );
            // TODO setRotation(q);
            rotation = q;
        }
    }

    _rotation = rotation;
    _scale    = scale;
    // setRotation(rotation);
    // setScale(scale);

    // column-vector: translate(pos) * rotate * scale * translate(-pivot)
    osg::dmat4 matrix = osg::translate( _position ) *
                        osg::rotate( dquat( rotation ) ) *
                        osg::scale( scale ) *
                        osg::translate( -_pivotPoint );

    return matrix;
}
