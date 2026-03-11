/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Color clamping control attribute. Disables automatic clamping
 * of vertex colors and fragment outputs for HDR rendering.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Encapsulates OpenGL ClampColor state. */
    class OSG_EXPORT ClampColor : public osg::Inherit<StateAttribute, ClampColor>
    {
        public:

            ClampColor();

            ClampColor( GLenum vertexMode,
                        GLenum fragmentMode,
                        GLenum readMode );

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            ClampColor( const ClampColor& rhs,
                        const CopyOp&     copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( rhs,
                         copyop ),
                _clampVertexColor( rhs._clampVertexColor ),
                _clampFragmentColor( rhs._clampFragmentColor ),
                _clampReadColor( rhs._clampReadColor )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               ClampColor )

            Type
            getType() const override
            {
                return Type::CLAMPCOLOR;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( ClampColor, sa )

                    // Compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _clampVertexColor )
                        COMPARE_StateAttribute_Parameter( _clampFragmentColor )
                            COMPARE_StateAttribute_Parameter(
                                _clampReadColor
                            ) return 0;    // Passed all the above comparison macros, so
                                           // must be equal.
            }

            void
            setClampVertexColor( GLenum mode )
            {
                _clampVertexColor = mode;
            }

            GLenum
            getClampVertexColor() const
            {
                return _clampVertexColor;
            }

            void
            setClampFragmentColor( GLenum mode )
            {
                _clampFragmentColor = mode;
            }

            GLenum
            getClampFragmentColor() const
            {
                return _clampFragmentColor;
            }

            void
            setClampReadColor( GLenum mode )
            {
                _clampReadColor = mode;
            }

            GLenum
            getClampReadColor() const
            {
                return _clampReadColor;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~ClampColor();

            GLenum _clampVertexColor;
            GLenum _clampFragmentColor;
            GLenum _clampReadColor;
    };

}
