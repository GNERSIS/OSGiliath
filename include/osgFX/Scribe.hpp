/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Wireframe overlay effect. Renders geometry twice: solid fill
 * then wireframe outline for engineering visualization.
 */
// osgFX - Copyright (C) 2003 Marco Jez

#pragma once

#include <osg/state/LineWidth.hpp>
#include <osg/state/Material.hpp>
#include <osgFX/Effect.hpp>
#include <osgFX/Export.hpp>

namespace osgFX
{

    /**
     This is a two-passes effect; the first pass renders the subgraph as usual
     while the second pass switches to wireframe mode, sets up lighting and
     material to obtain a fixed (user-defined) color and then renders the subgraph.
     This effect uses the PolygonOffset attribute to avoid Z-fighting, so it
     requires at least OpenGL version 1.1.
    */
    class OSGFX_EXPORT Scribe : public osg::Inherit<Effect, Scribe>
    {
        public:

            Scribe();
            Scribe( const Scribe&      copy,
                    const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            // effect class information
            META_Effect(
                osgFX,
                Scribe,

                "Scribe",

                "This is a two-passes effect; the first pass renders the subgraph as "
                "usual "
                "while the second pass switches to wireframe mode, sets up lighting and "
                "material to obtain a fixed (user-defined) color and then renders the "
                "subgraph.\n"
                "This effect uses the PolygonOffset attribute to avoid Z-fighting, so "
                "it "
                "requires at least OpenGL version 1.1.",

                "Marco Jez"
            );

            /** get the wireframe color */
            inline const osg::vec4&
            getWireframeColor() const;

            /** set the wireframe color */
            inline void
            setWireframeColor( const osg::vec4& color );

            /** get the wireframe line width */
            inline float
            getWireframeLineWidth() const;

            /** set the wireframe line width */
            inline void
            setWireframeLineWidth( float w );

        protected:

            virtual ~Scribe()
            {
            }

            Scribe&
            operator=( const Scribe& )
            {
                return *this;
            }

            bool
            define_techniques();

        private:

            osg::ref_ptr<osg::Material>  _wf_mat;
            osg::ref_ptr<osg::LineWidth> _wf_lw;
    };

    // INLINE METHODS

    inline const osg::vec4&
    Scribe::getWireframeColor() const
    {
        return _wf_mat->getEmission( osg::Material::FRONT_AND_BACK );
    }

    inline void
    Scribe::setWireframeColor( const osg::vec4& color )
    {
        _wf_mat->setEmission( osg::Material::FRONT_AND_BACK, color );
    }

    inline float
    Scribe::getWireframeLineWidth() const
    {
        return _wf_lw->getWidth();
    }

    inline void
    Scribe::setWireframeLineWidth( float w )
    {
        _wf_lw->setWidth( w );
    }

}
