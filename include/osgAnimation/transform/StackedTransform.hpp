/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Ordered list of stacked transform elements. Composes
 * translate, rotate, and scale in sequence for bone transforms.
 */
#pragma once

#include <osg/core/MixinVector.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/transform/StackedTransformElement.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT StackedTransform
        : public osg::MixinVector<osg::ref_ptr<StackedTransformElement>>
    {
        public:

            StackedTransform();
            StackedTransform( const StackedTransform&,
                              const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );
            StackedTransform&
            operator=( const StackedTransform& ) = default;

            void
            update( float t = 0.0 );
            const osg::dmat4&
            getMatrix() const;

        protected:

            osg::dmat4 _matrix;
    };

}
