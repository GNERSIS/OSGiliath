/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Generates a reflection cube map from a scene viewpoint.
 * Renders the scene into six faces for mirror effects.
 */
#pragma once

#include <osgUtil/utils/CubeMapGenerator.hpp>

namespace osgUtil
{

    /** This is the most simple cube map generator. It performs a direct association
        between reflection vector and RGBA color (C = R).
    */
    class ReflectionMapGenerator : public CubeMapGenerator
    {
        public:

            inline ReflectionMapGenerator( int texture_size = 64 );
            inline ReflectionMapGenerator( const ReflectionMapGenerator& copy,
                                           const osg::CopyOp&            copyop =
                                               osg::CopyOp::SHALLOW_COPY );

        protected:

            virtual ~ReflectionMapGenerator()
            {
            }

            ReflectionMapGenerator&
            operator=( const ReflectionMapGenerator& )
            {
                return *this;
            }

            inline virtual osg::vec4
            compute_color( const osg::vec3& R ) const;
    };

    // INLINE METHODS

    inline ReflectionMapGenerator::ReflectionMapGenerator( int texture_size ) :
        CubeMapGenerator( texture_size )
    {
    }

    inline ReflectionMapGenerator::ReflectionMapGenerator(
        const ReflectionMapGenerator& copy,
        const osg::CopyOp&            copyop
    ) :
        CubeMapGenerator( copy,
                          copyop )
    {
    }

    inline osg::vec4
    ReflectionMapGenerator::compute_color( const osg::vec3& R ) const
    {
        return vector_to_color( R / osg::length( R ) );
    }

}
