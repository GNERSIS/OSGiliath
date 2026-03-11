/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Surface material properties (ambient, diffuse, specular, emission,
 * shininess). Applied as a state attribute, uploads to osg_FrontMaterial.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Material - encapsulates OpenGL glMaterial state.*/
    class OSG_EXPORT Material : public osg::Inherit<StateAttribute, Material>
    {
        public:

            Material();

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Material( const Material& mat,
                      const CopyOp&   copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( mat,
                         copyop ),
                _colorMode( mat._colorMode ),
                _ambientFrontAndBack( mat._ambientFrontAndBack ),
                _ambientFront( mat._ambientFront ),
                _ambientBack( mat._ambientBack ),
                _diffuseFrontAndBack( mat._diffuseFrontAndBack ),
                _diffuseFront( mat._diffuseFront ),
                _diffuseBack( mat._diffuseBack ),
                _specularFrontAndBack( mat._specularFrontAndBack ),
                _specularFront( mat._specularFront ),
                _specularBack( mat._specularBack ),
                _emissionFrontAndBack( mat._emissionFrontAndBack ),
                _emissionFront( mat._emissionFront ),
                _emissionBack( mat._emissionBack ),
                _shininessFrontAndBack( mat._shininessFrontAndBack ),
                _shininessFront( mat._shininessFront ),
                _shininessBack( mat._shininessBack )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               Material )

            Type
            getType() const override
            {
                return Type::MATERIAL;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( Material, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter(
                        _colorMode
                    ) COMPARE_StateAttribute_Parameter( _ambientFrontAndBack )
                        COMPARE_StateAttribute_Parameter(
                            _ambientFront
                        ) COMPARE_StateAttribute_Parameter( _ambientBack )
                            COMPARE_StateAttribute_Parameter(
                                _diffuseFrontAndBack
                            ) COMPARE_StateAttribute_Parameter( _diffuseFront )
                                COMPARE_StateAttribute_Parameter(
                                    _diffuseBack
                                ) COMPARE_StateAttribute_Parameter( _specularFrontAndBack )
                                    COMPARE_StateAttribute_Parameter(
                                        _specularFront
                                    ) COMPARE_StateAttribute_Parameter( _specularBack )
                                        COMPARE_StateAttribute_Parameter(
                                            _emissionFrontAndBack
                                        ) COMPARE_StateAttribute_Parameter( _emissionFront )
                                            COMPARE_StateAttribute_Parameter(
                                                _emissionBack
                                            )
                                                COMPARE_StateAttribute_Parameter(
                                                    _shininessFrontAndBack
                                                )
                                                    COMPARE_StateAttribute_Parameter(
                                                        _shininessFront
                                                    )
                                                        COMPARE_StateAttribute_Parameter(
                                                            _shininessBack
                                                        ) return 0;    // passed all the
                                                                       // above
                                                                       // comparison
                                                                       // macros, must be
                                                                       // equal.
            }

            Material&
            operator=( const Material& rhs );

            bool
            getModeUsage( StateAttribute::ModeUsage& /*usage*/ ) const override
            {
                // note, since Material does it's own glEnable/glDisable of
                // GL_COLOR_MATERIAL we shouldn't declare usage of that mode, so
                // commenting out the below usage. usage.usesMode(GL_COLOR_MATERIAL);
                return true;
            }

            void
            apply( State& state ) const override;

            enum Face
            {
                FRONT          = GL_FRONT,
                BACK           = GL_BACK,
                FRONT_AND_BACK = GL_FRONT_AND_BACK,
            };

            enum ColorMode
            {
                AMBIENT             = GL_AMBIENT,
                DIFFUSE             = GL_DIFFUSE,
                SPECULAR            = GL_SPECULAR,
                EMISSION            = GL_EMISSION,
                AMBIENT_AND_DIFFUSE = GL_AMBIENT_AND_DIFFUSE,
                OFF,
            };

            inline void
            setColorMode( ColorMode mode )
            {
                _colorMode = mode;
            }

            inline ColorMode
            getColorMode() const
            {
                return _colorMode;
            }

            void
            setAmbient( Face        face,
                        const vec4& ambient );
            const vec4&
            getAmbient( Face face ) const;

            inline bool
            getAmbientFrontAndBack() const
            {
                return _ambientFrontAndBack;
            }

            void
            setDiffuse( Face        face,
                        const vec4& diffuse );
            const vec4&
            getDiffuse( Face face ) const;

            inline bool
            getDiffuseFrontAndBack() const
            {
                return _diffuseFrontAndBack;
            }

            /** Set specular value of specified face(s) of the material,
             * valid specular[0..3] range is 0.0 to 1.0.
             */
            void
            setSpecular( Face        face,
                         const vec4& specular );

            /** Get the specular value for specified face. */
            const vec4&
            getSpecular( Face face ) const;

            /** Return whether specular values are equal for front and back faces
             * or not.
             */
            inline bool
            getSpecularFrontAndBack() const
            {
                return _specularFrontAndBack;
            }

            /** Set emission value of specified face(s) of the material,
             * valid emission[0..3] range is 0.0 to 1.0.
             */
            void
            setEmission( Face        face,
                         const vec4& emission );

            /** Get the emission value for specified face. */
            const vec4&
            getEmission( Face face ) const;

            /** Return whether emission values are equal for front and back faces
             * or not.
             */
            inline bool
            getEmissionFrontAndBack() const
            {
                return _emissionFrontAndBack;
            }

            /** Set shininess of specified face(s) of the material.
             * valid shininess range is 0.0 to 128.0.
             */
            void
            setShininess( Face  face,
                          float shininess );

            /** Get the shininess value for specified face. */
            float
            getShininess( Face face ) const;

            /** Return whether shininess values are equal for front and back faces
             * or not.
             */
            inline bool
            getShininessFrontAndBack() const
            {
                return _shininessFrontAndBack;
            }

            /** Set the alpha value of ambient, diffuse, specular and emission
             * colors of specified face, to 1-transparency.
             * Valid transparency range is 0.0 to 1.0.
             */
            void
            setTransparency( Face  face,
                             float trans );

            /** Set the alpha value of ambient, diffuse, specular and emission
             * colors. Valid transparency range is 0.0 to 1.0.
             */
            void
            setAlpha( Face  face,
                      float alpha );

        protected:

            virtual ~Material();

            ColorMode _colorMode;

            bool      _ambientFrontAndBack;
            vec4      _ambientFront;      // r, g, b, w
            vec4      _ambientBack;       // r, g, b, w

            bool      _diffuseFrontAndBack;
            vec4      _diffuseFront;      // r, g, b, w
            vec4      _diffuseBack;       // r, g, b, w

            bool      _specularFrontAndBack;
            vec4      _specularFront;     // r, g, b, w
            vec4      _specularBack;      // r, g, b, w

            bool      _emissionFrontAndBack;
            vec4      _emissionFront;     // r, g, b, w
            vec4      _emissionBack;      // r, g, b, w

            bool      _shininessFrontAndBack;
            float     _shininessFront;    // values 0 - 128.0
            float     _shininessBack;     // values 0 - 128.0
    };

}
