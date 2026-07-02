/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Shadow map using draw-pass bounds for frustum optimization.
 * Tighter shadow projection from GPU-side draw results.
 */
#include <osgShadow/MinimalDrawBoundsShadowMap.hpp>

#include <iostream>
#include <osg/images/Image.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/rendering/FrameBufferObject.hpp>
#include <osg/state/BlendEquation.hpp>
#include <osg/state/Depth.hpp>
#include <osg/state/PolygonOffset.hpp>
#include <osgShadow/ConvexPolyhedron.hpp>
#include <osgShadow/ShadowedScene.hpp>
#include <osgUtil/culling/RenderLeaf.hpp>
#include <string.h>

#define ANALYSIS_DEPTH  1
#define USE_FLOAT_IMAGE 1

using namespace osgShadow;

MinimalDrawBoundsShadowMap::MinimalDrawBoundsShadowMap()
{
}

MinimalDrawBoundsShadowMap::MinimalDrawBoundsShadowMap(
    const MinimalDrawBoundsShadowMap& copy,
    const osg::CopyOp&                copyop
) :
    Inherit( copy,
             copyop )
{
}

MinimalDrawBoundsShadowMap::~MinimalDrawBoundsShadowMap()
{
}

void
MinimalDrawBoundsShadowMap::ViewData::resizeGLObjectBuffers( unsigned int maxSize )
{
    BaseClass::ViewData::resizeGLObjectBuffers( maxSize );

    _boundAnalysisTexture->resizeGLObjectBuffers( maxSize );
    _boundAnalysisCamera->resizeGLObjectBuffers( maxSize );
}

void
MinimalDrawBoundsShadowMap::ViewData::releaseGLObjects( osg::State* state ) const
{
    BaseClass::ViewData::releaseGLObjects( state );

    _boundAnalysisTexture->releaseGLObjects( state );
    _boundAnalysisCamera->releaseGLObjects( state );
}

void
MinimalDrawBoundsShadowMap::ViewData::cullShadowReceivingScene()
{
    BaseClass::ViewData::cullShadowReceivingScene();
    ThisClass::ViewData::cullBoundAnalysisScene();
}

void
MinimalDrawBoundsShadowMap::ViewData::cullBoundAnalysisScene()
{
    _boundAnalysisCamera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    _boundAnalysisCamera->setViewMatrix( *_cv->getModelViewMatrix() );
    _boundAnalysisCamera->setProjectionMatrix( _clampedProjection );

    osg::dmat4::value_type l, r, b, t, n, f;
    _boundAnalysisCamera->getProjectionMatrixAsFrustum( l, r, b, t, n, f );

    _mainCamera = _cv->getRenderStage()->getCamera();

    extendProjection( _boundAnalysisCamera->getProjectionMatrix(),
                      _boundAnalysisCamera->getViewport(),
                      osg::vec2( 2, 2 ) );

    // record the traversal mask on entry so we can reapply it later.
    unsigned int traversalMask = _cv->getTraversalMask();

    _cv->setTraversalMask( traversalMask &
                           _st->getShadowedScene()->getReceivesShadowTraversalMask() );

    // do RTT camera traversal
    _boundAnalysisCamera->accept( *_cv );

    // reapply the original traversal mask
    _cv->setTraversalMask( traversalMask );
}

void
MinimalDrawBoundsShadowMap::ViewData::createDebugHUD()
{
    // _hudSize[0] *= 2;
    _viewportSize[0] *= 2;
    _orthoSize[0]    *= 2;

    MinimalShadowMap::ViewData::createDebugHUD();

    osg::Camera* camera = _cameraDebugHUD.get();

    osg::Geode*  geode  = new osg::Geode;
    camera->addChild( geode );

    osg::Geometry* geometry = osg::createTexturedQuadGeometry(
        osg::vec3( _hudOrigin[0] + _hudSize[0], _hudOrigin[1], 0 ),
        osg::vec3( _hudSize[0], 0, 0 ),
        osg::vec3( 0, _hudSize[1], 0 )
    );

    geode->addDrawable( geometry );

    osg::StateSet* stateset = geometry->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes( 0,
                                           _boundAnalysisTexture.get(),
                                           osg::StateAttribute::ON );

#if ANALYSIS_DEPTH
    osg::Program* program = new osg::Program;
    program->addShader( _depthColorFragmentShader.get() );
    stateset->setAttribute( program );
    stateset->addUniform( new osg::Uniform( "texture", 0 ) );
#else

#endif
}

osg::box
MinimalDrawBoundsShadowMap::ViewData::scanImage( const osg::Image* image,
                                                 osg::dmat4        m )
{
    osg::box bb, bbProj;

    int      components =
        static_cast<int>( osg::Image::computeNumComponents( image->getPixelFormat() ) );

    if( image->getDataType() == GL_FLOAT )
    {
        float  scale = 255.F / 254.F;
        float* pf    = ( float* )image->data();
        for( int y = 0; y < image->t(); y++ )
        {
            float fY =
                ( 0.5F + static_cast<float>( y ) ) / static_cast<float>( image->t() );
            for( int x = 0; x < image->s(); x++ )
            {
                float fX = ( 0.5F + static_cast<float>( x ) ) /
                           static_cast<float>( image->s() );

                if( pf[0] < 1.0 )
                {
                    float fMinZ = pf[0] * scale;
                    bbProj.expandBy( osg::vec3( fX, fY, fMinZ ) );
                    bb.expandBy( osg::vec3( fX, fY, fMinZ ) * m );

                    if( components > 1 )
                    {
                        float fMaxZ = scale * ( 1.F - pf[1] );
                        bbProj.expandBy( osg::vec3( fX, fY, fMaxZ ) );
                        bb.expandBy( osg::vec3( fX, fY, fMaxZ ) * m );
                    }
                }

                pf += components;
            }
        }
    }
    else if( image->getDataType() == GL_UNSIGNED_BYTE )
    {

        unsigned char* pb    = ( unsigned char* )image->data();

        float          scale = 1.F / 254;
        for( int y = 0; y < image->t(); y++ )
        {
            float fY =
                ( 0.5F + static_cast<float>( y ) ) / static_cast<float>( image->t() );
            for( int x = 0; x < image->s(); x++ )
            {
                float fX = ( 0.5F + static_cast<float>( x ) ) /
                           static_cast<float>( image->s() );

                if( pb[0] < 255 )
                {
                    float fMinZ = scale * ( pb[0] - 0.5F );
                    fMinZ       = osg::clampTo( fMinZ, 0.F, 1.F );

                    bbProj.expandBy( osg::vec3( fX, fY, fMinZ ) );
                    bb.expandBy( osg::vec3( fX, fY, fMinZ ) * m );

                    if( components > 1 )
                    {
                        float fMaxZ = scale * ( 255 - pb[1] + 0.5F );
                        fMaxZ       = osg::clampTo( fMaxZ, 0.F, 1.F );

                        bbProj.expandBy( osg::vec3( fX, fY, fMaxZ ) );
                        bb.expandBy( osg::vec3( fX, fY, fMaxZ ) * m );
                    }
                }

                pb += components;
            }
        }
    }

    return bb;
}

void
MinimalDrawBoundsShadowMap::ViewData::performBoundAnalysis( const osg::Camera& camera )
{
    if( !_projection.valid() )
    {
        return;
    }

    osg::Camera::BufferAttachmentMap& bam =
        const_cast<osg::Camera&>( camera ).getBufferAttachmentMap();
#if ANALYSIS_DEPTH
    osg::Camera::Attachment& attachment = bam[osg::Camera::DEPTH_BUFFER];
#else
    osg::Camera::Attachment& attachment = bam[osg::Camera::COLOR_BUFFER];
#endif

    const osg::ref_ptr<osg::Image> image = attachment._image.get();
    if( !image.valid() )
    {
        return;
    }

    osg::dmat4 m = osg::inverse( *_modellingSpaceToWorldPtr *
                                 camera.getViewMatrix() *
                                 camera.getProjectionMatrix() );

    m*         osg::scale( osg::vec3( 2.F, 2.F, 2.F ) *
                           osg::translate( osg::vec3( -1.F, -1.F, -1.F ) ) );

    osg::box   bb = scanImage( image.get(), m );

    if( getDebugDraw() )
    {
        ConvexPolyhedron p;
        p.setToBoundingBox( bb );
        p.transform( *_modellingSpaceToWorldPtr,
                     osg::inverse( *_modellingSpaceToWorldPtr ) );

        setDebugPolytope( "scan",
                          p,
                          osg::vec4( 0, 0, 0, 1 ),
                          osg::vec4( 0, 0, 0, 0.1F ) );
    }

    cutScenePolytope( *_modellingSpaceToWorldPtr,
                      osg::inverse( *_modellingSpaceToWorldPtr ),
                      bb );

    frameShadowCastingCamera( _mainCamera.get(), _camera.get() );

    _projection->set( _camera->getProjectionMatrix() );

    updateDebugGeometry( _mainCamera.get(), _camera.get() );
}

void
MinimalDrawBoundsShadowMap::ViewData::recordShadowMapParams()
{
    const osgUtil::RenderStage* rs = _cv->getCurrentRenderBin()->getStage();

    setShadowCameraProjectionMatrixPtr( _cv->getProjectionMatrix() );

    if( !rs->getRenderBinList().empty() || rs->getBinNum() != 0 )
    {
    }
#if 0
    MinimalShadowMap::RenderLeafList rll;

    static unsigned pass = 0, c = 0;

    pass++;

    std::set< osg::ref_ptr< osg::RefMatrix > > projections;

    MinimalShadowMap::GetRenderLeaves( , rll );
    for( unsigned i =0; i < rll.size(); i++ ) {
        if( rll[i]->_projection.get() != _projection.get() ) {
            osg::RefMatrix * projection = rll[i]->_projection.get();
            projections.insert( rll[i]->_projection );
            c++;
        }
    }

    if( projections.size() > 0 )
        _projection = (*projections.begin()).get();


    c = 0;
#endif
}

void
MinimalDrawBoundsShadowMap::ViewData::init( ThisClass*            st,
                                            osgUtil::CullVisitor* cv )
{
    BaseClass::ViewData::init( st, cv );

    _frameShadowCastingCameraPasses = 2;
    _camera->setCullCallback( new CameraCullCallback( this,
                                                      _camera->getCullCallback() ) );

    _boundAnalysisTexture = new osg::Texture2D;
    _boundAnalysisTexture->setTextureSize( _boundAnalysisSize[0],
                                           _boundAnalysisSize[1] );

    _boundAnalysisImage = new osg::Image;

#if ANALYSIS_DEPTH
    _boundAnalysisImage->allocateImage( _boundAnalysisSize[0],
                                        _boundAnalysisSize[1],
                                        1,
                                        GL_DEPTH_COMPONENT,
                                        GL_FLOAT );

    _boundAnalysisTexture->setInternalFormat( GL_DEPTH_COMPONENT );
    // _boundAnalysisTexture->setShadowComparison(true);
    _boundAnalysisTexture->setShadowTextureMode( osg::Texture2D::LUMINANCE );

    _boundAnalysisImage->setInternalTextureFormat( GL_DEPTH_COMPONENT );
    _boundAnalysisTexture->setInternalFormat( GL_DEPTH_COMPONENT );
#else

    #if USE_FLOAT_IMAGE
    _boundAnalysisImage->allocateImage( _boundAnalysisSize[0],
                                        _boundAnalysisSize[1],
                                        1,
                                        GL_RGBA,
                                        GL_FLOAT );

    _boundAnalysisImage->setInternalTextureFormat( GL_RGBA16F_ARB );
    _boundAnalysisTexture->setInternalFormat( GL_RGBA16F_ARB );
    #else
    _boundAnalysisImage->allocateImage( _boundAnalysisSize[0],
                                        _boundAnalysisSize[1],
                                        1,
                                        GL_RGBA,
                                        GL_UNSIGNED_BYTE );

    _boundAnalysisImage->setInternalTextureFormat( GL_RGBA );
    _boundAnalysisTexture->setInternalFormat( GL_RGBA );
    #endif

#endif
    memset( _boundAnalysisImage->data(), 0, _boundAnalysisImage->getImageSizeInBytes() );

    if( getDebugDraw() )
    {
        _boundAnalysisTexture->setImage( 0, _boundAnalysisImage.get() );
    }

    _boundAnalysisTexture->setFilter( osg::Texture2D::MIN_FILTER,
                                      osg::Texture2D::NEAREST );
    _boundAnalysisTexture->setFilter( osg::Texture2D::MAG_FILTER,
                                      osg::Texture2D::NEAREST );

    // the shadow comparison should fail if object is outside the texture
    _boundAnalysisTexture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
    _boundAnalysisTexture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );

    // set up the render to texture camera.
    // create the camera
    _boundAnalysisCamera = new osg::Camera;
    _boundAnalysisCamera->setName( "AnalysisCamera" );

    _boundAnalysisCamera->setCullCallback( new BaseClass::CameraCullCallback( st ) );
    // _boundAnalysisCamera->setPreDrawCallback( _camera->getPreDrawCallback() );
    _boundAnalysisCamera->setPostDrawCallback( new CameraPostDrawCallback( this ) );

    _boundAnalysisCamera->setClearColor( osg::vec4( 1, 1, 1, 1 ) );
    _boundAnalysisCamera->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
    _boundAnalysisCamera->setComputeNearFarMode( osg::Camera::DO_NOT_COMPUTE_NEAR_FAR );

    // set viewport
    _boundAnalysisCamera->setViewport( 0,
                                       0,
                                       _boundAnalysisSize[0],
                                       _boundAnalysisSize[1] );

    // set the camera to render before the main camera.
    _boundAnalysisCamera->setRenderOrder( osg::Camera::PRE_RENDER );

    // tell the camera to use OpenGL frame buffer object where supported.
    _boundAnalysisCamera->setRenderTargetImplementation(
        osg::Camera::FRAME_BUFFER_OBJECT
    );
    //_boundAnalysisCamera->setRenderTargetImplementation(osg::Camera::SEPERATE_WINDOW);

    const int OVERRIDE_ON   = osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON;
    const int OVERRIDE_OFF  = osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF;

    osg::StateSet* stateset = _boundAnalysisCamera->getOrCreateStateSet();
    stateset->setAttributeAndModes(
        new osg::Depth( osg::Depth::Function::LESS, 0.0, 254.F / 255.F ),
        OVERRIDE_ON
    );

    stateset->setRenderBinDetails( 0,
                                   "RenderBin",
                                   osg::StateSet::OVERRIDE_RENDERBIN_DETAILS );

    osg::Program* program = new osg::Program;

    program->addShader( new osg::Shader(
        osg::Shader::FRAGMENT,
        "#version 460 core\n"
        "uniform sampler2D texture;                                          \n"
        "in vec4 texCoord_0;                                                 \n"
        "out vec4 fragColor;                                                 \n"
        "void main(void)                                                     \n"
        "{                                                                   \n"
#if ANALYSIS_DEPTH
        " fragColor = texture( texture, texCoord_0.xy );                     \n"
#else
        " fragColor = vec4( gl_FragCoord.z,                                  \n"
        "                      1.-gl_FragCoord.z,                            \n"
        "                      1.,                                           \n"
        "                       texture( texture, texCoord_0.xy ).a );       \n"
#endif
        "}                                                                   \n"
    ) );    // program->addShader Fragment

    program->addShader( new osg::Shader(
        osg::Shader::VERTEX,
        "#version 460 core\n"
        "layout(location = 0) in vec4 osg_Vertex;                            \n"
        "layout(location = 8) in vec4 osg_MultiTexCoord0;                    \n"
        "uniform mat4 osg_ModelViewProjectionMatrix;                         \n"
        "out vec4 texCoord_0;                                                \n"
        "void main(void)                                                  \n"
        "{                                                                \n"
        "   gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;     \n"
        "   texCoord_0 = osg_MultiTexCoord0;                              \n"
        "}                                                                \n"
    ) );    // program->addShader Vertex

    stateset->setAttribute( program, OVERRIDE_ON );

    // attach the texture and use it as the color buffer.
#if ANALYSIS_DEPTH
    // _boundAnalysisCamera->attach(osg::Camera::DEPTH_BUFFER,
    // _boundAnalysisTexture.get());
    _boundAnalysisCamera->attach( osg::Camera::DEPTH_BUFFER, _boundAnalysisImage.get() );

    stateset->setMode( GL_BLEND, OVERRIDE_OFF );
#else
    // _boundAnalysisCamera->attach(osg::Camera::COLOR_BUFFER,
    // _boundAnalysisTexture.get());
    _boundAnalysisCamera->attach( osg::Camera::COLOR_BUFFER, _boundAnalysisImage.get() );

    stateset->setAttributeAndModes(
        new osg::BlendEquation( osg::BlendEquation::RGBA_MIN ),
        OVERRIDE_ON
    );

    stateset->setMode( GL_DEPTH_TEST, OVERRIDE_OFF );
#endif
}
