/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Overrides the projection matrix for its subgraph.
 * Used for HUD overlays and orthographic sub-scenes.
 */
#include <osg/nodes/Projection.hpp>

using namespace osg;

Projection::Projection()
{
}

Projection::Projection( const Projection& projection,
                        const CopyOp&     copyop ) :
    Inherit<Group,
            Projection>( projection,
                         copyop ),
    _matrix( projection._matrix )
{
}

Projection::Projection( const dmat4& mat )
{
    _matrix = mat;
}

Projection::~Projection()
{
}
