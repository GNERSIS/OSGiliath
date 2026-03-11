/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Cull callback using a cluster normal and deviation angle.
 * Efficiently rejects back-facing terrain patches and tile clusters.
 */
#include <osg/traversal/ClusterCullingCallback.hpp>

#include <osg/geometry/TriangleFunctor.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>
#include <osg/traversal/CullSettings.hpp>

using namespace osg;

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Cluster culling callback
//

ClusterCullingCallback::ClusterCullingCallback() :
    _radius( -1.0F ),
    _deviation( -1.0F )
{
}

ClusterCullingCallback::ClusterCullingCallback( const ClusterCullingCallback& ccc,
                                                const CopyOp&                 copyop ) :
    Object( ccc,
            copyop ),
    Callback( ccc,
              copyop ),
    osg::Inherit<DrawableCullCallback,
                 ClusterCullingCallback>( ccc,
                                          copyop ),
    NodeCallback( ccc,
                  copyop ),
    _controlPoint( ccc._controlPoint ),
    _normal( ccc._normal ),
    _radius( ccc._radius ),
    _deviation( ccc._deviation )
{
}

ClusterCullingCallback::ClusterCullingCallback( const osg::vec3& controlPoint,
                                                const osg::vec3& normal,
                                                float            deviation,
                                                float            radius ) :
    _controlPoint( controlPoint ),
    _normal( normal ),
    _radius( radius ),
    _deviation( deviation )
{
}

ClusterCullingCallback::ClusterCullingCallback( const osg::Drawable* drawable )
{
    computeFrom( drawable );
}

struct ComputeAveragesFunctor
{

        ComputeAveragesFunctor() :
            _num( 0 )
        {
        }

        inline void
        operator()( const osg::vec3& v1,
                    const osg::vec3& v2,
                    const osg::vec3& v3 )
        {
            // calc orientation of triangle.
            osg::dvec3 normal =
                osg::cross( dvec3( v2 ) - dvec3( v1 ), dvec3( v3 ) - dvec3( v1 ) );
            if( osg::length( normal ) != 0.0F )
            {
                normal   = osg::normalize( normal );
                _normal += normal;
            }
            _center += dvec3( v1 );
            _center += dvec3( v2 );
            _center += dvec3( v3 );

            ++_num;
        }

        osg::vec3
        center()
        {
            return vec3( _center / ( double )( 3 * _num ) );
        }

        osg::vec3
        normal()
        {
            _normal = osg::normalize( _normal );
            return vec3( _normal );
        }

        unsigned int _num;
        dvec3        _center;
        dvec3        _normal;
};

struct ComputeDeviationFunctor
{

        ComputeDeviationFunctor() :
            _deviation( 1.0 ),
            _radius2( 0.0 )
        {
        }

        void
        set( const osg::vec3& center,
             const osg::vec3& normal )
        {
            _center = center;
            _normal = normal;
        }

        inline void
        operator()( const osg::vec3& v1,
                    const osg::vec3& v2,
                    const osg::vec3& v3 )
        {
            // calc orientation of triangle.
            osg::vec3 normal = osg::cross( v2 - v1, v3 - v1 );
            if( osg::length( normal ) != 0.0F )
            {
                normal     = osg::normalize( normal );
                _deviation = std::min( osg::dot( _normal, normal ), _deviation );
            }
            _radius2 = std::max( osg::length2( v1 - _center ), _radius2 );
            _radius2 = std::max( osg::length2( v2 - _center ), _radius2 );
            _radius2 = std::max( osg::length2( v3 - _center ), _radius2 );
        }

        osg::vec3 _center;
        osg::vec3 _normal;
        float     _deviation;
        float     _radius2;
};

void
ClusterCullingCallback::computeFrom( const osg::Drawable* drawable )
{
    TriangleFunctor<ComputeAveragesFunctor> caf;
    drawable->accept( caf );

    _controlPoint = caf.center();
    _normal       = caf.normal();

    TriangleFunctor<ComputeDeviationFunctor> cdf;
    cdf.set( _controlPoint, _normal );
    drawable->accept( cdf );

    // OSG_NOTICE<<"ClusterCullingCallback::computeFrom()
    // _controlPoint="<<_controlPoint<<std::endl; OSG_NOTICE<<"
    // _normal="<<_normal<<std::endl; OSG_NOTICE<<"
    // cdf._deviation="<<cdf._deviation<<std::endl;

    if( osg::length2( _normal ) == 0.0 )
    {
        _deviation = -1.0F;
    }
    else
    {
        float angle = acosf( cdf._deviation ) + osg::PIf * 0.5F;
        if( angle < osg::PIf )
        {
            _deviation = cosf( angle );
        }
        else
        {
            _deviation = -1.0F;
        }
    }

    _radius = sqrtf( cdf._radius2 );
}

void
ClusterCullingCallback::set( const osg::vec3& controlPoint,
                             const osg::vec3& normal,
                             float            deviation,
                             float            radius )
{
    _controlPoint = controlPoint;
    _normal       = normal;
    _deviation    = deviation;
    _radius       = radius;
}

void
ClusterCullingCallback::transform( const osg::dmat4& matrix )
{
    _controlPoint = vec3( matrix * dvec3( _controlPoint ) );
    _normal       = vec3( osg::inverse( matrix ) * dvec3( _normal ) );
    _normal       = osg::normalize( _normal );
}

bool
ClusterCullingCallback::cull( osg::NodeVisitor* nv,
                              osg::Drawable*,
                              osg::State* ) const
{
    CullSettings* cs = dynamic_cast<CullSettings*>( nv );
    if( cs && !( cs->getCullingMode() & CullSettings::CLUSTER_CULLING ) )
    {
        // cluster culling switched off cull settings.
        return false;
    }

    if( _deviation <= -1.0F )
    {
        // cluster culling switch off by deviation.
        return false;
    }

    osg::vec3 eye_cp = nv->getViewPoint() - _controlPoint;
    float     radius = osg::length( eye_cp );

    if( radius < _radius )
    {
        return false;
    }

    float deviation = ( osg::dot( eye_cp, _normal ) ) / radius;

    // OSG_NOTICE<<"ClusterCullingCallback::cull() _normal="<<_normal<<"
    // _controlPointtest="<<_controlPoint<<" eye_cp="<<eye_cp<<std::endl; OSG_NOTICE<<"
    // deviation="<<deviation<<" _deviation="<<_deviation<<" test="<<(deviation <
    // _deviation)<<std::endl;

    return deviation < _deviation;
}

void
ClusterCullingCallback::operator()( Node*        node,
                                    NodeVisitor* nv )
{
    if( nv )
    {
        if( cull( nv, 0, static_cast<State*>( 0 ) ) )
        {
            return;
        }

        traverse( node, nv );
    }
}
