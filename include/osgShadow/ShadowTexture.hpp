/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Projective shadow texture technique. Projects a shadow texture
 * from the light source onto receiver geometry.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/state/Material.hpp>
#include <osgShadow/ShadowTechnique.hpp>

namespace osgShadow
{

    /** ShadowedTexture provides an implementation of shadow textures.*/
    class OSGSHADOW_EXPORT ShadowTexture
        : public osg::Inherit<ShadowTechnique, ShadowTexture>
    {
        public:

            ShadowTexture();

            ShadowTexture( const ShadowTexture& es,
                           const osg::CopyOp&   copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgShadow,
                               ShadowTexture )

            /** Set the texture unit that the shadow texture will be applied on.*/
            void
            setTextureUnit( unsigned int unit );

            /** Get the texture unit that the shadow texture will be applied on.*/
            unsigned int
            getTextureUnit() const
            {
                return _textureUnit;
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

        protected:

            virtual ~ShadowTexture()
            {
            }

            osg::ref_ptr<osg::Camera>    _camera;
            osg::ref_ptr<osg::Texture2D> _texture;
            osg::ref_ptr<osg::StateSet>  _stateset;
            osg::ref_ptr<osg::Material>  _material;
            unsigned int                 _textureUnit;
    };

}
