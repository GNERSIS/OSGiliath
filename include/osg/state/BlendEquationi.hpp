/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Indexed blending equation for MRT. Sets per-draw-buffer
 * blend equations when rendering to multiple color attachments.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/BlendEquation.hpp>

namespace osg
{

    /** Encapsulates glBlendEquationi function : the index version of glBlendEquation for
     * multiple render target.
     */
    class OSG_EXPORT BlendEquationi : public osg::Inherit<BlendEquation, BlendEquationi>
    {
        public:

            BlendEquationi();

            BlendEquationi( unsigned int buf,
                            Equation     equation ) :
                Inherit( equation ),
                _index( buf )
            {
            }

            BlendEquationi( unsigned int buf,
                            Equation     equationRGB,
                            Equation     equationAlpha ) :
                Inherit( equationRGB,
                         equationAlpha ),
                _index( buf )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            BlendEquationi( const BlendEquationi& cm,
                            const CopyOp&         copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( cm,
                         copyop ),
                _index( cm._index )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               BlendEquationi )

            Type
            getType() const override
            {
                return Type::BLENDEQUATION;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( BlendEquationi, sa )

                    COMPARE_StateAttribute_Parameter( _index );

                return BlendEquation::compare( sa );
            }

            /** Return the buffer index as the member identifier.*/
            unsigned int
            getMember() const override
            {
                return _index;
            }

            /** Set the renderbuffer index of the BlendEquationi. */
            void
            setIndex( unsigned int buf );

            /** Get the renderbuffer index of the BlendEquationi. */
            unsigned int
            getIndex() const
            {
                return _index;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~BlendEquationi();

            unsigned int _index;
    };

}
