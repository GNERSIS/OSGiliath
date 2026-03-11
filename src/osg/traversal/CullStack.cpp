/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stack-based traversal state for culling. Tracks modelview, projection,
 * viewport, and frustum through the scene graph hierarchy.
 */
#include <osg/traversal/CullStack.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/Timer.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>

using namespace osg;

CullStack::CullStack()
{
    _frustumVolume               = -1.0F;
    _bbCornerNear                = 0;
    _bbCornerFar                 = 7;
    _currentReuseMatrixIndex     = 0;
    _identity                    = new RefMatrix();

    _index_modelviewCullingStack = 0;
    _back_modelviewCullingStack  = 0;

    _referenceViewPoints.push_back( osg::vec3( 0.0F, 0.0F, 0.0F ) );
}

CullStack::CullStack( const CullStack& cs ) :
    CullSettings( cs )
{
    _frustumVolume               = -1.0F;
    _bbCornerNear                = 0;
    _bbCornerFar                 = 7;
    _currentReuseMatrixIndex     = 0;
    _identity                    = new RefMatrix();

    _index_modelviewCullingStack = 0;
    _back_modelviewCullingStack  = 0;

    _referenceViewPoints.push_back( osg::vec3( 0.0F, 0.0F, 0.0F ) );
}

CullStack::~CullStack()
{
    reset();
}

void
CullStack::reset()
{

    //
    // first unref all referenced objects and then empty the containers.
    //
    _projectionStack.clear();
    _modelviewStack.clear();
    _viewportStack.clear();

    _referenceViewPoints.clear();
    _referenceViewPoints.push_back( osg::vec3( 0.0F, 0.0F, 0.0F ) );

    _eyePointStack.clear();
    _viewPointStack.clear();

    _clipspaceCullingStack.clear();
    _projectionCullingStack.clear();

    //_modelviewCullingStack.clear();
    _index_modelviewCullingStack = 0;
    _back_modelviewCullingStack  = 0;

    osg::vec3 lookVector( 0.0, 0.0, -1.0 );

    _bbCornerFar             = ( lookVector.x >= 0 ? 1 : 0 ) |
                               ( lookVector.y >= 0 ? 2 : 0 ) |
                               ( lookVector.z >= 0 ? 4 : 0 );

    _bbCornerNear            = ( ~_bbCornerFar ) & 7;

    _currentReuseMatrixIndex = 0;
}

void
CullStack::pushCullingSet()
{
    _MVPW_Stack.push_back( 0L );

    if( _index_modelviewCullingStack == 0 )
    {
        if( _modelviewCullingStack.empty() )
        {
            _modelviewCullingStack.push_back( CullingSet() );
        }

        _modelviewCullingStack[_index_modelviewCullingStack++].set(
            _projectionCullingStack.back()
        );
    }
    else
    {

        const osg::Viewport& W    = *_viewportStack.back();
        const osg::dmat4&    P    = *_projectionStack.back();
        const osg::dmat4&    M    = *_modelviewStack.back();

        osg::vec4 pixelSizeVector = CullingSet::computePixelSizeVector( W, P, M );

        if( _index_modelviewCullingStack >= _modelviewCullingStack.size() )
        {
            _modelviewCullingStack.push_back( CullingSet() );
        }

        _modelviewCullingStack[_index_modelviewCullingStack++].set(
            _projectionCullingStack.back(),
            *_modelviewStack.back(),
            pixelSizeVector
        );
    }

    _back_modelviewCullingStack =
        &_modelviewCullingStack[_index_modelviewCullingStack - 1];

    // const osg::Polytope& polytope = _modelviewCullingStack.back()->getFrustum();
    // const osg::Polytope::PlaneList& pl = polytope.getPlaneList();
    // std::cout <<"new cull stack"<<std::endl;
    // for(osg::Polytope::PlaneList::const_iterator pl_itr=pl.begin();
    //     pl_itr!=pl.end();
    //     ++pl_itr)
    // {
    //     std::cout << "    plane "<<*pl_itr<<std::endl;
    // }
}

void
CullStack::popCullingSet()
{
    _MVPW_Stack.pop_back();

    --_index_modelviewCullingStack;
    if( _index_modelviewCullingStack > 0 )
    {
        _back_modelviewCullingStack =
            &_modelviewCullingStack[_index_modelviewCullingStack - 1];
    }
}

void
CullStack::pushViewport( osg::Viewport* viewport )
{
    _viewportStack.push_back( viewport );
    _MVPW_Stack.push_back( 0L );
}

void
CullStack::popViewport()
{
    _viewportStack.pop_back();
    _MVPW_Stack.pop_back();
}

void
CullStack::pushProjectionMatrix( RefMatrix* matrix )
{
    _projectionStack.push_back( matrix );

    _projectionCullingStack.push_back( osg::CullingSet() );
    osg::CullingSet& cullingSet = _projectionCullingStack.back();

    // set up view frustum.
    cullingSet.getFrustum().setToUnitFrustum( ( _cullingMode & NEAR_PLANE_CULLING ) != 0,
                                              ( _cullingMode & FAR_PLANE_CULLING ) !=
                                                  0 );
    cullingSet.getFrustum().transformProvidingInverse( *matrix );

    // set the culling mask ( There should be a more elegant way!)  Nikolaus H.
    cullingSet.setCullingMask( _cullingMode );

    // set the small feature culling.
    cullingSet.setSmallFeatureCullingPixelSize( _smallFeatureCullingPixelSize );

    // set up the relevant occluders which a related to this projection.
    for( ShadowVolumeOccluderList::iterator itr = _occluderList.begin();
         itr != _occluderList.end();
         ++itr )
    {
        // std::cout << " ** testing occluder"<<std::endl;
        if( itr->matchProjectionMatrix( *matrix ) )
        {
            // std::cout << " ** activating occluder"<<std::endl;
            cullingSet.addOccluder( *itr );
        }
    }

    // need to recompute frustum volume.
    _frustumVolume = -1.0F;

    pushCullingSet();
}

void
CullStack::popProjectionMatrix()
{

    _projectionStack.pop_back();

    _projectionCullingStack.pop_back();

    // need to recompute frustum volume.
    _frustumVolume = -1.0F;

    popCullingSet();
}

void
CullStack::pushModelViewMatrix( RefMatrix*                matrix,
                                Transform::ReferenceFrame referenceFrame )
{
    osg::RefMatrix* originalModelView =
        _modelviewStack.empty() ? 0 : _modelviewStack.back().get();

    _modelviewStack.push_back( matrix );

    pushCullingSet();

    osg::dmat4 inv = osg::inverse( static_cast<const dmat4&>( *matrix ) );

    switch( referenceFrame )
    {
        case( Transform::RELATIVE_RF ) :
            _eyePointStack.push_back( osg::vec3( osg::getTrans( inv ) ) );
            _referenceViewPoints.push_back( getReferenceViewPoint() );
            _viewPointStack.push_back(
                osg::vec3( inv * osg::dvec3( getReferenceViewPoint() ) )
            );
            break;
        case( Transform::ABSOLUTE_RF ) :
            _eyePointStack.push_back( osg::vec3( osg::getTrans( inv ) ) );
            _referenceViewPoints.push_back( osg::vec3( 0.0, 0.0, 0.0 ) );
            _viewPointStack.push_back( _eyePointStack.back() );
            break;
        case( Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT ) :
            {
                _eyePointStack.push_back( osg::vec3( osg::getTrans( inv ) ) );

                osg::vec3 referenceViewPoint = getReferenceViewPoint();
                if( originalModelView )
                {
                    osg::dmat4 viewPointTransformMatrix =
                        osg::inverse( static_cast<const dmat4&>( *originalModelView ) );
                    viewPointTransformMatrix =
                        static_cast<const dmat4&>( *matrix ) * viewPointTransformMatrix;
                    referenceViewPoint = osg::vec3( viewPointTransformMatrix *
                                                    osg::dvec3( referenceViewPoint ) );
                }

                _referenceViewPoints.push_back( referenceViewPoint );
                _viewPointStack.push_back(
                    osg::vec3( inv * osg::dvec3( getReferenceViewPoint() ) )
                );
                break;
            }
    }

    osg::vec3 lookVector = getLookVectorLocal();

    _bbCornerFar         = ( lookVector.x >= 0 ? 1 : 0 ) |
                           ( lookVector.y >= 0 ? 2 : 0 ) |
                           ( lookVector.z >= 0 ? 4 : 0 );

    _bbCornerNear        = ( ~_bbCornerFar ) & 7;
}

void
CullStack::popModelViewMatrix()
{
    _modelviewStack.pop_back();

    _eyePointStack.pop_back();
    _referenceViewPoints.pop_back();
    _viewPointStack.pop_back();

    popCullingSet();

    osg::vec3 lookVector( 0.0F, 0.0F, -1.0F );
    if( !_modelviewStack.empty() )
    {
        lookVector = getLookVectorLocal();
    }
    _bbCornerFar  = ( lookVector.x >= 0 ? 1 : 0 ) |
                    ( lookVector.y >= 0 ? 2 : 0 ) |
                    ( lookVector.z >= 0 ? 4 : 0 );

    _bbCornerNear = ( ~_bbCornerFar ) & 7;
}

void
CullStack::computeFrustumVolume()
{
    osg::dmat4 invP =
        osg::inverse( static_cast<const dmat4&>( *getProjectionMatrix() ) );

    osg::vec3 f1   = osg::vec3( osg::vec3( -1, -1, -1 ) * invP );
    osg::vec3 f2   = osg::vec3( osg::vec3( -1, 1, -1 ) * invP );
    osg::vec3 f3   = osg::vec3( osg::vec3( 1, 1, -1 ) * invP );
    osg::vec3 f4   = osg::vec3( osg::vec3( 1, -1, -1 ) * invP );

    osg::vec3 b1   = osg::vec3( osg::vec3( -1, -1, 1 ) * invP );
    osg::vec3 b2   = osg::vec3( osg::vec3( -1, 1, 1 ) * invP );
    osg::vec3 b3   = osg::vec3( osg::vec3( 1, 1, 1 ) * invP );
    osg::vec3 b4   = osg::vec3( osg::vec3( 1, -1, 1 ) * invP );

    _frustumVolume = computeVolume( f1, f2, f3, b1, b2, b3 ) +
                     computeVolume( f2, f3, f4, b1, b3, b4 );
}
