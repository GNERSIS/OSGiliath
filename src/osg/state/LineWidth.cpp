/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Line width state attribute. Sets GL line width for
 * GL_LINES, GL_LINE_STRIP, and GL_LINE_LOOP primitives.
 */
#include <osg/state/LineWidth.hpp>

#include <osg/core/Notify.hpp>
#include <osg/GL>

using namespace osg;

LineWidth::LineWidth( float width )
{
    _width = width;
}

LineWidth::~LineWidth()
{
}

void
LineWidth::setWidth( float width )
{
    _width = width;
}

void
LineWidth::apply( State& ) const
{
    glLineWidth( _width );
}
