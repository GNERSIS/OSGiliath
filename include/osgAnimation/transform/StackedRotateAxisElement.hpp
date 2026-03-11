/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stacked axis-angle rotation element. Contributes a rotation
 * around a fixed axis to the composite transform.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/vec3.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/core/Target.hpp>
#include <osgAnimation/transform/StackedTransformElement.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT StackedRotateAxisElement
        : public osg::Inherit<StackedTransformElement, StackedRotateAxisElement>
    {
        public:

            OSG_REGISTER_TYPE( osgAnimation,
                               StackedRotateAxisElement )

            StackedRotateAxisElement();
            StackedRotateAxisElement( const StackedRotateAxisElement&,
                                      const osg::CopyOp& );
            StackedRotateAxisElement( const std::string& name,
                                      const osg::vec3&   axis,
                                      double             angle );
            StackedRotateAxisElement( const osg::vec3& axis,
                                      double           angle );

            void
            applyToMatrix( osg::dmat4& matrix ) const;
            osg::dmat4
            getAsMatrix() const;

            bool
            isIdentity() const
            {
                return ( _angle == 0 );
            }

            void
            update( float t = 0.0 );

            const osg::vec3&
            getAxis() const;
            double
            getAngle() const;
            void
            setAxis( const osg::vec3& );
            void
            setAngle( double );

            virtual Target*
            getOrCreateTarget();

            virtual Target*
            getTarget()
            {
                return _target.get();
            }

            virtual const Target*
            getTarget() const
            {
                return _target.get();
            }

        protected:

            osg::vec3                 _axis;
            double                    _angle;
            osg::ref_ptr<FloatTarget> _target;
    };

}
