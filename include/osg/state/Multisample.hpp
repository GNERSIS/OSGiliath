/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Multisample anti-aliasing state attribute. Enables/configures
 * MSAA sample coverage and alpha-to-coverage modes.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/GL>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Multisample - encapsulates the OpenGL Multisample state.*/
    class OSG_EXPORT Multisample : public osg::Inherit<StateAttribute, Multisample>
    {
        public:

            enum Mode
            {
                FASTEST   = GL_FASTEST,
                NICEST    = GL_NICEST,
                DONT_CARE = GL_DONT_CARE,
            };

            Multisample();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            Multisample( const Multisample& trans,
                         const CopyOp&      copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( trans,
                         copyop ),
                _coverage( trans._coverage ),
                _invert( trans._invert ),
                _mode( trans._mode )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               Multisample )

            Type
            getType() const override
            {
                return Type::MULTISAMPLE;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
            int
            compare( const StateAttribute& sa ) const override
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( Multisample, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _coverage )
                        COMPARE_StateAttribute_Parameter( _invert )
                            COMPARE_StateAttribute_Parameter(
                                _mode
                            ) return 0;    // passed all the above comparison macros,
                                           // must be equal.
            }

            void
            setSampleCoverage( float coverage,
                               bool  invert )
            {
                _coverage = coverage;
                _invert   = invert;
            }

            inline void
            setCoverage( float coverage )
            {
                _coverage = coverage;
            }

            inline float
            getCoverage() const
            {
                return _coverage;
            }

            inline void
            setInvert( bool invert )
            {
                _invert = invert;
            }

            inline bool
            getInvert() const
            {
                return _invert;
            }

            inline void
            setHint( Mode mode )
            {
                _mode = mode;
            }

            inline Mode
            getHint() const
            {
                return _mode;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~Multisample();

            float _coverage;
            bool  _invert;
            Mode  _mode;
    };

}
