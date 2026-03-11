/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Rectangular viewport mapping NDC to window coordinates.
 * Defines the rendering area within the graphics context.
 */
#include <osg/state/Viewport.hpp>

using namespace osg;

Viewport::Viewport()
{
    _x      = 0;
    _y      = 0;
    _width  = 800;
    _height = 600;
}

Viewport::~Viewport()
{
}

void
Viewport::apply( State& ) const
{
    glViewport( static_cast<GLint>( _x ),
                static_cast<GLint>( _y ),
                static_cast<GLsizei>( _width ),
                static_cast<GLsizei>( _height ) );
}
