/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Primitive restart index attribute. Sets the special index value
 * that restarts a triangle strip or line strip mid-stream.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /**
     *  osg::PrimitiveRestartIndex does nothing if OpenGL 3.1 is not available.
     */
    class OSG_EXPORT PrimitiveRestartIndex
        : public osg::Inherit<StateAttribute, PrimitiveRestartIndex>
    {
        public:

            PrimitiveRestartIndex();
            PrimitiveRestartIndex( unsigned int restartIndex );

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            PrimitiveRestartIndex( const PrimitiveRestartIndex& primitiveRestartIndex,
                                   const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               PrimitiveRestartIndex )

            Type
            getType() const override
            {
                return Type::PRIMITIVERESTARTINDEX;
            } /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/

            int
            compare( const StateAttribute& sa ) const override;

            inline void
            setRestartIndex( unsigned int restartIndex )
            {
                _restartIndex = restartIndex;
            }

            inline unsigned int
            getRestartIndex() const
            {
                return _restartIndex;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~PrimitiveRestartIndex();

            unsigned int _restartIndex;
    };

}
