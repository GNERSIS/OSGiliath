/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stacked arbitrary matrix element. Contributes a raw 4x4
 * matrix to the composite bone/node transform.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/maths/compat.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/core/Target.hpp>
#include <osgAnimation/transform/StackedTransformElement.hpp>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT StackedMatrixElement
        : public osg::Inherit<StackedTransformElement, StackedMatrixElement>
    {
        public:

            OSG_REGISTER_TYPE( osgAnimation,
                               StackedMatrixElement )

            StackedMatrixElement();
            StackedMatrixElement( const StackedMatrixElement&,
                                  const osg::CopyOp& );
            StackedMatrixElement( const std::string& name,
                                  const osg::dmat4&  matrix );
            StackedMatrixElement( const osg::dmat4& matrix );

            void
            applyToMatrix( osg::dmat4& matrix ) const
            {
                matrix = _matrix * matrix;
            }

            osg::dmat4
            getAsMatrix() const
            {
                return _matrix;
            }

            const osg::dmat4&
            getMatrix() const
            {
                return _matrix;
            }

            void
            setMatrix( const osg::dmat4& matrix )
            {
                _matrix = matrix;
            }

            bool
            isIdentity() const
            {
                return osg::isIdentity( _matrix );
            }

            void
            update( float t = 0.0 );
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

            osg::dmat4                 _matrix;
            osg::ref_ptr<MatrixTarget> _target;
    };

}
