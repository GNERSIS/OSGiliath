/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 3D volume texture attribute. Used for volumetric rendering, 3D
 * noise, and medical/scientific data visualization.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/textures/Texture.hpp>

namespace osg
{

    /** Encapsulates OpenGL 3D texture functionality. Doesn't support cube maps,
     * so ignore \a face parameters.
     */
    class OSG_EXPORT Texture3D : public osg::Inherit<Texture, Texture3D>
    {

        public:

            Texture3D();

            Texture3D( Image* image );

            template<class T>
            Texture3D( const osg::ref_ptr<T>& image ) :
                _textureWidth( 0 ),
                _textureHeight( 0 ),
                _textureDepth( 0 ),
                _numMipmapLevels( 0 )
            {
                setImage( image.get() );
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Texture3D( const Texture3D& text,
                       const CopyOp&    copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               Texture3D )

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
                return GL_TEXTURE_3D;
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

            /** Sets the texture width, height, and depth. If width, height, or
             * depth are zero, calculate the respective value from the source
             * image size. */
            inline void
            setTextureSize( int width,
                            int height,
                            int depth ) const
            {
                _textureWidth  = width;
                _textureHeight = height;
                _textureDepth  = depth;
            }

            /** Gets the texture subload width. */
            inline void
            getTextureSize( int& width,
                            int& height,
                            int& depth ) const
            {
                width  = _textureWidth;
                height = _textureHeight;
                depth  = _textureDepth;
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

            void
            setTextureDepth( int depth )
            {
                _textureDepth = depth;
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
                return _textureDepth;
            }

            class OSG_EXPORT SubloadCallback : public Referenced
            {
                public:

                    virtual void
                    load( const Texture3D& texture,
                          State&           state ) const = 0;
                    virtual void
                    subload( const Texture3D& texture,
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

            /** Copies a two-dimensional texture subimage, as per
             * glCopyTexSubImage3D. Updates a portion of an existing OpenGL
             * texture object from the current OpenGL background framebuffer
             * contents at position \a x, \a y with width \a width and height
             * \a height. Loads framebuffer data into the texture using offsets
             * \a xoffset, \a yoffset, and \a zoffset. \a width and \a height
             * must be powers of two. */
            void
            copyTexSubImage3D( State& state,
                               int    xoffset,
                               int    yoffset,
                               int    zoffset,
                               int    x,
                               int    y,
                               int    width,
                               int    height );

            /** Bind the texture object. If the texture object hasn't already been
             * compiled, create the texture mipmap levels. */
            void
            apply( State& state ) const override;

        protected:

            virtual ~Texture3D();

            void
            computeRequiredTextureDimensions( State&            state,
                                              const osg::Image& image,
                                              GLsizei&          width,
                                              GLsizei&          height,
                                              GLsizei&          depth,
                                              GLsizei&          numMipmapLevels ) const;

            void
            computeInternalFormat() const override;
            void
            allocateMipmap( State& state ) const override;

            void
                                     applyTexImage3D( GLenum   target,
                                                      Image*   image,
                                                      State&   state,
                                                      GLsizei& inwidth,
                                                      GLsizei& inheight,
                                                      GLsizei& indepth,
                                                      GLsizei& numMipmapLevels ) const;

            /** It's not ideal that _image is mutable, but it's required since
             * Image::ensureDimensionsArePowerOfTwo() can only be called in a
             * valid OpenGL context, and therefore within Texture::apply, which
             * is const. */
            mutable ref_ptr<Image>   _image;

            /** Subloaded images can have different texture and image sizes. */
            mutable GLsizei          _textureWidth, _textureHeight, _textureDepth;

            /** Number of mip map levels the texture has been created with, */
            mutable GLsizei          _numMipmapLevels;

            ref_ptr<SubloadCallback> _subloadCallback;

            typedef buffered_value<unsigned int> ImageModifiedCount;
            mutable ImageModifiedCount           _modifiedCount;
    };

}
