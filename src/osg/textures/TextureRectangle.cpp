/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Rectangle texture using non-normalized [0,width]x[0,height]
 * coordinates. Used for exact-pixel screen-space effects.
 */
#include <osg/textures/TextureRectangle.hpp>

#include <osg/core/Notify.hpp>
#include <osg/core/Timer.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/rendering/GLU.hpp>
#include <osg/state/State.hpp>

// Apple-specific GL constants removed (not in GL 4.6 Core Profile)

// #define DO_TIMING

using namespace osg;

TextureRectangle::TextureRectangle() :
    _textureWidth( 0 ),
    _textureHeight( 0 )
{
    setWrap( WRAP_S, CLAMP );
    setWrap( WRAP_T, CLAMP );

    setFilter( MIN_FILTER, LINEAR );
    setFilter( MAG_FILTER, LINEAR );
}

TextureRectangle::TextureRectangle( Image* image ) :
    _textureWidth( 0 ),
    _textureHeight( 0 )
{
    setWrap( WRAP_S, CLAMP );
    setWrap( WRAP_T, CLAMP );

    setFilter( MIN_FILTER, LINEAR );
    setFilter( MAG_FILTER, LINEAR );

    setImage( image );
}

TextureRectangle::TextureRectangle( const TextureRectangle& text,
                                    const CopyOp&           copyop ) :
    Inherit( text,
             copyop ),
    _textureWidth( text._textureWidth ),
    _textureHeight( text._textureHeight ),
    _subloadCallback( text._subloadCallback )
{
    setImage( copyop( text._image.get() ) );
}

TextureRectangle::~TextureRectangle()
{
    setImage( NULL );
}

int
TextureRectangle::compare( const StateAttribute& sa ) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types( TextureRectangle,
                                  sa ) if( _image !=
                                           rhs._image )    // smart pointer comparison.
    {
        if( _image.valid() )
        {
            if( rhs._image.valid() )
            {
                int result = _image->compare( *rhs._image );
                if( result != 0 )
                {
                    return result;
                }
            }
            else
            {
                return 1;    // valid lhs._image is greater than null.
            }
        }
        else if( rhs._image.valid() )
        {
            return -1;    // valid rhs._image is greater than null.
        }
    }

    if( !_image && !rhs._image )
    {
        // no image attached to either Texture2D
        // but could these textures already be downloaded?
        // check the _textureObjectBuffer to see if they have been
        // downloaded

        int result = compareTextureObjects( rhs );
        if( result != 0 )
        {
            return result;
        }
    }

    int result = compareTexture( rhs );
    if( result != 0 )
    {
        return result;
    }

    // compare each parameter in turn against the rhs.
    COMPARE_StateAttribute_Parameter( _textureWidth )
        COMPARE_StateAttribute_Parameter( _textureHeight )
            COMPARE_StateAttribute_Parameter(
                _subloadCallback
            ) return 0;    // passed all the above comparison macros, must be equal.
}

void
TextureRectangle::setImage( Image* image )
{
    if( _image == image )
    {
        return;
    }

    if( _image.valid() )
    {
        _image->removeClient( this );

        if( _image->requiresUpdateCall() )
        {
            setUpdateCallback( 0 );
            setDataVariance( osg::Object::DataVariance::STATIC );
        }
    }

    // delete old texture objects.
    dirtyTextureObject();

    _image = image;

    if( _image.valid() )
    {
        _image->addClient( this );

        if( _image->requiresUpdateCall() )
        {
            setUpdateCallback( new Image::UpdateCallback() );
            setDataVariance( osg::Object::DataVariance::DYNAMIC );
        }
    }
}

void
TextureRectangle::apply( State& state ) const
{
    if( !state.get<GLExtensions>()->isRectangleSupported )
    {
        OSG_WARN << "Warning: TextureRectangle::apply(..) failed, texture rectangle is "
                    "not support by your OpenGL drivers."
                 << std::endl;
        return;
    }

    // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();

    // get the texture object for the current contextID.
    TextureObject*     textureObject = getTextureObject( contextID );

    if( textureObject )
    {
        if( _image.valid() &&
            getModifiedCount( contextID ) != _image->getModifiedCount() )
        {
            // compute the internal texture format, this set the _internalFormat to an
            // appropriate value.
            computeInternalFormat();

            GLsizei new_width, new_height, new_numMipmapLevels;

            // compute the dimensions of the texture.
            computeRequiredTextureDimensions( state,
                                              *_image,
                                              new_width,
                                              new_height,
                                              new_numMipmapLevels );

            if( !textureObject->match( GL_TEXTURE_RECTANGLE,
                                       new_numMipmapLevels,
                                       static_cast<GLenum>( _internalFormat ),
                                       new_width,
                                       new_height,
                                       1,
                                       _borderWidth ) )
            {
                _textureObjectBuffer[contextID]->release();
                _textureObjectBuffer[contextID] = 0;
                textureObject                   = 0;
            }
        }
    }

    if( textureObject )
    {
        textureObject->bind( state );

        if( getTextureParameterDirty( state.getContextID() ) )
        {
            applyTexParameters( GL_TEXTURE_RECTANGLE, state );
        }

        if( _subloadCallback.valid() )
        {
            _subloadCallback->subload( *this, state );
        }
        else if( _image.valid() &&
                 getModifiedCount( contextID ) != _image->getModifiedCount() )
        {
            // update the modified count to show that it is up to date.
            getModifiedCount( contextID ) = _image->getModifiedCount();

            applyTexImage_subload( GL_TEXTURE_RECTANGLE,
                                   _image.get(),
                                   state,
                                   _textureWidth,
                                   _textureHeight,
                                   _internalFormat );
        }
    }
    else if( _subloadCallback.valid() )
    {
        // we don't have a applyTexImage1D_subload yet so can't reuse.. so just generate
        // a new texture object.
        textureObject =
            generateAndAssignTextureObject( contextID, GL_TEXTURE_RECTANGLE );

        textureObject->bind( state );

        applyTexParameters( GL_TEXTURE_RECTANGLE, state );

        _subloadCallback->load( *this, state );

        textureObject->setAllocated( 1,
                                     static_cast<GLenum>( _internalFormat ),
                                     _textureWidth,
                                     _textureHeight,
                                     1,
                                     0 );

        // in theory the following line is redundant, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        // glBindTexture(GL_TEXTURE_RECTANGLE, handle);
    }
    else if( _image.valid() && _image->data() )
    {
        GLExtensions*            extensions = state.get<GLExtensions>();

        // keep the image around at least till we go out of scope.
        osg::ref_ptr<osg::Image> image = _image;

        // compute the internal texture format, this set the _internalFormat to an
        // appropriate value.
        computeInternalFormat();

        // get sizedInternalFormat if TexStorage available
        GLenum texStorageSizedInternalFormat =
            extensions->isTextureStorageEnabled && ( _borderWidth == 0 )
                ? selectSizedInternalFormat( image.get() )
                : 0;

        _textureWidth  = image->s();
        _textureHeight = image->t();

        textureObject =
            generateAndAssignTextureObject( contextID,
                                            GL_TEXTURE_RECTANGLE,
                                            1,
                                            texStorageSizedInternalFormat != 0
                                                ? texStorageSizedInternalFormat
                                                : static_cast<GLenum>( _internalFormat ),
                                            _textureWidth,
                                            _textureHeight,
                                            1,
                                            0 );

        textureObject->bind( state );

        applyTexParameters( GL_TEXTURE_RECTANGLE, state );

        if( textureObject->isAllocated() )
        {
            applyTexImage_subload( GL_TEXTURE_RECTANGLE,
                                   _image.get(),
                                   state,
                                   _textureWidth,
                                   _textureHeight,
                                   _internalFormat );
        }
        else
        {
            applyTexImage_load( GL_TEXTURE_RECTANGLE,
                                _image.get(),
                                state,
                                _textureWidth,
                                _textureHeight );
            textureObject->setAllocated( true );
        }

        // unref image data?
        if( isSafeToUnrefImageData( state ) &&
            _image->getDataVariance() == DataVariance::STATIC )
        {
            TextureRectangle* non_const_this = const_cast<TextureRectangle*>( this );
            non_const_this->_image           = NULL;
        }
    }
    else if( ( _textureWidth != 0 ) &&
             ( _textureHeight != 0 ) &&
             ( _internalFormat != 0 ) )
    {
        // no image present, but dimensions at set so lets create the texture
        GLExtensions* extensions = state.get<GLExtensions>();
        GLenum        texStorageSizedInternalFormat =
            extensions->isTextureStorageEnabled ? selectSizedInternalFormat() : 0;
        if( texStorageSizedInternalFormat != 0 )
        {
            textureObject =
                generateAndAssignTextureObject( contextID,
                                                GL_TEXTURE_RECTANGLE,
                                                0,
                                                texStorageSizedInternalFormat,
                                                _textureWidth,
                                                _textureHeight,
                                                1,
                                                0 );
            textureObject->bind( state );
            applyTexParameters( GL_TEXTURE_RECTANGLE, state );
            if( !textureObject->_allocated )
            {
                extensions->glTexStorage2D( GL_TEXTURE_RECTANGLE,
                                            1,
                                            texStorageSizedInternalFormat,
                                            _textureWidth,
                                            _textureHeight );
            }
        }
        else
        {
            GLenum internalFormat =
                _sourceFormat ? _sourceFormat : static_cast<GLenum>( _internalFormat );
            textureObject = generateAndAssignTextureObject( contextID,
                                                            GL_TEXTURE_RECTANGLE,
                                                            0,
                                                            internalFormat,
                                                            _textureWidth,
                                                            _textureHeight,
                                                            1,
                                                            0 );
            textureObject->bind( state );
            applyTexParameters( GL_TEXTURE_RECTANGLE, state );

            glTexImage2D( GL_TEXTURE_RECTANGLE,
                          0,
                          _internalFormat,
                          _textureWidth,
                          _textureHeight,
                          _borderWidth,
                          internalFormat,
                          _sourceType ? _sourceType : GL_UNSIGNED_BYTE,
                          0 );
        }

        if( _readPBuffer.valid() )
        {
            _readPBuffer->bindPBufferToTexture( GL_FRONT );
        }

        textureObject->setAllocated( 0,
                                     texStorageSizedInternalFormat != 0
                                         ? texStorageSizedInternalFormat
                                         : static_cast<GLenum>( _internalFormat ),
                                     _textureWidth,
                                     _textureHeight,
                                     1,
                                     0 );
    }
    else
    {
        glBindTexture( GL_TEXTURE_RECTANGLE, 0 );
    }
}

void
TextureRectangle::applyTexImage_load( GLenum   target,
                                      Image*   image,
                                      State&   state,
                                      GLsizei& inwidth,
                                      GLsizei& inheight ) const
{
    // if we don't have a valid image we can't create a texture!
    if( !image || !image->data() )
    {
        return;
    }

    // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const unsigned int  contextID  = state.getContextID();
    const GLExtensions* extensions = state.get<GLExtensions>();

    // update the modified count to show that it is up to date.
    getModifiedCount( contextID ) = image->getModifiedCount();

    // compute the internal texture format, sets _internalFormat.
    computeInternalFormat();

    glPixelStorei( GL_UNPACK_ALIGNMENT, static_cast<GLint>( image->getPacking() ) );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, image->getRowLength() );

    // Apple client storage hints removed (not in GL 4.6 Core Profile)

    const unsigned char* dataPtr = image->data();
    GLBufferObject*      pbo     = image->getOrCreateGLBufferObject( contextID );
    if( pbo )
    {
        state.bindPixelBufferObject( pbo );
        dataPtr = reinterpret_cast<unsigned char*>(
            pbo->getOffset( image->getBufferIndex() )
        );
    }

    if( isCompressedInternalFormat( _internalFormat ) &&
        extensions->isCompressedTexImage2DSupported() )
    {
        extensions->glCompressedTexImage2D(
            target,
            0,
            static_cast<GLenum>( _internalFormat ),
            image->s(),
            image->t(),
            0,
            static_cast<GLsizei>( image->getImageSizeInBytes() ),
            dataPtr
        );
    }
    else
    {
        glTexImage2D( target,
                      0,
                      _internalFormat,
                      image->s(),
                      image->t(),
                      0,
                      ( GLenum )image->getPixelFormat(),
                      ( GLenum )image->getDataType(),
                      dataPtr );
    }

    if( pbo )
    {
        state.unbindPixelBufferObject();
    }

    inwidth  = image->s();
    inheight = image->t();
}

void
TextureRectangle::applyTexImage_subload( GLenum   target,
                                         Image*   image,
                                         State&   state,
                                         GLsizei& inwidth,
                                         GLsizei& inheight,
                                         GLint&   inInternalFormat ) const
{
    // if we don't have a valid image we can't create a texture!
    if( !image || !image->data() )
    {
        return;
    }

    if( image->s() !=
        inwidth ||
        image->t() !=
        inheight ||
        image->getInternalTextureFormat() != inInternalFormat )
    {
        applyTexImage_load( target, image, state, inwidth, inheight );
        return;
    }

    // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const unsigned int  contextID  = state.getContextID();
    const GLExtensions* extensions = state.get<GLExtensions>();

    // update the modified count to show that it is up to date.
    getModifiedCount( contextID ) = image->getModifiedCount();

    // compute the internal texture format, sets _internalFormat.
    computeInternalFormat();

    glPixelStorei( GL_UNPACK_ALIGNMENT, static_cast<GLint>( image->getPacking() ) );
    int rowLength = image->getRowLength();

#ifdef DO_TIMING
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    OSG_NOTICE << "TextureRectangle::apply pixelFormat = " << std::hex
               << image->getPixelFormat() << std::dec << std::endl;
#endif
    const unsigned char* dataPtr = image->data();
    GLBufferObject*      pbo     = image->getOrCreateGLBufferObject( contextID );
    if( pbo )
    {
        state.bindPixelBufferObject( pbo );
        dataPtr = reinterpret_cast<unsigned char*>(
            pbo->getOffset( image->getBufferIndex() )
        );
        rowLength = 0;
#ifdef DO_TIMING
        OSG_NOTICE << "after PBO "
                   << osg::Timer::instance()->delta_m( start_tick,
                                                       osg::Timer::instance()->tick() )
                   << "ms" << std::endl;
#endif
    }

    glPixelStorei( GL_UNPACK_ROW_LENGTH, static_cast<GLint>( rowLength ) );

    if( isCompressedInternalFormat( _internalFormat ) &&
        extensions->isCompressedTexSubImage2DSupported() )
    {
        extensions->glCompressedTexSubImage2D(
            target,
            0,
            0,
            0,
            image->s(),
            image->t(),
            ( GLenum )image->getPixelFormat(),
            static_cast<GLsizei>( image->getDataType() ),
            dataPtr
        );
    }
    else
    {
        glTexSubImage2D( target,
                         0,
                         0,
                         0,
                         image->s(),
                         image->t(),
                         ( GLenum )image->getPixelFormat(),
                         ( GLenum )image->getDataType(),
                         dataPtr );
    }

    if( pbo )
    {
        state.unbindPixelBufferObject();
    }

#ifdef DO_TIMING
    OSG_NOTICE << "glTexSubImage2D "
               << osg::Timer::instance()->delta_m( start_tick,
                                                   osg::Timer::instance()->tick() )
               << "ms" << std::endl;
#endif
}

void
TextureRectangle::computeInternalFormat() const
{
    if( _image.valid() )
    {
        computeInternalFormatWithImage( *_image );
    }
    else
    {
        computeInternalFormatType();
    }
}

void
TextureRectangle::copyTexImage2D( State& state,
                                  int    x,
                                  int    y,
                                  int    width,
                                  int    height )
{
    const unsigned int contextID = state.getContextID();

    if( _internalFormat == 0 )
    {
        _internalFormat = GL_RGBA;
    }

    // get the globj for the current contextID.
    TextureObject* textureObject = getTextureObject( contextID );

    if( textureObject )
    {
        if( width == ( int )_textureWidth && height == ( int )_textureHeight )
        {
            // we have a valid texture object which is the right size
            // so lets play clever and use copyTexSubImage2D instead.
            // this allows use to reuse the texture object and avoid
            // expensive memory allocations.
            copyTexSubImage2D( state, 0, 0, x, y, width, height );
            return;
        }
        // the relevant texture object is not of the right size so
        // needs to been deleted
        // remove previously bound textures.
        dirtyTextureObject();
        // note, dirtyTextureObject() dirties all the texture objects for
        // this texture, is this right?  Perhaps we should dirty just the
        // one for this context.  Note sure yet will leave till later.
        // RO July 2001.
    }

    // remove any previously assigned images as these are nolonger valid.
    _image = NULL;

    // switch off mip-mapping.
    //
    textureObject = generateAndAssignTextureObject( contextID, GL_TEXTURE_RECTANGLE );

    textureObject->bind( state );

    applyTexParameters( GL_TEXTURE_RECTANGLE, state );

    glCopyTexImage2D( GL_TEXTURE_RECTANGLE,
                      0,
                      static_cast<GLenum>( _internalFormat ),
                      x,
                      y,
                      width,
                      height,
                      0 );

    _textureWidth  = width;
    _textureHeight = height;
    // _numMipmapLevels = 1;

    textureObject->setAllocated( 1,
                                 static_cast<GLenum>( _internalFormat ),
                                 _textureWidth,
                                 _textureHeight,
                                 1,
                                 0 );

    // inform state that this texture is the current one bound.
    state.haveAppliedTextureAttribute( state.getActiveTextureUnit(), this );
}

void
TextureRectangle::copyTexSubImage2D( State& state,
                                     int    xoffset,
                                     int    yoffset,
                                     int    x,
                                     int    y,
                                     int    width,
                                     int    height )
{
    const unsigned int contextID = state.getContextID();

    if( _internalFormat == 0 )
    {
        _internalFormat = GL_RGBA;
    }

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject( contextID );

    if( textureObject )
    {
        // we have a valid image
        textureObject->bind( state );

        applyTexParameters( GL_TEXTURE_RECTANGLE, state );

        glCopyTexSubImage2D( GL_TEXTURE_RECTANGLE,
                             0,
                             xoffset,
                             yoffset,
                             x,
                             y,
                             width,
                             height );

        // inform state that this texture is the current one bound.
        state.haveAppliedTextureAttribute( state.getActiveTextureUnit(), this );
    }
    else
    {
        // no texture object already exsits for this context so need to
        // create it upfront - simply call copyTexImage2D.
        copyTexImage2D( state, x, y, width, height );
    }
}

void
TextureRectangle::allocateMipmap( State& ) const
{
    OSG_NOTICE << "Warning: TextureRectangle::allocateMipmap(State&) called eroneously, "
                  "GL_TEXTURE_RECTANGLE does not support mipmapping."
               << std::endl;
}
