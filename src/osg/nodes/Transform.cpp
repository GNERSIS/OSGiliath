/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for scene graph transform nodes. Defines the
 * interface for computing local-to-world and world-to-local matrices.
 */
#include <osg/nodes/Transform.hpp>

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/transform.hpp>
#include <osg/nodes/Camera.hpp>

using namespace osg;

class TransformVisitor : public DualModeVisitor
{
    public:

        enum CoordMode
        {
            WORLD_TO_LOCAL,
            LOCAL_TO_WORLD,
        };

        CoordMode _coordMode;
        dmat4&    _matrix;
        bool      _ignoreCameras;

        TransformVisitor( dmat4&    matrix,
                          CoordMode coordMode,
                          bool      ignoreCameras ) :
            DualModeVisitor(),
            _coordMode( coordMode ),
            _matrix( matrix ),
            _ignoreCameras( ignoreCameras )
        {
        }

        using ConstNodeVisitor::apply;
        using NodeVisitor::apply;

        void
        apply( Transform& transform ) override
        {
            if( _coordMode == LOCAL_TO_WORLD )
            {
                transform.computeLocalToWorldMatrix( _matrix, this );
            }
            else    // worldToLocal
            {
                transform.computeWorldToLocalMatrix( _matrix, this );
            }
        }

        void
        accumulate( const NodePath& nodePath )
        {
            if( nodePath.empty() )
            {
                return;
            }

            unsigned int i = 0;
            if( _ignoreCameras )
            {
                // we need to found out the last absolute Camera in NodePath and
                // set the i index to after it so the final accumulation set ignores it.
                i = static_cast<unsigned int>( nodePath.size() );
                NodePath::const_reverse_iterator ritr;
                for( ritr = nodePath.rbegin(); ritr != nodePath.rend(); ++ritr, --i )
                {
                    const osg::Camera* camera = ( *ritr )->asCamera();
                    if( camera && ( camera->getReferenceFrame() !=
                                    osg::Transform::RELATIVE_RF ||
                                    camera->getParents().empty() ) )
                    {
                        break;
                    }
                }
            }

            // do the accumulation of the active part of nodepath.
            for( ; i < nodePath.size(); ++i )
            {
                nodePath[i]->accept( *this );
            }
        }

    protected:

        TransformVisitor&
        operator=( const TransformVisitor& )
        {
            return *this;
        }
};

dmat4
osg::computeLocalToWorld( const NodePath& nodePath,
                          bool            ignoreCameras )
{
    dmat4            matrix;
    TransformVisitor tv( matrix, TransformVisitor::LOCAL_TO_WORLD, ignoreCameras );
    tv.accumulate( nodePath );
    return matrix;
}

dmat4
osg::computeWorldToLocal( const NodePath& nodePath,
                          bool            ignoreCameras )
{
    osg::dmat4       matrix;
    TransformVisitor tv( matrix, TransformVisitor::WORLD_TO_LOCAL, ignoreCameras );
    tv.accumulate( nodePath );
    return matrix;
}

dmat4
osg::computeLocalToEye( const dmat4&    modelview,
                        const NodePath& nodePath,
                        bool            ignoreCameras )
{
    dmat4            matrix( modelview );
    TransformVisitor tv( matrix, TransformVisitor::LOCAL_TO_WORLD, ignoreCameras );
    tv.accumulate( nodePath );
    return matrix;
}

dmat4
osg::computeEyeToLocal( const dmat4&    modelview,
                        const NodePath& nodePath,
                        bool            ignoreCameras )
{
    dmat4 matrix;
    matrix = osg::inverse( modelview );
    TransformVisitor tv( matrix, TransformVisitor::WORLD_TO_LOCAL, ignoreCameras );
    tv.accumulate( nodePath );
    return matrix;
}

Transform::Transform()
{
    _referenceFrame = RELATIVE_RF;
}

Transform::Transform( const Transform& transform,
                      const CopyOp&    copyop ) :
    Inherit( transform,
             copyop ),
    _referenceFrame( transform._referenceFrame )
{
}

Transform::~Transform()
{
}

void
Transform::setReferenceFrame( ReferenceFrame rf )
{
    if( _referenceFrame == rf )
    {
        return;
    }

    _referenceFrame = rf;

    // switch off culling if transform is absolute.
    setCullingActive( _referenceFrame == RELATIVE_RF );
}

sphere
Transform::computeBound() const
{
    sphere bsphere = Group::computeBound();
    if( !bsphere.valid() )
    {
        return bsphere;
    }

    // note, NULL pointer for NodeVisitor, so compute's need
    // to handle this case gracefully, normally this should not be a problem.
    dmat4 l2w;

    computeLocalToWorldMatrix( l2w, NULL );

    osg::vec3 xdash                       = bsphere.center;
    xdash.x                              += bsphere.radius;
    xdash                                 = vec3( l2w * dvec3( xdash ) );

    osg::vec3 ydash                       = bsphere.center;
    ydash.y                              += bsphere.radius;
    ydash                                 = vec3( l2w * dvec3( ydash ) );

    osg::vec3 zdash                       = bsphere.center;
    zdash.z                              += bsphere.radius;
    zdash                                 = vec3( l2w * dvec3( zdash ) );

    bsphere.center                        = vec3( l2w * dvec3( bsphere.center ) );

    xdash                                -= bsphere.center;
    osg::sphere::value_type sqrlen_xdash  = osg::length2( xdash );

    ydash                                -= bsphere.center;
    osg::sphere::value_type sqrlen_ydash  = osg::length2( ydash );

    zdash                                -= bsphere.center;
    osg::sphere::value_type sqrlen_zdash  = osg::length2( zdash );

    bsphere.radius                        = sqrlen_xdash;
    if( bsphere.radius < sqrlen_ydash )
    {
        bsphere.radius = sqrlen_ydash;
    }
    if( bsphere.radius < sqrlen_zdash )
    {
        bsphere.radius = sqrlen_zdash;
    }
    bsphere.radius = ( osg::sphere::value_type )sqrt( bsphere.radius );

    return bsphere;
}
