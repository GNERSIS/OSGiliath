/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Generates a specular highlight cube map. Computes Phong
 * specular intensity for each cube face texel direction.
 */
#include <osgUtil/utils/HighlightMapGenerator.hpp>

#include <osg/maths/common.hpp>

using namespace osgUtil;

HighlightMapGenerator::HighlightMapGenerator( const osg::vec3& light_direction,
                                              const osg::vec4& light_color,
                                              float            specular_exponent,
                                              int              texture_size ) :
    CubeMapGenerator( texture_size ),
    ldir_( light_direction ),
    lcol_( light_color ),
    sexp_( specular_exponent )
{
    ldir_ = osg::normalize( ldir_ );
}

HighlightMapGenerator::HighlightMapGenerator( const HighlightMapGenerator& copy,
                                              const osg::CopyOp&           copyop ) :
    CubeMapGenerator( copy,
                      copyop ),
    ldir_( copy.ldir_ ),
    lcol_( copy.lcol_ ),
    sexp_( copy.sexp_ )
{
}
