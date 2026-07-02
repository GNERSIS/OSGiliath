/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Environment-mapped specular highlights. Generates a sphere-map
 * texture for glossy surface reflections.
 */
// osgFX - Copyright (C) 2003 Marco Jez

#pragma once

#include <osgFX/Effect.hpp>
#include <osgFX/Export.hpp>

namespace osgFX
{

    /**
      This effect applies additive specular highlights at fragment level (instead
     of OpenGL's vertex-level lighting) by using a cube map and reflective texgen.
     A texture matrix is computed to rotate the cube map automatically; this makes
     the specular effect consistent with respect to view direction and light position.
     The user can choose which light should be used to compute the texture matrix.
     This effect requires the GL_ARB_texture_env_add extension and one of the cube map
     extensions (GL_EXT_texture_cube_map, GL_ARB_texture_cube_map or OpenGL v1.3).
     */
    class OSGFX_EXPORT SpecularHighlights
        : public osg::Inherit<Effect, SpecularHighlights>
    {
        public:

            SpecularHighlights();
            SpecularHighlights( const SpecularHighlights& copy,
                                const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            META_Effect( osgFX,
                         SpecularHighlights,

                         "Specular Highlights",

                         "This effect applies additive specular highlights at fragment "
                         "level (instead "
                         "of OpenGL's vertex-level lighting) by using a cube map and "
                         "reflective texgen. "
                         "A texture matrix is computed to rotate the cube map "
                         "automatically; this makes "
                         "the specular effect consistent with respect to view direction "
                         "and light position. "
                         "The user can choose which light should be used to compute the "
                         "texture matrix.\n"
                         "This effect requires the GL_ARB_texture_env_add extension and "
                         "one of the cube map "
                         "extensions (GL_EXT_texture_cube_map, GL_ARB_texture_cube_map "
                         "or OpenGL v1.3).",

                         "Marco Jez" );

            /** get the OpenGL light number */
            inline int
            getLightNumber() const;

            /** set the OpenGL light number that will be used in lighting computations */
            inline void
            setLightNumber( int n );

            /** get the texture unit number */
            inline int
            getTextureUnit() const;

            /** set the texture unit that will be used to apply the cube map */
            inline void
            setTextureUnit( int n );

            /** get the specular color */
            inline const osg::vec4&
            getSpecularColor() const;

            /** set the specular color */
            inline void
            setSpecularColor( const osg::vec4& color );

            /** get the specular exponent */
            inline float
            getSpecularExponent() const;

            /** set the specular exponent */
            inline void
            setSpecularExponent( float e );

        protected:

            virtual ~SpecularHighlights()
            {
            }

            SpecularHighlights&
            operator=( const SpecularHighlights& )
            {
                return *this;
            }

            bool
            define_techniques();

        private:

            int       _lightnum;
            int       _unit;
            osg::vec4 _color;
            float     _sexp;
    };

    // INLINE METHODS

    inline int
    SpecularHighlights::getLightNumber() const
    {
        return _lightnum;
    }

    inline void
    SpecularHighlights::setLightNumber( int n )
    {
        _lightnum = n;
        dirtyTechniques();
    }

    inline int
    SpecularHighlights::getTextureUnit() const
    {
        return _unit;
    }

    inline void
    SpecularHighlights::setTextureUnit( int n )
    {
        _unit = n;
        dirtyTechniques();
    }

    inline const osg::vec4&
    SpecularHighlights::getSpecularColor() const
    {
        return _color;
    }

    inline void
    SpecularHighlights::setSpecularColor( const osg::vec4& color )
    {
        _color = color;
        dirtyTechniques();
    }

    inline float
    SpecularHighlights::getSpecularExponent() const
    {
        return _sexp;
    }

    inline void
    SpecularHighlights::setSpecularExponent( float e )
    {
        _sexp = e;
        dirtyTechniques();
    }

}
