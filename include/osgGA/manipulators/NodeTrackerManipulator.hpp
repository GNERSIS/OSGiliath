/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Camera manipulator that tracks a scene node. Keeps the camera
 * focused on a moving target with configurable tracking modes.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/ObserverNodePath.hpp>
#include <osgGA/manipulators/OrbitManipulator.hpp>

namespace osgGA
{

    class OSGGA_EXPORT NodeTrackerManipulator
        : public osg::Inherit<OrbitManipulator, NodeTrackerManipulator>
    {
            typedef OrbitManipulator inherited;

        public:

            NodeTrackerManipulator( int flags = DEFAULT_SETTINGS );

            NodeTrackerManipulator( const NodeTrackerManipulator& om,
                                    const osg::CopyOp&            copyOp =
                                        osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgGA,
                               NodeTrackerManipulator )

            void
            setTrackNodePath( const osg::NodePath& nodePath );

            void
            setTrackNodePath( const osg::ObserverNodePath& nodePath )
            {
                _trackNodePath = nodePath;
            }

            osg::ObserverNodePath&
            getTrackNodePath()
            {
                return _trackNodePath;
            }

            void
            setTrackNode( osg::Node* node );

            osg::Node*
            getTrackNode()
            {
                osg::NodePath nodePath;
                return _trackNodePath.getNodePath( nodePath ) && !nodePath.empty()
                         ? nodePath.back()
                         : 0;
            }

            const osg::Node*
            getTrackNode() const
            {
                osg::NodePath nodePath;
                return _trackNodePath.getNodePath( nodePath ) && !nodePath.empty()
                         ? nodePath.back()
                         : 0;
            }

            enum TrackerMode
            {
                /** Track the center of the node's bounding sphere, but not rotations of
                 * the node. For databases which have a CoordinateSystemNode, the
                 * orientation is kept relative the coordinate frame if the center of the
                 * node.
                 */
                NODE_CENTER,
                /** Track the center of the node's bounding sphere, and the azimuth
                 * rotation (about the z axis of the current coordinate frame). For
                 * databases which have a CoordinateSystemNode, the orientation is kept
                 * relative the coordinate frame if the center of the node.
                 */
                NODE_CENTER_AND_AZIM,
                /** Tack the center of the node's bounding sphere, and the all rotations
                 * of the node.
                 */
                NODE_CENTER_AND_ROTATION,
            };

            void
            setTrackerMode( TrackerMode mode );

            TrackerMode
            getTrackerMode() const
            {
                return _trackerMode;
            }

            enum RotationMode
            {
                /** Use a trackball style manipulation of the view direction w.r.t the
                 * tracked orientation.
                 */
                TRACKBALL,
                /** Allow the elevation and azimuth angles to be adjust w.r.t the tracked
                 * orientation.
                 */
                ELEVATION_AZIM,
            };

            void
            setRotationMode( RotationMode mode );
            RotationMode
            getRotationMode() const;

            void
            setByMatrix( const osg::dmat4& matrix ) override;
            osg::dmat4
            getMatrix() const override;
            osg::dmat4
            getInverseMatrix() const override;

            void
            setNode( osg::Node* ) override;

            virtual void
            computeHomePosition();

            void
            computeHomePosition( const osg::Camera* camera,
                                 bool               useBoundingBox ) override
            {
                CameraManipulator::computeHomePosition( camera, useBoundingBox );
            }

        protected:

            bool
            performMovementLeftMouseButton( const double eventTimeDelta,
                                            const double dx,
                                            const double dy ) override;
            bool
            performMovementMiddleMouseButton( const double eventTimeDelta,
                                              const double dx,
                                              const double dy ) override;
            bool
            performMovementRightMouseButton( const double eventTimeDelta,
                                             const double dx,
                                             const double dy ) override;

            void
            computeNodeWorldToLocal( osg::dmat4& worldToLocal ) const;
            void
            computeNodeLocalToWorld( osg::dmat4& localToWorld ) const;

            void
            computeNodeCenterAndRotation( osg::dvec3& center,
                                          osg::quat&  rotation ) const;

            void
                                  computePosition( const osg::dvec3& eye,
                                                   const osg::dvec3& lv,
                                                   const osg::dvec3& up );

            osg::ObserverNodePath _trackNodePath;
            TrackerMode           _trackerMode;
    };

}
