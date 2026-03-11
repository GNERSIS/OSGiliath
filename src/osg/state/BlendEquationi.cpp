/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Indexed blending equation for MRT. Sets per-draw-buffer
 * blend equations when rendering to multiple color attachments.
 */
#include <osg/state/BlendEquationi.hpp>

#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

BlendEquationi::BlendEquationi() :
    _index( 0 )
{
}

BlendEquationi::~BlendEquationi()
{
}

void
BlendEquationi::setIndex( unsigned int buf )
{
    if( _index == buf )
    {
        return;
    }

    ReassignToParents needToReassingToParentsWhenMemberValueChanges( this );

    _index = buf;
}

void
BlendEquationi::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( _equationRGB == _equationAlpha )
    {
        if( extensions->glBlendEquationi )
        {
            extensions->glBlendEquationi( static_cast<GLuint>( _index ),
                                          static_cast<GLenum>( _equationRGB ) );
        }
        else
        {
            OSG_WARN
                << "Warning: BlendEquationi::apply(..) not supported by OpenGL driver."
                << std::endl;
        }
    }
    else
    {
        if( extensions->glBlendEquationSeparatei )
        {
            extensions->glBlendEquationSeparatei(
                static_cast<GLuint>( _index ),
                static_cast<GLenum>( _equationRGB ),
                static_cast<GLenum>( _equationAlpha )
            );
        }
        else
        {
            OSG_WARN << "Warning: BlendEquation::apply(..) failed, "
                        "glBlendEquationSeparatei not supported by OpenGL driver."
                     << std::endl;
        }
    }
}
