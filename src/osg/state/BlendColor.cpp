/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Constant blend color used with GL_CONSTANT_ALPHA and
 * GL_CONSTANT_COLOR blend factors.
 */
#include <osg/state/BlendColor.hpp>

#include <osg/core/buffered_value.hpp>
#include <osg/core/Notify.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

BlendColor::BlendColor() :
    _constantColor( 1.0F,
                    1.0F,
                    1.0F,
                    1.0F )
{
}

BlendColor::BlendColor( const osg::vec4& constantColor ) :
    _constantColor( constantColor )
{
}

BlendColor::~BlendColor()
{
}

void
BlendColor::apply( State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( !extensions->isBlendColorSupported )
    {
        OSG_WARN << "Warning: BlendColor::apply(..) failed, BlendColor is not support "
                    "by OpenGL driver."
                 << std::endl;
        return;
    }

    extensions->glBlendColor( _constantColor[0],
                              _constantColor[1],
                              _constantColor[2],
                              _constantColor[3] );
}
