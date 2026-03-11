/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Depth buffer test configuration. Controls depth function, range,
 * and write mask for correct z-ordering.
 */
#include <osg/state/Depth.hpp>

using namespace osg;

Depth::Depth( Function func,
              double   zNear,
              double   zFar,
              bool     writeMask ) :
    _func( func ),
    _zNear( zNear ),
    _zFar( zFar ),
    _depthWriteMask( writeMask )
{
}

Depth::~Depth()
{
}

void
Depth::apply( State& ) const
{
    glDepthFunc( ( GLenum )_func );
    glDepthMask( ( GLboolean )_depthWriteMask );
    glDepthRange( _zNear, _zFar );
}
