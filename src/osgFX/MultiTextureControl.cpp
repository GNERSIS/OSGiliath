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

#include <osgFX/MultiTextureControl>

using namespace osg;
using namespace osgFX;

MultiTextureControl::MultiTextureControl():
    _useTextureWeightsUniform(true)
{
    _textureWeights = new TextureWeights;
}

MultiTextureControl::MultiTextureControl(const MultiTextureControl& copy, const osg::CopyOp& copyop):
    Group(copy,copyop),
    _textureWeights(osg::clone(copy._textureWeights.get(), osg::CopyOp::DEEP_COPY_ALL)),
    _useTextureWeightsUniform(copy._useTextureWeightsUniform)
{
    updateStateSet();
}

void MultiTextureControl::setTextureWeight(unsigned int unit, float weight)
{
    if (unit >= _textureWeights->size())
    {
        _textureWeights->resize(unit+1,0.0f);
    }
    (*_textureWeights)[unit] = weight;

    updateStateSet();
}

void MultiTextureControl::updateStateSet()
{
    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;

    if (_useTextureWeightsUniform && _textureWeights->size()>0)
    {
        osg::ref_ptr<osg::Uniform> uniform = new osg::Uniform(osg::Uniform::FLOAT, "TextureWeights", _textureWeights->size());
#if 1
        uniform->setArray(_textureWeights.get());
#else
        for(unsigned int i=0; i<_textureWeights->size(); ++i)
        {
            uniform->setElement(i, (*_textureWeights)[i]);
        }
#endif
        stateset->addUniform(uniform.get());
        stateset->setDefine("TEXTURE_WEIGHTS");
    }

    setStateSet(stateset.get());
}

