/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Normal generation visitor. Computes smooth per-vertex normals
 * from face normals with crease-angle thresholds.
 */
#pragma once

#include <osg/geometry/Geometry.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgUtil/Export>

namespace osgUtil
{

    /** A smoothing visitor for calculating smoothed normals for
     * osg::GeoSet's which contains surface primitives.
     */
    class OSGUTIL_EXPORT SmoothingVisitor : public osg::DualModeVisitor
    {
        public:

            using DualModeVisitor::apply;

            /// default to traversing all children.
            SmoothingVisitor();
            virtual ~SmoothingVisitor();

            /// smooth geoset by creating per vertex normals.
            static void
            smooth( osg::Geometry& geoset,
                    double         creaseAngle = osg::PI );

            /// apply smoothing method to all geometries.
            virtual void
            apply( osg::Geometry& geom );

            /// set the maximum angle, in radians, at which angle between adjacent
            /// triangles that normals are smoothed for edges that greater the shared
            /// vertices are duplicated
            void
            setCreaseAngle( double angle )
            {
                _creaseAngle = angle;
            }

            double
            getCreaseAngle() const
            {
                return _creaseAngle;
            }

        protected:

            double _creaseAngle;
    };

}
