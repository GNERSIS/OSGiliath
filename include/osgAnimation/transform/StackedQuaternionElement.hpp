/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stacked quaternion rotation element. Contributes a quaternion
 * rotation to the composite bone/node transform.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/core/Target.hpp>
#include <osgAnimation/transform/StackedTransformElement.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT StackedQuaternionElement
        : public osg::Inherit<StackedTransformElement, StackedQuaternionElement>
    {
        public:

            OSG_REGISTER_TYPE( osgAnimation,
                               StackedQuaternionElement )

            StackedQuaternionElement();
            StackedQuaternionElement( const StackedQuaternionElement&,
                                      const osg::CopyOp& );
            StackedQuaternionElement( const std::string&,
                                      const osg::quat& q = osg::quat( 0,
                                                                      0,
                                                                      0,
                                                                      1 ) );
            StackedQuaternionElement( const osg::quat& );

            void
            applyToMatrix( osg::dmat4& matrix ) const;
            osg::dmat4
            getAsMatrix() const;
            bool
            isIdentity() const;
            void
            update( float t = 0.0 );

            const osg::quat&
            getQuaternion() const;
            void
            setQuaternion( const osg::quat& );
            virtual Target*
            getOrCreateTarget();
            virtual Target*
            getTarget();
            virtual const Target*
            getTarget() const;

        protected:

            osg::quat                _quaternion;
            osg::ref_ptr<QuatTarget> _target;
    };

}
