/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Transform that automatically orients toward the camera (billboarding)
 * or auto-scales based on screen size. Used for labels, sprites, and
 * annotations that must remain screen-aligned.
 */
#pragma once

#include <osg/maths/quat.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/state/Viewport.hpp>

namespace osg
{

    /** AutoTransform is a derived form of Transform that automatically
     * scales or rotates to keep its children aligned with screen coordinates.
     */
    class OSG_EXPORT AutoTransform : public Transform
    {
        public:

            AutoTransform();

            AutoTransform( const AutoTransform& pat,
                           const CopyOp&        copyop = CopyOp::SHALLOW_COPY );

            virtual osg::Object*
            cloneType() const
            {
                return new AutoTransform();
            }

            virtual osg::Object*
            clone( const osg::CopyOp& copyop ) const
            {
                return new AutoTransform( *this, copyop );
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const AutoTransform*>( obj ) != NULL;
            }

            virtual const char*
            className() const
            {
                return "AutoTransform";
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual AutoTransform*
            asAutoTransform()
            {
                return this;
            }

            virtual const AutoTransform*
            asAutoTransform() const
            {
                return this;
            }

            inline void
            setPosition( const dvec3& pos )
            {
                _position = pos;
                dirtyBound();
            }

            inline const dvec3&
            getPosition() const
            {
                return _position;
            }

            inline void
            setRotation( const quat& quat )
            {
                _rotation = quat;
                dirtyBound();
            }

            inline const quat&
            getRotation() const
            {
                return _rotation;
            }

            inline void
            setScale( double scale )
            {
                setScale( osg::dvec3( scale, scale, scale ) );
            }

            void
            setScale( const dvec3& scale );

            inline const dvec3&
            getScale() const
            {
                return _scale;
            }

            void
            setMinimumScale( double minimumScale )
            {
                _minimumScale = minimumScale;
            }

            double
            getMinimumScale() const
            {
                return _minimumScale;
            }

            void
            setMaximumScale( double maximumScale )
            {
                _maximumScale = maximumScale;
            }

            double
            getMaximumScale() const
            {
                return _maximumScale;
            }

            inline void
            setPivotPoint( const dvec3& pivot )
            {
                _pivotPoint = pivot;
                dirtyBound();
            }

            inline const dvec3&
            getPivotPoint() const
            {
                return _pivotPoint;
            }

            void
            setAutoUpdateEyeMovementTolerance( float tolerance )
            {
                _autoUpdateEyeMovementTolerance = tolerance;
            }

            float
            getAutoUpdateEyeMovementTolerance() const
            {
                return static_cast<float>( _autoUpdateEyeMovementTolerance );
            }

            enum AutoRotateMode
            {
                NO_ROTATION,
                ROTATE_TO_SCREEN,
                ROTATE_TO_CAMERA,
                ROTATE_TO_AXIS,
            };

            void
            setAutoRotateMode( AutoRotateMode mode );

            AutoRotateMode
            getAutoRotateMode() const
            {
                return _autoRotateMode;
            }

            /** Set the rotation axis for the AutoTransform's child nodes.
             * Only utilized when _autoRotateMode==ROTATE_TO_AXIS. */
            void
            setAxis( const vec3& axis );

            /** Get the rotation axis. */
            inline const vec3&
            getAxis() const
            {
                return _axis;
            }

            /** This normal defines child Nodes' front face direction when unrotated. */
            void
            setNormal( const vec3& normal );

            /** Get the front face direction normal. */
            inline const vec3&
            getNormal() const
            {
                return _normal;
            }

            void
            setAutoScaleToScreen( bool autoScaleToScreen );

            bool
            getAutoScaleToScreen() const
            {
                return _autoScaleToScreen;
            }

            void
            setAutoScaleTransitionWidthRatio( float ratio )
            {
                _autoScaleTransitionWidthRatio = ratio;
            }

            float
            getAutoScaleTransitionWidthRatio() const
            {
                return static_cast<float>( _autoScaleTransitionWidthRatio );
            }

            virtual bool
            computeLocalToWorldMatrix( dmat4&       matrix,
                                       NodeVisitor* nv ) const;

            virtual bool
            computeWorldToLocalMatrix( dmat4&       matrix,
                                       NodeVisitor* nv ) const;

        protected:

            virtual ~AutoTransform()
            {
            }

            dvec3          _position;
            dvec3          _pivotPoint;
            double         _autoUpdateEyeMovementTolerance;

            AutoRotateMode _autoRotateMode;

            bool           _autoScaleToScreen;

            mutable quat   _rotation;
            mutable dvec3  _scale;

            double         _minimumScale;
            double         _maximumScale;
            double         _autoScaleTransitionWidthRatio;

            osg::dmat4
            computeMatrix( const osg::NodeVisitor* nv ) const;

            enum AxisAligned
            {
                AXIAL_ROT_X_AXIS = ROTATE_TO_AXIS + 1,
                AXIAL_ROT_Y_AXIS,
                AXIAL_ROT_Z_AXIS,
                CACHE_DIRTY,
            };

            vec3 _axis;
            vec3 _normal;

            // used internally as cache of which what _axis is aligned to help
            // decide which method of rotation to use.
            int  _cachedMode;
            vec3 _side;
            void
            updateCache();
    };

}
