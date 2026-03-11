/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Multisample 2D texture for MSAA render targets. Used as FBO
 * color/depth attachments for anti-aliased offscreen rendering.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/textures/Texture.hpp>

namespace osg
{

    /** Texture2DMultisample state class which encapsulates OpenGL 2D multisampled
     * texture functionality. Multisampled texture were introduced with OpenGL 3.1 and
     * extension GL_ARB_texture_multisample. See
     * http://www.opengl.org/registry/specs/ARB/texture_multisample.txt for more info.
     */

    class OSG_EXPORT Texture2DMultisample
        : public osg::Inherit<Texture, Texture2DMultisample>
    {
        public:

            Texture2DMultisample();

            Texture2DMultisample( GLsizei   numSamples,
                                  GLboolean fixedsamplelocations );

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Texture2DMultisample( const Texture2DMultisample& text,
                                  const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               Texture2DMultisample )

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
                return GL_TEXTURE_2D_MULTISAMPLE;
            }

            /** Texture2DMultisample is related to non fixed pipeline usage only so isn't
             * appropriate to enable/disable.*/
            bool
            getModeUsage( StateAttribute::ModeUsage& ) const override
            {
                return false;
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

            inline void
            setNumSamples( int samples )
            {
                _numSamples = samples;
            }

            GLsizei
            getNumSamples() const
            {
                return _numSamples;
            }

            inline void
            setFixedSampleLocations( GLboolean fixedSampleLocations )
            {
                _fixedsamplelocations = fixedSampleLocations;
            }

            inline GLboolean
            getFixedSampleLocations() const
            {
                return _fixedsamplelocations;
            }

            // unnecessary for Texture2DMultisample
            void
            setImage( unsigned int /*face*/,
                      Image* /*image*/ ) override
            {
            }

            Image*
            getImage( unsigned int /*face*/ ) override
            {
                return NULL;
            }

            const Image*
            getImage( unsigned int /*face*/ ) const override
            {
                return NULL;
            }

            unsigned int
            getNumImages() const override
            {
                return 0;
            }

            void
            allocateMipmap( State& /*state*/ ) const override
            {
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

            /** Bind the texture object. If the texture object hasn't already been
             * compiled, create the texture mipmap levels. */
            void
            apply( State& state ) const override;

        protected:

            virtual ~Texture2DMultisample();

            void
                              computeInternalFormat() const override;

            /** Subloaded images can have different texture and image sizes. */
            mutable GLsizei   _textureWidth, _textureHeight;

            mutable GLsizei   _numSamples;

            mutable GLboolean _fixedsamplelocations;
    };

}
