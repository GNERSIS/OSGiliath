/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Buffer texture backed by a buffer object. Provides random-access
 * reads from large data arrays in shaders via texelFetch.
 */
#include <osg/textures/TextureBuffer.hpp>

#include <osg/geometry/PrimitiveSet.hpp>
#include <osg/state/State.hpp>

using namespace osg;

TextureBuffer::TextureBuffer() :
    _textureWidth( 0 )
{
}

TextureBuffer::TextureBuffer( osg::BufferData* image ) :
    _textureWidth( 0 )
{
    setBufferData( image );
}

TextureBuffer::TextureBuffer( const TextureBuffer& text,
                              const CopyOp&        copyop ) :
    Inherit( text,
             copyop ),
    _textureWidth( text._textureWidth )
{
    if( text._bufferData.valid() )
    {
        setBufferData( osg::clone( text._bufferData.get(), copyop ) );
    }
}

TextureBuffer::~TextureBuffer()
{
    _bufferData = NULL;
}

void
TextureBuffer::setBufferData( BufferData* bufferdata )
{
    if( _bufferData == bufferdata )
    {
        return;
    }

    if( _bufferData.valid() )
    {
        _bufferData->removeClient( this );
    }

    _bufferData = bufferdata;
    _modifiedCount.setAllElementsTo( 0 );

    if( _bufferData.valid() )
    {
        _bufferData->addClient( this );

        /// set BufferObject if not set by user
        if( !_bufferData->getBufferObject() )
        {
            VertexBufferObject* bo = new VertexBufferObject();
            bo->setUsage( GL_STREAM_DRAW_ARB );
            bufferdata->setBufferObject( bo );
        }
    }
}

int
TextureBuffer::compare( const StateAttribute& sa ) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types(
        TextureBuffer,
        sa
    ) if( _bufferData != rhs._bufferData )    // smart pointer comparison.
    {
        if( _bufferData.valid() )
        {
            if( rhs._bufferData.valid() )
            {
                // int result = _bufferData->compare(*rhs._bufferData);
                int result = 0;
                if( _bufferData.get() < rhs._bufferData.get() )
                {
                    result = 1;
                }
                if( _bufferData.get() > rhs._bufferData.get() )
                {
                    result = -1;
                }
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
        else if( rhs._bufferData.valid() )
        {
            return -1;    // valid rhs._image is greater than null.
        }
    }

    if( !_bufferData && !rhs._bufferData )
    {
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
        // COMPARE_StateAttribute_Parameter(_usageHint)

        return 0;
}

void
TextureBuffer::setImage( Image* image )
{
    if( getImage() == image )
    {
        return;
    }

    setBufferData( image );
}

void
TextureBuffer::apply( State& state ) const
{
    const unsigned int contextID     = state.getContextID();

    TextureObject*     textureObject = getTextureObject( contextID );

/// This code could be useful but would require to watch BufferData changes
/// perhaps a mutable percontext dirtyflag (all reset to true onTextureObjectChanged)
/// would do the trick
#if 0
   if (textureObject)
    {
        bool textureObjectInvalidated = false;
        if (textureObject->_profile._internalFormat != _internalFormat )
        {
            textureObjectInvalidated=true;
        }

        if (textureObjectInvalidated)
        {
            //OSG_NOTICE<<"Discarding TextureObject"<<std::endl;
            _textureObjectBuffer[contextID]->release();
            _textureObjectBuffer[contextID] = 0;
            textureObject = 0;
        }
    }
#endif
    if( textureObject )
    {
        if( _bufferData.valid() &&
            _modifiedCount[contextID] != _bufferData->getModifiedCount() )
        {
            _modifiedCount[contextID] = _bufferData->getModifiedCount();

            GLBufferObject* glBufferObject =
                _bufferData->getBufferObject()->getOrCreateGLBufferObject( contextID );
            if( glBufferObject )
            {
                if( glBufferObject->isDirty() )
                {
                    // OSG_NOTICE<<"buffer upload"<<glBufferObject<<std::endl;
                    glBufferObject->compileBuffer();
                }
            }
        }
        textureObject->bind( state );
    }
    else if( _bufferData.valid() &&
             _bufferData
                 ->getBufferObject() )    //&& _bufferObject->getNumBufferData()>0 )
    {
        /// now compile bufferobject if required
        GLBufferObject* glBufferObject =
            _bufferData->getBufferObject()->getOrCreateGLBufferObject( contextID );
        if( glBufferObject )
        {
            const GLExtensions* extensions = state.get<GLExtensions>();

            _modifiedCount[contextID]      = _bufferData->getModifiedCount();

            textureObject =
                generateAndAssignTextureObject( contextID, GL_TEXTURE_BUFFER );
            textureObject->_profile._internalFormat =
                static_cast<GLenum>( _internalFormat );
            textureObject->bind( state );

            getTextureParameterDirty( state.getContextID() ) = false;

            computeInternalFormat();

            if( glBufferObject->isDirty() )
            {
                glBufferObject->compileBuffer();
            }

            textureObject->setAllocated( true );
            extensions->glBindBuffer( _bufferData->getBufferObject()->getTarget(), 0 );

            textureObject->bind( state );
            extensions->glTexBuffer( GL_TEXTURE_BUFFER,
                                     static_cast<GLenum>( _internalFormat ),
                                     glBufferObject->getGLObjectID() );
        }
    }
}

void
TextureBuffer::computeInternalFormat() const
{
    if( getImage() )
    {
        computeInternalFormatWithImage( *getImage() );
    }
    else
    {
        computeInternalFormatType();
    }
}
