/* OpenSceneGraph example, osgteapot.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>

#include <osgDB/ReadFile>

#include <osgViewer/Viewer>


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

static int patchdata[][16] =
{
    /* rim */
  {102, 103, 104, 105, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15},
    /* body */
  {12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27},
  {24, 25, 26, 27, 29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 39, 40},
    /* lid */
  {96, 96, 96, 96, 97, 98, 99, 100, 101, 101, 101,
    101, 0, 1, 2, 3,},
  {0, 1, 2, 3, 106, 107, 108, 109, 110, 111, 112,
    113, 114, 115, 116, 117},
    /* bottom */
  {118, 118, 118, 118, 124, 122, 119, 121, 123, 126,
    125, 120, 40, 39, 38, 37},
    /* handle */
  {41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
    53, 54, 55, 56},
  {53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
    28, 65, 66, 67},
    /* spout */
  {68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83},
  {80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
    92, 93, 94, 95}
};
/* *INDENT-OFF* */

static float cpdata[][3] =
{
    {0.2f, 0.f, 2.7f}, {0.2f, -0.112f, 2.7f}, {0.112f, -0.2f, 2.7f}, {0.f,
    -0.2f, 2.7f}, {1.3375f, 0.f, 2.53125f}, {1.3375f, -0.749f, 2.53125f},
    {0.749f, -1.3375f, 2.53125f}, {0.f, -1.3375f, 2.53125f}, {1.4375f,
    0.f, 2.53125f}, {1.4375f, -0.805f, 2.53125f}, {0.805f, -1.4375f,
    2.53125f}, {0.f, -1.4375f, 2.53125f}, {1.5f, 0.f, 2.4f}, {1.5f, -0.84f,
    2.4f}, {0.84f, -1.5f, 2.4f}, {0.f, -1.5f, 2.4f}, {1.75f, 0.f, 1.875f},
    {1.75f, -0.98f, 1.875f}, {0.98f, -1.75f, 1.875f}, {0.f, -1.75f,
    1.875f}, {2.f, 0.f, 1.35f}, {2.f, -1.12f, 1.35f}, {1.12f, -2.f, 1.35f},
    {0.f, -2.f, 1.35f}, {2.f, 0.f, 0.9f}, {2.f, -1.12f, 0.9f}, {1.12f, -2.f,
    0.9f}, {0.f, -2.f, 0.9f}, {-2.f, 0.f, 0.9f}, {2.f, 0.f, 0.45f}, {2.f, -1.12f,
    0.45f}, {1.12f, -2.f, 0.45f}, {0.f, -2.f, 0.45f}, {1.5f, 0.f, 0.225f},
    {1.5f, -0.84f, 0.225f}, {0.84f, -1.5f, 0.225f}, {0.f, -1.5f, 0.225f},
    {1.5f, 0.f, 0.15f}, {1.5f, -0.84f, 0.15f}, {0.84f, -1.5f, 0.15f}, {0.f,
    -1.5f, 0.15f}, {-1.6f, 0.f, 2.025f}, {-1.6f, -0.3f, 2.025f}, {-1.5f,
    -0.3f, 2.25f}, {-1.5f, 0.f, 2.25f}, {-2.3f, 0.f, 2.025f}, {-2.3f, -0.3f,
    2.025f}, {-2.5f, -0.3f, 2.25f}, {-2.5f, 0.f, 2.25f}, {-2.7f, 0.f,
    2.025f}, {-2.7f, -0.3f, 2.025f}, {-3.f, -0.3f, 2.25f}, {-3.f, 0.f,
    2.25f}, {-2.7f, 0.f, 1.8f}, {-2.7f, -0.3f, 1.8f}, {-3.f, -0.3f, 1.8f},
    {-3.f, 0.f, 1.8f}, {-2.7f, 0.f, 1.575f}, {-2.7f, -0.3f, 1.575f}, {-3.f,
    -0.3f, 1.35f}, {-3.f, 0.f, 1.35f}, {-2.5f, 0.f, 1.125f}, {-2.5f, -0.3f,
    1.125f}, {-2.65f, -0.3f, 0.9375f}, {-2.65f, 0.f, 0.9375f}, {-2.f,
    -0.3f, 0.9f}, {-1.9f, -0.3f, 0.6f}, {-1.9f, 0.f, 0.6f}, {1.7f, 0.f,
    1.425f}, {1.7f, -0.66f, 1.425f}, {1.7f, -0.66f, 0.6f}, {1.7f, 0.f,
    0.6f}, {2.6f, 0.f, 1.425f}, {2.6f, -0.66f, 1.425f}, {3.1f, -0.66f,
    0.825f}, {3.1f, 0.f, 0.825f}, {2.3f, 0.f, 2.1f}, {2.3f, -0.25f, 2.1f},
    {2.4f, -0.25f, 2.025f}, {2.4f, 0.f, 2.025f}, {2.7f, 0.f, 2.4f}, {2.7f,
    -0.25f, 2.4f}, {3.3f, -0.25f, 2.4f}, {3.3f, 0.f, 2.4f}, {2.8f, 0.f,
    2.475f}, {2.8f, -0.25f, 2.475f}, {3.525f, -0.25f, 2.49375f},
    {3.525f, 0.f, 2.49375f}, {2.9f, 0.f, 2.475f}, {2.9f, -0.15f, 2.475f},
    {3.45f, -0.15f, 2.5125f}, {3.45f, 0.f, 2.5125f}, {2.8f, 0.f, 2.4f},
    {2.8f, -0.15f, 2.4f}, {3.2f, -0.15f, 2.4f}, {3.2f, 0.f, 2.4f}, {0.f, 0.f,
    3.15f}, {0.8f, 0.f, 3.15f}, {0.8f, -0.45f, 3.15f}, {0.45f, -0.8f,
    3.15f}, {0.f, -0.8f, 3.15f}, {0.f, 0.f, 2.85f}, {1.4f, 0.f, 2.4f}, {1.4f,
    -0.784f, 2.4f}, {0.784f, -1.4f, 2.4f}, {0.f, -1.4f, 2.4f}, {0.4f, 0.f,
    2.55f}, {0.4f, -0.224f, 2.55f}, {0.224f, -0.4f, 2.55f}, {0.f, -0.4f,
    2.55f}, {1.3f, 0.f, 2.55f}, {1.3f, -0.728f, 2.55f}, {0.728f, -1.3f,
    2.55f}, {0.f, -1.3f, 2.55f}, {1.3f, 0.f, 2.4f}, {1.3f, -0.728f, 2.4f},
    {0.728f, -1.3f, 2.4f}, {0.f, -1.3f, 2.4f}, {0.f, 0.f, 0.f}, {1.425f,
    -0.798f, 0.f}, {1.5f, 0.f, 0.075f}, {1.425f, 0.f, 0.f}, {0.798f, -1.425f,
    0.f}, {0.f, -1.5f, 0.075f}, {0.f, -1.425f, 0.f}, {1.5f, -0.84f, 0.075f},
    {0.84f, -1.5f, 0.075f}
};

/* *INDENT-ON* */


// Bernstein basis functions for cubic Bezier
static inline float bernstein(int i, float t)
{
    float mt = 1.0f - t;
    switch (i)
    {
        case 0: return mt * mt * mt;
        case 1: return 3.0f * t * mt * mt;
        case 2: return 3.0f * t * t * mt;
        case 3: return t * t * t;
    }
    return 0.0f;
}

// Derivative of Bernstein basis functions for cubic Bezier
static inline float dbernstein(int i, float t)
{
    float mt = 1.0f - t;
    switch (i)
    {
        case 0: return -3.0f * mt * mt;
        case 1: return 3.0f * (mt * mt - 2.0f * t * mt);
        case 2: return 3.0f * (2.0f * t * mt - t * t);
        case 3: return 3.0f * t * t;
    }
    return 0.0f;
}

// Evaluate a bicubic Bezier patch at (u,v) and compute partial derivatives
static void evaluatePatch(const float patch[4][4][3], float u, float v,
                          osg::Vec3f& pos, osg::Vec3f& du, osg::Vec3f& dv)
{
    pos.set(0.0f, 0.0f, 0.0f);
    du.set(0.0f, 0.0f, 0.0f);
    dv.set(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < 4; ++i)
    {
        float bi  = bernstein(i, u);
        float dbi = dbernstein(i, u);
        for (int j = 0; j < 4; ++j)
        {
            float bj  = bernstein(j, v);
            float dbj = dbernstein(j, v);

            osg::Vec3f cp(patch[i][j][0], patch[i][j][1], patch[i][j][2]);

            pos += cp * (bi * bj);
            du  += cp * (dbi * bj);
            dv  += cp * (bi * dbj);
        }
    }
}

// Tessellate a single Bezier patch into the given arrays.
// baseVertex is the starting index for this patch's vertices in the global arrays.
static void tessellatePatch(const float patch[4][4][3], int grid,
                            osg::Vec3Array* vertices,
                            osg::Vec3Array* normals,
                            osg::Vec2Array* texcoords,
                            osg::DrawElementsUShort* indices,
                            unsigned short baseVertex)
{
    int vertsPerSide = grid + 1;

    // Generate vertices, normals, and texcoords for this patch
    for (int i = 0; i <= grid; ++i)
    {
        float u = (float)i / (float)grid;
        for (int j = 0; j <= grid; ++j)
        {
            float v = (float)j / (float)grid;

            osg::Vec3f pos, du, dv;
            evaluatePatch(patch, u, v, pos, du, dv);

            osg::Vec3f normal = dv ^ du;
            float len = normal.length();
            if (len > 1.0e-6f)
                normal /= len;
            else
                normal.set(0.0f, 0.0f, 1.0f);

            vertices->push_back(pos);
            normals->push_back(normal);
            texcoords->push_back(osg::Vec2f(u, v));
        }
    }

    // Generate triangle indices for this patch
    for (int i = 0; i < grid; ++i)
    {
        for (int j = 0; j < grid; ++j)
        {
            unsigned short v00 = baseVertex + i * vertsPerSide + j;
            unsigned short v01 = baseVertex + i * vertsPerSide + (j + 1);
            unsigned short v10 = baseVertex + (i + 1) * vertsPerSide + j;
            unsigned short v11 = baseVertex + (i + 1) * vertsPerSide + (j + 1);

            // Two triangles per quad
            indices->push_back(v00);
            indices->push_back(v10);
            indices->push_back(v01);

            indices->push_back(v01);
            indices->push_back(v10);
            indices->push_back(v11);
        }
    }
}


osg::Geometry* createTeapotGeometry(int grid = 14)
{
    osg::Vec3Array* vertices  = new osg::Vec3Array;
    osg::Vec3Array* normals   = new osg::Vec3Array;
    osg::Vec2Array* texcoords = new osg::Vec2Array;
    osg::DrawElementsUShort* indices = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES);

    // Build all the reflected/mirrored patches exactly as the original code did.
    // Patches 0-5 (rim, body, lid, bottom) are reflected in both x and y (4 variants).
    // Patches 6-9 (handle, spout) are reflected across y axis only (2 variants).
    for (int i = 0; i < 10; ++i)
    {
        float p[4][4][3], q[4][4][3], r[4][4][3], s[4][4][3];

        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                for (int l = 0; l < 3; ++l)
                {
                    p[j][k][l] = cpdata[patchdata[i][j * 4 + k]][l];
                    q[j][k][l] = cpdata[patchdata[i][j * 4 + (3 - k)]][l];
                    if (l == 1)
                        q[j][k][l] *= -1.0f;
                    if (i < 6)
                    {
                        r[j][k][l] = cpdata[patchdata[i][j * 4 + (3 - k)]][l];
                        if (l == 0)
                            r[j][k][l] *= -1.0f;
                        s[j][k][l] = cpdata[patchdata[i][j * 4 + k]][l];
                        if (l == 0)
                            s[j][k][l] *= -1.0f;
                        if (l == 1)
                            s[j][k][l] *= -1.0f;
                    }
                }
            }
        }

        // Tessellate patch p
        unsigned short base = (unsigned short)vertices->size();
        tessellatePatch(p, grid, vertices, normals, texcoords, indices, base);

        // Tessellate patch q (y-reflected)
        base = (unsigned short)vertices->size();
        tessellatePatch(q, grid, vertices, normals, texcoords, indices, base);

        if (i < 6)
        {
            // Tessellate patch r (x-reflected)
            base = (unsigned short)vertices->size();
            tessellatePatch(r, grid, vertices, normals, texcoords, indices, base);

            // Tessellate patch s (x and y reflected)
            base = (unsigned short)vertices->size();
            tessellatePatch(s, grid, vertices, normals, texcoords, indices, base);
        }
    }

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray(vertices);
    geom->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
    geom->setTexCoordArray(0, texcoords);
    geom->addPrimitiveSet(indices);

    return geom;
}


osg::Geode* createTeapot()
{
    osg::Geode* geode = new osg::Geode();

    // add the teapot to the geode.
    geode->addDrawable( createTeapotGeometry() );

    // add a reflection map to the teapot.
    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile("Images/reflect.rgb");
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);

        osg::StateSet* stateset = new osg::StateSet;
        stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);

        // Activate sphere-map texture coordinate generation in shader pipeline.
        // Replaces osg::TexGen(SPHERE_MAP) from the fixed-function version.
        stateset->setTextureMode(0, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);

        geode->setStateSet(stateset);
    }

    return geode;
}

int main(int , char **)
{
#if 1

    // create viewer on heap as a test, this looks to be causing problems
    // on init on some platforms, and seg fault on exit when multi-threading on linux.
    // Normal stack based version below works fine though...

    // construct the viewer.
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
