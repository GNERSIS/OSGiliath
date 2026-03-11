/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Multisample 2D texture for MSAA render targets. Used as FBO
 * color/depth attachments for anti-aliased offscreen rendering.
 */
#include <osg/textures/Texture2DMultisample.hpp>

#include <osg/core/Notify.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

Texture2DMultisample::Texture2DMultisample() :
    _textureWidth( 0 ),
    _textureHeight( 0 ),
    _numSamples( 1 ),
    _fixedsamplelocations( GL_FALSE )
{
}

Texture2DMultisample::Texture2DMultisample( GLsizei   numSamples,
                                            GLboolean fixedsamplelocations ) :
    _textureWidth( 0 ),
    _textureHeight( 0 ),
    _numSamples( numSamples ),
    _fixedsamplelocations( fixedsamplelocations )
{
}

Texture2DMultisample::Texture2DMultisample( const Texture2DMultisample& text,
                                            const CopyOp&               copyop ) :
    Inherit( text,
             copyop ),
    _textureWidth( text._textureWidth ),
    _textureHeight( text._textureHeight ),
    _numSamples( text._numSamples ),
    _fixedsamplelocations( text._fixedsamplelocations )
{
}

Texture2DMultisample::~Texture2DMultisample()
{
}

int
Texture2DMultisample::compare( const StateAttribute& sa ) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types( Texture2DMultisample, sa ) int result =
        compareTexture( rhs );
    if( result != 0 )
    {
        return result;
    }

    // compare each parameter in turn against the rhs.
    if( _textureWidth != 0 && rhs._textureWidth != 0 )
    {
        COMPARE_StateAttribute_Parameter( _textureWidth )
    }
    if( _textureHeight != 0 && rhs._textureHeight != 0 )
    {
        COMPARE_StateAttribute_Parameter( _textureHeight )
    }
    if( _numSamples != 0 && rhs._numSamples != 0 )
    {
        COMPARE_StateAttribute_Parameter( _numSamples )
    }
    if( _fixedsamplelocations != 0 && rhs._fixedsamplelocations != 0 )
    {
        COMPARE_StateAttribute_Parameter( _fixedsamplelocations )
    }

    return 0;    // passed all the above comparison macros, must be equal.
}

void
Texture2DMultisample::apply( State& state ) const
{
    // current OpenGL context.
    const unsigned int  contextID  = state.getContextID();
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( !extensions->isTextureMultisampledSupported )
    {
        OSG_INFO << "Texture2DMultisample not supported." << std::endl;
        return;
    }

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject( contextID );

    if( textureObject )
    {
        textureObject->bind( state );
    }
    else if( ( _textureWidth != 0 ) && ( _textureHeight != 0 ) && ( _numSamples != 0 ) )
    {
        // no image present, but dimensions at set so lets create the texture
        GLenum texStorageSizedInternalFormat =
            extensions->isTextureStorageEnabled && ( _borderWidth == 0 )
                ? selectSizedInternalFormat()
                : 0;
        if( texStorageSizedInternalFormat != 0 )
        {
            textureObject =
                generateAndAssignTextureObject( contextID,
                                                getTextureTarget(),
                                                1,
                                                texStorageSizedInternalFormat,
                                                _textureWidth,
                                                _textureHeight,
                                                1,
                                                0 );
            textureObject->bind( state );
            if( !textureObject->_allocated )
            {
                extensions->glTexStorage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE,
                                                       _numSamples,
                                                       texStorageSizedInternalFormat,
                                                       _textureWidth,
                                                       _textureHeight,
                                                       _fixedsamplelocations );
            }
        }
        else
        {
            textureObject =
                generateAndAssignTextureObject( contextID,
                                                getTextureTarget(),
                                                1,
                                                static_cast<GLenum>( _internalFormat ),
                                                _textureWidth,
                                                _textureHeight,
                                                1,
                                                _borderWidth );
            textureObject->bind( state );

            extensions->glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE,
                                                 _numSamples,
                                                 _internalFormat,
                                                 _textureWidth,
                                                 _textureHeight,
                                                 _fixedsamplelocations );
        }
        textureObject->setAllocated( 1,
                                     texStorageSizedInternalFormat != 0
                                         ? texStorageSizedInternalFormat
                                         : static_cast<GLenum>( _internalFormat ),
                                     _textureWidth,
                                     _textureHeight,
                                     1,
                                     _borderWidth );
    }
    else
    {
        glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, 0 );
    }
}

void
Texture2DMultisample::computeInternalFormat() const
{
    computeInternalFormatType();
}
