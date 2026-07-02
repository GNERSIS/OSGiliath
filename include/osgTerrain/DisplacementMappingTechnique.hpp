/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * GPU displacement mapping terrain technique. Uses vertex
 * texture fetch to displace terrain mesh on the GPU.
 */
#pragma once

#include <mutex>
#include <osg/core/Inherit.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/state/Program.hpp>
#include <osgTerrain/GeometryTechnique.hpp>

namespace osgTerrain
{

    class OSGTERRAIN_EXPORT DisplacementMappingTechnique
        : public osg::Inherit<osgTerrain::TerrainTechnique, DisplacementMappingTechnique>
    {
        public:

            DisplacementMappingTechnique();

            DisplacementMappingTechnique( const DisplacementMappingTechnique&,
                                          const osg::CopyOp& copyop =
                                              osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgTerrain,
                               DisplacementMappingTechnique )

            virtual void
            init( int  dirtyMask,
                  bool assumeMultiThreaded );
            virtual void
            update( osgUtil::UpdateVisitor* uv );
            virtual void
            cull( osgUtil::CullVisitor* cv );
            virtual void
            traverse( osg::NodeVisitor& nv );
            virtual void
            cleanSceneGraph();
            virtual void
            releaseGLObjects( osg::State* state ) const;

        protected:

            virtual ~DisplacementMappingTechnique();

            mutable std::mutex                 _traversalMutex;

            mutable std::mutex                 _transformMutex;
            osg::ref_ptr<osg::MatrixTransform> _transform;

            std::atomic<unsigned>              _currentTraversalCount;
    };

}
