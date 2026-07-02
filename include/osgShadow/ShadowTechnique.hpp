/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for shadow rendering techniques.
 * Subclasses implement shadow map generation and projection.
 */
#pragma once

#include <osg/core/buffered_value.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgShadow/Export.hpp>
#include <osgUtil/culling/CullVisitor.hpp>

namespace osgShadow
{

    // forward declare ShadowedScene
    class ShadowedScene;

    /** ShadowTechnique is the base class for different shadow implementations.*/
    class OSGSHADOW_EXPORT ShadowTechnique : public osg::Object
    {
        public:

            ShadowTechnique();

            ShadowTechnique( const ShadowTechnique& es,
                             const osg::CopyOp&     copyop = osg::CopyOp::SHALLOW_COPY );

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const ShadowTechnique*>( obj ) != NULL;
            }

            virtual const char*
            libraryName() const
            {
                return "osgShadow";
            }

            virtual const char*
            className() const
            {
                return "ShadowTechnique";
            }

            virtual void
            setShadowedScene( ShadowedScene* ss );

            ShadowedScene*
            getShadowedScene()
            {
                return _shadowedScene;
            }

            const ShadowedScene*
            getShadowedScene() const
            {
                return _shadowedScene;
            }

            /** initialize the ShadowedScene and local cached data structures.*/
            virtual void
            init();

            /** run the update traversal of the ShadowedScene and update any local cached
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

            virtual void
            traverse( osg::NodeVisitor& nv );

            /** Dirty so that cached data structures are updated.*/
            virtual void
            dirty()
            {
                _dirty = true;
            }

            /** Resize any per context GLObject buffers to specified size. */
            virtual void
            resizeGLObjectBuffers( unsigned int maxSize ) = 0;

            /** If State is non-zero, this function releases any associated OpenGL
             * objects for the specified graphics context. Otherwise, releases OpenGL
             * objects for all graphics contexts. */
            virtual void
            releaseGLObjects( osg::State* = 0 ) const = 0;

        protected:

            class OSGSHADOW_EXPORT CameraCullCallback : public osg::NodeCallback
            {
                public:

                    CameraCullCallback( ShadowTechnique* st );

                    virtual void
                    operator()( osg::Node*,
                                osg::NodeVisitor* nv );

                protected:

                    ShadowTechnique* _shadowTechnique;
            };

            osg::vec3
            computeOrthogonalVector( const osg::vec3& direction ) const;

            virtual ~ShadowTechnique();

            friend class ShadowedScene;

            ShadowedScene* _shadowedScene;
            bool           _dirty;
    };

}
