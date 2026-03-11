/* OpenSceneGraph example, osgscribe.
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
#include <osg/Group>
#include <osg/Notify>
#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/PolygonMode>
#include <osg/Program>
#include <osg/Shader>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>

#include <osgUtil/Optimizer>

// Vertex shader for solid pass: passes color and normal for lighting
static const char* scribeVertexShader = R"glsl(
#version 460 core
layout(location = 0) in vec4 osg_Vertex;
layout(location = 2) in vec3 osg_Normal;
layout(location = 3) in vec4 osg_Color;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat3 osg_NormalMatrix;
out vec3 vNormal;
out vec4 vColor;
void main() {
    vNormal = normalize(osg_NormalMatrix * osg_Normal);
    vColor = osg_Color;
    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
}
)glsl";

// Fragment shader for solid pass: applies simple directional lighting
static const char* scribeFragmentShader = R"glsl(
#version 460 core
in vec3 vNormal;
in vec4 vColor;
out vec4 fragColor;
void main() {
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));
    float diff = max(dot(normalize(vNormal), lightDir), 0.0) * 0.7 + 0.3;
    fragColor = vec4(vColor.rgb * diff, vColor.a);
}
)glsl";

// Vertex shader for wireframe overlay: simple pass-through
static const char* wireVertexShader = R"glsl(
#version 460 core
layout(location = 0) in vec4 osg_Vertex;
uniform mat4 osg_ModelViewProjectionMatrix;
void main() {
    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
}
)glsl";

// Fragment shader for wireframe overlay: flat dark color
static const char* wireFragmentShader = R"glsl(
#version 460 core
out vec4 fragColor;
void main() {
    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
)glsl";

static void enableVBO(osg::Node* node)
{
    if (!node) return;
    osg::Geode* geode = node->asGeode();
    if (geode)
    {
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i)
        {
            osg::Geometry* geom = geode->getDrawable(i)->asGeometry();
            if (geom)
            {
                geom->setUseVertexBufferObjects(true);
            }
        }
    }
    osg::Group* group = node->asGroup();
    if (group)
    {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i)
            enableVBO(group->getChild(i));
    }
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readRefNodeFile("cow.osgt");

    if (!loadedModel)
    {
        osg::notify(osg::NOTICE)<<"Please specify a model filename on the command line."<<std::endl;
        return 1;
    }

    // Enable VBOs on all geometry for core profile compatibility
    enableVBO(loadedModel.get());

    // to do scribe mode we create a top most group to contain the
    // original model, and then a second group contains the same model
    // but overrides various state attributes, so that the second instance
    // is rendered as wireframe.

    osg::ref_ptr<osg::Group> rootnode = new osg::Group;

    osg::ref_ptr<osg::Group> decorator = new osg::Group;

    rootnode->addChild(loadedModel);


    rootnode->addChild(decorator);

    decorator->addChild(loadedModel);

    // Add shader program to the root for the solid pass
    {
        osg::StateSet* ss = rootnode->getOrCreateStateSet();
        osg::Program* prog = new osg::Program;
        prog->addShader(new osg::Shader(osg::Shader::VERTEX, scribeVertexShader));
        prog->addShader(new osg::Shader(osg::Shader::FRAGMENT, scribeFragmentShader));
        ss->setAttributeAndModes(prog, osg::StateAttribute::ON);
    }

    // set up the state so that the underlying color is not seen through
    // and that the drawing mode is changed to wireframe, and a polygon offset
    // is added to ensure that we see the wireframe itself, and turn off
    // so texturing too.
    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
    osg::ref_ptr<osg::PolygonOffset> polyoffset = new osg::PolygonOffset;
    polyoffset->setFactor(-1.0f);
    polyoffset->setUnits(-1.0f);
    osg::ref_ptr<osg::PolygonMode> polymode = new osg::PolygonMode;
    polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
    stateset->setAttributeAndModes(polyoffset,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
    stateset->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

    // Wireframe shader override for the decorator
    osg::Program* wireProg = new osg::Program;
    wireProg->addShader(new osg::Shader(osg::Shader::VERTEX, wireVertexShader));
    wireProg->addShader(new osg::Shader(osg::Shader::FRAGMENT, wireFragmentShader));
    stateset->setAttributeAndModes(wireProg, osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

    decorator->setStateSet(stateset);


    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode );

    return viewer.run();
}
