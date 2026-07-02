// -*-c++-*-

/*
 * OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
 */

/*
 * osgFX::Outline - Copyright (C) 2004,2009 Ulrich Hertlein
 */

#include <osgFX/Outline.hpp>

#include <osg/nodes/Group.hpp>
#include <osg/state/CullFace.hpp>
#include <osg/state/LineWidth.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/PolygonMode.hpp>
#include <osg/state/Stencil.hpp>
#include <osg/textures/Texture1D.hpp>
#include <osgFX/Registry.hpp>

namespace
{

    const unsigned int Override_On =
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE;
    const unsigned int Override_Off =
        osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE;

}

namespace osgFX
{

    /// Register prototype.
    Registry::Proxy proxy( new Outline );

    /**
     * Outline technique.
     */
    class Outline::OutlineTechnique : public Technique
    {
        public:

            /// Constructor.
            OutlineTechnique() :
                Technique(),
                _lineWidth(),
                _width( 2 ),
                _material(),
                _color( 1,
                        1,
                        1,
                        1 )
            {
            }

            /// Validate.
            bool
            validate( osg::State& ) const
            {
                return true;
            }

            /// Set outline width.
            void
            setWidth( float w )
            {
                _width = w;
                if( _lineWidth.valid() )
                {
                    _lineWidth->setWidth( w );
                }
            }

            /// Set outline color.
            void
            setColor( const osg::vec4& color )
            {
                _color = color;
                if( _material.valid() )
                {
                    const osg::Material::Face face = osg::Material::FRONT_AND_BACK;
                    _material->setAmbient( face, osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
                    _material->setDiffuse( face, osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
                    _material->setSpecular( face, osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
                    _material->setEmission( face, color );
                }
            }

        protected:

            /// Define render passes.
            void
            define_passes()
            {

                /*
                 * draw
                 * - set stencil buffer to ref=1 where draw occurs
                 * - clear stencil buffer to 0 where test fails
                 */
                {
                    osg::StateSet* state = new osg::StateSet;

                    // stencil op
                    osg::Stencil*  stencil = new osg::Stencil;
                    stencil->setFunction( osg::Stencil::Function::ALWAYS, 1, ~0U );
                    stencil->setOperation( osg::Stencil::Operation::KEEP,
                                           osg::Stencil::Operation::KEEP,
                                           osg::Stencil::Operation::REPLACE );
                    state->setAttributeAndModes( stencil, Override_On );

                    addPass( state );
                }

                /*
                 * post-draw
                 * - only draw where draw didn't set the stencil buffer
                 * - draw only back-facing polygons
                 * - draw back-facing polys as lines
                 * - disable depth-test, lighting & texture
                 */
                {
                    osg::StateSet* state = new osg::StateSet;

                    // stencil op
                    osg::Stencil*  stencil = new osg::Stencil;
                    stencil->setFunction( osg::Stencil::Function::NOTEQUAL, 1, ~0U );
                    stencil->setOperation( osg::Stencil::Operation::KEEP,
                                           osg::Stencil::Operation::KEEP,
                                           osg::Stencil::Operation::REPLACE );
                    state->setAttributeAndModes( stencil, Override_On );

                    // cull front-facing polys
                    osg::CullFace* cullFace = new osg::CullFace;
                    cullFace->setMode( osg::CullFace::Mode::FRONT );
                    state->setAttributeAndModes( cullFace, Override_On );

                    // draw back-facing polygon lines
                    osg::PolygonMode* polyMode = new osg::PolygonMode;
                    polyMode->setMode( osg::PolygonMode::Face::BACK,
                                       osg::PolygonMode::Mode::LINE );
                    state->setAttributeAndModes( polyMode, Override_On );

                    // outline width
                    _lineWidth = new osg::LineWidth;
                    setWidth( _width );
                    state->setAttributeAndModes( _lineWidth.get(), Override_On );

                    // outline color/material
                    _material = new osg::Material;
                    _material->setColorMode( osg::Material::OFF );
                    setColor( _color );
                    state->setAttributeAndModes( _material.get(), Override_On );

                    // disable modes
                    state->setMode( GL_BLEND, Override_Off );
                    // state->setMode(GL_DEPTH_TEST, Override_Off);

                    addPass( state );
                }
            }

        private:

            /// Outline width.
            osg::ref_ptr<osg::LineWidth> _lineWidth;
            float                        _width;

            /// Outline Material.
            osg::ref_ptr<osg::Material>  _material;
            osg::vec4                    _color;
    };

    /**
     * Outline effect.
     */
    Outline::Outline() :
        _width( 2 ),
        _color( 1,
                1,
                1,
                1 ),
        _technique( 0 )
    {
    }

    void
    Outline::setWidth( float w )
    {
        _width = w;
        if( _technique )
        {
            _technique->setWidth( w );
        }
    }

    void
    Outline::setColor( const osg::vec4& color )
    {
        _color = color;
        if( _technique )
        {
            _technique->setColor( color );
        }
    }

    bool
    Outline::define_techniques()
    {
        _technique = new OutlineTechnique;
        addTechnique( _technique );

        setWidth( _width );
        setColor( _color );

        return true;
    }

}
