/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Default terrain rendering technique. Builds triangle meshes
 * from HeightFieldLayer data with texture coordinates.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osgTerrain/Locator.hpp>
#include <osgTerrain/TerrainTechnique.hpp>

namespace osgTerrain
{

    class OSGTERRAIN_EXPORT GeometryTechnique
        : public osg::Inherit<TerrainTechnique, GeometryTechnique>
    {
        public:

            GeometryTechnique();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            GeometryTechnique( const GeometryTechnique&,
                               const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgTerrain,
                               GeometryTechnique )

            virtual void
            init( int  dirtyMask,
                  bool assumeMultiThreaded );

            virtual Locator*
            computeMasterLocator();

            virtual void
            update( osgUtil::UpdateVisitor* nv );

            virtual void
            cull( osgUtil::CullVisitor* nv );

            /** Traverse the terain subgraph.*/
            virtual void
            traverse( osg::NodeVisitor& nv );

            virtual void
            cleanSceneGraph();

            void
            setFilterBias( float filterBias );

            float
            getFilterBias() const
            {
                return _filterBias;
            }

            void
            setFilterWidth( float filterWidth );

            float
            getFilterWidth() const
            {
                return _filterWidth;
            }

            void
            setFilterMatrix( const osg::Matrix3& matrix );

            osg::Matrix3&
            getFilterMatrix()
            {
                return _filterMatrix;
            }

            const osg::Matrix3&
            getFilterMatrix() const
            {
                return _filterMatrix;
            }

            enum FilterType
            {
                GAUSSIAN,
                SMOOTH,
                SHARPEN
            };

            void
            setFilterMatrixAs( FilterType filterType );

            /** If State is non-zero, this function releases any associated OpenGL
             * objects for the specified graphics context. Otherwise, releases OpenGL
             * objects for all graphics contexts. */
            virtual void
            releaseGLObjects( osg::State* = 0 ) const;

        protected:

            virtual ~GeometryTechnique();

            class BufferData : public osg::Referenced
            {
                public:

                    BufferData()
                    {
                    }

                    osg::ref_ptr<osg::MatrixTransform> _transform;
                    osg::ref_ptr<osg::Geode>           _geode;
                    osg::ref_ptr<osg::Geometry>        _geometry;

                protected:

                    ~BufferData()
                    {
                    }
            };

            virtual osg::dvec3
            computeCenterModel( BufferData& buffer,
                                Locator*    masterLocator );

            virtual void
            generateGeometry( BufferData&       buffer,
                              Locator*          masterLocator,
                              const osg::dvec3& centerModel );

            virtual void
            applyColorLayers( BufferData& buffer );

            virtual void
                                       applyTransparency( BufferData& buffer );

            std::mutex                 _writeBufferMutex;
            osg::ref_ptr<BufferData>   _currentBufferData;
            osg::ref_ptr<BufferData>   _newBufferData;

            float                      _filterBias;
            osg::ref_ptr<osg::Uniform> _filterBiasUniform;
            float                      _filterWidth;
            osg::ref_ptr<osg::Uniform> _filterWidthUniform;
            osg::Matrix3               _filterMatrix;
            osg::ref_ptr<osg::Uniform> _filterMatrixUniform;
    };

}
