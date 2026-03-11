/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stacked scale element. Contributes a scale matrix
 * to the composite bone/node transform.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/core/Target.hpp>
#include <osgAnimation/transform/StackedTransformElement.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT StackedScaleElement
        : public osg::Inherit<StackedTransformElement, StackedScaleElement>
    {
        public:

            OSG_REGISTER_TYPE(osgAnimation, StackedScaleElement)StackedScaleElement();
            StackedScaleElement( const StackedScaleElement&,
                                 const osg::CopyOp& );
            StackedScaleElement( const std::string& name,
                                 const osg::vec3&   scale = osg::vec3( 1,
                                                                       1,
                                                                       1 ) );
            StackedScaleElement( const osg::vec3& scale );

            void
            applyToMatrix( osg::dmat4& matrix ) const;
            osg::dmat4
            getAsMatrix() const;
            bool
            isIdentity() const;
            void
            update( float t = 0.0 );
            const osg::vec3&
            getScale() const;
            void
            setScale( const osg::vec3& scale );

            virtual Target*
            getOrCreateTarget();
            virtual Target*
            getTarget();
            virtual const Target*
            getTarget() const;

        protected:

            osg::vec3                _scale;
            osg::ref_ptr<Vec3Target> _target;
    };

}
