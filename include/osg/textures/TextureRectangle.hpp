/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Rectangle texture using non-normalized [0,width]x[0,height]
 * coordinates. Used for exact-pixel screen-space effects.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/textures/Texture.hpp>

namespace osg
{

    /** Texture state class which encapsulates OpenGL texture functionality. */
    class OSG_EXPORT TextureRectangle : public osg::Inherit<Texture, TextureRectangle>
    {

        public:

            TextureRectangle();

            TextureRectangle( Image* image );

            template<class T>
            TextureRectangle( const osg::ref_ptr<T>& image ) :
                _textureWidth( 0 ),
                _textureHeight( 0 )
            {
                setWrap( WRAP_S, CLAMP );
                setWrap( WRAP_T, CLAMP );

                setFilter( MIN_FILTER, LINEAR );
                setFilter( MAG_FILTER, LINEAR );

                setImage( image.get() );
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            TextureRectangle( const TextureRectangle& text,
                              const CopyOp&           copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               TextureRectangle )

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
                return GL_TEXTURE_RECTANGLE;
            }

            /** Set the texture image. */
            void
            setImage( Image* image );

            template<class T>
            void
            setImage( const ref_ptr<T>& image )
            {
                setImage( image.get() );
            }

            /** Get the texture image. */
            Image*
            getImage()
            {
                return _image.get();
            }

            /** Get the const texture image. */
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

            /** Set the texture image, ignoring face value as there is only one image. */
            void
            setImage( unsigned int,
                      Image* image ) override
            {
                setImage( image );
            }

            /** Get the texture image, ignoring face value as there is only one image. */
            Image*
            getImage( unsigned int ) override
            {
                return _image.get();
            }

            /** Get the const texture image, ignoring face value as there is only one
             * image. */
            const Image*
            getImage( unsigned int ) const override
            {
                return _image.get();
            }

            /** Get the number of images that can be assigned to the Texture. */
            unsigned int
            getNumImages() const override
            {
                return 1;
            }

            /** Set the texture width and height. If width or height are zero then
             * the respective size value is calculated from the source image sizes.
             */
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

            class SubloadCallback : public Referenced
            {
                public:

                    virtual void
                    load( const TextureRectangle&,
                          State& ) const = 0;
                    virtual void
                    subload( const TextureRectangle&,
                             State& ) const = 0;
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

            /** On first apply (unless already compiled), create and bind the
             * texture, subsequent apply will simply bind to texture.
             */
            void
            apply( State& state ) const override;

        protected:

            virtual ~TextureRectangle();

            void
            computeInternalFormat() const override;
            void
            allocateMipmap( State& state ) const override;

            void
            applyTexImage_load( GLenum   target,
                                Image*   image,
                                State&   state,
                                GLsizei& inwidth,
                                GLsizei& inheight ) const;
            void
                            applyTexImage_subload( GLenum   target,
                                                   Image*   image,
                                                   State&   state,
                                                   GLsizei& inwidth,
                                                   GLsizei& inheight,
                                                   GLint&   inInternalFormat ) const;

            ref_ptr<Image>  _image;

            // subloaded images can have different texture and image sizes.
            mutable GLsizei _textureWidth, _textureHeight;

            ref_ptr<SubloadCallback>             _subloadCallback;

            typedef buffered_value<unsigned int> ImageModifiedCount;
            mutable ImageModifiedCount           _modifiedCount;
    };

}
