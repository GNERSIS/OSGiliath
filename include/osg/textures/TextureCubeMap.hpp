/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Cube map texture for environment mapping, skyboxes, and
 * reflection/refraction effects. Six faces addressed by direction vector.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/textures/Texture.hpp>

namespace osg
{

    /** TextureCubeMap state class which encapsulates OpenGL texture cubemap
     * functionality. */
    class OSG_EXPORT TextureCubeMap : public osg::Inherit<Texture, TextureCubeMap>
    {

        public:

            TextureCubeMap();

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            TextureCubeMap( const TextureCubeMap& cm,
                            const CopyOp&         copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               TextureCubeMap )

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
                return GL_TEXTURE_CUBE_MAP;
            }

            enum Face
            {
                POSITIVE_X = 0,
                NEGATIVE_X = 1,
                POSITIVE_Y = 2,
                NEGATIVE_Y = 3,
                POSITIVE_Z = 4,
                NEGATIVE_Z = 5,
            };

            /** Set the texture image for specified face. */
            void
            setImage( unsigned int face,
                      Image*       image ) override;

            template<class T>
            void
            setImage( unsigned int      face,
                      const ref_ptr<T>& image )
            {
                setImage( face, image.get() );
            }

            /** Get the texture image for specified face. */
            Image*
            getImage( unsigned int face ) override;

            /** Get the const texture image for specified face. */
            const Image*
            getImage( unsigned int face ) const override;

            /** Get the number of images that can be assigned to the Texture. */
            unsigned int
            getNumImages() const override
            {
                return 6;
            }

            /** return true if the texture image data has been modified and the
             * associated GL texture object needs to be updated.*/
            bool
            isDirty( unsigned int contextID ) const override
            {
                return ( _images[0].valid() &&
                         _images[0]->getModifiedCount() !=
                         _modifiedCount[0][contextID] ) ||
                       ( _images[1].valid() &&
                         _images[1]->getModifiedCount() !=
                         _modifiedCount[1][contextID] ) ||
                       ( _images[2].valid() &&
                         _images[2]->getModifiedCount() !=
                         _modifiedCount[2][contextID] ) ||
                       ( _images[3].valid() &&
                         _images[3]->getModifiedCount() !=
                         _modifiedCount[3][contextID] ) ||
                       ( _images[4].valid() &&
                         _images[4]->getModifiedCount() !=
                         _modifiedCount[4][contextID] ) ||
                       ( _images[5].valid() &&
                         _images[5]->getModifiedCount() !=
                         _modifiedCount[5][contextID] );
            }

            inline unsigned int&
            getModifiedCount( unsigned int face,
                              unsigned int contextID ) const
            {
                // get the modified count for the current contextID.
                return _modifiedCount[face][contextID];
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

            class OSG_EXPORT SubloadCallback : public Referenced
            {
                public:

                    virtual void
                    load( const TextureCubeMap& texture,
                          State&                state ) const = 0;
                    virtual void
                    subload( const TextureCubeMap& texture,
                             State&                state ) const = 0;
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

            /** Set the number of mip map levels the texture has been created with.
             * Should only be called within an osg::Texuture::apply() and custom OpenGL
             * texture load.
             */
            void
            setNumMipmapLevels( unsigned int num ) const
            {
                _numMipmapLevels = static_cast<GLsizei>( num );
            }

            /** Get the number of mip map levels the texture has been created with. */
            unsigned int
            getNumMipmapLevels() const
            {
                return static_cast<unsigned int>( _numMipmapLevels );
            }

            /** Copies a two-dimensional texture subimage, as per
             * glCopyTexSubImage2D. Updates a portion of an existing OpenGL
             * texture object from the current OpenGL background framebuffer
             * contents at position \a x, \a y with width \a width and height
             * \a height. Loads framebuffer data into the texture using offsets
             * \a xoffset and \a yoffset. \a width and \a height must be powers
             * of two. */
            void
            copyTexSubImageCubeMap( State& state,
                                    int    face,
                                    int    xoffset,
                                    int    yoffset,
                                    int    x,
                                    int    y,
                                    int    width,
                                    int    height );

            /** On first apply (unless already compiled), create the mipmapped
             * texture and bind it. Subsequent apply will simple bind to texture.
             */
            void
            apply( State& state ) const override;

        protected:

            virtual ~TextureCubeMap();

            bool
            imagesValid() const;

            void
            computeInternalFormat() const override;
            void
                                     allocateMipmap( State& state ) const override;

            ref_ptr<Image>           _images[6];

            // subloaded images can have different texture and image sizes.
            mutable GLsizei          _textureWidth, _textureHeight;

            // number of mip map levels the texture has been created with,
            mutable GLsizei          _numMipmapLevels;

            ref_ptr<SubloadCallback> _subloadCallback;

            typedef buffered_value<unsigned int> ImageModifiedCount;
            mutable ImageModifiedCount           _modifiedCount[6];
    };

}
