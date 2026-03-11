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

#include "LightPointDrawable.h"

#include <osg/Point>

#ifndef GL_PROGRAM_POINT_SIZE
#  define GL_PROGRAM_POINT_SIZE               0x8642
#endif

using namespace osgSim;

LightPointDrawable::LightPointDrawable():
    osg::Drawable(),
    _endian(osg::getCpuByteOrder()),
    _simulationTime(0.0),
    _simulationTimeInterval(0.0)
{


    _depthOff = new osg::Depth;
    _depthOff->setWriteMask(false);

    _depthOn = new osg::Depth;
    _depthOn->setWriteMask(true);

    _blendOne = new osg::BlendFunc;
    _blendOne->setFunction(osg::BlendFunc::SRC_ALPHA,osg::BlendFunc::ONE);

    _blendOneMinusSrcAlpha = new osg::BlendFunc;
    _blendOneMinusSrcAlpha->setFunction(osg::BlendFunc::SRC_ALPHA,osg::BlendFunc::ONE_MINUS_SRC_ALPHA);

    _colorMaskOff = new osg::ColorMask;
    _colorMaskOff->setMask(false,false,false,false);

}

LightPointDrawable::LightPointDrawable(const LightPointDrawable& lpd,const osg::CopyOp& copyop):
    osg::Drawable(lpd,copyop),
    _endian(lpd._endian),
    _simulationTime(lpd._simulationTime),
    _simulationTimeInterval(lpd._simulationTimeInterval),
    _sizedOpaqueLightPointList(lpd._sizedOpaqueLightPointList),
    _sizedAdditiveLightPointList(lpd._sizedAdditiveLightPointList),
    _sizedBlendedLightPointList(lpd._sizedBlendedLightPointList)
{
}

LightPointDrawable::~LightPointDrawable()
{
}

void LightPointDrawable::reset()
{
    SizedLightPointList::iterator itr;
    for(itr=_sizedOpaqueLightPointList.begin();
        itr!=_sizedOpaqueLightPointList.end();
        ++itr)
    {
        if (!itr->empty())
            itr->erase(itr->begin(),itr->end());
    }

    for(itr=_sizedAdditiveLightPointList.begin();
        itr!=_sizedAdditiveLightPointList.end();
        ++itr)
    {
        if (!itr->empty())
            itr->erase(itr->begin(),itr->end());
    }

    for(itr=_sizedBlendedLightPointList.begin();
        itr!=_sizedBlendedLightPointList.end();
        ++itr)
    {
        if (!itr->empty())
            itr->erase(itr->begin(),itr->end());
    }
}


void LightPointDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
    osg::State& state = *renderInfo.getState();

    state.applyMode(GL_BLEND,true);

    state.applyAttribute(_depthOn.get());

    state.applyAttribute(_blendOneMinusSrcAlpha.get());

    const GLuint vertexAttribLocation = state.getVertexAlias()._location;
    const GLuint colorAttribLocation = state.getColorAlias()._location;
    osg::GLExtensions* ext = state.get<osg::GLExtensions>();

    // Core Profile requires VAO and VBO — no client-side vertex arrays.
    // Create a temporary VAO and VBO for streaming light point data each frame.
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

                // Upload interleaved C4UB_V3F data to VBO
                // Each element: 4 bytes color (GLubyte[4]) + 12 bytes position (GLfloat[3]) = 16 bytes stride
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

    // Cleanup temporary VBO and VAO
    ext->glBindBuffer(GL_ARRAY_BUFFER, 0);
    ext->glDeleteBuffers(1, &vbo);
    ext->glDeleteVertexArrays(1, &vao);

    // Restore previous VAO
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

osg::BoundingBox LightPointDrawable::computeBoundingBox() const
{
    osg::BoundingBox bbox;

    SizedLightPointList::const_iterator sitr;
    for(sitr=_sizedOpaqueLightPointList.begin();
        sitr!=_sizedOpaqueLightPointList.end();
        ++sitr)
    {
        const LightPointList& lpl = *sitr;
        for(LightPointList::const_iterator litr=lpl.begin();
            litr!=lpl.end();
            ++litr)
        {
            bbox.expandBy(litr->second);
        }
    }
    for(sitr=_sizedAdditiveLightPointList.begin();
        sitr!=_sizedAdditiveLightPointList.end();
        ++sitr)
    {
        const LightPointList& lpl = *sitr;
        for(LightPointList::const_iterator litr=lpl.begin();
            litr!=lpl.end();
            ++litr)
        {
            bbox.expandBy(litr->second);
        }
    }
    for(sitr=_sizedBlendedLightPointList.begin();
        sitr!=_sizedBlendedLightPointList.end();
        ++sitr)
    {
        const LightPointList& lpl = *sitr;
        for(LightPointList::const_iterator litr=lpl.begin();
            litr!=lpl.end();
            ++litr)
        {
            bbox.expandBy(litr->second);
        }
    }

    return bbox;
}
