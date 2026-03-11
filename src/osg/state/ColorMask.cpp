/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Per-channel color write mask. Controls which RGBA components
 * are written to the framebuffer during rendering.
 */
#include <osg/state/ColorMask.hpp>

using namespace osg;

ColorMask::ColorMask()
{
    // set up same defaults as glColorMask.
    _red   = true;
    _green = true;
    _blue  = true;
    _alpha = true;
}

ColorMask::~ColorMask()
{
}

void
ColorMask::apply( State& ) const
{
    glColorMask( ( GLboolean )_red,
                 ( GLboolean )_green,
                 ( GLboolean )_blue,
                 ( GLboolean )_alpha );
}
