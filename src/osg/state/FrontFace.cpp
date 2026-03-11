/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Front-face winding order attribute. Selects whether CW or CCW
 * vertex winding defines the front face for culling and lighting.
 */
#include <osg/state/FrontFace.hpp>

#include <osg/GL>

using namespace osg;

FrontFace::FrontFace( Mode face )
{
    _mode = face;
}

FrontFace::~FrontFace()
{
}

void
FrontFace::apply( State& ) const
{
    glFrontFace( ( GLenum )_mode );
}
