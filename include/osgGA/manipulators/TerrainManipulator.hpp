/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Terrain-aware orbit manipulator. Constrains the camera above
 * the terrain surface for landscape viewing.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgGA/manipulators/OrbitManipulator.hpp>

namespace osgGA
{

    class OSGGA_EXPORT TerrainManipulator
        : public osg::Inherit<OrbitManipulator, TerrainManipulator>
    {
            typedef OrbitManipulator inherited;

        public:

            TerrainManipulator( int flags = DEFAULT_SETTINGS );
            TerrainManipulator( const TerrainManipulator& tm,
                                const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgGA,
                               TerrainManipulator )

            enum RotationMode
            {
                ELEVATION_AZIM_ROLL,
                ELEVATION_AZIM,
            };

            virtual void
            setRotationMode( RotationMode mode );
            RotationMode
            getRotationMode() const;

            virtual void
            setByMatrix( const osg::dmat4& matrix );

            virtual void
            setTransformation( const osg::dvec3& eye,
                               const osg::dvec3& center,
                               const osg::dvec3& up );

            virtual void
            setNode( osg::Node* node );

        protected:

            virtual bool
            performMovementMiddleMouseButton( const double eventTimeDelta,
                                              const double dx,
                                              const double dy );
            virtual bool
            performMovementRightMouseButton( const double eventTimeDelta,
                                             const double dx,
                                             const double dy );

            bool
            intersect( const osg::dvec3& start,
                       const osg::dvec3& end,
                       osg::dvec3&       intersection ) const;
            void
                       clampOrientation();

            osg::dvec3 _previousUp;
    };

}
