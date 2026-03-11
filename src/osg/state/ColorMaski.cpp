/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Indexed color write mask for MRT. Controls per-draw-buffer
 * RGBA write masks when rendering to multiple color attachments.
 */
#include <osg/state/ColorMaski.hpp>

#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

ColorMaski::ColorMaski() :
    _index( 0 )
{
}

ColorMaski::~ColorMaski()
{
}

void
ColorMaski::setIndex( unsigned int buf )
{
    if( _index == buf )
    {
        return;
    }

    ReassignToParents needToReassingToParentsWhenMemberValueChanges( this );

    _index = buf;
}

void
ColorMaski::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( extensions->glColorMaski )
    {
        extensions->glColorMaski( ( GLuint )_index,
                                  ( GLboolean )_red,
                                  ( GLboolean )_green,
                                  ( GLboolean )_blue,
                                  ( GLboolean )_alpha );
    }
    else
    {
        OSG_WARN << "Warning: ColorMaski::apply(..) failed, glColorMaski is not support "
                    "by OpenGL driver."
                 << std::endl;
    }
}
