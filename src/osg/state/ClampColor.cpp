/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Color clamping control attribute. Disables automatic clamping
 * of vertex colors and fragment outputs for HDR rendering.
 */
#include <osg/state/ClampColor.hpp>

#include <osg/core/buffered_value.hpp>
#include <osg/core/Notify.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/State.hpp>

using namespace osg;

ClampColor::ClampColor() :
    _clampVertexColor( GL_FIXED_ONLY ),
    _clampFragmentColor( GL_FIXED_ONLY ),
    _clampReadColor( GL_FIXED_ONLY )
{
}

ClampColor::ClampColor( GLenum vertexMode,
                        GLenum fragmentMode,
                        GLenum readMode ) :
    _clampVertexColor( vertexMode ),
    _clampFragmentColor( fragmentMode ),
    _clampReadColor( readMode )
{
}

ClampColor::~ClampColor()
{
}

void
ClampColor::apply( State& state ) const
{

    // get the contextID (user defined ID of 0 upwards) for the
    // current OpenGL context.
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( !extensions->isClampColorSupported )
    {
        OSG_WARN << "Warning: ClampColor::apply(..) failed, ClampColor is not support "
                    "by OpenGL driver."
                 << std::endl;
        return;
    }

    extensions->glClampColor( GL_CLAMP_READ_COLOR, _clampReadColor );
}
