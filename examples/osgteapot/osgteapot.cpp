/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgteapot example application
 */
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/core/Viewer.hpp>

// The classic OpenGL teapot... taken from glut-3.7/lib/glut/glut_teapot.c
// Modernized: CPU-side Bezier patch tessellation replacing deprecated GL evaluators.

/* Copyright (c) Mark J. Kilgard, 1994. */

/**
(c) Copyright 1993, Silicon Graphics, Inc.

ALL RIGHTS RESERVED

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies and that
both the copyright notice and this permission notice appear in
supporting documentation, and that the name of Silicon
Graphics, Inc. not be used in advertising or publicity
pertaining to distribution of the software without specific,
written prior permission.

THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU
"AS-IS" AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR
OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  IN NO
EVENT SHALL SILICON GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE
ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER,
INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS OF USE,
SAVINGS OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, WHETHER OR
NOT SILICON GRAPHICS, INC.  HAS BEEN ADVISED OF THE POSSIBILITY
OF SUCH LOSS, HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
ARISING OUT OF OR IN CONNECTION WITH THE POSSESSION, USE OR
PERFORMANCE OF THIS SOFTWARE.

US Government Users Restricted Rights

Use, duplication, or disclosure by the Government is subject to
restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
(c)(1)(ii) of the Rights in Technical Data and Computer
Software clause at DFARS 252.227-7013 and/or in similar or
successor clauses in the FAR or the DOD or NASA FAR
Supplement.  Unpublished-- rights reserved under the copyright
laws of the United States.  Contractor/manufacturer is Silicon
Graphics, Inc., 2011 N.  Shoreline Blvd., Mountain View, CA
94039-7311.

OpenGL(TM) is a trademark of Silicon Graphics, Inc.
*/

/* Rim, body, lid, and bottom data must be reflected in x and
   y; handle and spout data across the y axis only.  */

static int patchdata[][16] = {
    /* rim */
    { 102, 103, 104, 105, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    /* body */
    { 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 },
    { 24, 25, 26, 27, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40 },
    /* lid */
    {
     96, 96,
     96, 96,
     97, 98,
     99, 100,
     101, 101,
     101, 101,
     0, 1,
     2, 3,
     },
    { 0, 1, 2, 3, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117 },
    /* bottom */
    { 118, 118, 118, 118, 124, 122, 119, 121, 123, 126, 125, 120, 40, 39, 38, 37 },
    /* handle */
    { 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56 },
    { 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 28, 65, 66, 67 },
    /* spout */
    { 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83 },
    { 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 }
};
/* *INDENT-OFF* */

static float cpdata[][3] = {
    {   0.2F,      0.F,     2.7F},
    {   0.2F,  -0.112F,     2.7F},
    { 0.112F,    -0.2F,     2.7F},
    {    0.F,    -0.2F,     2.7F},
    {1.3375F,      0.F, 2.53125F},
    {1.3375F,  -0.749F, 2.53125F},
    { 0.749F, -1.3375F, 2.53125F},
    {    0.F, -1.3375F, 2.53125F},
    {1.4375F,      0.F, 2.53125F},
    {1.4375F,  -0.805F, 2.53125F},
    { 0.805F, -1.4375F, 2.53125F},
    {    0.F, -1.4375F, 2.53125F},
    {   1.5F,      0.F,     2.4F},
    {   1.5F,   -0.84F,     2.4F},
    {  0.84F,    -1.5F,     2.4F},
    {    0.F,    -1.5F,     2.4F},
    {  1.75F,      0.F,   1.875F},
    {  1.75F,   -0.98F,   1.875F},
    {  0.98F,   -1.75F,   1.875F},
    {    0.F,   -1.75F,   1.875F},
    {    2.F,      0.F,    1.35F},
    {    2.F,   -1.12F,    1.35F},
    {  1.12F,     -2.F,    1.35F},
    {    0.F,     -2.F,    1.35F},
    {    2.F,      0.F,     0.9F},
    {    2.F,   -1.12F,     0.9F},
    {  1.12F,     -2.F,     0.9F},
    {    0.F,     -2.F,     0.9F},
    {   -2.F,      0.F,     0.9F},
    {    2.F,      0.F,    0.45F},
    {    2.F,   -1.12F,    0.45F},
    {  1.12F,     -2.F,    0.45F},
    {    0.F,     -2.F,    0.45F},
    {   1.5F,      0.F,   0.225F},
    {   1.5F,   -0.84F,   0.225F},
    {  0.84F,    -1.5F,   0.225F},
    {    0.F,    -1.5F,   0.225F},
    {   1.5F,      0.F,    0.15F},
    {   1.5F,   -0.84F,    0.15F},
    {  0.84F,    -1.5F,    0.15F},
    {    0.F,    -1.5F,    0.15F},
    {  -1.6F,      0.F,   2.025F},
    {  -1.6F,    -0.3F,   2.025F},
    {  -1.5F,    -0.3F,    2.25F},
    {  -1.5F,      0.F,    2.25F},
    {  -2.3F,      0.F,   2.025F},
    {  -2.3F,    -0.3F,   2.025F},
    {  -2.5F,    -0.3F,    2.25F},
    {  -2.5F,      0.F,    2.25F},
    {  -2.7F,      0.F,   2.025F},
    {  -2.7F,    -0.3F,   2.025F},
    {   -3.F,    -0.3F,    2.25F},
    {   -3.F,      0.F,    2.25F},
    {  -2.7F,      0.F,     1.8F},
    {  -2.7F,    -0.3F,     1.8F},
    {   -3.F,    -0.3F,     1.8F},
    {   -3.F,      0.F,     1.8F},
    {  -2.7F,      0.F,   1.575F},
    {  -2.7F,    -0.3F,   1.575F},
    {   -3.F,    -0.3F,    1.35F},
    {   -3.F,      0.F,    1.35F},
    {  -2.5F,      0.F,   1.125F},
    {  -2.5F,    -0.3F,   1.125F},
    { -2.65F,    -0.3F,  0.9375F},
    { -2.65F,      0.F,  0.9375F},
    {   -2.F,    -0.3F,     0.9F},
    {  -1.9F,    -0.3F,     0.6F},
    {  -1.9F,      0.F,     0.6F},
    {   1.7F,      0.F,   1.425F},
    {   1.7F,   -0.66F,   1.425F},
    {   1.7F,   -0.66F,     0.6F},
    {   1.7F,      0.F,     0.6F},
    {   2.6F,      0.F,   1.425F},
    {   2.6F,   -0.66F,   1.425F},
    {   3.1F,   -0.66F,   0.825F},
    {   3.1F,      0.F,   0.825F},
    {   2.3F,      0.F,     2.1F},
    {   2.3F,   -0.25F,     2.1F},
    {   2.4F,   -0.25F,   2.025F},
    {   2.4F,      0.F,   2.025F},
    {   2.7F,      0.F,     2.4F},
    {   2.7F,   -0.25F,     2.4F},
    {   3.3F,   -0.25F,     2.4F},
    {   3.3F,      0.F,     2.4F},
    {   2.8F,      0.F,   2.475F},
    {   2.8F,   -0.25F,   2.475F},
    { 3.525F,   -0.25F, 2.49375F},
    { 3.525F,      0.F, 2.49375F},
    {   2.9F,      0.F,   2.475F},
    {   2.9F,   -0.15F,   2.475F},
    {  3.45F,   -0.15F,  2.5125F},
    {  3.45F,      0.F,  2.5125F},
    {   2.8F,      0.F,     2.4F},
    {   2.8F,   -0.15F,     2.4F},
    {   3.2F,   -0.15F,     2.4F},
    {   3.2F,      0.F,     2.4F},
    {    0.F,      0.F,    3.15F},
    {   0.8F,      0.F,    3.15F},
    {   0.8F,   -0.45F,    3.15F},
    {  0.45F,    -0.8F,    3.15F},
    {    0.F,    -0.8F,    3.15F},
    {    0.F,      0.F,    2.85F},
    {   1.4F,      0.F,     2.4F},
    {   1.4F,  -0.784F,     2.4F},
    { 0.784F,    -1.4F,     2.4F},
    {    0.F,    -1.4F,     2.4F},
    {   0.4F,      0.F,    2.55F},
    {   0.4F,  -0.224F,    2.55F},
    { 0.224F,    -0.4F,    2.55F},
    {    0.F,    -0.4F,    2.55F},
    {   1.3F,      0.F,    2.55F},
    {   1.3F,  -0.728F,    2.55F},
    { 0.728F,    -1.3F,    2.55F},
    {    0.F,    -1.3F,    2.55F},
    {   1.3F,      0.F,     2.4F},
    {   1.3F,  -0.728F,     2.4F},
    { 0.728F,    -1.3F,     2.4F},
    {    0.F,    -1.3F,     2.4F},
    {    0.F,      0.F,      0.F},
    { 1.425F,  -0.798F,      0.F},
    {   1.5F,      0.F,   0.075F},
    { 1.425F,      0.F,      0.F},
    { 0.798F,  -1.425F,      0.F},
    {    0.F,    -1.5F,   0.075F},
    {    0.F,  -1.425F,      0.F},
    {   1.5F,   -0.84F,   0.075F},
    {  0.84F,    -1.5F,   0.075F}
};

/* *INDENT-ON* */

// Bernstein basis functions for cubic Bezier
static inline float
bernstein( int   i,
           float t )
{
    float mt = 1.0F - t;
    switch( i )
    {
        case 0 :
            return mt * mt * mt;
        case 1 :
            return 3.0F * t * mt * mt;
        case 2 :
            return 3.0F * t * t * mt;
        case 3 :
            return t * t * t;
    }
    return 0.0F;
}

// Derivative of Bernstein basis functions for cubic Bezier
static inline float
dbernstein( int   i,
            float t )
{
    float mt = 1.0F - t;
    switch( i )
    {
        case 0 :
            return -3.0F * mt * mt;
        case 1 :
            return 3.0F * ( mt * mt - 2.0F * t * mt );
        case 2 :
            return 3.0F * ( 2.0F * t * mt - t * t );
        case 3 :
            return 3.0F * t * t;
    }
    return 0.0F;
}

// Evaluate a bicubic Bezier patch at (u,v) and compute partial derivatives
static void
evaluatePatch( const float patch[4][4][3],
               float       u,
               float       v,
               osg::vec3&  pos,
               osg::vec3&  du,
               osg::vec3&  dv )
{
    pos.set( 0.0F, 0.0F, 0.0F );
    du.set( 0.0F, 0.0F, 0.0F );
    dv.set( 0.0F, 0.0F, 0.0F );

    for( int i = 0; i < 4; ++i )
    {
        float bi  = bernstein( i, u );
        float dbi = dbernstein( i, u );
        for( int j = 0; j < 4; ++j )
        {
            float     bj  = bernstein( j, v );
            float     dbj = dbernstein( j, v );

            osg::vec3 cp( patch[i][j][0], patch[i][j][1], patch[i][j][2] );

            pos += cp * ( bi * bj );
            du  += cp * ( dbi * bj );
            dv  += cp * ( bi * dbj );
        }
    }
}

// Tessellate a single Bezier patch into the given arrays.
// baseVertex is the starting index for this patch's vertices in the global arrays.
static void
tessellatePatch( const float              patch[4][4][3],
                 int                      grid,
                 osg::Vec3Array*          vertices,
                 osg::Vec3Array*          normals,
                 osg::Vec2Array*          texcoords,
                 osg::DrawElementsUShort* indices,
                 unsigned short           baseVertex )
{
    int vertsPerSide = grid + 1;

    // Generate vertices, normals, and texcoords for this patch
    for( int i = 0; i <= grid; ++i )
    {
        float u = ( float )i / ( float )grid;
        for( int j = 0; j <= grid; ++j )
        {
            float     v = ( float )j / ( float )grid;

            osg::vec3 pos, du, dv;
            evaluatePatch( patch, u, v, pos, du, dv );

            osg::vec3 normal = dv ^ du;
            float     len    = osg::length( normal );
            if( len > 1.0E-6F )
            {
                normal /= len;
            }
            else
            {
                normal.set( 0.0F, 0.0F, 1.0F );
            }

            vertices->push_back( pos );
            normals->push_back( normal );
            texcoords->push_back( osg::vec2( u, v ) );
        }
    }

    // Generate triangle indices for this patch
    for( int i = 0; i < grid; ++i )
    {
        for( int j = 0; j < grid; ++j )
        {
            unsigned short v00 = baseVertex + i * vertsPerSide + j;
            unsigned short v01 = baseVertex + i * vertsPerSide + ( j + 1 );
            unsigned short v10 = baseVertex + ( i + 1 ) * vertsPerSide + j;
            unsigned short v11 = baseVertex + ( i + 1 ) * vertsPerSide + ( j + 1 );

            // Two triangles per quad
            indices->push_back( v00 );
            indices->push_back( v10 );
            indices->push_back( v01 );

            indices->push_back( v01 );
            indices->push_back( v10 );
            indices->push_back( v11 );
        }
    }
}

osg::Geometry*
createTeapotGeometry( int grid = 14 )
{
    osg::Vec3Array*          vertices  = new osg::Vec3Array;
    osg::Vec3Array*          normals   = new osg::Vec3Array;
    osg::Vec2Array*          texcoords = new osg::Vec2Array;
    osg::DrawElementsUShort* indices =
        new osg::DrawElementsUShort( osg::PrimitiveSet::TRIANGLES );

    // Build all the reflected/mirrored patches exactly as the original code did.
    // Patches 0-5 (rim, body, lid, bottom) are reflected in both x and y (4 variants).
    // Patches 6-9 (handle, spout) are reflected across y axis only (2 variants).
    for( int i = 0; i < 10; ++i )
    {
        float p[4][4][3], q[4][4][3], r[4][4][3], s[4][4][3];

        for( int j = 0; j < 4; ++j )
        {
            for( int k = 0; k < 4; ++k )
            {
                for( int l = 0; l < 3; ++l )
                {
                    p[j][k][l] = cpdata[patchdata[i][j * 4 + k]][l];
                    q[j][k][l] = cpdata[patchdata[i][j * 4 + ( 3 - k )]][l];
                    if( l == 1 )
                    {
                        q[j][k][l] *= -1.0F;
                    }
                    if( i < 6 )
                    {
                        r[j][k][l] = cpdata[patchdata[i][j * 4 + ( 3 - k )]][l];
                        if( l == 0 )
                        {
                            r[j][k][l] *= -1.0F;
                        }
                        s[j][k][l] = cpdata[patchdata[i][j * 4 + k]][l];
                        if( l == 0 )
                        {
                            s[j][k][l] *= -1.0F;
                        }
                        if( l == 1 )
                        {
                            s[j][k][l] *= -1.0F;
                        }
                    }
                }
            }
        }

        // Tessellate patch p
        unsigned short base = ( unsigned short )vertices->size();
        tessellatePatch( p, grid, vertices, normals, texcoords, indices, base );

        // Tessellate patch q (y-reflected)
        base = ( unsigned short )vertices->size();
        tessellatePatch( q, grid, vertices, normals, texcoords, indices, base );

        if( i < 6 )
        {
            // Tessellate patch r (x-reflected)
            base = ( unsigned short )vertices->size();
            tessellatePatch( r, grid, vertices, normals, texcoords, indices, base );

            // Tessellate patch s (x and y reflected)
            base = ( unsigned short )vertices->size();
            tessellatePatch( s, grid, vertices, normals, texcoords, indices, base );
        }
    }

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray( vertices );
    geom->setNormalArray( normals, osg::Array::BIND_PER_VERTEX );
    geom->setTexCoordArray( 0, texcoords );
    geom->addPrimitiveSet( indices );

    return geom;
}

osg::Geode*
createTeapot()
{
    osg::Geode* geode = new osg::Geode();

    // add the teapot to the geode.
    geode->addDrawable( createTeapotGeometry() );

    // add a reflection map to the teapot.
    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile( "Images/reflect.rgb" );
    if( image )
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage( image );

        osg::StateSet* stateset = new osg::StateSet;
        stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );

        // Activate sphere-map texture coordinate generation in shader pipeline.
        // Replaces osg::TexGen(SPHERE_MAP) from the fixed-function version.
        stateset->setTextureMode( 0, GL_TEXTURE_GEN_S, osg::StateAttribute::ON );

        geode->setStateSet( stateset );
    }

    return geode;
}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
#if 1

    // create viewer on heap as a test, this looks to be causing problems
    // on init on some platforms, and seg fault on exit when multi-threading on linux.
    // Normal stack based version below works fine though...

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;

    // add model to viewer.
    viewer->setSceneData( createTeapot() );

    return viewer->run();

#else

    // construct the viewer.
    osgViewer::Viewer viewer;

    // add model to viewer.
    viewer.setSceneData( createTeapot() );

    // create the windows and run the threads.
    return viewer.run();
#endif
}
