/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Generates a half-angle cube map for specular lighting.
 * Computes half-vector directions for each cube face texel.
 */
#include <osgUtil/utils/HalfWayMapGenerator.hpp>

#include <osg/maths/common.hpp>

using namespace osgUtil;

HalfWayMapGenerator::HalfWayMapGenerator( const osg::vec3& light_direction,
                                          int              texture_size ) :
    CubeMapGenerator( texture_size ),
    ldir_( light_direction )
{
    ldir_ = osg::normalize( ldir_ );
}

HalfWayMapGenerator::HalfWayMapGenerator( const HalfWayMapGenerator& copy,
                                          const osg::CopyOp&         copyop ) :
    CubeMapGenerator( copy,
                      copyop ),
    ldir_( copy.ldir_ )
{
}
