/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Image load/store binding for compute shaders. Binds a texture
 * level/layer to an image unit for shader read/write access.
 */
/// author: Julien Valentin 2017 (mp3butcher@hotmail.com)

#pragma once

#include <osg/core/Export.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/textures/Texture.hpp>

namespace osg
{

    /** Bind texture to an image unit (available only if GL version is 4.2 or greater)
     * The format parameter for the image unit need not exactly match the texture
     * internal format, but if it is set to 0, the texture internal format will be used.
     * See http://www.opengl.org/registry/specs/ARB/shader_image_load_store.txt
     * void bindToImageUnit(unsigned int unit, GLenum access, GLenum format=0, int
     * level=0, bool layered=false, int layer=0);
     **/
    class OSG_EXPORT BindImageTexture
        : public osg::Inherit<osg::StateAttribute, BindImageTexture>
    {
        public:

            /** Type of access that will be performed on the texture image. */
            enum Access
            {
                NOT_USED   = 0,
                READ_ONLY  = GL_READ_ONLY_ARB,
                WRITE_ONLY = GL_WRITE_ONLY_ARB,
                READ_WRITE = GL_READ_WRITE_ARB,
            };

            BindImageTexture( GLuint        imageunit = 0,
                              osg::Texture* target    = 0,
                              Access        access    = READ_ONLY,
                              GLenum        format    = GL_RGBA8,
                              int           level     = 0,
                              bool          layered   = GL_FALSE,
                              int           layer     = 0 ) :
                Inherit(),
                _target( target ),
                _imageunit( imageunit ),
                _level( level ),
                _layered( layered ),
                _layer( layer ),
                _access( access ),
                _format( format )
            {
            }

            BindImageTexture( const BindImageTexture& o,
                              osg::CopyOp             op = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( o,
                         op ),
                _target( o._target ),
                _imageunit( o._imageunit ),
                _level( o._level ),
                _layered( o._layered ),
                _layer( o._layer ),
                _access( o._access ),
                _format( o._format )
            {
            }

            virtual ~BindImageTexture()
            {
            }

            OSG_REGISTER_TYPE( osg,
                               BindImageTexture )

            Type
            getType() const override
            {
                return Type::BINDIMAGETEXTURE;
            }

            inline void
            setImageUnit( GLuint i )
            {
                _imageunit = i;
            }

            inline GLuint
            getImageUnit() const
            {
                return _imageunit;
            }

            inline void
            setLevel( GLint i )
            {
                _level = i;
            }

            inline GLint
            getLevel() const
            {
                return _level;
            }

            inline void
            setIsLayered( GLboolean i )
            {
                _layered = i;
            }

            inline GLboolean
            getIsLayered() const
            {
                return _layered;
            }

            inline void
            setLayer( GLint i )
            {
                _layer = i;
            }

            inline GLint
            getLayer() const
            {
                return _layer;
            }

            inline void
            setAccess( Access i )
            {
                _access = i;
            }

            inline Access
            getAccess() const
            {
                return _access;
            }

            inline void
            setFormat( GLenum i )
            {
                _format = i;
            }

            inline GLenum
            getFormat() const
            {
                return _format;
            }

            inline void
            setTexture( osg::Texture* target )
            {
                _target = target;
            }

            inline osg::Texture*
            getTexture()
            {
                return _target.get();
            }

            inline const osg::Texture*
            getTexture() const
            {
                return _target.get();
            }

            void
            apply( osg::State& state ) const override;

            int
            compare( const osg::StateAttribute& sa ) const override;

            unsigned
            getMember() const override
            {
                return static_cast<unsigned int>( _imageunit );
            }

        protected:

            osg::ref_ptr<osg::Texture> _target;
            GLuint                     _imageunit;
            GLint                      _level;
            GLboolean                  _layered;
            GLint                      _layer;
            Access                     _access;
            GLenum                     _format;
    };

}
