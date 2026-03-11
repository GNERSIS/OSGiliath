/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Geometry node that auto-rotates its drawables to face the camera.
 * Supports axial and point-to-eye rotation modes.
 */
#include <osg/nodes/Billboard.hpp>

#include <math.h>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>
#include <stdio.h>

using namespace osg;

#define square( x ) ( ( x ) * ( x ) )

Billboard::Billboard()
{
    _mode = AXIAL_ROT;
    _axis.set( 0.0F, 0.0F, 1.0F );
    //_normal.set(0.0f,-1.0f,0.0f);
    setNormal( vec3( 0.0F, -1.0F, 0.0F ) );
    updateCache();
}

Billboard::Billboard( const Billboard& billboard,
                      const CopyOp&    copyop ) :
    Inherit( billboard,
             copyop ),
    _mode( billboard._mode ),
    _axis( billboard._axis ),
    _normal( billboard._normal ),
    _positionList( billboard._positionList ),
    _cachedMode( billboard._cachedMode ),
    _side( billboard._side )
{
    setNormal( _normal );
}

Billboard::~Billboard()
{
}

void
Billboard::setMode( Mode mode )
{
    _mode       = mode;
    _cachedMode = CACHE_DIRTY;
    updateCache();
}

void
Billboard::setAxis( const vec3& axis )
{
    _axis = axis;
    _axis = osg::normalize( _axis );
    updateCache();
}

void
Billboard::setNormal( const vec3& normal )
{
    _normal = normal;
    _normal = osg::normalize( _normal );
    updateCache();

    // Build rotation from normal to z-axis,
    // for use with POINT_ROT_EYE
    vec3  z( 0.0, 0.0, 1.0 );
    vec3  cp( osg::cross( z, _normal ) );
    float dot_val = osg::dot( z, _normal );
    float cp_len  = osg::length( cp );
    if( cp_len != 0.0F )
    {
        cp                   /= cp_len;
        float rotation_cp     = acosf( dot_val );
        _rotateNormalToZAxis  = osg::rotate( ( double )-rotation_cp, dvec3( cp ) );
    }
    else
    {
        _rotateNormalToZAxis = dmat4();
    }
}

void
Billboard::updateCache()
{
    if( _mode == AXIAL_ROT )
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
            _cachedMode = AXIAL_ROT;
        }
    }
    else if( _mode == POINT_ROT_WORLD )
    {
        if( _axis == vec3( 0.0F, 0.0, 1.0F ) && _normal == vec3( 0.0F, -1.0F, 0.0F ) )
        {
            _cachedMode = POINT_ROT_WORLD_Z_AXIS;
        }
        else
        {
            _cachedMode = _mode;
        }
    }
    else
    {
        _cachedMode = _mode;
    }

    _side = osg::cross( _axis, _normal );
    _side = osg::normalize( _side );
}

bool
Billboard::addDrawable( Drawable* gset )
{
    if( Geode::addDrawable( gset ) )
    {
        vec3 pos( 0.0F, 0.0F, 0.0F );
        while( _positionList.size() < _children.size() )
        {
            _positionList.push_back( pos );
        }
        return true;
    }
    return false;
}

bool
Billboard::addDrawable( Drawable*   gset,
                        const vec3& pos )
{
    if( Geode::addDrawable( gset ) )
    {
        while( _positionList.size() < _children.size() )
        {
            _positionList.push_back( pos );
        }
        return true;
    }
    return false;
}

bool
Billboard::removeDrawable( Drawable* gset )
{
    PositionList::iterator pitr = _positionList.begin();
    for( NodeList::iterator itr = _children.begin(); itr != _children.end();
         ++itr, ++pitr )
    {
        if( itr->get() == gset )
        {
            // note ref_ptr<> automatically handles decrementing gset's reference count.
            _children.erase( itr );
            _positionList.erase( pitr );
            dirtyBound();
            return true;
        }
    }
    return false;
}

bool
Billboard::computeMatrix( dmat4&      modelview,
                          const vec3& eye_local,
                          const vec3& pos_local ) const
{
    // vec3 up_local(matrix(0,1),matrix(1,1),matrix(2,1));

    dmat4 matrix;

    vec3  ev( eye_local - pos_local );
    switch( _cachedMode )
    {
        case( AXIAL_ROT_Z_AXIS ) :
            {

                ev.z            = 0.0F;
                float ev_length = osg::length( ev );
                if( ev_length > 0.0F )
                {
                    // float rotation_zrotation_z = atan2f(ev.x,ev.y);
                    // mat = osg::quat(inRadians(rotation_z), osg::vec3(0.0f,0.0f,1.0f));
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
                    // mat = osg::quat(inRadians(rotation_z), osg::vec3(0.0f,0.0f,1.0f));
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
                    // mat = osg::quat(inRadians(rotation_z), osg::vec3(0.0f,0.0f,1.0f));
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
        case( AXIAL_ROT ) :    // need to implement
            {
                float ev_side   = osg::dot( ev, _side );
                float ev_normal = osg::dot( ev, _normal );
                float rotation  = atan2f( ev_side, ev_normal );
                matrix          = osg::rotate( ( double )rotation, dvec3( _axis ) );
                break;
            }
        case( POINT_ROT_WORLD ) :
            {
                float ev_len = osg::length( ev );
                if( ev_len != 0.0F )
                {
                    ev /= ev_len;

                    vec3  cp( osg::cross( ev, _normal ) );
                    float dot_val = osg::dot( ev, _normal );

                    float cp_len  = osg::length( cp );
                    if( cp_len != 0.0F )
                    {
                        cp                /= cp_len;

                        float rotation_cp  = acosf( dot_val );
                        matrix = osg::rotate( ( double )-inRadians( rotation_cp ),
                                              ( double )cp[0],
                                              ( double )cp[1],
                                              ( double )cp[2] );
                    }
                }
                break;
            }
        case( POINT_ROT_EYE ) :
            {
                float ev_len = osg::length( ev );
                if( ev_len != 0.0F )
                {
                    ev /= ev_len;

                    // Row 1 of MV = camera up in model space
                    vec3 up( static_cast<float>( modelview( 1, 0 ) ),
                             static_cast<float>( modelview( 1, 1 ) ),
                             static_cast<float>( modelview( 1, 2 ) ) );
                    vec3 right( osg::cross( up, ev ) );
                    right          = osg::normalize( right );
                    up             = osg::cross( ev, right );
                    up             = osg::normalize( up );

                    matrix( 0, 0 ) = right.x;
                    matrix( 0, 1 ) = right.y;
                    matrix( 0, 2 ) = right.z;
                    matrix( 1, 0 ) = up.x;
                    matrix( 1, 1 ) = up.y;
                    matrix( 1, 2 ) = up.z;
                    matrix( 2, 0 ) = ev.x;
                    matrix( 2, 1 ) = ev.y;
                    matrix( 2, 2 ) = ev.z;

                    matrix         = _rotateNormalToZAxis * matrix;
                }
                break;
            }
        case( POINT_ROT_WORLD_Z_AXIS ) :
            {
                // float rotation_about_z = atan2( -ev.y, ev.x );
                // float xy_distance = sqrt( ev.x*ev.x + ev.y*ev.y );
                // float rotation_from_xy = atan2( xy_distance, -ev.z );

                vec2 about_z( -ev.y, ev.x );
                {
                    float len = osg::length( about_z );
                    if( len == 0.0F )
                    {
                        about_z.x = 1.0F;
                    }
                    else
                    {
                        about_z = about_z / len;
                    }
                }
                float xy_distance = sqrt( ev.x * ev.x + ev.y * ev.y );
                vec2  from_xy( xy_distance, -ev.z );
                {
                    float len = osg::length( from_xy );
                    if( len == 0.0F )
                    {
                        from_xy.x = 1.0F;
                    }
                    else
                    {
                        from_xy = from_xy / len;
                    }
                }

                matrix( 0, 0 ) = about_z.x;
                matrix( 0, 1 ) = about_z.y;
                matrix( 1, 0 ) = -about_z.y * from_xy.x;
                matrix( 1, 1 ) = about_z.x * from_xy.x;
                matrix( 1, 2 ) = from_xy.y;
                matrix( 2, 0 ) = about_z.y * from_xy.y;
                matrix( 2, 1 ) = -about_z.x * from_xy.y;
                matrix( 2, 2 ) = from_xy.x;

                break;
            }
    }

    osg::setTrans( matrix, dvec3( pos_local ) );

    // Column-vector: post-multiply billboard rotation
    modelview = modelview * matrix;

    return true;
}

sphere
Billboard::computeBound() const
{
    int i;
    int ngsets = static_cast<int>( _children.size() );

    if( ngsets == 0 )
    {
        return sphere();
    }

    sphere bsphere;
    bsphere.center.set( 0.0F, 0.0F, 0.0F );

    for( i = 0; i < ngsets; i++ )
    {
        const Drawable* drawable =
            _children[static_cast<std::size_t>( i )].valid()
                ? _children[static_cast<std::size_t>( i )]->asDrawable()
                : 0;
        if( drawable )
        {
            const box& bbox  = drawable->getBoundingBox();

            bsphere.center  += bbox.center();
            bsphere.center  += _positionList[static_cast<std::size_t>( i )];
        }
    }

    bsphere.center /= ( float )( ngsets );

    float maxd      = 0.0;
    for( i = 0; i < ngsets; ++i )
    {
        const Drawable* drawable =
            _children[static_cast<std::size_t>( i )].valid()
                ? _children[static_cast<std::size_t>( i )]->asDrawable()
                : 0;
        if( drawable )
        {
            const box& bbox = drawable->getBoundingBox();
            vec3       local_center =
                bsphere.center - _positionList[static_cast<std::size_t>( i )];
            for( unsigned int c = 0; c < 8; ++c )
            {
                float d = osg::length2( bbox.corner( c ) - local_center );
                if( d > maxd )
                {
                    maxd = d;
                }
            }
        }
    }
    bsphere.radius = sqrtf( maxd );

    return bsphere;
}
