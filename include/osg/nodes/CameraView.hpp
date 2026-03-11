/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Specifies a named viewpoint within the scene graph.
 * Provides position and orientation for camera presets.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/traversal/AnimationPath.hpp>

namespace osg
{

    /** CameraView - is a Transform that is used to specify camera views from within the
     * scene graph. The application must attach a camera to a CameraView via the NodePath
     * from the top of the scene graph to the CameraView node itself, and accumulate the
     * view matrix from this NodePath.
     */
    class OSG_EXPORT CameraView : public osg::Inherit<Transform, CameraView>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               CameraView )

            CameraView();

            CameraView( const CameraView& pat,
                        const CopyOp&     copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( pat,
                         copyop ),
                _position( pat._position ),
                _attitude( pat._attitude ),
                _fieldOfView( pat._fieldOfView ),
                _fieldOfViewMode( pat._fieldOfViewMode ),
                _focalLength( pat._focalLength )
            {
            }

            /** Set the position of the camera view.*/
            inline void
            setPosition( const dvec3& pos )
            {
                _position = pos;
                dirtyBound();
            }

            /** Get the position of the camera view.*/
            inline const dvec3&
            getPosition() const
            {
                return _position;
            }

            /** Set the attitude of the camera view.*/
            inline void
            setAttitude( const quat& quat )
            {
                _attitude = quat;
                dirtyBound();
            }

            /** Get the attitude of the camera view.*/
            inline const quat&
            getAttitude() const
            {
                return _attitude;
            }

            /** Set the field of view.
             * The camera's field of view can be constrained to either the horizontal or
             * vertical axis of the camera, or unconstrained in which case the
             * camera/application are left to choose an appropriate field of view. The
             * default value if 60 degrees. */
            inline void
            setFieldOfView( double fieldOfView )
            {
                _fieldOfView = fieldOfView;
            }

            /** Get the field of view.*/
            inline double
            getFieldOfView() const
            {
                return _fieldOfView;
            }

            enum FieldOfViewMode
            {
                UNCONSTRAINED,
                HORIZONTAL,
                VERTICAL,
            };

            /** Set the field of view mode - controlling how the field of view of the
             * camera is constrained by the CameraView settings.*/
            inline void
            setFieldOfViewMode( FieldOfViewMode mode )
            {
                _fieldOfViewMode = mode;
            }

            /** Get the field of view mode.*/
            inline FieldOfViewMode
            getFieldOfViewMode() const
            {
                return _fieldOfViewMode;
            }

            /** Set the focal length of the camera.
             * A focal length of 0.0 indicates that the camera/application should
             * determine the focal length. The default value is 0.0. */
            inline void
            setFocalLength( double focalLength )
            {
                _focalLength = focalLength;
            }

            /** Get the focal length of the camera.*/
            inline double
            getFocalLength() const
            {
                return _focalLength;
            }

            virtual bool
            computeLocalToWorldMatrix( dmat4&       matrix,
                                       NodeVisitor* nv ) const;
            virtual bool
            computeWorldToLocalMatrix( dmat4&       matrix,
                                       NodeVisitor* nv ) const;

        protected:

            virtual ~CameraView()
            {
            }

            dvec3           _position;
            quat            _attitude;
            double          _fieldOfView;
            FieldOfViewMode _fieldOfViewMode;
            double          _focalLength;
    };

}
