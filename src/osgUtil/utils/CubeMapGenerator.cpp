/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for cube map face generators. Renders or
 * computes six cube faces for environment mapping.
 */
#include <osgUtil/utils/CubeMapGenerator.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/transform.hpp>
#include <stdlib.h>

using namespace osgUtil;

CubeMapGenerator::CubeMapGenerator( int texture_size ) :
    osg::Referenced(),
    texture_size_( texture_size )
{
    for( int i = 0; i < 6; ++i )
    {
        osg::ref_ptr<osg::Image> image = new osg::Image;
        unsigned char*           data =
            new unsigned char[static_cast<std::size_t>( texture_size ) *
                              static_cast<std::size_t>( texture_size ) *
                              4];
        image->setImage( texture_size,
                         texture_size,
                         1,
                         4,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         data,
                         osg::Image::USE_NEW_DELETE );
        images_.push_back( image );
    }
}

CubeMapGenerator::CubeMapGenerator( const CubeMapGenerator& copy,
                                    const osg::CopyOp&      copyop ) :
    osg::Referenced( copy ),
    texture_size_( copy.texture_size_ )
{
    Image_list::const_iterator i;
    for( i = copy.images_.begin(); i != copy.images_.end(); ++i )
    {
        images_.push_back( static_cast<osg::Image*>( copyop( i->get() ) ) );
    }
}

void
CubeMapGenerator::generateMap( bool use_osg_system )
{
    osg::dmat4 M;

    if( use_osg_system )
    {
        M = osg::rotate( osg::PI_2, 1.0, 0.0, 0.0 );
    }
    else
    {
        M = osg::dmat4();
    }

    const float dst = 2.0F / static_cast<float>( texture_size_ - 1 );

    float       t   = -1;
    for( int i = 0; i < texture_size_; ++i )
    {
        float s = -1;
        for( int j = 0; j < texture_size_; ++j )
        {
            set_pixel( 0,
                       j,
                       i,
                       compute_color( osg::vec3( osg::dvec3( 1, -t, -s ) * M ) ) );
            set_pixel( 1,
                       j,
                       i,
                       compute_color( osg::vec3( osg::dvec3( -1, -t, s ) * M ) ) );
            set_pixel( 2,
                       j,
                       i,
                       compute_color( osg::vec3( osg::dvec3( s, 1, t ) * M ) ) );
            set_pixel( 3,
                       j,
                       i,
                       compute_color( osg::vec3( osg::dvec3( s, -1, -t ) * M ) ) );
            set_pixel( 4,
                       j,
                       i,
                       compute_color( osg::vec3( osg::dvec3( s, -t, 1 ) * M ) ) );
            set_pixel( 5,
                       j,
                       i,
                       compute_color( osg::vec3( osg::dvec3( -s, -t, -1 ) * M ) ) );
            s += dst;
        }
        t += dst;
    }
}
