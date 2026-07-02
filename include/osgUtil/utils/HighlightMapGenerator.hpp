/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Generates a specular highlight cube map. Computes Phong
 * specular intensity for each cube face texel direction.
 */
#pragma once

#include <osgUtil/Export.hpp>
#include <osgUtil/utils/CubeMapGenerator.hpp>

namespace osgUtil
{

    /** This cube map generator produces a specular highlight map.
     * The vector-color association is: C = (R dot (-L)) ^ n, where C is the
     * resulting color, R is the reflection vector, L is the light direction
     * and n is the specular exponent.
     */
    class OSGUTIL_EXPORT HighlightMapGenerator : public CubeMapGenerator
    {
        public:

            HighlightMapGenerator( const osg::vec3& light_direction,
                                   const osg::vec4& light_color,
                                   float            specular_exponent,
                                   int              texture_size = 64 );

            HighlightMapGenerator( const HighlightMapGenerator& copy,
                                   const osg::CopyOp&           copyop =
                                       osg::CopyOp::SHALLOW_COPY );

        protected:

            virtual ~HighlightMapGenerator()
            {
            }

            HighlightMapGenerator&
            operator=( const HighlightMapGenerator& )
            {
                return *this;
            }

            inline virtual osg::vec4
            compute_color( const osg::vec3& R ) const;

        private:

            osg::vec3 ldir_;
            osg::vec4 lcol_;
            float     sexp_;
    };

    // INLINE METHODS

    inline osg::vec4
    HighlightMapGenerator::compute_color( const osg::vec3& R ) const
    {
        float v = -osg::dot( ldir_, R / osg::length( R ) );
        if( v < 0 )
        {
            v = 0;
        }
        osg::vec4 color( lcol_ * powf( v, sexp_ ) );
        color.w = 1;
        return color;
    }

}
