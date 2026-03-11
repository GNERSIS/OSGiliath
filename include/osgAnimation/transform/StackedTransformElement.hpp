/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract element in a StackedTransform. Provides
 * a matrix contribution for one transform operation.
 */
#pragma once

#include <osg/core/Object.hpp>
#include <osg/maths/mat4.hpp>
#include <osgAnimation/core/Export.hpp>

namespace osgAnimation
{

    class Target;

    class OSGANIMATION_EXPORT StackedTransformElement : public osg::Object
    {
        public:

            StackedTransformElement()
            {
            }

            StackedTransformElement( const StackedTransformElement& rhs,
                                     const osg::CopyOp&             c ) :
                osg::Object( rhs,
                             c )
            {
            }

            virtual void
            applyToMatrix( osg::dmat4& matrix ) const = 0;
            virtual osg::dmat4
            getAsMatrix() const = 0;
            virtual bool
            isIdentity() const = 0;
            virtual void
            update( float t ) = 0;

            virtual Target*
            getOrCreateTarget()
            {
                return 0;
            }

            virtual Target*
            getTarget()
            {
                return 0;
            }

            virtual const Target*
            getTarget() const
            {
                return 0;
            }
    };

}
