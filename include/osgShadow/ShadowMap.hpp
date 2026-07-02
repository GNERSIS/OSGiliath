/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Basic shadow mapping technique. Renders a depth map from the
 * light's view and projects it for shadow lookup.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/lighting/LightSource.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/state/Material.hpp>
#include <osgShadow/ShadowTechnique.hpp>

namespace osgShadow
{

    /** ShadowedTexture provides an implementation of shadow textures.*/
    class OSGSHADOW_EXPORT ShadowMap : public osg::Inherit<ShadowTechnique, ShadowMap>
    {
        public:

            ShadowMap();

            ShadowMap( const ShadowMap&   es,
                       const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgShadow,
                               ShadowMap )

            /** Set the texture unit that the shadow texture will be applied on.*/
            void
            setTextureUnit( unsigned int unit );

            /** Get the texture unit that the shadow texture will be applied on.*/
            unsigned int
            getTextureUnit() const
            {
                return _shadowTextureUnit;
            }

            /** set the polygon offset used initially */
            void
            setPolygonOffset( const osg::vec2& polyOffset );

            /** get the used polygon offset */
            const osg::vec2&
            getPolygonOffset() const
            {
                return _polyOffset;
            }

            /** Set the values for the ambient bias the shader will use.*/
            void
            setAmbientBias( const osg::vec2& ambientBias );

            /** Get the values that are used for the ambient bias in the shader.*/
            const osg::vec2&
            getAmbientBias() const
            {
                return _ambientBias;
            }

            /** set the size in pixels x / y for the shadow texture.*/
            void
            setTextureSize( const osg::svec2& textureSize );

            /** Get the values that are used for the ambient bias in the shader.*/
            const osg::svec2&
            getTextureSize() const
            {
                return _textureSize;
            }

            /** Set the Light that will cast shadows */
            void
            setLight( osg::Light* light );
            void
            setLight( osg::LightSource* ls );

            typedef std::vector<osg::ref_ptr<osg::Uniform>> UniformList;

            typedef std::vector<osg::ref_ptr<osg::Shader>>  ShaderList;

            /** Add a shader to internal list, will be used instead of the default ones
             */
            inline void
            addShader( osg::Shader* shader )
            {
                _shaderList.push_back( shader );
            }

            template<class T>
            void
            addShader( const osg::ref_ptr<T>& shader )
            {
                addShader( shader.get() );
            }

            /** Reset internal shader list */
            inline void
            clearShaderList()
            {
                _shaderList.clear();
            }

            /** initialize the ShadowedScene and local cached data structures.*/
            virtual void
            init();

            /** run the update traversal of the ShadowedScene and update any loca chached
             * data structures.*/
            virtual void
            update( osg::NodeVisitor& nv );

            /** run the cull traversal of the ShadowedScene and set up the rendering for
             * this ShadowTechnique.*/
            virtual void
            cull( osgUtil::CullVisitor& cv );

            /** Clean scene graph from any shadow technique specific nodes, state and
             * drawables.*/
            virtual void
            cleanSceneGraph();

            /** Resize any per context GLObject buffers to specified size. */
            virtual void
            resizeGLObjectBuffers( unsigned int maxSize );

            /** If State is non-zero, this function releases any associated OpenGL
             * objects for the specified graphics context. Otherwise, releases OpenGL
             * objects for all graphics contexts. */
            virtual void
            releaseGLObjects( osg::State* = 0 ) const;

            // debug methods

            osg::ref_ptr<osg::Camera>
            makeDebugHUD();

        protected:

            virtual ~ShadowMap( void ) {};

            /** Create the managed Uniforms */
            virtual void
            createUniforms();

            virtual void
            createShaders();

            // forward declare, interface and implementation provided in ShadowMap.cpp
            class DrawableDrawWithDepthShadowComparisonOffCallback;

            osg::ref_ptr<osg::Camera>      _camera;
            osg::ref_ptr<osg::Texture2D>   _texture;
            osg::ref_ptr<osg::StateSet>    _stateset;
            osg::ref_ptr<osg::Program>     _program;
            osg::ref_ptr<osg::Light>       _light;

            osg::ref_ptr<osg::LightSource> _ls;

            osg::ref_ptr<osg::Uniform>     _ambientBiasUniform;
            osg::ref_ptr<osg::Uniform>     _shadowTextureMatrixUniform;
            UniformList                    _uniformList;
            ShaderList                     _shaderList;
            unsigned int                   _baseTextureUnit;
            unsigned int                   _shadowTextureUnit;
            osg::vec2                      _polyOffset;
            osg::vec2                      _ambientBias;
            osg::svec2                     _textureSize;
    };

}
