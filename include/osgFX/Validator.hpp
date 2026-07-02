/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Effect technique validator. Tests technique availability by
 * checking OpenGL extension support at runtime.
 */
// osgFX - Copyright (C) 2003 Marco Jez

#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/state/StateAttribute.hpp>
#include <osgFX/Effect.hpp>
#include <vector>

namespace osgFX
{

    /**
     This class is used internally by osgFX::Effect to choose between different
     techniques dynamically. The apply() method will call each technique's
     validate() method and store the results in a buffered array. The Effect
     class will then choose the first technique that could be validated in all
     active rendering contexts.
     */
    class OSGFX_EXPORT Validator : public osg::Inherit<osg::StateAttribute, Validator>
    {
        public:

            Validator();
            Validator( Effect* effect );
            Validator( const Validator&   copy,
                       const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgFX,
                               Validator )

            Type
            getType() const override
            {
                return Type::VALIDATOR;
            }

            void
            apply( osg::State& state ) const override;
            void
            compileGLObjects( osg::State& state ) const override;

            inline int
            compare( const osg::StateAttribute& sa ) const override;

            inline void
            disable()
            {
                _effect = 0;
            }

        protected:

            virtual ~Validator()
            {
            }

            Validator&
            operator=( const Validator& )
            {
                return *this;
            }

        private:

            mutable Effect* _effect;
    };

    // INLINE METHODS

    inline int
    Validator::compare( const osg::StateAttribute& sa ) const
    {
        // check the types are equal and then create the rhs variable
        // used by the COMPARE_StateAttribute_Parameter macros below.
        COMPARE_StateAttribute_Types( Validator, sa )

            // compare parameters
            COMPARE_StateAttribute_Parameter( _effect ) return 0;
    }

}
