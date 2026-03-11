/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Buffer texture backed by a buffer object. Provides random-access
 * reads from large data arrays in shaders via texelFetch.
 */
// -*-c++-*-

#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/geometry/BufferObject.hpp>
#include <osg/textures/Texture.hpp>

namespace osg
{

    /** Encapsulates OpenGL texture buffer functionality in a Texture delegating its
     * content to attached BufferObject
     */
    class OSG_EXPORT TextureBuffer : public osg::Inherit<Texture, TextureBuffer>
    {

        public:

            TextureBuffer();

            TextureBuffer( BufferData* image );

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            TextureBuffer( const TextureBuffer& text,
                           const CopyOp&        copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               TextureBuffer )

            Type
            getType() const override
            {
                return Type::TEXTURE;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& rhs ) const override;

            GLenum
            getTextureTarget() const override
            {
                return GL_TEXTURE_BUFFER;
            }

            /** Sets the texture image. */
            void
            setImage( Image* image );

            /** Gets the texture image. */
            Image*
            getImage()
            {
                return dynamic_cast<Image*>( _bufferData.get() );
            }

            /** Gets the const texture image. */
            inline const Image*
            getImage() const
            {
                return dynamic_cast<Image*>( _bufferData.get() );
            }

            /** return true if the texture image data has been modified and the
             * associated GL texture object needs to be updated.*/
            bool
            isDirty( unsigned int contextID ) const override
            {
                return ( _bufferData.valid() &&
                         _bufferData->getModifiedCount() != _modifiedCount[contextID] );
            }

            inline unsigned int&
            getModifiedCount( unsigned int contextID ) const
            {
                // get the modified count for the current contextID.
                return _modifiedCount[contextID];
            }

            /** Sets the texture image, ignoring face. */
            void
            setImage( unsigned int,
                      Image* image ) override
            {
                setImage( image );
            }

            /** Gets the texture image, ignoring face. */
            Image*
            getImage( unsigned int ) override
            {
                return getImage();
            }

            /** Gets the const texture image, ignoring face. */
            const Image*
            getImage( unsigned int ) const override
            {
                return getImage();
            }

            /** Gets the number of images that can be assigned to the Texture. */
            unsigned int
            getNumImages() const override
            {
                return 1;
            }

            /** Sets the texture width. If width is zero, calculate the value
             * from the source image width. */
            inline void
            setTextureWidth( int width )
            {
                _textureWidth = width;
            }

            /** Gets the texture width. */
            int
            getTextureWidth() const override
            {
                return _textureWidth;
            }

            int
            getTextureHeight() const override
            {
                return 1;
            }

            int
            getTextureDepth() const override
            {
                return 1;
            }

            void
            allocateMipmap( State& /*state*/ ) const override {};

            /** Bind the texture buffer.*/
            void
            apply( State& state ) const override;

            /**  Set setBufferData attached */
            void
            setBufferData( BufferData* bo );

            /**  Set setBufferData attached */
            const BufferData*
            getBufferData() const
            {
                return _bufferData.get();
            }

        protected:

            virtual ~TextureBuffer();

            void
                                                 computeInternalFormat() const override;

            ref_ptr<BufferData>                  _bufferData;

            GLsizei                              _textureWidth;

            typedef buffered_value<unsigned int> BufferDataModifiedCount;
            mutable BufferDataModifiedCount      _modifiedCount;
    };

}
