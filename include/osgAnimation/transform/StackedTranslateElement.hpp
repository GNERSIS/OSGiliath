/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stacked translation element. Contributes a translation
 * matrix to the composite bone/node transform.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/core/Target.hpp>
#include <osgAnimation/transform/StackedTransformElement.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT StackedTranslateElement
        : public osg::Inherit<StackedTransformElement, StackedTranslateElement>
    {
        public:

            OSG_REGISTER_TYPE( osgAnimation,
                               StackedTranslateElement )

            StackedTranslateElement();
            StackedTranslateElement( const StackedTranslateElement&,
                                     const osg::CopyOp& );
            StackedTranslateElement( const std::string&,
                                     const osg::vec3& translate = osg::vec3( 0,
                                                                             0,
                                                                             0 ) );
            StackedTranslateElement( const osg::vec3& translate );

            void
            applyToMatrix( osg::dmat4& matrix ) const;
            osg::dmat4
            getAsMatrix() const;
            bool
            isIdentity() const;
            void
            update( float t = 0.0 );

            const osg::vec3&
            getTranslate() const;
            void
            setTranslate( const osg::vec3& );
            virtual Target*
            getOrCreateTarget();
            virtual Target*
            getTarget();
            virtual const Target*
            getTarget() const;

        protected:

            osg::vec3                _translate;
            osg::ref_ptr<Vec3Target> _target;
    };

}
