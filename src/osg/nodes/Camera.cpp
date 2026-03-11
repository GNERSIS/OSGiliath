/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Configures view and projection matrices, manages render targets
 * (FBO, pbuffer, window), and controls rendering order (PRE_RENDER,
 * POST_RENDER) for multi-pass rendering techniques.
 */
#include <osg/nodes/Camera.hpp>

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>
#include <osg/rendering/RenderInfo.hpp>
#include <osg/textures/Texture1D.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osg/textures/Texture2DArray.hpp>
#include <osg/textures/Texture3D.hpp>
#include <osg/textures/TextureCubeMap.hpp>
#include <osg/textures/TextureRectangle.hpp>

using namespace osg;

const unsigned int Camera::FACE_CONTROLLED_BY_GEOMETRY_SHADER = 0XFF'FF'FF'FF;

Camera::Camera() :
    _view( 0 ),
    _allowEventFocus( true ),
    _clearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ),
    _clearColor( osg::vec4( 0.0F,
                            0.0F,
                            0.0F,
                            1.0F ) ),
    _clearAccum( osg::vec4( 0.0F,
                            0.0F,
                            0.0F,
                            1.0F ) ),
    _clearDepth( 1.0 ),
    _clearStencil( 0 ),
    _transformOrder( PRE_MULTIPLY ),
    _projectionResizePolicy( HORIZONTAL ),
    _renderOrder( POST_RENDER ),
    _renderOrderNum( 0 ),
    _drawBuffer( GL_NONE ),
    _readBuffer( GL_NONE ),
    _renderTargetImplementation( FRAME_BUFFER ),
    _renderTargetFallback( FRAME_BUFFER ),
    _implicitBufferAttachmentRenderMask( USE_DISPLAY_SETTINGS_MASK ),
    _implicitBufferAttachmentResolveMask( USE_DISPLAY_SETTINGS_MASK ),
    _attachmentMapModifiedCount( 0 )
{
    setStateSet( new StateSet );
}

Camera::Camera( const Camera& camera,
                const CopyOp& copyop ) :
    Inherit( camera,
             copyop ),
    CullSettings( camera ),
    _view( camera._view ),
    _allowEventFocus( camera._allowEventFocus ),
    _displaySettings( camera._displaySettings ),
    _clearMask( camera._clearMask ),
    _clearColor( camera._clearColor ),
    _clearAccum( camera._clearAccum ),
    _clearDepth( camera._clearDepth ),
    _clearStencil( camera._clearStencil ),
    _colorMask( camera._colorMask ),
    _viewport( camera._viewport ),
    _transformOrder( camera._transformOrder ),
    _projectionResizePolicy( camera._projectionResizePolicy ),
    _projectionMatrix( camera._projectionMatrix ),
    _viewMatrix( camera._viewMatrix ),
    _renderOrder( camera._renderOrder ),
    _renderOrderNum( camera._renderOrderNum ),
    _drawBuffer( camera._drawBuffer ),
    _readBuffer( camera._readBuffer ),
    _renderTargetImplementation( camera._renderTargetImplementation ),
    _renderTargetFallback( camera._renderTargetFallback ),
    _bufferAttachmentMap( camera._bufferAttachmentMap ),
    _implicitBufferAttachmentRenderMask( camera._implicitBufferAttachmentRenderMask ),
    _implicitBufferAttachmentResolveMask( camera._implicitBufferAttachmentResolveMask ),
    _attachmentMapModifiedCount( camera._attachmentMapModifiedCount ),
    _affinity( camera._affinity ),
    _initialDrawCallback( camera._initialDrawCallback ),
    _preDrawCallback( camera._preDrawCallback ),
    _postDrawCallback( camera._postDrawCallback ),
    _finalDrawCallback( camera._finalDrawCallback )
{
    // need to copy/share graphics context?
}

Camera::~Camera()
{
    setCameraThread( 0 );

    if( _graphicsContext.valid() )
    {
        _graphicsContext->removeCamera( this );
    }
}

void
Camera::DrawCallback::operator()( osg::RenderInfo& renderInfo ) const
{
    if( renderInfo.getCurrentCamera() )
    {
        operator()( *( renderInfo.getCurrentCamera() ) );
    }
    else
    {
        OSG_WARN << "Error: Camera::DrawCallback called without valid camera."
                 << std::endl;
    }
}

void
Camera::setGraphicsContext( GraphicsContext* context )
{
    if( _graphicsContext == context )
    {
        return;
    }

    if( _graphicsContext.valid() )
    {
        _graphicsContext->removeCamera( this );
    }

    _graphicsContext = context;

    if( _graphicsContext.valid() )
    {
        _graphicsContext->addCamera( this );
    }
}

bool
Camera::isRenderToTextureCamera() const
{
    return ( !_bufferAttachmentMap.empty() );
}

void
Camera::setRenderTargetImplementation( RenderTargetImplementation impl )
{
    _renderTargetImplementation = impl;
    if( impl < FRAME_BUFFER )
    {
        _renderTargetFallback = ( RenderTargetImplementation )( impl + 1 );
    }
    else
    {
        _renderTargetFallback = impl;
    }
}

void
Camera::setRenderTargetImplementation( RenderTargetImplementation impl,
                                       RenderTargetImplementation fallback )
{
    if( impl < fallback || ( impl == FRAME_BUFFER && fallback == FRAME_BUFFER ) )
    {
        _renderTargetImplementation = impl;
        _renderTargetFallback       = fallback;
    }
    else
    {
        OSG_NOTICE
            << "Warning: Camera::setRenderTargetImplementation(impl,fallback) must have "
               "a lower rated fallback than the main target implementation."
            << std::endl;
        setRenderTargetImplementation( impl );
    }
}

void
Camera::setColorMask( osg::ColorMask* colorMask )
{
    if( _colorMask == colorMask )
    {
        return;
    }

    osg::StateSet* stateset = getOrCreateStateSet();
    if( _colorMask.valid() && stateset )
    {
        stateset->removeAttribute( _colorMask.get() );
    }

    _colorMask = colorMask;

    if( _colorMask.valid() && stateset )
    {
        stateset->setAttribute( _colorMask.get() );
    }
}

void
Camera::setColorMask( bool red,
                      bool green,
                      bool blue,
                      bool alpha )
{
    if( !_colorMask )
    {
        setColorMask( new osg::ColorMask );
    }
    if( _colorMask.valid() )
    {
        _colorMask->setMask( red, green, blue, alpha );
    }
}

void
Camera::setViewport( osg::Viewport* viewport )
{
    if( _viewport == viewport )
    {
        return;
    }

    osg::StateSet* stateset = getOrCreateStateSet();
    if( _viewport.valid() && stateset )
    {
        stateset->removeAttribute( _viewport.get() );
    }

    _viewport = viewport;

    if( _viewport.valid() && stateset )
    {
        stateset->setAttribute( _viewport.get() );
    }
}

void
Camera::setViewport( int x,
                     int y,
                     int width,
                     int height )
{
    if( !_viewport )
    {
        setViewport( new osg::Viewport );
    }
    if( _viewport.valid() )
    {
        _viewport->setViewport( x, y, width, height );
    }
}

dmat4
Camera::getInverseViewMatrix() const
{
    dmat4 inv = osg::inverse( _viewMatrix );
    return inv;
}

void
Camera::setProjectionMatrixAsOrtho( double left,
                                    double right,
                                    double bottom,
                                    double top,
                                    double zNear,
                                    double zFar )
{
    setProjectionMatrix( osg::orthographic( left, right, bottom, top, zNear, zFar ) );
}

void
Camera::setProjectionMatrixAsOrtho2D( double left,
                                      double right,
                                      double bottom,
                                      double top )
{
    setProjectionMatrix( osg::orthographic( left, right, bottom, top, -1.0, 1.0 ) );
}

void
Camera::setProjectionMatrixAsFrustum( double left,
                                      double right,
                                      double bottom,
                                      double top,
                                      double zNear,
                                      double zFar )
{
    setProjectionMatrix( osg::perspective( left, right, bottom, top, zNear, zFar ) );
}

void
Camera::setProjectionMatrixAsPerspective( double fovy,
                                          double aspectRatio,
                                          double zNear,
                                          double zFar )
{
    setProjectionMatrix( osg::perspective( fovy, aspectRatio, zNear, zFar ) );
}

bool
Camera::getProjectionMatrixAsOrtho( double& left,
                                    double& right,
                                    double& bottom,
                                    double& top,
                                    double& zNear,
                                    double& zFar ) const
{
    return osg::getOrtho( _projectionMatrix, left, right, bottom, top, zNear, zFar );
}

bool
Camera::getProjectionMatrixAsFrustum( double& left,
                                      double& right,
                                      double& bottom,
                                      double& top,
                                      double& zNear,
                                      double& zFar ) const
{
    return osg::getFrustum( _projectionMatrix, left, right, bottom, top, zNear, zFar );
}

bool
Camera::getProjectionMatrixAsPerspective( double& fovy,
                                          double& aspectRatio,
                                          double& zNear,
                                          double& zFar ) const
{
    return osg::getPerspective( _projectionMatrix, fovy, aspectRatio, zNear, zFar );
}

void
Camera::setViewMatrixAsLookAt( const dvec3& eye,
                               const dvec3& center,
                               const dvec3& up )
{
    setViewMatrix( osg::lookAt( eye, center, up ) );
}

void
Camera::getViewMatrixAsLookAt( dvec3& eye,
                               dvec3& center,
                               dvec3& up,
                               double lookDistance ) const
{
    osg::getLookAt( _viewMatrix, eye, center, up, lookDistance );
}

void
Camera::getViewMatrixAsLookAt( vec3& eye,
                               vec3& center,
                               vec3& up,
                               float lookDistance ) const
{
    dvec3 deye, dcenter, dup;
    osg::getLookAt( _viewMatrix, deye, dcenter, dup, ( double )lookDistance );
    eye    = vec3( deye );
    center = vec3( dcenter );
    up     = vec3( dup );
}

void
Camera::attach( BufferComponent buffer,
                GLenum          internalFormat )
{
    switch( buffer )
    {
        case DEPTH_BUFFER :
            if( _bufferAttachmentMap.find( PACKED_DEPTH_STENCIL_BUFFER ) !=
                _bufferAttachmentMap.end() )
            {
                OSG_WARN << "Camera: DEPTH_BUFFER already attached as "
                            "PACKED_DEPTH_STENCIL_BUFFER !"
                         << std::endl;
            }
            break;

        case STENCIL_BUFFER :
            if( _bufferAttachmentMap.find( PACKED_DEPTH_STENCIL_BUFFER ) !=
                _bufferAttachmentMap.end() )
            {
                OSG_WARN << "Camera: STENCIL_BUFFER already attached as "
                            "PACKED_DEPTH_STENCIL_BUFFER !"
                         << std::endl;
            }
            break;

        case PACKED_DEPTH_STENCIL_BUFFER :
            if( _bufferAttachmentMap.find( DEPTH_BUFFER ) != _bufferAttachmentMap.end() )
            {
                OSG_WARN << "Camera: DEPTH_BUFFER already attached !" << std::endl;
            }
            if( _bufferAttachmentMap.find( STENCIL_BUFFER ) !=
                _bufferAttachmentMap.end() )
            {
                OSG_WARN << "Camera: STENCIL_BUFFER already attached !" << std::endl;
            }
            break;
        default :
            break;
    }
    _bufferAttachmentMap[buffer]._internalFormat = internalFormat;
}

void
Camera::attach( BufferComponent buffer,
                osg::Texture*   texture,
                unsigned int    level,
                unsigned int    face,
                bool            mipMapGeneration,
                unsigned int    multisampleSamples,
                unsigned int    multisampleColorSamples )
{
    _bufferAttachmentMap[buffer]._texture                 = texture;
    _bufferAttachmentMap[buffer]._level                   = level;
    _bufferAttachmentMap[buffer]._face                    = face;
    _bufferAttachmentMap[buffer]._mipMapGeneration        = mipMapGeneration;
    _bufferAttachmentMap[buffer]._multisampleSamples      = multisampleSamples;
    _bufferAttachmentMap[buffer]._multisampleColorSamples = multisampleColorSamples;
}

void
Camera::attach( BufferComponent buffer,
                osg::Image*     image,
                unsigned int    multisampleSamples,
                unsigned int    multisampleColorSamples )
{
    _bufferAttachmentMap[buffer]._image                   = image;
    _bufferAttachmentMap[buffer]._multisampleSamples      = multisampleSamples;
    _bufferAttachmentMap[buffer]._multisampleColorSamples = multisampleColorSamples;
}

void
Camera::detach( BufferComponent buffer )
{
    _bufferAttachmentMap.erase( buffer );
}

void
Camera::resizeGLObjectBuffers( unsigned int maxSize )
{
    if( _renderer.valid() )
    {
        _renderer->resizeGLObjectBuffers( maxSize );
    }
    if( _renderingCache.valid() )
    {
        _renderingCache->resizeGLObjectBuffers( maxSize );
    }

    if( _initialDrawCallback.valid() )
    {
        _initialDrawCallback->resizeGLObjectBuffers( maxSize );
    }
    if( _preDrawCallback.valid() )
    {
        _preDrawCallback->resizeGLObjectBuffers( maxSize );
    }
    if( _postDrawCallback.valid() )
    {
        _postDrawCallback->resizeGLObjectBuffers( maxSize );
    }
    if( _finalDrawCallback.valid() )
    {
        _finalDrawCallback->resizeGLObjectBuffers( maxSize );
    }

    Transform::resizeGLObjectBuffers( maxSize );
}

void
Camera::releaseGLObjects( osg::State* state ) const
{
    if( _renderer.valid() )
    {
        _renderer->releaseGLObjects( state );
    }
    if( _renderingCache.valid() )
    {
        _renderingCache->releaseGLObjects( state );
    }

    if( _initialDrawCallback.valid() )
    {
        _initialDrawCallback->releaseGLObjects( state );
    }
    if( _preDrawCallback.valid() )
    {
        _preDrawCallback->releaseGLObjects( state );
    }
    if( _postDrawCallback.valid() )
    {
        _postDrawCallback->releaseGLObjects( state );
    }
    if( _finalDrawCallback.valid() )
    {
        _finalDrawCallback->releaseGLObjects( state );
    }

    Transform::releaseGLObjects( state );
}

bool
Camera::computeLocalToWorldMatrix( dmat4& matrix,
                                   NodeVisitor* ) const
{
    if( _referenceFrame == RELATIVE_RF )
    {
        if( _transformOrder == PRE_MULTIPLY )
        {
            // column-vector: pre-multiply means viewMatrix applied before parent
            matrix = _viewMatrix * matrix;
        }
        else
        {
            // post-multiply
            matrix = matrix * _viewMatrix;
        }
    }
    else    // absolute
    {
        matrix = _viewMatrix;
    }
    return true;
}

bool
Camera::computeWorldToLocalMatrix( dmat4& matrix,
                                   NodeVisitor* ) const
{
    const dmat4& inv = getInverseViewMatrix();

    if( _referenceFrame == RELATIVE_RF )
    {
        if( _transformOrder == PRE_MULTIPLY )
        {
            // note doing inverse so pre becomes post.
            matrix = matrix * inv;
        }
        else
        {
            // note doing inverse so post becomes pre.
            matrix = inv * matrix;
        }
    }
    else    // absolute
    {
        matrix = inv;
    }
    return true;
}

void
Camera::inheritCullSettings( const CullSettings& settings,
                             unsigned int        inheritanceMask )
{
    CullSettings::inheritCullSettings( settings, inheritanceMask );

    const Camera* camera = dynamic_cast<const Camera*>( &settings );
    if( camera )
    {
        // OSG_NOTICE<<"Inheriting slave Camera"<<std::endl;
        if( inheritanceMask & CLEAR_COLOR )
        {
            _clearColor = camera->_clearColor;
        }

        if( inheritanceMask & CLEAR_MASK )
        {
            _clearMask = camera->_clearMask;
        }

        if( inheritanceMask & DRAW_BUFFER )
        {
            _drawBuffer = camera->_drawBuffer;
        }

        if( inheritanceMask & READ_BUFFER )
        {
            _readBuffer = camera->_readBuffer;
        }
    }
}

void
Camera::resizeAttachments( int width,
                           int height )
{
    bool modified = false;
    for( BufferAttachmentMap::iterator itr = _bufferAttachmentMap.begin();
         itr != _bufferAttachmentMap.end();
         ++itr )
    {
        Attachment& attachment = itr->second;
        if( attachment._texture.valid() )
        {
            {
                osg::Texture1D* texture =
                    dynamic_cast<osg::Texture1D*>( attachment._texture.get() );
                if( texture && ( texture->getTextureWidth() != width ) )
                {
                    modified = true;
                    texture->setTextureWidth( width );
                    texture->dirtyTextureObject();
                }
            }

            {
                osg::Texture2D* texture =
                    dynamic_cast<osg::Texture2D*>( attachment._texture.get() );
                if( texture && ( ( texture->getTextureWidth() != width ) ||
                                 ( texture->getTextureHeight() != height ) ) )
                {
                    modified = true;
                    texture->setTextureSize( width, height );
                    texture->dirtyTextureObject();
                }
            }

            {
                osg::Texture3D* texture =
                    dynamic_cast<osg::Texture3D*>( attachment._texture.get() );
                if( texture && ( ( texture->getTextureWidth() != width ) ||
                                 ( texture->getTextureHeight() != height ) ) )
                {
                    modified = true;
                    texture->setTextureSize( width, height, texture->getTextureDepth() );
                    texture->dirtyTextureObject();
                }
            }

            {
                osg::Texture2DArray* texture =
                    dynamic_cast<osg::Texture2DArray*>( attachment._texture.get() );
                if( texture && ( ( texture->getTextureWidth() != width ) ||
                                 ( texture->getTextureHeight() != height ) ) )
                {
                    modified = true;
                    texture->setTextureSize( width, height, texture->getTextureDepth() );
                    texture->dirtyTextureObject();
                }
            }
        }

        if( attachment._image.valid() &&
            ( attachment._image->s() != width || attachment._image->s() != height ) )
        {
            modified          = true;
            osg::Image* image = attachment._image.get();
            image->allocateImage( width,
                                  height,
                                  image->r(),
                                  image->getPixelFormat(),
                                  image->getDataType(),
                                  static_cast<int>( image->getPacking() ) );
        }
    }

    if( modified )
    {
        dirtyAttachmentMap();
    }
}

void
Camera::resize( int width,
                int height,
                int resizeMask )
{
    if( getViewport() )
    {
        double previousWidth  = getViewport()->width();
        double previousHeight = getViewport()->height();
        double newWidth       = width;
        double newHeight      = height;

        if( ( previousWidth != newWidth ) || ( previousHeight != newHeight ) )
        {
            if( ( resizeMask & RESIZE_PROJECTIONMATRIX ) !=
                0 &&
                ( getProjectionResizePolicy() != FIXED ) )
            {
                double widthChangeRatio  = newWidth / previousWidth;
                double heigtChangeRatio  = newHeight / previousHeight;
                double aspectRatioChange = widthChangeRatio / heigtChangeRatio;
                if( aspectRatioChange != 1.0 )
                {
                    switch( getProjectionResizePolicy() )
                    {
                        case( HORIZONTAL ) :
                            getProjectionMatrix() =
                                osg::scale( 1.0 / aspectRatioChange, 1.0, 1.0 ) *
                                getProjectionMatrix();
                            break;
                        case( VERTICAL ) :
                            getProjectionMatrix() =
                                osg::scale( 1.0, aspectRatioChange, 1.0 ) *
                                getProjectionMatrix();
                            break;
                        case( FIXED ) :
                            break;
                    }
                }
            }

            if( ( resizeMask & RESIZE_VIEWPORT ) != 0 )
            {
                setViewport( 0, 0, width, height );
            }
        }
    }

    if( ( resizeMask & RESIZE_ATTACHMENTS ) != 0 )
    {
        resizeAttachments( width, height );
    }
}

void
Camera::setProcessorAffinity( const osg::Thread::Affinity& affinity )
{
    _affinity = affinity;

    if( _cameraThread.valid() )
    {
        _cameraThread->setProcessorAffinity( affinity );
    }
}

void
Camera::createCameraThread()
{
    if( !_cameraThread )
    {
        setCameraThread( new OperationThread );
    }
}

void
Camera::setCameraThread( OperationThread* gt )
{
    if( _cameraThread == gt )
    {
        return;
    }

    if( _cameraThread.valid() )
    {
        // need to kill the thread in some way...
        _cameraThread->cancel();
        _cameraThread->setParent( 0 );
    }

    _cameraThread = gt;

    if( _cameraThread.valid() )
    {
        _cameraThread->setProcessorAffinity( _affinity );
        _cameraThread->setParent( this );
    }
}
