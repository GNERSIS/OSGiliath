/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Face culling state attribute. Configures front/back face
 * culling for correct rendering of closed meshes.
 */
#include <osg/state/CullFace.hpp>

#include <osg/GL>

using namespace osg;

CullFace::~CullFace()
{
}

void
CullFace::apply( State& ) const
{
    glCullFace( ( GLenum )_mode );
}
