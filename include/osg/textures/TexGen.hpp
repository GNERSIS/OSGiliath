/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Texture coordinate generation parameters. Retained for scene
 * graph compatibility — Core Profile generates tex coords in shaders.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/plane.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** TexGen encapsulates texture coordinate generation state.
     * In Core Profile, this generates shader defines for the shader pipeline
     * instead of using fixed-function glTexGen calls. */
    class OSG_EXPORT TexGen : public osg::Inherit<StateAttribute, TexGen>
    {
        public:

            TexGen();

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            TexGen( const TexGen& texgen,
                    const CopyOp& copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( texgen,
                         copyop ),
                _mode( texgen._mode ),
                _plane_s( texgen._plane_s ),
                _plane_t( texgen._plane_t ),
                _plane_r( texgen._plane_r ),
                _plane_q( texgen._plane_q )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               TexGen )

            Type
            getType() const override
            {
                return Type::TEXGEN;
            }

            bool
            isTextureAttribute() const override
            {
                return true;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                COMPARE_StateAttribute_Types( TexGen, sa )
                    COMPARE_StateAttribute_Parameter(
                        _mode
                    ) COMPARE_StateAttribute_Parameter( _plane_s )
                        COMPARE_StateAttribute_Parameter( _plane_t )
                            COMPARE_StateAttribute_Parameter( _plane_r )
                                COMPARE_StateAttribute_Parameter( _plane_q ) return 0;
            }

            bool
            getModeUsage( StateAttribute::ModeUsage& usage ) const override
            {
                usage.usesTextureMode( GL_TEXTURE_GEN_S );
                usage.usesTextureMode( GL_TEXTURE_GEN_T );
                usage.usesTextureMode( GL_TEXTURE_GEN_R );
                usage.usesTextureMode( GL_TEXTURE_GEN_Q );
                return true;
            }

            /** No-op in Core Profile. Shader pipeline handles tex coord generation. */
            void
            apply( State& state ) const override;

            enum Mode
            {
                OBJECT_LINEAR  = GL_OBJECT_LINEAR,
                EYE_LINEAR     = GL_EYE_LINEAR,
                SPHERE_MAP     = GL_SPHERE_MAP,
                NORMAL_MAP     = GL_NORMAL_MAP_ARB,
                REFLECTION_MAP = GL_REFLECTION_MAP_ARB,
            };

            inline void
            setMode( Mode mode )
            {
                _mode = mode;
            }

            Mode
            getMode() const
            {
                return _mode;
            }

            enum Coord
            {
                S,
                T,
                R,
                Q,
            };

            void
            setPlane( Coord        which,
                      const Plane& plane );
            Plane&
            getPlane( Coord which );
            const Plane&
            getPlane( Coord which ) const;

            void
            setPlanesFromMatrix( const dmat4& matrix );

        protected:

            virtual ~TexGen( void );

            Mode  _mode;

            Plane _plane_s, _plane_t, _plane_r, _plane_q;
    };

}
