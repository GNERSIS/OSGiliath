#include <osgFX/Scribe.hpp>

#include <osg/GL>
#include <osg/state/LineWidth.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/PolygonMode.hpp>
#include <osg/state/PolygonOffset.hpp>
#include <osgFX/Registry.hpp>
#include <string.h>

using namespace osgFX;

namespace
{

    // register a prototype for this effect
    Registry::Proxy proxy( new Scribe );

    // default technique class
    class DefaultTechnique : public Technique
    {
        public:

            DefaultTechnique( osg::Material*  wf_mat,
                              osg::LineWidth* wf_lw ) :
                Technique(),
                _wf_mat( wf_mat ),
                _wf_lw( wf_lw )
            {
            }

            bool
            validate( osg::State& ) const
            {
                return strncmp( ( const char* )glGetString( GL_VERSION ), "1.1", 3 ) >=
                       0;
            }

        protected:

            void
            define_passes()
            {
                // implement pass #1
                {
                    osg::ref_ptr<osg::StateSet>      ss         = new osg::StateSet;

                    osg::ref_ptr<osg::PolygonOffset> polyoffset = new osg::PolygonOffset;
                    polyoffset->setFactor( 1.0F );
                    polyoffset->setUnits( 1.0F );
                    ss->setAttributeAndModes( polyoffset.get(),
                                              osg::StateAttribute::OVERRIDE |
                                                  osg::StateAttribute::ON );

                    addPass( ss.get() );
                }

                // implement pass #2
                {
                    osg::ref_ptr<osg::StateSet>    ss       = new osg::StateSet;

                    osg::ref_ptr<osg::PolygonMode> polymode = new osg::PolygonMode;
                    polymode->setMode( osg::PolygonMode::Face::FRONT_AND_BACK,
                                       osg::PolygonMode::Mode::LINE );
                    ss->setAttributeAndModes( polymode.get(),
                                              osg::StateAttribute::OVERRIDE |
                                                  osg::StateAttribute::ON );

                    ss->setAttributeAndModes( _wf_lw.get(),
                                              osg::StateAttribute::OVERRIDE |
                                                  osg::StateAttribute::ON );

                    ss->setAttributeAndModes( _wf_mat.get(),
                                              osg::StateAttribute::OVERRIDE |
                                                  osg::StateAttribute::ON );

                    addPass( ss.get() );
                }
            }

        private:

            osg::ref_ptr<osg::Material>  _wf_mat;
            osg::ref_ptr<osg::LineWidth> _wf_lw;
    };

}

Scribe::Scribe() :
    _wf_mat( new osg::Material ),
    _wf_lw( new osg::LineWidth )
{
    _wf_lw->setWidth( 1.0F );

    _wf_mat->setColorMode( osg::Material::OFF );
    _wf_mat->setDiffuse( osg::Material::FRONT_AND_BACK,
                         osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
    _wf_mat->setAmbient( osg::Material::FRONT_AND_BACK,
                         osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
    _wf_mat->setSpecular( osg::Material::FRONT_AND_BACK,
                          osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
    _wf_mat->setEmission( osg::Material::FRONT_AND_BACK,
                          osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );
}

Scribe::Scribe( const Scribe&      copy,
                const osg::CopyOp& copyop ) :
    Inherit( copy,
             copyop ),
    _wf_mat( static_cast<osg::Material*>( copyop( copy._wf_mat.get() ) ) ),
    _wf_lw( static_cast<osg::LineWidth*>( copyop( copy._wf_lw.get() ) ) )
{
}

bool
Scribe::define_techniques()
{
    addTechnique( new DefaultTechnique( _wf_mat.get(), _wf_lw.get() ) );
    return true;
}
