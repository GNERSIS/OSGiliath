/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
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

/**
 * \brief    Shader generator framework.
 * \author   Maciej Krol
 */

#include <osgUtil/ShaderGen>
#include <osg/Geode>
#include <osg/Geometry> // for ShaderGenVisitor::update
#include <osg/TexGen>
#include <sstream>

#include "shaders/shadergen_vert.cpp"
#include "shaders/shadergen_frag.cpp"

using namespace osgUtil;

namespace osgUtil
{

ShaderGenVisitor::ShaderGenVisitor():
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
}

void ShaderGenVisitor::assignUberProgram(osg::StateSet *stateSet)
{
    if (stateSet)
    {
        osg::ref_ptr<osg::Program> uberProgram = new osg::Program;
        uberProgram->addShader(new osg::Shader(osg::Shader::VERTEX, shadergen_vert));
        uberProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, shadergen_frag));

        stateSet->setAttribute(uberProgram.get());
        stateSet->addUniform(new osg::Uniform("diffuseMap", 0));

        remapStateSet(stateSet);
    }
}

void ShaderGenVisitor::apply(osg::Node &node)
{
    osg::StateSet* stateSet = node.getStateSet();
    if (stateSet) remapStateSet(stateSet);

    traverse(node);
}


void ShaderGenVisitor::remapStateSet(osg::StateSet* stateSet)
{
    if (!stateSet) return;

    // remove any modes that won't be appropriate when using shaders, and remap them to the appropriate Uniform/Define combination

    if (!stateSet->getTextureModeList().empty())
    {
        osg::StateSet::ModeList& textureModes = stateSet->getTextureModeList()[0];

        if (textureModes.count(GL_TEXTURE_2D)>0)
        {
            osg::StateAttribute::GLModeValue textureMode = textureModes[GL_TEXTURE_2D];
            stateSet->removeTextureMode(0, GL_TEXTURE_2D);
            stateSet->setDefine("OSG_TEXTURE_2D", textureMode);
        }

        // Check for TexGen sphere-map mode
        if (textureModes.count(GL_TEXTURE_GEN_S)>0)
        {
            osg::StateAttribute::GLModeValue texgenMode = textureModes[GL_TEXTURE_GEN_S];
            stateSet->removeTextureMode(0, GL_TEXTURE_GEN_S);
            stateSet->setDefine("OSG_TEXGEN_SPHERE_MAP", texgenMode);
        }
    }
}

} // namespace osgUtil