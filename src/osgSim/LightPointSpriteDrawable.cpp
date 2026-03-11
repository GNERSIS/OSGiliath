/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include "LightPointSpriteDrawable.h"

#include <osg/Point>

#ifndef GL_PROGRAM_POINT_SIZE
#  define GL_PROGRAM_POINT_SIZE               0x8642
#endif

using namespace osgSim;

LightPointSpriteDrawable::LightPointSpriteDrawable():
    osgSim::LightPointDrawable()
{
}

LightPointSpriteDrawable::LightPointSpriteDrawable(const LightPointSpriteDrawable& lpsd,const osg::CopyOp& copyop):
    osgSim::LightPointDrawable(lpsd,copyop)
{
}

void LightPointSpriteDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
    osg::State& state = *renderInfo.getState();

    state.applyMode(GL_BLEND,true);

    state.applyAttribute(_depthOn.get());

    state.applyAttribute(_blendOneMinusSrcAlpha.get());

    const GLuint vertexAttribLocation = state.getVertexAlias()._location;
    const GLuint colorAttribLocation = state.getColorAlias()._location;
    osg::GLExtensions* ext = state.get<osg::GLExtensions>();

    // Core Profile requires VAO and VBO — no client-side vertex arrays.
    GLuint savedVAO = state.getCurrentVertexArrayObject();

    GLuint vao = 0;
    ext->glGenVertexArrays(1, &vao);
    state.bindVertexArrayObject(vao);

    GLuint vbo = 0;
    ext->glGenBuffers(1, &vbo);
    ext->glBindBuffer(GL_ARRAY_BUFFER, vbo);

    auto drawLightPoints = [&](const SizedLightPointList& list)
    {
        unsigned int pointsize = 1;
        for (auto sitr = list.begin(); sitr != list.end(); ++sitr, ++pointsize)
        {
            const LightPointList& lpl = *sitr;
            if (!lpl.empty())
            {
                glPointSize(pointsize);

                const GLsizei stride = sizeof(ColorPosition);
                const GLsizeiptr dataSize = lpl.size() * stride;
                ext->glBufferData(GL_ARRAY_BUFFER, dataSize, &lpl.front(), GL_STREAM_DRAW);

                ext->glEnableVertexAttribArray(colorAttribLocation);
                ext->glVertexAttribPointer(colorAttribLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const GLvoid*)0);

                ext->glEnableVertexAttribArray(vertexAttribLocation);
                ext->glVertexAttribPointer(vertexAttribLocation, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)4);

                glDrawArrays(GL_POINTS, 0, lpl.size());
            }
        }
    };

    // Opaque light points
    drawLightPoints(_sizedOpaqueLightPointList);

    state.applyMode(GL_BLEND,true);
    state.applyAttribute(_depthOff.get());

    // Blended light points
    state.applyAttribute(_blendOneMinusSrcAlpha.get());
    drawLightPoints(_sizedBlendedLightPointList);

    // Additive light points
    state.applyAttribute(_blendOne.get());
    drawLightPoints(_sizedAdditiveLightPointList);

    ext->glDisableVertexAttribArray(vertexAttribLocation);
    ext->glDisableVertexAttribArray(colorAttribLocation);

    ext->glBindBuffer(GL_ARRAY_BUFFER, 0);
    ext->glDeleteBuffers(1, &vbo);
    ext->glDeleteVertexArrays(1, &vao);

    if (savedVAO)
        state.bindVertexArrayObject(savedVAO);
    else
        state.unbindVertexArrayObject();

    glPointSize(1);

    state.haveAppliedAttribute(osg::StateAttribute::POINT);

    state.dirtyAllVertexArrays();
    state.disableAllVertexArrays();

    // restore the state afterwards.
    state.apply();
}


