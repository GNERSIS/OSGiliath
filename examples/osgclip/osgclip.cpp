/* OpenSceneGraph example, osgclip.
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

#include <osg/MatrixTransform>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Material>
#include <osg/PolygonOffset>
#include <osg/PolygonMode>
#include <osg/BoundingBox>
#include <osg/BoundingSphere>
#include <osg/Plane>
#include <osg/AnimationPath>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgViewer/Viewer>

#include <osgUtil/Optimizer>
#include <osgText/Text>

#include <vector>

#ifndef GL_CLIP_PLANE0
    #define GL_CLIP_PLANE0 0x3000
#endif

// Vertex shader: transforms vertices and computes gl_ClipDistance for 6 clip planes
static const char* clipVertexShader = R"glsl(
#version 460 core
layout(location = 0) in vec4 osg_Vertex;
layout(location = 2) in vec3 osg_Normal;
uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat3 osg_NormalMatrix;
uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane2;
uniform vec4 clipPlane3;
uniform vec4 clipPlane4;
uniform vec4 clipPlane5;
uniform int numClipPlanes;

out vec3 vNormal;
out vec3 vFragPos;

void main() {
    vec4 eyePos = osg_ModelViewMatrix * osg_Vertex;
    vFragPos = eyePos.xyz;
    vNormal = normalize(osg_NormalMatrix * osg_Normal);

    if (numClipPlanes > 0) gl_ClipDistance[0] = dot(eyePos, clipPlane0);
    if (numClipPlanes > 1) gl_ClipDistance[1] = dot(eyePos, clipPlane1);
    if (numClipPlanes > 2) gl_ClipDistance[2] = dot(eyePos, clipPlane2);
    if (numClipPlanes > 3) gl_ClipDistance[3] = dot(eyePos, clipPlane3);
    if (numClipPlanes > 4) gl_ClipDistance[4] = dot(eyePos, clipPlane4);
    if (numClipPlanes > 5) gl_ClipDistance[5] = dot(eyePos, clipPlane5);

    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
}
)glsl";

// Fragment shader: basic directional lighting
static const char* clipFragmentShader = R"glsl(
#version 460 core
in vec3 vNormal;
in vec3 vFragPos;
out vec4 fragColor;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(vNormal), lightDir), 0.0) * 0.7 + 0.3;
    fragColor = vec4(diff, diff, diff, 1.0);
}
)glsl";

// Vertex shader for wireframe pass (no clip distances)
static const char* wireVertexShader = R"glsl(
#version 460 core
layout(location = 0) in vec4 osg_Vertex;
layout(location = 2) in vec3 osg_Normal;
uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat3 osg_NormalMatrix;
out vec3 vNormal;
out vec3 vFragPos;

void main() {
    vec4 eyePos = osg_ModelViewMatrix * osg_Vertex;
    vFragPos = eyePos.xyz;
    vNormal = normalize(osg_NormalMatrix * osg_Normal);
    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
}
)glsl";

// Fragment shader for wireframe pass (flat dark color)
static const char* wireFragmentShader = R"glsl(
#version 460 core
in vec3 vNormal;
in vec3 vFragPos;
out vec4 fragColor;

void main() {
    fragColor = vec4(0.3, 0.3, 0.3, 1.0);
}
)glsl";

// Simple container for clip plane equations (replaces the removed osg::ClipNode).
// Stores clip planes as Vec4d values for use with shader-based gl_ClipDistance.
struct ClipBox : public osg::Referenced
{
    typedef std::vector<osg::Vec4d> PlaneList;
    PlaneList _planes;

    // Creates six clip planes corresponding to the given BoundingBox.
    void createClipBox(const osg::BoundingBox& bb)
    {
        _planes.clear();
        _planes.push_back(osg::Vec4d( 1.0, 0.0, 0.0, -bb.xMin()));
        _planes.push_back(osg::Vec4d(-1.0, 0.0, 0.0,  bb.xMax()));
        _planes.push_back(osg::Vec4d( 0.0, 1.0, 0.0, -bb.yMin()));
        _planes.push_back(osg::Vec4d( 0.0,-1.0, 0.0,  bb.yMax()));
        _planes.push_back(osg::Vec4d( 0.0, 0.0, 1.0, -bb.zMin()));
        _planes.push_back(osg::Vec4d( 0.0, 0.0,-1.0,  bb.zMax()));
    }

    unsigned int getNumPlanes() const { return _planes.size(); }
    const osg::Vec4d& getPlane(unsigned int i) const { return _planes[i]; }
};

// Callback that reads clip plane equations from a ClipBox and updates uniforms.
// The clip planes are in model-local coordinates of the MatrixTransform.
// We transform them so that the vertex shader (which multiplies by ModelView
// of the clipped subgraph) produces the correct clip distance.
class ClipPlaneUniformCallback : public osg::NodeCallback
{
public:
    ClipPlaneUniformCallback(ClipBox* cb, osg::MatrixTransform* mt)
        : _clipBox(cb), _transform(mt) {}

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        osg::StateSet* ss = node->getStateSet();
        if (ss && _clipBox.valid())
        {
            int numPlanes = static_cast<int>(_clipBox->getNumPlanes());

            // Get the transform matrix that positions the clip planes
            osg::Matrix mat = _transform.valid() ? _transform->getMatrix() : osg::Matrix::identity();
            osg::Matrix invMat = osg::Matrix::inverse(mat);

            const char* uniformNames[] = {
                "clipPlane0", "clipPlane1", "clipPlane2",
                "clipPlane3", "clipPlane4", "clipPlane5"
            };

            for (int i = 0; i < numPlanes && i < 6; ++i)
            {
                osg::Vec4d planeEq = _clipBox->getPlane(i);
                osg::Plane plane(planeEq);
                plane.transformProvidingInverse(invMat);

                osg::Uniform* u = ss->getUniform(uniformNames[i]);
                if (u)
                    u->set(osg::Vec4f(plane[0], plane[1], plane[2], plane[3]));
            }

            osg::Uniform* numU = ss->getUniform("numClipPlanes");
            if (numU)
                numU->set(numPlanes);
        }
        traverse(node, nv);
    }

private:
    osg::ref_ptr<ClipBox> _clipBox;
    osg::observer_ptr<osg::MatrixTransform> _transform;
};

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

osg::ref_ptr<osg::Node> decorate_with_clip_node(const osg::ref_ptr<osg::Node>& subgraph)
{
    osg::ref_ptr<osg::Group> rootnode = new osg::Group;

    // Create wireframe view of the model so the user can see
    // what parts are being culled away.
    osg::StateSet* wireStateset = new osg::StateSet;
    osg::PolygonMode* polymode = new osg::PolygonMode;
    polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
    wireStateset->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

    // Add wireframe shader
    osg::Program* wireProg = new osg::Program;
    wireProg->addShader(new osg::Shader(osg::Shader::VERTEX, wireVertexShader));
    wireProg->addShader(new osg::Shader(osg::Shader::FRAGMENT, wireFragmentShader));
    wireStateset->setAttributeAndModes(wireProg, osg::StateAttribute::ON);

    osg::Group* wireframe_subgraph = new osg::Group;
    wireframe_subgraph->setStateSet(wireStateset);
    wireframe_subgraph->addChild(subgraph);
    rootnode->addChild(wireframe_subgraph);

    // Animated MatrixTransform for rotating clip box
    osg::MatrixTransform* transform= new osg::MatrixTransform;

    osg::NodeCallback* nc = new osg::AnimationPathCallback(subgraph->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    transform->setUpdateCallback(nc);

    // Create clip box planes from bounding sphere
    osg::ref_ptr<ClipBox> clipBox = new ClipBox;
    osg::BoundingSphere bs = subgraph->getBound();
    bs.radius()*= 0.4f;

    osg::BoundingBox bb;
    bb.expandBy(bs);

    clipBox->createClipBox(bb);

    rootnode->addChild(transform);

    // Create clipped subgraph with shader-based clipping
    osg::Group* clipped_subgraph = new osg::Group;

    // Set up shader program for clipping
    osg::Program* clipProg = new osg::Program;
    clipProg->addShader(new osg::Shader(osg::Shader::VERTEX, clipVertexShader));
    clipProg->addShader(new osg::Shader(osg::Shader::FRAGMENT, clipFragmentShader));

    osg::StateSet* clipStateset = new osg::StateSet;
    clipStateset->setAttributeAndModes(clipProg, osg::StateAttribute::ON);

    // Add clip plane uniforms
    clipStateset->addUniform(new osg::Uniform("clipPlane0", osg::Vec4f(0,0,0,0)));
    clipStateset->addUniform(new osg::Uniform("clipPlane1", osg::Vec4f(0,0,0,0)));
    clipStateset->addUniform(new osg::Uniform("clipPlane2", osg::Vec4f(0,0,0,0)));
    clipStateset->addUniform(new osg::Uniform("clipPlane3", osg::Vec4f(0,0,0,0)));
    clipStateset->addUniform(new osg::Uniform("clipPlane4", osg::Vec4f(0,0,0,0)));
    clipStateset->addUniform(new osg::Uniform("clipPlane5", osg::Vec4f(0,0,0,0)));
    clipStateset->addUniform(new osg::Uniform("numClipPlanes", 0));

    // Enable GL_CLIP_DISTANCE for all 6 planes
    // GL_CLIP_DISTANCE0 = 0x3000 (same as GL_CLIP_PLANE0)
    for (int i = 0; i < 6; ++i)
        clipStateset->setMode(GL_CLIP_PLANE0 + i, osg::StateAttribute::ON);

    clipped_subgraph->setStateSet(clipStateset);
    clipped_subgraph->addChild(subgraph);

    // Add callback to update clip plane uniforms from the animated transform
    clipped_subgraph->setUpdateCallback(new ClipPlaneUniformCallback(clipBox, transform));

    rootnode->addChild(clipped_subgraph);

    return rootnode;
}


osg::ref_ptr<osg::Node> simple_decorate_with_clip_node(const osg::ref_ptr<osg::Node>& subgraph)
{
    osg::ref_ptr<osg::Group> rootnode = new osg::Group;

    // Animated MatrixTransform
    osg::MatrixTransform* transform= new osg::MatrixTransform;

    osg::NodeCallback* nc = new osg::AnimationPathCallback(subgraph->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    transform->setUpdateCallback(nc);

    // Create clip box planes from bounding sphere
    osg::ref_ptr<ClipBox> clipBox = new ClipBox;
    osg::BoundingSphere bs = subgraph->getBound();
    bs.radius()*= 0.4f;

    osg::BoundingBox bb;
    bb.expandBy(bs);

    clipBox->createClipBox(bb);

    rootnode->addChild(transform);

    // Create clipped subgraph with shader-based clipping
    osg::Group* clipped_subgraph = new osg::Group;

    // Set up shader program for clipping
    osg::Program* clipProg = new osg::Program;
    clipProg->addShader(new osg::Shader(osg::Shader::VERTEX, clipVertexShader));
    clipProg->addShader(new osg::Shader(osg::Shader::FRAGMENT, clipFragmentShader));

    osg::StateSet* clipStateset = new osg::StateSet;
    clipStateset->setAttributeAndModes(clipProg, osg::StateAttribute::ON);

    // Add clip plane uniforms
    clipStateset->addUniform(new osg::Uniform("clipPlane0", osg::Vec4f(0,0,0,0)));
    clipStateset->addUniform(new osg::Uniform("clipPlane1", osg::Vec4f(0,0,0,0)));
    clipStateset->addUniform(new osg::Uniform("clipPlane2", osg::Vec4f(0,0,0,0)));
    clipStateset->addUniform(new osg::Uniform("clipPlane3", osg::Vec4f(0,0,0,0)));
    clipStateset->addUniform(new osg::Uniform("clipPlane4", osg::Vec4f(0,0,0,0)));
    clipStateset->addUniform(new osg::Uniform("clipPlane5", osg::Vec4f(0,0,0,0)));
    clipStateset->addUniform(new osg::Uniform("numClipPlanes", 0));

    // Enable GL_CLIP_DISTANCE for all 6 planes
    for (int i = 0; i < 6; ++i)
        clipStateset->setMode(GL_CLIP_PLANE0 + i, osg::StateAttribute::ON);

    clipped_subgraph->setStateSet(clipStateset);
    clipped_subgraph->addChild(subgraph);

    // Add callback to update clip plane uniforms from the animated transform
    clipped_subgraph->setUpdateCallback(new ClipPlaneUniformCallback(clipBox, transform));

    rootnode->addChild(clipped_subgraph);

    return rootnode;
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Group> scene = new osg::Group;

    std::string textString;
    while(arguments.read("--text", textString))
    {
        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setFont("fonts/times.ttf");
        text->setAxisAlignment(osgText::Text::XZ_PLANE);
        text->setDrawMode(osgText::Text::TEXT|osgText::Text::ALIGNMENT|osgText::Text::BOUNDINGBOX|osgText::Text::FILLEDBOUNDINGBOX);
        text->setText(textString);
        scene->addChild(text);
    }


    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
    if (loadedModel.valid())
    {
        scene->addChild(loadedModel.get());
    }


    // if not loaded assume no arguments passed in, try use default mode instead.
    if (scene->getNumChildren()==0)
    {
        loadedModel = osgDB::readRefNodeFile("cow.osgt");
        scene->addChild(loadedModel.get());
    }


    if (scene->getNumChildren()==0)
    {
        osg::notify(osg::NOTICE)<<"Please specify a filename on the command line"<<std::endl;
        return 1;
    }

    // Enable VBOs on all geometry for core profile compatibility
    enableVBO(scene.get());

    // decorate the scenegraph with shader-based clipping.
    osg::ref_ptr<osg::Node> rootnode;


    if (arguments.read("--simple"))
    {
        rootnode = simple_decorate_with_clip_node(scene);
    }
    else
    {
        rootnode = decorate_with_clip_node(scene);
    }

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);

    // set the scene to render
    viewer.setSceneData(rootnode);

    return viewer.run();
}
