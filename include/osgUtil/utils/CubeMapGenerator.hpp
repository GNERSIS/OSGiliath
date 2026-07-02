/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for cube map face generators. Renders or
 * computes six cube faces for environment mapping.
 */
#pragma once

#include <osg/core/CopyOp.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/Referenced.hpp>
#include <osg/images/Image.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/textures/TextureCubeMap.hpp>
#include <osgUtil/Export.hpp>
#include <vector>

namespace osgUtil
{

    /** This is the base class for cube map generators.
        It exposes the necessary interface to access the six generated images;
        descendants should only override the compute_color() method.
    */
    class OSGUTIL_EXPORT CubeMapGenerator : public osg::Referenced
    {
        public:

            explicit CubeMapGenerator( int texture_size = 64 );
            CubeMapGenerator( const CubeMapGenerator& copy,
                              const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            inline osg::Image*
            getImage( osg::TextureCubeMap::Face face );
            inline const osg::Image*
            getImage( osg::TextureCubeMap::Face face ) const;

            /** Generate the six cube images.
                If use_osg_system is true, then the OSG's coordinate system is used
               instead of the default OpenGL one.
            */
            void
            generateMap( bool use_osg_system = true );

        protected:

            virtual ~CubeMapGenerator()
            {
            }

            CubeMapGenerator&
            operator=( const CubeMapGenerator& )
            {
                return *this;
            }

            inline void
            set_pixel( int              index,
                       int              c,
                       int              r,
                       const osg::vec4& color );
            inline static osg::vec4
            vector_to_color( const osg::vec3& vec );

            /** Override this method to define how colors are computed.
                The parameter R is the reflection vector, pointing from the center of the
               cube. The return value should be the RGBA color associated with that
               reflection ray.
            */
            virtual osg::vec4
            compute_color( const osg::vec3& R ) const = 0;

        private:

            int                                           texture_size_;

            typedef std::vector<osg::ref_ptr<osg::Image>> Image_list;
            Image_list                                    images_;
    };

    // INLINE METHODS

    inline osg::Image*
    CubeMapGenerator::getImage( osg::TextureCubeMap::Face face )
    {
        switch( face )
        {
            case osg::TextureCubeMap::POSITIVE_X :
                return images_[0].get();
            case osg::TextureCubeMap::NEGATIVE_X :
                return images_[1].get();
            case osg::TextureCubeMap::POSITIVE_Y :
                return images_[2].get();
            case osg::TextureCubeMap::NEGATIVE_Y :
                return images_[3].get();
            case osg::TextureCubeMap::POSITIVE_Z :
                return images_[4].get();
            case osg::TextureCubeMap::NEGATIVE_Z :
                return images_[5].get();
            default :
                return 0;
        }
    }

    inline const osg::Image*
    CubeMapGenerator::getImage( osg::TextureCubeMap::Face face ) const
    {
        switch( face )
        {
            case osg::TextureCubeMap::POSITIVE_X :
                return images_[0].get();
            case osg::TextureCubeMap::NEGATIVE_X :
                return images_[1].get();
            case osg::TextureCubeMap::POSITIVE_Y :
                return images_[2].get();
            case osg::TextureCubeMap::NEGATIVE_Y :
                return images_[3].get();
            case osg::TextureCubeMap::POSITIVE_Z :
                return images_[4].get();
            case osg::TextureCubeMap::NEGATIVE_Z :
                return images_[5].get();
            default :
                return 0;
        }
    }

    inline void
    CubeMapGenerator::set_pixel( int              index,
                                 int              c,
                                 int              r,
                                 const osg::vec4& color )
    {
        osg::Image* i = images_[static_cast<std::size_t>( index )].get();
        if( i )
        {
            *( i->data( static_cast<unsigned int>( c ),
                        static_cast<unsigned int>( r ) ) +
               0 ) =
                static_cast<unsigned char>( osg::clampBetween( color.x, 0.0F, 1.0F ) *
                                            255 );
            *( i->data( static_cast<unsigned int>( c ),
                        static_cast<unsigned int>( r ) ) +
               1 ) =
                static_cast<unsigned char>( osg::clampBetween( color.y, 0.0F, 1.0F ) *
                                            255 );
            *( i->data( static_cast<unsigned int>( c ),
                        static_cast<unsigned int>( r ) ) +
               2 ) =
                static_cast<unsigned char>( osg::clampBetween( color.z, 0.0F, 1.0F ) *
                                            255 );
            *( i->data( static_cast<unsigned int>( c ),
                        static_cast<unsigned int>( r ) ) +
               3 ) =
                static_cast<unsigned char>( osg::clampBetween( color.w, 0.0F, 1.0F ) *
                                            255 );
        }
        else
        {
            osg::notify( osg::WARN )
                << "Warning: CubeMapGenerator::set_pixel(): invalid image index\n";
        }
    }

    inline osg::vec4
    CubeMapGenerator::vector_to_color( const osg::vec3& vec )
    {
        float len = osg::length( vec );
        return osg::vec4( vec.x / len / 2 + 0.5F,
                          vec.y / len / 2 + 0.5F,
                          vec.z / len / 2 + 0.5F,
                          1 );
    }

}
