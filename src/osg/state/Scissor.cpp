/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Scissor test attribute defining a rectangular clip region
 * in window coordinates. Pixels outside are discarded.
 */
#include <osg/state/Scissor.hpp>

using namespace osg;

Scissor::Scissor()
{
    _x      = 0;
    _y      = 0;
    _width  = 800;
    _height = 600;
}

Scissor::~Scissor()
{
}

void
Scissor::apply( State& ) const
{
    glScissor( _x, _y, _width, _height );
}
