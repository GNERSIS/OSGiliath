/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Shadow map using cull-pass bounds for frustum optimization.
 * Tighter shadow projection from CPU-side bounding volumes.
 */
#include <osgShadow/MinimalCullBoundsShadowMap.hpp>

#include <osg/maths/Matrix.hpp>
#include <osgUtil/culling/RenderLeaf.hpp>
#include <string.h>

#define IGNORE_OBJECTS_LARGER_THAN_HEIGHT 0

using namespace osgShadow;

MinimalCullBoundsShadowMap::MinimalCullBoundsShadowMap()
{
}

MinimalCullBoundsShadowMap::MinimalCullBoundsShadowMap(
    const MinimalCullBoundsShadowMap& copy,
    const osg::CopyOp&                copyop
) :
    Inherit( copy,
             copyop )
{
}

MinimalCullBoundsShadowMap::~MinimalCullBoundsShadowMap()
{
}

void
MinimalCullBoundsShadowMap::ViewData::init( ThisClass*            st,
                                            osgUtil::CullVisitor* cv )
{
    BaseClass::ViewData::init( st, cv );
    _frameShadowCastingCameraPasses = 2;
}

void
MinimalCullBoundsShadowMap::ViewData::aimShadowCastingCamera( const osg::Light* light,
                                                              const osg::vec4&  lightPos,
                                                              const osg::vec3&  lightDir,
                                                              const osg::vec3&  lightUp )
{
    MinimalShadowMap::ViewData::aimShadowCastingCamera( light,
                                                        lightPos,
                                                        lightDir,
                                                        lightUp );

    frameShadowCastingCamera( _cv->getCurrentRenderBin()->getStage()->getCamera(),
                              _camera.get() );
}

void
MinimalCullBoundsShadowMap::ViewData::cullShadowReceivingScene()
{
    RenderLeafList rllOld, rllNew;

    GetRenderLeaves( _cv->getRenderStage(), rllOld );

    MinimalShadowMap::ViewData::cullShadowReceivingScene();

    GetRenderLeaves( _cv->getRenderStage(), rllNew );

    RemoveOldRenderLeaves( rllNew, rllOld );
    RemoveIgnoredRenderLeaves( rllNew );

    osg::dmat4 projectionToModelSpace = osg::inverse( *_modellingSpaceToWorldPtr *
                                                      *_cv->getModelViewMatrix() *
                                                      *_cv->getProjectionMatrix() );

    osg::box   bb;
    if( *_cv->getProjectionMatrix() != _clampedProjection )
    {

        osg::Polytope polytope;
#if 1
        polytope.setToUnitFrustum();
#else
        polytope.add( osg::Plane( 0.0, 0.0, -1.0, 1.0 ) );    // only far plane
#endif
        polytope.transformProvidingInverse(
            *_modellingSpaceToWorldPtr * *_cv->getModelViewMatrix() * _clampedProjection
        );

        bb = ComputeRenderLeavesBounds( rllNew, projectionToModelSpace, polytope );
    }
    else
    {
        bb = ComputeRenderLeavesBounds( rllNew, projectionToModelSpace );
    }

    cutScenePolytope( *_modellingSpaceToWorldPtr,
                      osg::inverse( *_modellingSpaceToWorldPtr ),
                      bb );
}

void
MinimalCullBoundsShadowMap::ViewData::GetRenderLeaves( osgUtil::RenderBin* rb,
                                                       RenderLeafList&     rll )
{
    osgUtil::RenderBin::RenderBinList&                bins = rb->getRenderBinList();
    osgUtil::RenderBin::RenderBinList::const_iterator rbitr;

    // scan pre render bins
    for( rbitr = bins.begin(); rbitr != bins.end() && rbitr->first < 0; ++rbitr )
    {
        GetRenderLeaves( rbitr->second.get(), rll );
    }

    // scan fine grained ordering.
    osgUtil::RenderBin::RenderLeafList& renderLeafList = rb->getRenderLeafList();
    osgUtil::RenderBin::RenderLeafList::const_iterator rlitr;
    for( rlitr = renderLeafList.begin(); rlitr != renderLeafList.end(); ++rlitr )
    {
        rll.push_back( *rlitr );
    }

    // scan coarse grained ordering.
    osgUtil::RenderBin::StateGraphList& stateGraphList = rb->getStateGraphList();
    osgUtil::RenderBin::StateGraphList::const_iterator oitr;
    for( oitr = stateGraphList.begin(); oitr != stateGraphList.end(); ++oitr )
    {
        for( osgUtil::StateGraph::LeafList::const_iterator dw_itr =
                 ( *oitr )->_leaves.begin();
             dw_itr != ( *oitr )->_leaves.end();
             ++dw_itr )
        {
            rll.push_back( dw_itr->get() );
        }
    }

    // scan post render bins
    for( ; rbitr != bins.end(); ++rbitr )
    {
        GetRenderLeaves( rbitr->second.get(), rll );
    }
}

class CompareRenderLeavesByMatrices
{
    public:

        bool
        operator()( const osgUtil::RenderLeaf* a,
                    const osgUtil::RenderLeaf* b )
        {
            if( !a )
            {
                return false;    // NULL render leaf goes last
            }
            return !b ||
                   a->_projection <
                   b->_projection ||
                   ( a->_projection == b->_projection && a->_modelview < b->_modelview );
        }
};

inline bool
CheckAndMultiplyBoxIfWithinPolytope( osg::box&      bb,
                                     osg::dmat4&    m,
                                     osg::Polytope& p )
{
    if( !bb.valid() )
    {
        return false;
    }

    osg::vec3 o = osg::vec3( bb.min * m ), s[3];

    for( std::size_t i = 0; i < 3; i++ )
    {
        s[i] = osg::vec3( static_cast<float>( m( i, 0 ) ),
                          static_cast<float>( m( i, 1 ) ),
                          static_cast<float>( m( i, 2 ) ) ) *
               ( bb.max[i] - bb.min[i] );
    }

    for( osg::Polytope::PlaneList::iterator it = p.getPlaneList().begin();
         it != p.getPlaneList().end();
         ++it )
    {
        float dist = static_cast<float>( it->distance( o ) ), dist_min = dist,
              dist_max = dist;
        ( void )dist_min;

        for( int i = 0; i < 3; i++ )
        {
            dist = static_cast<float>( it->dotProductNormal( s[i] ) );
            if( dist < 0 )
            {
                dist_min += dist;
            }
            else
            {
                dist_max += dist;
            }
        }

        if( dist_max < 0 )
        {
            return false;
        }
    }

    bb.max = bb.min = o;
#if 1
    for( std::size_t i = 0; i < 3; i++ )
    {
        for( std::size_t j = 0; j < 3; j++ )
        {
            if( s[i][j] < 0 )
            {
                bb.min[j] += s[i][j];
            }
            else
            {
                bb.max[j] += s[i][j];
            }
        }
    }
#else
    b.expandBy( o + s[0] );
    b.expandBy( o + s[1] );
    b.expandBy( o + s[2] );
    b.expandBy( o + s[0] + s[1] );
    b.expandBy( o + s[0] + s[2] );
    b.expandBy( o + s[1] + s[2] );
    b.expandBy( o + s[0] + s[1] + s[2] );
#endif

#if ( IGNORE_OBJECTS_LARGER_THAN_HEIGHT > 0 )
    if( bb.max[2] -
        bb.min[2] > IGNORE_OBJECTS_LARGER_THAN_HEIGHT )    // ignore huge objects
    {
        return false;
    }
#endif

    return true;
}

unsigned
MinimalCullBoundsShadowMap::ViewData::RemoveOldRenderLeaves( RenderLeafList& rllNew,
                                                             RenderLeafList& rllOld )
{
    unsigned count = 0;

    std::sort( rllOld.begin(), rllOld.end() );
    RenderLeafList::iterator itNew, itOld;
    for( itNew = rllNew.begin(); itNew != rllNew.end() && rllOld.size(); ++itNew )
    {
        itOld = std::lower_bound( rllOld.begin(), rllOld.end(), *itNew );

        if( itOld == rllOld.end() || *itOld != *itNew )
        {
            continue;
        }
        // found !
        rllOld.erase( itOld );    // remove it from old range to speed up search
        *itNew = NULL;            // its not new = invalidate it among new render leaves
        count++;
    }

    return count;
}

unsigned
MinimalCullBoundsShadowMap::ViewData::RemoveIgnoredRenderLeaves( RenderLeafList& rll )
{
    unsigned count = 0;

    for( RenderLeafList::iterator it = rll.begin(); it != rll.end(); ++it )
    {
        if( !*it )
        {
            continue;
        }

        const char* name = ( *it )->_drawable->className();

        // Its a dirty but quick test (not very future proof)
        if( !name || name[0] != 'L' )
        {
            continue;
        }

        if( !strcmp( name, "LightPointDrawable" ) ||
            !strcmp( name, "LightPointSpriteDrawable" ) )
        {
            *it = NULL;    // ignored = invalidate this in new render leaves list
            count++;
        }
    }

    return count;
}

osg::box
MinimalCullBoundsShadowMap::ViewData::ComputeRenderLeavesBounds(
    RenderLeafList& rll,
    osg::dmat4&     projectionToWorld
)
{
    osg::box bbResult;

    if( rll.size() == 0 )
    {
        return bbResult;
    }

    std::sort( rll.begin(), rll.end(), CompareRenderLeavesByMatrices() );

    osg::ref_ptr<osg::RefMatrix> modelview;
    osg::ref_ptr<osg::RefMatrix> projection;
    osg::dmat4                   viewToWorld, modelToWorld,
        *ptrProjection = NULL, *ptrViewToWorld = &projectionToWorld, *ptrModelToWorld;

    osg::box bb;

    // compute bounding boxes but skip old ones (placed at the end as NULLs)
    for( RenderLeafList::iterator it = rll.begin();; ++it )
    {
        // we actually allow to pass one element behind end to flush bb queue
        osgUtil::RenderLeaf* rl = ( it != rll.end() ? *it : NULL );

        // Don't trust already computed bounds for cull generated drawables
        // LightPointDrawable & LightPointSpriteDrawable are such examples
        // they store wrong recorded bounds from very first pass
        if( rl && rl->_modelview == NULL )
        {
            rl->_drawable->dirtyBound();
        }

        // Stay as long as possible in model space to minimize matrix ops
        if( rl && rl->_modelview == modelview && rl->_projection == projection )
        {
            bb.expandBy( rl->_drawable->getBoundingBox() );
        }
        else
        {
            if( bb.valid() )
            {
                // Conditions to avoid matrix multiplications
                if( projection.valid() )
                {
                    if( projection.get() != ptrProjection )
                    {
                        ptrProjection = projection.get();
                        viewToWorld   = *ptrProjection * projectionToWorld;
                    }
                    ptrViewToWorld = &viewToWorld;
                }
                else
                {
                    ptrViewToWorld = &projectionToWorld;
                }

                if( modelview.valid() )
                {
                    modelToWorld    = *modelview.get() * *ptrViewToWorld;
                    ptrModelToWorld = &modelToWorld;
                }
                else
                {
                    ptrModelToWorld = ptrViewToWorld;
                }

                for( int i = 0; i < 8; i++ )
                {
                    bbResult.expandBy( bb.corner( static_cast<unsigned int>( i ) ) *
                                       *ptrModelToWorld );
                }
            }
            if( !rl )
            {
                break;
            }
            bb         = rl->_drawable->getBoundingBox();
            modelview  = rl->_modelview;
            projection = rl->_projection;
        }
    }

    rll.clear();

    return bbResult;
}

osg::box
MinimalCullBoundsShadowMap::ViewData::ComputeRenderLeavesBounds(
    RenderLeafList& rll,
    osg::dmat4&     projectionToWorld,
    osg::Polytope&  p
)
{
    osg::box bbResult, bb;

    if( rll.size() == 0 )
    {
        return bbResult;
    }

    std::sort( rll.begin(), rll.end(), CompareRenderLeavesByMatrices() );

    osg::ref_ptr<osg::RefMatrix> modelview;
    osg::ref_ptr<osg::RefMatrix> projection;
    osg::dmat4                   viewToWorld, modelToWorld, *ptrProjection = NULL,
                                                            *ptrViewToWorld = &projectionToWorld,
                                                            *ptrModelToWorld = NULL;

    // compute bounding boxes but skip old ones (placed at the end as NULLs)
    for( RenderLeafList::iterator it = rll.begin(); it != rll.end(); ++it )
    {
        // we actually allow to pass one element behind end to flush bb queue
        osgUtil::RenderLeaf* rl = *it;
        if( !rl )
        {
            break;
        }

        // Don't trust already computed bounds for cull generated drawables
        // LightPointDrawable & LightPointSpriteDrawable are such examples
        // they store wrong recorded bounds from very first pass
        if( rl->_modelview == NULL )
        {
            rl->_drawable->dirtyBound();
        }

        bb = rl->_drawable->getBoundingBox();
        if( !bb.valid() )
        {
            continue;
        }

        // Stay as long as possible in model space to minimize matrix ops
        if( rl->_modelview != modelview || rl->_projection != projection )
        {
            projection = rl->_projection;
            if( projection.valid() )
            {
                if( projection.get() != ptrProjection )
                {
                    ptrProjection = projection.get();
                    viewToWorld   = *ptrProjection * projectionToWorld;
                }
                ptrViewToWorld = &viewToWorld;
            }
            else
            {
                ptrViewToWorld = &projectionToWorld;
            }

            modelview = rl->_modelview;
            if( modelview.valid() )
            {
                modelToWorld    = *modelview.get() * *ptrViewToWorld;
                ptrModelToWorld = &modelToWorld;
            }
            else
            {
                ptrModelToWorld = ptrViewToWorld;
            }
        }

        if( ptrModelToWorld &&
            CheckAndMultiplyBoxIfWithinPolytope( bb, *ptrModelToWorld, p ) )
        {
            bbResult.expandBy( bb );
        }
    }

    rll.clear();

    return bbResult;
}
