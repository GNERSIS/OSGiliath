/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Perlin noise generator. Creates 3D noise textures for
 * procedural effects (clouds, terrain, turbulence).
 */
/************************************************************************
 *                                                                      *
 *                   Copyright (C) 2002  3Dlabs Inc. Ltd.               *
 *                                                                      *
 ***********************************************************************/

/* Adapted from osgshaders example by Robert Osfield for use as part of osgUtil.*/

#pragma once

#include <osg/textures/Texture3D.hpp>
#include <osgUtil/Export>

namespace osgUtil
{

    class OSGUTIL_EXPORT PerlinNoise
    {
        public:

            PerlinNoise();

            void
            SetNoiseFrequency( int frequency );

            double
            noise1( double arg );
            double
            noise2( double vec[2] );
            double
            noise3( double vec[3] );
            void
            normalize2( double vec[2] );
            void
            normalize3( double vec[3] );

            /*
            In what follows "alpha" is the weight when the sum is formed.
            Typically it is 2, As this approaches 1 the function is noisier.
            "beta" is the harmonic scaling/spacing, typically 2.
            */

            double
            PerlinNoise1D( double x,
                           double alpha,
                           double beta,
                           int    n );
            double
            PerlinNoise2D( double x,
                           double y,
                           double alpha,
                           double beta,
                           int    n );
            double
            PerlinNoise3D( double x,
                           double y,
                           double z,
                           double alpha,
                           double beta,
                           int    n );

            osg::Image*
            create3DNoiseImage( int texSize );
            osg::Texture3D*
            create3DNoiseTexture( int texSize );

        protected:

            void
            initNoise( void );

            enum
            {
                MAXB = 0X1'00,
            };

            int    p[MAXB + MAXB + 2];
            double g3[MAXB + MAXB + 2][3];
            double g2[MAXB + MAXB + 2][2];
            double g1[MAXB + MAXB + 2];

            int    start;
            int    B;
            int    BM;
    };

    inline osg::Image*
    create3DNoiseImage( int texSize )
    {
        PerlinNoise pn;
        return pn.create3DNoiseImage( texSize );
    }

    inline osg::Texture3D*
    create3DNoiseTexture( int texSize )
    {
        PerlinNoise pn;
        return pn.create3DNoiseTexture( texSize );
    }

}
