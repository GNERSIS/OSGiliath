/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Framebuffer object wrapper for offscreen rendering. Manages
 * color/depth/stencil attachments using textures or renderbuffers.
 */
// initial FBO support written by Marco Jez, June 2005.

#pragma once

#include <osg/core/buffered_value.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/GL>
#include <osg/nodes/Camera.hpp>
#include <osg/textures/Texture.hpp>

namespace osg
{

    /**************************************************************************
     * RenderBuffer
     **************************************************************************/

    class OSG_EXPORT RenderBuffer : public osg::Inherit<Object, RenderBuffer>
    {
        public:

            RenderBuffer();
            RenderBuffer( int    width,
                          int    height,
                          GLenum internalFormat,
                          int    samples      = 0,
                          int    colorSamples = 0 );
            RenderBuffer( const RenderBuffer& copy,
                          const CopyOp&       copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               RenderBuffer )

            inline int
            getWidth() const;
            inline int
            getHeight() const;
            inline void
            setWidth( int w );
            inline void
            setHeight( int h );
            inline void
            setSize( int w,
                     int h );
            inline GLenum
            getInternalFormat() const;
            inline void
            setInternalFormat( GLenum format );
            inline int
            getSamples() const;
            inline int
            getColorSamples() const;
            inline void
            setSamples( int samples );
            inline void
            setColorSamples( int colorSamples );

            GLuint
            getObjectID( unsigned int        contextID,
                         const GLExtensions* ext ) const;
            inline int
            compare( const RenderBuffer& rb ) const;

            static int
            getMaxSamples( unsigned int        contextID,
                           const GLExtensions* ext );

            /** Resize any per context GLObject buffers to specified size. */
            virtual void
            resizeGLObjectBuffers( unsigned int maxSize );

            /** If State is non-zero, this function releases any associated OpenGL
             * objects for the specified graphics context. Otherwise, releases OpenGL
             * objexts for all graphics contexts. */
            virtual void
            releaseGLObjects( osg::State* = 0 ) const;

        protected:

            virtual ~RenderBuffer();

            RenderBuffer&
            operator=( const RenderBuffer& )
            {
                return *this;
            }

            inline void
            dirtyAll() const;

        private:

            mutable buffered_value<GLuint> _objectID;
            mutable buffered_value<int>    _dirty;

            GLenum                         _internalFormat;
            int                            _width;
            int                            _height;
            // "samples" in the framebuffer_multisample extension is equivalent to
            // "coverageSamples" in the framebuffer_multisample_coverage extension.
            int                            _samples;
            int                            _colorSamples;
    };

    // INLINE METHODS

    inline int
    RenderBuffer::getWidth() const
    {
        return _width;
    }

    inline int
    RenderBuffer::getHeight() const
    {
        return _height;
    }

    inline void
    RenderBuffer::setWidth( int w )
    {
        _width = w;
        dirtyAll();
    }

    inline void
    RenderBuffer::setHeight( int h )
    {
        _height = h;
        dirtyAll();
    }

    inline void
    RenderBuffer::setSize( int w,
                           int h )
    {
        _width  = w;
        _height = h;
        dirtyAll();
    }

    inline GLenum
    RenderBuffer::getInternalFormat() const
    {
        return _internalFormat;
    }

    inline void
    RenderBuffer::setInternalFormat( GLenum format )
    {
        _internalFormat = format;
        dirtyAll();
    }

    inline int
    RenderBuffer::getSamples() const
    {
        return _samples;
    }

    inline int
    RenderBuffer::getColorSamples() const
    {
        return _colorSamples;
    }

    inline void
    RenderBuffer::setSamples( int samples )
    {
        _samples = samples;
        dirtyAll();
    }

    inline void
    RenderBuffer::setColorSamples( int colorSamples )
    {
        _colorSamples = colorSamples;
        dirtyAll();
    }

    inline void
    RenderBuffer::dirtyAll() const
    {
        _dirty.setAllElementsTo( 1 );
    }

    inline int
    RenderBuffer::compare( const RenderBuffer& rb ) const
    {
        if( &rb == this )
        {
            return 0;
        }
        if( _internalFormat < rb._internalFormat )
        {
            return -1;
        }
        if( _internalFormat > rb._internalFormat )
        {
            return 1;
        }
        if( _width < rb._width )
        {
            return -1;
        }
        if( _width > rb._width )
        {
            return 1;
        }
        if( _height < rb._height )
        {
            return -1;
        }
        if( _height > rb._height )
        {
            return 1;
        }
        return 0;
    }

    /**************************************************************************
     * FrameBufferAttachement
     **************************************************************************/
    class Texture1D;
    class Texture2D;
    class Texture2DMultisample;
    class Texture3D;
    class Texture2DArray;
    class TextureCubeMap;
    class TextureRectangle;

    class OSG_EXPORT FrameBufferAttachment
    {
        public:

            FrameBufferAttachment();
            FrameBufferAttachment( const FrameBufferAttachment& copy );

            explicit FrameBufferAttachment( RenderBuffer* target );
            explicit FrameBufferAttachment( Texture1D*   target,
                                            unsigned int level = 0 );
            explicit FrameBufferAttachment( Texture2D*   target,
                                            unsigned int level = 0 );
            explicit FrameBufferAttachment( Texture2DMultisample* target,
                                            unsigned int          level = 0 );
            explicit FrameBufferAttachment( Texture3D*   target,
                                            unsigned int zoffset,
                                            unsigned int level = 0 );
            explicit FrameBufferAttachment( Texture2DArray* target,
                                            unsigned int    layer,
                                            unsigned int    level = 0 );
            explicit FrameBufferAttachment( TextureCubeMap* target,
                                            unsigned int    face,
                                            unsigned int    level = 0 );
            explicit FrameBufferAttachment( TextureRectangle* target );
            explicit FrameBufferAttachment( Camera::Attachment& attachment );

            ~FrameBufferAttachment();

            FrameBufferAttachment&
            operator=( const FrameBufferAttachment& copy );

            bool
            isMultisample() const;
            void
            createRequiredTexturesAndApplyGenerateMipMap(
                State&              state,
                const GLExtensions* ext
            ) const;
            void
            attach( State&              state,
                    GLenum              target,
                    GLenum              attachment_point,
                    const GLExtensions* ext ) const;
            int
            compare( const FrameBufferAttachment& fa ) const;

            RenderBuffer*
            getRenderBuffer();
            const RenderBuffer*
            getRenderBuffer() const;

            Texture*
            getTexture();
            const Texture*
            getTexture() const;

            unsigned int
            getCubeMapFace() const;
            unsigned int
            getTextureLevel() const;
            unsigned int
            getTexture3DZOffset() const;
            unsigned int
            getTextureArrayLayer() const;

            void
            resizeGLObjectBuffers( unsigned int maxSize );
            void
            releaseGLObjects( osg::State* = 0 ) const;

        private:

            // use the Pimpl idiom to avoid dependency from
            // all Texture* headers
            struct Pimpl;
            Pimpl* _ximpl;
    };

    /**************************************************************************
     * FrameBufferObject
     **************************************************************************/
    class OSG_EXPORT FrameBufferObject
        : public osg::Inherit<StateAttribute, FrameBufferObject>
    {
        public:

            typedef std::map<Camera::BufferComponent, FrameBufferAttachment>
                                            AttachmentMap;
            typedef std::vector<GLenum>     MultipleRenderingTargets;

            typedef Camera::BufferComponent BufferComponent;

            FrameBufferObject();
            FrameBufferObject( const FrameBufferObject& copy,
                               const CopyOp&            copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               FrameBufferObject )

            Type
            getType() const override
            {
                return Type::FRAME_BUFFER_OBJECT;
            }

            inline const AttachmentMap&
            getAttachmentMap() const;

            void
            setAttachment( BufferComponent              attachment_point,
                           const FrameBufferAttachment& attachment );
            inline const FrameBufferAttachment&
            getAttachment( BufferComponent attachment_point ) const;
            inline bool
            hasAttachment( BufferComponent attachment_point ) const;

            inline bool
            hasMultipleRenderingTargets() const
            {
                return !_drawBuffers.empty();
            }

            inline const MultipleRenderingTargets&
            getMultipleRenderingTargets() const
            {
                return _drawBuffers;
            }

            bool
            isMultisample() const;

            int
            compare( const StateAttribute& sa ) const override;

            void
            apply( State& state ) const override;

            inline GLuint
            getHandle( unsigned int contextID ) const
            {
                return _fboID[contextID];
            }

            enum BindTarget
            {
                READ_FRAMEBUFFER      = GL_READ_FRAMEBUFFER_EXT,
                DRAW_FRAMEBUFFER      = GL_DRAW_FRAMEBUFFER_EXT,
                READ_DRAW_FRAMEBUFFER = GL_FRAMEBUFFER_EXT,
            };

            /** Bind the FBO as either the read or draw target, or both. */
            void
            apply( State&     state,
                   BindTarget target ) const;

            /** Resize any per context GLObject buffers to specified size. */
            void
            resizeGLObjectBuffers( unsigned int maxSize ) override;

            /** If State is non-zero, this function releases any associated OpenGL
             * objects for the specified graphics context. Otherwise, releases OpenGL
             * objexts for all graphics contexts. */
            void
            releaseGLObjects( osg::State* = 0 ) const override;

        protected:

            virtual ~FrameBufferObject();

            FrameBufferObject&
            operator=( const FrameBufferObject& )
            {
                return *this;
            }

            void
            updateDrawBuffers();

            inline void
            dirtyAll();

            GLenum
            convertBufferComponentToGLenum( BufferComponent attachment_point ) const;

        private:

            AttachmentMap                  _attachments;

            // Buffers passed to glDrawBuffers when using multiple render targets.
            MultipleRenderingTargets       _drawBuffers;

            mutable buffered_value<int>    _dirtyAttachmentList;
            mutable buffered_value<int>    _unsupported;
            mutable buffered_value<GLuint> _fboID;
    };

    // INLINE METHODS

    inline const FrameBufferObject::AttachmentMap&
    FrameBufferObject::getAttachmentMap() const
    {
        return _attachments;
    }

    inline bool
    FrameBufferObject::hasAttachment(
        FrameBufferObject::BufferComponent attachment_point
    ) const
    {
        return _attachments.find( attachment_point ) != _attachments.end();
    }

    inline const FrameBufferAttachment&
    FrameBufferObject::getAttachment(
        FrameBufferObject::BufferComponent attachment_point
    ) const
    {
        return _attachments.find( attachment_point )->second;
    }

    inline void
    FrameBufferObject::dirtyAll()
    {
        _dirtyAttachmentList.setAllElementsTo( 1 );
    }

    class OSG_EXPORT GLRenderBufferManager : public GLObjectManager
    {
        public:

            GLRenderBufferManager( unsigned int contextID );
            virtual void
            deleteGLObject( GLuint globj );
    };

    class OSG_EXPORT GLFrameBufferObjectManager : public GLObjectManager
    {
        public:

            GLFrameBufferObjectManager( unsigned int contextID );
            virtual void
            deleteGLObject( GLuint globj );
    };

}
