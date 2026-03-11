/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Shadow map technique that minimizes the shadow frustum.
 * Tightens the light projection to the visible scene.
 */
#include <osgShadow/MinimalShadowMap>

#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/traversal/ComputeBoundsVisitor.hpp>
#include <osgShadow/ConvexPolyhedron>
#include <osgShadow/ShadowedScene>

using namespace osgShadow;

#define PRINT_SHADOW_TEXEL_TO_PIXEL_ERROR 0

MinimalShadowMap::MinimalShadowMap() :

    _maxFarPlane( FLT_MAX ),
    _minLightMargin( 0 ),
    _shadowReceivingCoarseBoundAccuracy( BOUNDING_BOX )
{
}

MinimalShadowMap::MinimalShadowMap( const MinimalShadowMap& copy,
                                    const osg::CopyOp&      copyop ) :
    Inherit( copy,
             copyop ),
    _maxFarPlane( copy._maxFarPlane ),
    _minLightMargin( copy._minLightMargin ),
    _shadowReceivingCoarseBoundAccuracy( copy._shadowReceivingCoarseBoundAccuracy )
{
}

MinimalShadowMap::~MinimalShadowMap()
{
}

osg::box
MinimalShadowMap::ViewData::computeShadowReceivingCoarseBounds()
{
    // Default slowest but most precise
    ShadowReceivingCoarseBoundAccuracy accuracy = DEFAULT_ACCURACY;

    MinimalShadowMap* msm = dynamic_cast<MinimalShadowMap*>( _st.get() );
    if( msm )
    {
        accuracy = msm->getShadowReceivingCoarseBoundAccuracy();
    }

    if( accuracy == MinimalShadowMap::EMPTY_BOX )
    {
        // One may skip coarse scene bounds computation if light is infinite.
        // Empty box will be intersected with view frustum so in the end
        // view frustum will be used as bounds approximation.
        // But if light is nondirectional and bounds come out too large
        // they may bring the effect of almost 180 deg perspective set
        // up for shadow camera. Such projection will significantly impact
        // precision of further math.

        return osg::box();
    }

    if( accuracy == MinimalShadowMap::BOUNDING_SPHERE )
    {
        // faster but less precise rough scene bound computation
        // however if compute near far is active it may bring quite good result
        osg::Camera*     camera = _cv->getRenderStage()->getCamera();
        osg::dmat4       m      = camera->getViewMatrix() * _clampedProjection;

        ConvexPolyhedron frustum;
        frustum.setToUnitFrustum();
        frustum.transform( osg::inverse( m ), m );

        osg::sphere bs = _st->getShadowedScene()->getBound();
        osg::box    bb;
        bb.expandBy( bs );
        osg::Polytope box;
        box.setToBoundingBox( bb );

        frustum.cut( box );

        // approximate sphere with octahedron. Ie first cut by box then
        // additionally cut with the same box rotated 45, 45, 45 deg.
        box.transform(    // rotate box around its center
            osg::translate( -bs.center ) *
            osg::rotate( osg::PI_4, 0.0, 0.0, 1.0 ) *
            osg::rotate( osg::PI_4, 1.0, 1.0, 0.0 ) *
            osg::translate( bs.center )
        );
        frustum.cut( box );

        return frustum.computeBoundingBox();
    }

    if( accuracy == MinimalShadowMap::BOUNDING_BOX )    // Default
    {
        // more precise method but slower method
        // bound visitor traversal takes lot of time for complex scenes
        // (note that this adds to cull time)

        osg::ComputeBoundsVisitor cbbv( osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN );
        cbbv.setTraversalMask( _st->getShadowedScene()->getCastsShadowTraversalMask() );
        _st->getShadowedScene()->osg::Group::traverse( cbbv );

        return cbbv.getBoundingBox();
    }

    return osg::box();
}

void
MinimalShadowMap::ViewData::
    aimShadowCastingCamera( const osg::sphere& bs,
                            const osg::Light*  light,
                            const osg::vec4&   lightPos,
                            const osg::vec3&   lightDir,
                            const osg::vec3&   lightUpVector
                            /* by default = osg::vec3( 0, 1 0 )*/ )
{
    BaseClass::ViewData::aimShadowCastingCamera( bs,
                                                 light,
                                                 lightPos,
                                                 lightDir,
                                                 lightUpVector );
}

void
MinimalShadowMap::ViewData::aimShadowCastingCamera( const osg::Light* light,
                                                    const osg::vec4&  lightPos,
                                                    const osg::vec3&  lightDir,
                                                    const osg::vec3&  lightUp )
{
    osg::box bb = computeScenePolytopeBounds();
    if( !bb.valid() )
    {    // empty scene or looking at the sky - substitute something
        bb.expandBy( osg::sphere( _cv->getEyePoint(), 1 ) );
    }

    osg::vec3 up = lightUp;

    if( osg::length2( up ) <= 0 )
    {
        // This is extra step (not really needed but helpful in debugging)
        // Compute such lightUp vector that shadow cam is intuitively aligned with eye
        // We compute this vector on -ZY view plane, perpendicular to light direction
        // dmat4 m = ViewToWorld
#if 0
        osg::dmat4 m = osg::inverse( _cv->getModelViewMatrix() );
        osg::vec3 camFw( -m( 2, 0 ), -m( 2, 1 ), -m( 2, 2 ) );
        camFw = osg::normalize(camFw);

        osg::vec3 camUp( m( 1, 0 ), m( 1, 1 ), m( 1, 2 ) );
        camUp = osg::normalize(camUp);

        up = camUp * ( camFw * lightDir ) - camFw * ( camUp * lightDir );
        up = osg::normalize(up);
#else
        osg::dmat4 m = osg::inverse( *_cv->getModelViewMatrix() );
        // OpenGL std cam looks along -Z axis so Cam Fw = [ 0  0  -1  0 ] * m
        up.set( static_cast<float>( -m( 2, 0 ) ),
                static_cast<float>( -m( 2, 1 ) ),
                static_cast<float>( -m( 2, 2 ) ) );
#endif
    }

    aimShadowCastingCamera( osg::sphere( bb ), light, lightPos, lightDir, up );

    // Intersect scene Receiving Shadow Polytope with shadow camera frustum
    // Important for cases where Scene extend beyond shadow camera frustum
    // From this moment shadowed scene portion is fully contained by both
    // main camera frustum and shadow camera frustum
    osg::dmat4 mvp = _camera->getProjectionMatrix() * _camera->getViewMatrix();
    cutScenePolytope( osg::inverse( mvp ), mvp );

    frameShadowCastingCamera( _cv->getRenderStage()->getCamera(), _camera.get(), 0 );
}

void
MinimalShadowMap::ViewData::frameShadowCastingCamera( const osg::Camera* cameraMain,
                                                      osg::Camera*       cameraShadow,
                                                      int                pass )
{
    osg::dmat4 mvp = cameraShadow->getProjectionMatrix() * cameraShadow->getViewMatrix();

    ConvexPolyhedron        polytope = _sceneReceivingShadowPolytope;
    std::vector<osg::dvec3> points   = _sceneReceivingShadowPolytopePoints;

    osg::box                bb       = computeScenePolytopeBounds( mvp );

    // projection was trimmed above, need to recompute mvp
    if( bb.valid() && *_minLightMarginPtr > 0 )
    {
        // bb.max += osg::vec3( 1, 1, 1 );
        // bb.min -= osg::vec3( 1, 1, 1 );

        osg::dmat4 transform = osg::inverse( mvp );

        // Code below was working only for directional lights ie when projection was
        // ortho osg::dvec3 normal = osg::transform3x3( osg::dvec3( 0,0,-1)., transform
        // );

        // So I replaced it with safer code working with spot lights as well
        osg::dvec3 normal =
            osg::dvec3( 0, 0, -1 ) * transform - osg::dvec3( 0, 0, 1 ) * transform;

        normal = osg::normalize( normal );
        _sceneReceivingShadowPolytope.extrude( normal * *_minLightMarginPtr );

        // Zero pass does crude shadowed scene hull approximation.
        // Its important to cut it to coarse light frustum properly
        // at this stage.
        // If not cut and polytope extends beyond shadow projection clip
        // space (-1..1), it may get "twisted" by precisely adjusted shadow cam
        // projection in second pass.

        if( pass == 0 && _frameShadowCastingCameraPasses > 1 )
        {    // Make sure extruded polytope does not extend beyond light frustum
            osg::Polytope lightFrustum;
            lightFrustum.setToUnitFrustum();
            lightFrustum.transformProvidingInverse( mvp );
            _sceneReceivingShadowPolytope.cut( lightFrustum );
        }

        _sceneReceivingShadowPolytopePoints.clear();
        _sceneReceivingShadowPolytope.getPoints( _sceneReceivingShadowPolytopePoints );

        bb = computeScenePolytopeBounds( mvp );
    }

    setDebugPolytope( "extended",
                      _sceneReceivingShadowPolytope,
                      osg::vec4( 1, 0.5, 0, 1 ),
                      osg::vec4( 1, 0.5F, 0, 0.1F ) );

    _sceneReceivingShadowPolytope       = polytope;
    _sceneReceivingShadowPolytopePoints = points;

    // Warning: Trim light projection at near plane may remove shadowing
    // from objects outside of view space but still casting shadows into it.
    // I have not noticed this issue so I left mask at default: all bits set.
    if( bb.valid() )
    {
        trimProjection( cameraShadow->getProjectionMatrix(),
                        bb,
                        1 | 2 | 4 | 8 | 16 | 32 );
    }

    ///// Debugging stuff //////////////////////////////////////////////////////////
    setDebugPolytope( "scene", _sceneReceivingShadowPolytope, osg::vec4( 0, 1, 0, 1 ) );

#if PRINT_SHADOW_TEXEL_TO_PIXEL_ERROR
    if( pass == 1 )
    {
        displayShadowTexelToPixelErrors( cameraMain,
                                         cameraShadow,
                                         &_sceneReceivingShadowPolytope );
    }
#endif

    if( pass == _frameShadowCastingCameraPasses - 1 )
    {
#if 1
        {
            osg::dmat4 debug_mvp =
                cameraShadow->getProjectionMatrix() * cameraShadow->getViewMatrix();
            ConvexPolyhedron frustum;
            frustum.setToUnitFrustum();
            frustum.transform( osg::inverse( debug_mvp ), debug_mvp );

            setDebugPolytope( "shadowCamFrustum", frustum, osg::vec4( 0, 0, 1, 1 ) );
        }

        {
            osg::dmat4 debug_mvp =
                cameraMain->getProjectionMatrix() * cameraMain->getViewMatrix();
            ConvexPolyhedron frustum;
            frustum.setToUnitFrustum();
            frustum.transform( osg::inverse( debug_mvp ), debug_mvp );

            setDebugPolytope( "mainCamFrustum", frustum, osg::vec4( 1, 1, 1, 1 ) );
        }
#endif
        std::string* filename = getDebugDump();
        if( filename && !filename->empty() )
        {
            dump( *filename );
            filename->clear();
        }
    }
}

void
MinimalShadowMap::ViewData::cullShadowReceivingScene()
{
    BaseClass::ViewData::cullShadowReceivingScene();

    _clampedProjection = *_cv->getProjectionMatrix();

    if( _cv->getComputeNearFarMode() )
    {

        // Redo steps from CullVisitor::popProjectionMatrix()
        // which clamps projection matrix when Camera & Projection
        // completes traversal of their children

        // We have to do this now manually
        // because we did not complete camera traversal yet but
        // we need to know how this clamped projection matrix will be

        _cv->computeNearPlane();

        osgUtil::CullVisitor::value_type n = _cv->getCalculatedNearPlane();
        osgUtil::CullVisitor::value_type f = _cv->getCalculatedFarPlane();

        if( n < f )
        {
            _cv->clampProjectionMatrix( _clampedProjection, n, f );
        }
    }

    // Additionally clamp far plane if shadows don't need to be cast as
    // far as main projection far plane
    if( 0 < *_maxFarPlanePtr )
    {
        clampProjection( _clampedProjection, 0.F, *_maxFarPlanePtr );
    }

    // Give derived classes chance to initialize _sceneReceivingShadowPolytope
    osg::box bb = computeShadowReceivingCoarseBounds();
    if( bb.valid() )
    {
        _sceneReceivingShadowPolytope.setToBoundingBox( bb );
    }
    else
    {
        _sceneReceivingShadowPolytope.clear();
    }

    // Cut initial scene using main camera frustum.
    // Cutting will work correctly on empty polytope too.
    // Take into consideration near far calculation and _maxFarPlane variable

    osg::dmat4 mvp = *_cv->getModelViewMatrix() * _clampedProjection;

    cutScenePolytope( osg::inverse( mvp ), mvp );

    setDebugPolytope( "frustum",
                      _sceneReceivingShadowPolytope,
                      osg::vec4( 1, 0, 1, 1 ) );
}

void
MinimalShadowMap::ViewData::init( ThisClass*            st,
                                  osgUtil::CullVisitor* cv )
{
    BaseClass::ViewData::init( st, cv );

    _modellingSpaceToWorldPtr       = &st->_modellingSpaceToWorld;
    _minLightMarginPtr              = &st->_minLightMargin;
    _maxFarPlanePtr                 = &st->_maxFarPlane;

    _frameShadowCastingCameraPasses = 1;
}

void
MinimalShadowMap::ViewData::cutScenePolytope( const osg::dmat4& /*transform*/,
                                              const osg::dmat4& inverse,
                                              const osg::box&   bb )
{
    _sceneReceivingShadowPolytopePoints.clear();

    if( bb.valid() )
    {
        osg::Polytope polytope;
        polytope.setToBoundingBox( bb );
        polytope.transformProvidingInverse( inverse );
        _sceneReceivingShadowPolytope.cut( polytope );
        _sceneReceivingShadowPolytope.getPoints( _sceneReceivingShadowPolytopePoints );
    }
    else
    {
        _sceneReceivingShadowPolytope.clear();
    }
}

osg::box
MinimalShadowMap::ViewData::computeScenePolytopeBounds()
{
    osg::box bb;

    for( unsigned i = 0; i < _sceneReceivingShadowPolytopePoints.size(); ++i )
    {
        bb.expandBy( _sceneReceivingShadowPolytopePoints[i] );
    }

    return bb;
}

osg::box
MinimalShadowMap::ViewData::computeScenePolytopeBounds( const osg::dmat4& m )
{
    osg::box bb;

    for( unsigned i = 0; i < _sceneReceivingShadowPolytopePoints.size(); ++i )
    {
        bb.expandBy( _sceneReceivingShadowPolytopePoints[i] * m );
    }

    return bb;
}

// Utility methods for adjusting projection matrices

void
MinimalShadowMap::ViewData::trimProjection( osg::dmat4&  projectionMatrix,
                                            osg::box     bb,
                                            unsigned int trimMask )
{
#if 1
    if( !bb.valid() || !( trimMask & ( 1 | 2 | 4 | 8 | 16 | 32 ) ) )
    {
        return;
    }
    double l = -1, r = 1, b = -1, t = 1, n = 1, f = -1;

    #if 0
    // make sure bounding box does not extend beyond unit frustum clip range
    for( int i = 0; i < 3; i ++ ) {
        if( bb.min[i] < -1 ) bb.min[i] = -1;
        if( bb.max[i] >  1 ) bb.max[i] =  1;
    }
    #endif

    if( trimMask & 1 )
    {
        l = bb.min[0];
    }
    if( trimMask & 2 )
    {
        r = bb.max[0];
    }
    if( trimMask & 4 )
    {
        b = bb.min[1];
    }
    if( trimMask & 8 )
    {
        t = bb.max[1];
    }
    if( trimMask & 16 )
    {
        n = -bb.min[2];
    }
    if( trimMask & 32 )
    {
        f = -bb.max[2];
    }

    projectionMatrix = projectionMatrix * osg::ortho( l, r, b, t, n, f );
#else
    if( !bb.valid() || !( trimMask & ( 1 | 2 | 4 | 8 | 16 | 32 ) ) )
    {
        return;
    }
    double l, r, t, b, n, f;
    bool   ortho = osg::getOrtho( projectionMatrix, l, r, b, t, n, f );
    if( !ortho && !osg::getFrustum( projectionMatrix, l, r, b, t, n, f ) )
    {
        return;    // rotated or skewed or other crooked projection - give up
    }

    // make sure bounding box does not extend beyond unit frustum clip range
    for( int i = 0; i < 3; i++ )
    {
        if( bb.min[i] < -1 )
        {
            bb.min[i] = -1;
        }
        if( bb.max[i] > 1 )
        {
            bb.max[i] = 1;
        }
    }

    osg::dmat4 projectionToView = osg::inverse( projectionMatrix );

    osg::vec3  min = osg::vec3( bb.min[0], bb.min[1], bb.min[2] ) * projectionToView;

    osg::vec3  max = osg::vec3( bb.max[0], bb.max[1], bb.max[2] ) * projectionToView;

    if( trimMask & 16 )
    {        // trim near
        if( !ortho )
        {    // recalc frustum corners on new near plane
            l *= -min[2] / n;
            r *= -min[2] / n;
            b *= -min[2] / n;
            t *= -min[2] / n;
        }
        n = -min[2];
    }

    if( trimMask & 32 )    // trim far
    {
        f = -max[2];
    }

    if( !ortho )
    {
        min[0] *= -n / min[2];
        min[1] *= -n / min[2];
        max[0] *= -n / max[2];
        max[1] *= -n / max[2];
    }

    if( l < r )
    {    // check for inverted X range
        if( l < min[0] && ( trimMask & 1 ) )
        {
            l = min[0];
        }
        if( r > max[0] && ( trimMask & 2 ) )
        {
            r = max[0];
        }
    }
    else
    {
        if( l > min[0] && ( trimMask & 1 ) )
        {
            l = min[0];
        }
        if( r < max[0] && ( trimMask & 2 ) )
        {
            r = max[0];
        }
    }

    if( b < t )
    {    // check for inverted Y range
        if( b < min[1] && ( trimMask & 4 ) )
        {
            b = min[1];
        }
        if( t > max[1] && ( trimMask & 8 ) )
        {
            t = max[1];
        }
    }
    else
    {
        if( b > min[1] && ( trimMask & 4 ) )
        {
            b = min[1];
        }
        if( t < max[1] && ( trimMask & 8 ) )
        {
            t = max[1];
        }
    }

    if( ortho )
    {
        projectionMatrix = osg::orthographic( l, r, b, t, n, f );
    }
    else
    {
        projectionMatrix = osg::frustum( l, r, b, t, n, f );
    }
#endif
}

void
MinimalShadowMap::ViewData::clampProjection( osg::dmat4& projection,
                                             float       new_near,
                                             float       new_far )
{
    double r, l, t, b, n, f;
    bool   perspective = osg::getFrustum( projection, l, r, b, t, n, f );
    if( !perspective && !osg::getOrtho( projection, l, r, b, t, n, f ) )
    {
        // What to do here ?
        OSG_WARN
            << "MinimalShadowMap::clampProjectionFarPlane failed - non standard matrix"
            << std::endl;
    }
    else if( n < new_near || new_far < f )
    {

        if( n < new_near && new_near < f )
        {
            if( perspective )
            {
                l *= new_near / n;
                r *= new_near / n;
                b *= new_near / n;
                t *= new_near / n;
            }
            n = new_near;
        }

        if( n < new_far && new_far < f )
        {
            f = new_far;
        }

        if( perspective )
        {
            projection = osg::frustum( l, r, b, t, n, f );
        }
        else
        {
            projection = osg::orthographic( l, r, b, t, n, f );
        }
    }
}

// Imagine following scenario:
// We stand in the room and look through the window.
// How should our view change if we were looking through larger window ?
// In other words how should projection be adjusted if
// window had grown by some margin ?
// Method computes such new projection which maintains perpective/world ratio

void
MinimalShadowMap::ViewData::extendProjection( osg::dmat4&      projection,
                                              osg::Viewport*   viewport,
                                              const osg::vec2& margin )
{
    double l, r, b, t, n, f;

    // osg::dmat4 projection = camera.getProjectionMatrix();

    bool   frustum = osg::getFrustum( projection, l, r, b, t, n, f );

    if( !frustum && !osg::getOrtho( projection, l, r, b, t, n, f ) )
    {
        OSG_WARN << " Awkward projection matrix. ComputeExtendedProjection failed"
                 << std::endl;
        return;
    }

    osg::dmat4 window = viewport->computeWindowMatrix();

    osg::vec3  vMin( static_cast<float>( viewport->x() ) - margin.x,
                     static_cast<float>( viewport->y() ) - margin.y,
                     0.0F );

    osg::vec3  vMax( static_cast<float>( viewport->width() ) + margin.x * 2 + vMin.x,
                     static_cast<float>( viewport->height() ) + margin.y * 2 + vMin.y,
                     0.0F );

    osg::dmat4 inversePW = osg::inverse( projection * window );

    vMin                 = vMin * inversePW;
    vMax                 = vMax * inversePW;

    l                    = vMin.x;
    r                    = vMax.x;
    b                    = vMin.y;
    t                    = vMax.y;

    if( frustum )
    {
        projection = osg::frustum( l, r, b, t, n, f );
    }
    else
    {
        projection = osg::orthographic( l, r, b, t, n, f );
    }
}
