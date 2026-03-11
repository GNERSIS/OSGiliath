/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Generates a half-angle cube map for specular lighting.
 * Computes half-vector directions for each cube face texel.
 */
#pragma once

#include <osgUtil/Export>
#include <osgUtil/utils/CubeMapGenerator.hpp>

namespace osgUtil
{

    /** This cube map generator produces an Half-way vector map, useful for
     * hardware-based specular lighting effects.
     * It computes: C = normalize(R - L), where C is the resulting color,
     * R is the reflection vector and L is the light direction.
     */
    class OSGUTIL_EXPORT HalfWayMapGenerator : public CubeMapGenerator
    {
        public:

            HalfWayMapGenerator( const osg::vec3& light_direction,
                                 int              texture_size = 64 );
            HalfWayMapGenerator( const HalfWayMapGenerator& copy,
                                 const osg::CopyOp&         copyop );

        protected:

            virtual ~HalfWayMapGenerator()
            {
            }

            HalfWayMapGenerator&
            operator=( const HalfWayMapGenerator& )
            {
                return *this;
            }

            inline virtual osg::vec4
            compute_color( const osg::vec3& R ) const;

        private:

            osg::vec3 ldir_;
    };

    // INLINE METHODS

    inline osg::vec4
    HalfWayMapGenerator::compute_color( const osg::vec3& R ) const
    {
        const osg::vec3 V = ( R / osg::length( R ) ) - ldir_;
        return vector_to_color( V / osg::length( V ) );
    }

}
