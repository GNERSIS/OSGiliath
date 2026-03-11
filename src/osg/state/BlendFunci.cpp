/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Indexed alpha blending for MRT. Configures per-draw-buffer
 * blend factors when rendering to multiple color attachments.
 */
#include <osg/state/BlendFunci.hpp>

#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

BlendFunci::BlendFunci() :
    _index( 0 )
{
}

BlendFunci::~BlendFunci()
{
}

void
BlendFunci::setIndex( unsigned int buf )
{
    if( _index == buf )
    {
        return;
    }

    ReassignToParents needToReassingToParentsWhenMemberValueChanges( this );

    _index = buf;
}

void
BlendFunci::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( _source_factor !=
        _source_factor_alpha ||
        _destination_factor != _destination_factor_alpha )
    {
        if( extensions->glBlendFuncSeparatei )
        {
            extensions->glBlendFuncSeparatei( static_cast<GLuint>( _index ),
                                              _source_factor,
                                              _destination_factor,
                                              _source_factor_alpha,
                                              _destination_factor_alpha );
        }
        else
        {
            OSG_WARN << "Warning: BlendFunc::apply(..) failed, BlendFuncSeparatei is "
                        "not support by OpenGL driver."
                     << std::endl;
        }
    }
    else
    {
        if( extensions->glBlendFunci )
        {
            extensions->glBlendFunci( static_cast<GLuint>( _index ),
                                      _source_factor,
                                      _destination_factor );
        }
        else
        {
            OSG_WARN << "Warning: BlendFunc::apply(..) failed, BlendFunci is not "
                        "support by OpenGL driver."
                     << std::endl;
        }
    }
}
