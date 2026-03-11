/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Generic GL capability toggle attribute. Wraps glEnable/glDisable
 * for capabilities not covered by specialized state attributes.
 */
#include <osg/state/Capability.hpp>

#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

Capability::Capability() :
    _capability( 0 )
{
}

Capability::~Capability()
{
}

Capabilityi::Capabilityi() :
    _index( 0 )
{
}

Capabilityi::~Capabilityi()
{
}

void
Enablei::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( extensions->glEnablei )
    {
        OSG_INFO << "extensions->glEnablei(" << _capability << ", " << _index << ")"
                 << std::endl;
        if( _capability )
        {
            extensions->glEnablei( _capability, static_cast<GLuint>( _index ) );
        }
    }
    else
    {
        OSG_WARN << "Warning: Enablei::apply(..) failed, Enablei is not support by "
                    "OpenGL driver."
                 << std::endl;
    }
}

void
Disablei::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( extensions->glDisablei )
    {
        OSG_INFO << "extensions->glDisablei(" << _capability << ", " << _index << ")"
                 << std::endl;
        if( _capability )
        {
            extensions->glDisablei( _capability, static_cast<GLuint>( _index ) );
        }
    }
    else
    {
        OSG_WARN << "Warning: Enablei::apply(..) failed, Enablei is not support by "
                    "OpenGL driver."
                 << std::endl;
    }
}
