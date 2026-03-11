/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 2D texture attribute. Manages texture object creation, image loading,
 * mipmap generation, and sampler parameters via DSA.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/textures/Texture.hpp>

namespace osg
{

    /** Encapsulates OpenGL 2D texture functionality. Doesn't support cube maps,
     * so ignore \a face parameters.
     */
    class OSG_EXPORT Texture2D : public osg::Inherit<Texture, Texture2D>
    {

        public:

            Texture2D();

            Texture2D( Image* image );

            template<class T>
            Texture2D( const osg::ref_ptr<T>& image ) :
                _textureWidth( 0 ),
                _textureHeight( 0 ),
                _numMipmapLevels( 0 )
            {
                setUseHardwareMipMapGeneration( true );
                setImage( image.get() );
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Texture2D( const Texture2D& text,
                       const CopyOp&    copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               Texture2D )

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
                return GL_TEXTURE_2D;
            }

            /** Sets the texture image. */
            void
            setImage( Image* image );

            template<class T>
            void
            setImage( const ref_ptr<T>& image )
            {
                setImage( image.get() );
            }

            /** Gets the texture image. */
            Image*
            getImage()
            {
                return _image.get();
            }

            /** Gets the const texture image. */
            inline const Image*
            getImage() const
            {
                return _image.get();
            }

            /** return true if the texture image data has been modified and the
             * associated GL texture object needs to be updated.*/
            bool
            isDirty( unsigned int contextID ) const override
            {
                return ( _image.valid() &&
                         _image->getModifiedCount() != _modifiedCount[contextID] );
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

            template<class T>
            void
            setImage( unsigned int,
                      const ref_ptr<T>& image )
            {
                setImage( image.get() );
            }

            /** Gets the texture image, ignoring face. */
            Image*
            getImage( unsigned int ) override
            {
                return _image.get();
            }

            /** Gets the const texture image, ignoring face. */
            const Image*
            getImage( unsigned int ) const override
            {
                return _image.get();
            }

            /** Gets the number of images that can be assigned to the Texture. */
            unsigned int
            getNumImages() const override
            {
                return 1;
            }

            /** Sets the texture width and height. If width or height are zero,
             * calculate the respective value from the source image size. */
            inline void
            setTextureSize( int width,
                            int height ) const
            {
                _textureWidth  = width;
                _textureHeight = height;
            }

            void
            setTextureWidth( int width )
            {
                _textureWidth = width;
            }

            void
            setTextureHeight( int height )
            {
                _textureHeight = height;
            }

            int
            getTextureWidth() const override
            {
                return _textureWidth;
            }

            int
            getTextureHeight() const override
            {
                return _textureHeight;
            }

            int
            getTextureDepth() const override
            {
                return 1;
            }

            class OSG_EXPORT SubloadCallback : public Referenced
            {
                public:

                    virtual bool
                    textureObjectValid( const Texture2D& texture,
                                        State&           state ) const
                    {
                        return texture.textureObjectValid( state );
                    }

                    virtual osg::ref_ptr<TextureObject>
                    generateTextureObject( const Texture2D& texture,
                                           State&           state ) const
                    {
                        return osg::Texture::generateTextureObject( &texture,
                                                                    state.getContextID(),
                                                                    GL_TEXTURE_2D );
                    }

                    virtual void
                    load( const Texture2D& texture,
                          State&           state ) const = 0;
                    virtual void
                    subload( const Texture2D& texture,
                             State&           state ) const = 0;
            };

            void
            setSubloadCallback( SubloadCallback* cb )
            {
                _subloadCallback = cb;
                ;
            }

            SubloadCallback*
            getSubloadCallback()
            {
                return _subloadCallback.get();
            }

            const SubloadCallback*
            getSubloadCallback() const
            {
                return _subloadCallback.get();
            }

            /** Helper function. Sets the number of mipmap levels created for this
             * texture. Should only be called within an osg::Texture::apply(), or
             * during a custom OpenGL texture load. */
            void
            setNumMipmapLevels( unsigned int num ) const
            {
                _numMipmapLevels = static_cast<GLsizei>( num );
            }

            /** Gets the number of mipmap levels created. */
            unsigned int
            getNumMipmapLevels() const
            {
                return static_cast<unsigned int>( _numMipmapLevels );
            }

            /** Copies pixels into a 2D texture image, as per glCopyTexImage2D.
             * Creates an OpenGL texture object from the current OpenGL background
             * framebuffer contents at position \a x, \a y with width \a width and
             * height \a height. \a width and \a height must be a power of two. */
            void
            copyTexImage2D( State& state,
                            int    x,
                            int    y,
                            int    width,
                            int    height );

            /** Copies a two-dimensional texture subimage, as per
             * glCopyTexSubImage2D. Updates a portion of an existing OpenGL
             * texture object from the current OpenGL background framebuffer
             * contents at position \a x, \a y with width \a width and height
             * \a height. Loads framebuffer data into the texture using offsets
             * \a xoffset and \a yoffset. \a width and \a height must be powers
             * of two. */
            void
            copyTexSubImage2D( State& state,
                               int    xoffset,
                               int    yoffset,
                               int    x,
                               int    y,
                               int    width,
                               int    height );

            /** Bind the texture object. If the texture object hasn't already been
             * compiled, create the texture mipmap levels. */
            void
            apply( State& state ) const override;

        protected:

            virtual ~Texture2D();

            void
            computeInternalFormat() const override;
            void
            allocateMipmap( State& state ) const override;

            /** Return true of the TextureObject assigned to the context associate with
             * osg::State object is valid.*/
            bool
            textureObjectValid( State& state ) const;

            friend class SubloadCallback;

            ref_ptr<Image>                       _image;

            /** Subloaded images can have different texture and image sizes. */
            mutable GLsizei                      _textureWidth, _textureHeight;

            /** Number of mipmap levels created. */
            mutable GLsizei                      _numMipmapLevels;

            ref_ptr<SubloadCallback>             _subloadCallback;

            typedef buffered_value<unsigned int> ImageModifiedCount;
            mutable ImageModifiedCount           _modifiedCount;
    };

}
