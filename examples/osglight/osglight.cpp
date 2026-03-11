/* OpenSceneGraph example, osglight.
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

#include <osgViewer/Viewer>

#include <osg/Group>
#include <osg/Node>

#include <osg/Light>
#include <osg/LightSource>
#include <osg/StateAttribute>
#include <osg/Geometry>
#include <osg/Point>

#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>

#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

#include <cmath>
#include "stdio.h"

///////////////////////////////////////////////////////////////////////////
// Inline GLSL shaders for lighting (OpenGL 4.6 Core Profile)

static const char* vertexShaderSource =
    "#version 460 core\n"
    "layout(location = 0) in vec4 osg_Vertex;\n"
    "layout(location = 2) in vec3 osg_Normal;\n"
    "layout(location = 3) in vec4 osg_Color;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "uniform mat4 osg_ModelViewMatrix;\n"
    "uniform mat3 osg_NormalMatrix;\n"
    "out vec3 vNormal;\n"
    "out vec3 vFragPos;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    vFragPos = vec3(osg_ModelViewMatrix * osg_Vertex);\n"
    "    vNormal = normalize(osg_NormalMatrix * osg_Normal);\n"
    "    vColor = osg_Color;\n"
    "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
    "}\n";

static const char* fragmentShaderSource =
    "#version 460 core\n"
    "in vec3 vNormal;\n"
    "in vec3 vFragPos;\n"
    "in vec4 vColor;\n"
    "out vec4 fragColor;\n"
    "\n"
    "struct Light {\n"
    "    vec4 position;\n"
    "    vec4 ambient;\n"
    "    vec4 diffuse;\n"
    "    vec4 specular;\n"
    "    vec3 spotDirection;\n"
    "    float spotCutoff;\n"
    "    float spotExponent;\n"
    "    float constantAttenuation;\n"
    "    float linearAttenuation;\n"
    "    float quadraticAttenuation;\n"
    "};\n"
    "\n"
    "struct Material {\n"
    "    vec4 ambient;\n"
    "    vec4 diffuse;\n"
    "    vec4 specular;\n"
    "    vec4 emission;\n"
    "    float shininess;\n"
    "};\n"
    "\n"
    "vec3 computeLight(Light light, Material mat, vec3 normal, vec3 viewDir, vec3 fragPos) {\n"
    "    vec3 lightDir;\n"
    "    float attenuation = 1.0;\n"
    "    if (light.position.w == 0.0) {\n"
    "        lightDir = normalize(light.position.xyz);\n"
    "    } else {\n"
    "        vec3 lightVec = light.position.xyz - fragPos;\n"
    "        float dist = length(lightVec);\n"
    "        lightDir = lightVec / dist;\n"
    "        attenuation = 1.0 / (light.constantAttenuation + light.linearAttenuation * dist + light.quadraticAttenuation * dist * dist);\n"
    "        if (light.spotCutoff > -0.5) {\n"
    "            float spotCos = dot(-lightDir, normalize(light.spotDirection));\n"
    "            if (spotCos < light.spotCutoff) attenuation = 0.0;\n"
    "            else attenuation *= pow(spotCos, light.spotExponent);\n"
    "        }\n"
    "    }\n"
    "    float diff = max(dot(normal, lightDir), 0.0);\n"
    "    vec3 halfDir = normalize(lightDir + viewDir);\n"
    "    float spec = (diff > 0.0) ? pow(max(dot(normal, halfDir), 0.0), mat.shininess) : 0.0;\n"
    "    return light.ambient.rgb * mat.ambient.rgb + (light.diffuse.rgb * mat.diffuse.rgb * diff + light.specular.rgb * mat.specular.rgb * spec) * attenuation;\n"
    "}\n"
    "\n"
    "uniform Light light0;\n"
    "uniform Light light1;\n"
    "uniform Material material;\n"
    "uniform vec4 sceneAmbient;\n"
    "\n"
    "void main() {\n"
    "    vec3 normal = normalize(vNormal);\n"
    "    vec3 viewDir = normalize(-vFragPos);\n"
    "    vec3 color = sceneAmbient.rgb * material.ambient.rgb + material.emission.rgb;\n"
    "    color += computeLight(light0, material, normal, viewDir, vFragPos);\n"
    "    color += computeLight(light1, material, normal, viewDir, vFragPos);\n"
    "    vec4 baseColor = (dot(vColor.rgb, vColor.rgb) > 0.0) ? vColor : vec4(1.0);\n"
    "    fragColor = vec4(color, material.diffuse.a) * baseColor;\n"
    "}\n";


// callback to make the loaded model oscillate up and down.
class ModelTransformCallback : public osg::NodeCallback
{
    public:

        ModelTransformCallback(const osg::BoundingSphere& bs)
        {
            _firstTime = 0.0;
            _period = 4.0f;
            _range = bs.radius()*0.5f;
        }

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            osg::PositionAttitudeTransform* pat = dynamic_cast<osg::PositionAttitudeTransform*>(node);
            const osg::FrameStamp* frameStamp = nv->getFrameStamp();
            if (pat && frameStamp)
            {
                if (_firstTime==0.0)
                {
                    _firstTime = frameStamp->getSimulationTime();
                }

                double phase = (frameStamp->getSimulationTime()-_firstTime)/_period;
                phase -= floor(phase);
                phase *= (2.0 * osg::PI);

                osg::Quat rotation;
                rotation.makeRotate(phase,1.0f,1.0f,1.0f);

                pat->setAttitude(rotation);

                pat->setPosition(osg::Vec3(0.0f,0.0f,sin(phase))*_range);
            }

            // must traverse the Node's subgraph
            traverse(node,nv);
        }

        double _firstTime;
        double _period;
        double _range;

};


// Callback that updates light uniforms each frame.
// Transforms light positions to eye space using the camera's view matrix.
class LightUniformUpdateCallback : public osg::NodeCallback
{
public:
    LightUniformUpdateCallback(
        osg::Camera* camera,
        osg::Light* light0, osg::Light* light1,
        osg::MatrixTransform* light1Transform,
        osg::Uniform* light0Pos, osg::Uniform* light1Pos,
        osg::Uniform* light0SpotDir, osg::Uniform* light1SpotDir)
        : _camera(camera)
        , _light0(light0)
        , _light1(light1)
        , _light1Transform(light1Transform)
        , _light0PosUniform(light0Pos)
        , _light1PosUniform(light1Pos)
        , _light0SpotDirUniform(light0SpotDir)
        , _light1SpotDirUniform(light1SpotDir)
    {}

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        if (_camera.get())
        {
            osg::Matrixd viewMatrix = _camera->getViewMatrix();

            // Light 0: static position in world space, transform to eye space
            {
                osg::Vec4 pos = _light0->getPosition();
                osg::Vec4 eyePos = pos * viewMatrix;
                _light0PosUniform->set(eyePos);

                // Transform spot direction to eye space (direction only, no translation)
                osg::Vec3 dir = _light0->getDirection();
                osg::Matrix viewRot = viewMatrix;
                viewRot.setTrans(0.0, 0.0, 0.0);
                osg::Vec3 eyeDir = viewRot.preMult(dir);
                eyeDir.normalize();
                _light0SpotDirUniform->set(eyeDir);
            }

            // Light 1: animated position - get world position from MatrixTransform
            {
                osg::Vec4 localPos = _light1->getPosition();
                osg::Matrixd worldMatrix;
                if (_light1Transform.get())
                    worldMatrix = _light1Transform->getMatrix();
                // Transform local position to world space, then to eye space
                osg::Vec4 worldPos = localPos * worldMatrix;
                osg::Vec4 eyePos = worldPos * viewMatrix;
                _light1PosUniform->set(eyePos);

                // Light 1 has no spotlight, but update direction for completeness
                osg::Vec3 dir = _light1->getDirection();
                osg::Matrix combinedRot = worldMatrix * viewMatrix;
                combinedRot.setTrans(0.0, 0.0, 0.0);
                osg::Vec3 eyeDir = combinedRot.preMult(dir);
                eyeDir.normalize();
                _light1SpotDirUniform->set(eyeDir);
            }
        }

        traverse(node, nv);
    }

private:
    osg::ref_ptr<osg::Camera> _camera;
    osg::ref_ptr<osg::Light> _light0;
    osg::ref_ptr<osg::Light> _light1;
    osg::ref_ptr<osg::MatrixTransform> _light1Transform;
    osg::ref_ptr<osg::Uniform> _light0PosUniform;
    osg::ref_ptr<osg::Uniform> _light1PosUniform;
    osg::ref_ptr<osg::Uniform> _light0SpotDirUniform;
    osg::ref_ptr<osg::Uniform> _light1SpotDirUniform;
};


// Helper: set light uniforms from an osg::Light on a StateSet
static void setLightUniforms(osg::StateSet* ss, const std::string& name, osg::Light* light,
                             osg::Uniform*& posUniform, osg::Uniform*& spotDirUniform)
{
    // Position - will be updated per-frame to eye space by the callback
    posUniform = new osg::Uniform((name + ".position").c_str(), light->getPosition());
    ss->addUniform(posUniform);

    ss->addUniform(new osg::Uniform((name + ".ambient").c_str(), light->getAmbient()));
    ss->addUniform(new osg::Uniform((name + ".diffuse").c_str(), light->getDiffuse()));
    ss->addUniform(new osg::Uniform((name + ".specular").c_str(), light->getSpecular()));

    // Spot direction - will be updated per-frame by the callback
    spotDirUniform = new osg::Uniform((name + ".spotDirection").c_str(), light->getDirection());
    ss->addUniform(spotDirUniform);

    // Spot cutoff: pass cos(radians(cutoff)), or -1.0 if no spotlight (cutoff==180)
    float cutoff = light->getSpotCutoff();
    float cosCutoff;
    if (cutoff >= 180.0f)
        cosCutoff = -1.0f;
    else
        cosCutoff = cosf(osg::DegreesToRadians(cutoff));
    ss->addUniform(new osg::Uniform((name + ".spotCutoff").c_str(), cosCutoff));

    ss->addUniform(new osg::Uniform((name + ".spotExponent").c_str(), light->getSpotExponent()));
    ss->addUniform(new osg::Uniform((name + ".constantAttenuation").c_str(), light->getConstantAttenuation()));
    ss->addUniform(new osg::Uniform((name + ".linearAttenuation").c_str(), light->getLinearAttenuation()));
    ss->addUniform(new osg::Uniform((name + ".quadraticAttenuation").c_str(), light->getQuadraticAttenuation()));
}


// Struct to hold references needed after createLights returns
struct LightSetupData
{
    osg::ref_ptr<osg::Light> light0;
    osg::ref_ptr<osg::Light> light1;
    osg::ref_ptr<osg::MatrixTransform> light1Transform;
    osg::Uniform* light0PosUniform;
    osg::Uniform* light1PosUniform;
    osg::Uniform* light0SpotDirUniform;
    osg::Uniform* light1SpotDirUniform;
};


osg::Node* createLights(osg::BoundingBox& bb, osg::StateSet* rootStateSet, LightSetupData& setupData)
{
    osg::Group* lightGroup = new osg::Group;

    float modelSize = bb.radius();

    // create a spot light.
    osg::Light* myLight1 = new osg::Light;
    myLight1->setLightNum(0);
    myLight1->setPosition(osg::Vec4(bb.corner(4),1.0f));
    myLight1->setAmbient(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    myLight1->setDiffuse(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    myLight1->setSpotCutoff(20.0f);
    myLight1->setSpotExponent(50.0f);
    myLight1->setDirection(osg::Vec3(1.0f,1.0f,-1.0f));

    osg::LightSource* lightS1 = new osg::LightSource;
    lightS1->setLight(myLight1);
    lightS1->setLocalStateSetModes(osg::StateAttribute::ON);

    lightGroup->addChild(lightS1);


    // create a local light.
    osg::Light* myLight2 = new osg::Light;
    myLight2->setLightNum(1);
    myLight2->setPosition(osg::Vec4(0.0,0.0,0.0,1.0f));
    myLight2->setAmbient(osg::Vec4(0.0f,1.0f,1.0f,1.0f));
    myLight2->setDiffuse(osg::Vec4(0.0f,1.0f,1.0f,1.0f));
    myLight2->setConstantAttenuation(1.0f);
    myLight2->setLinearAttenuation(2.0f/modelSize);
    myLight2->setQuadraticAttenuation(2.0f/osg::square(modelSize));

    osg::LightSource* lightS2 = new osg::LightSource;
    lightS2->setLight(myLight2);
    lightS2->setLocalStateSetModes(osg::StateAttribute::ON);

    osg::MatrixTransform* mt = new osg::MatrixTransform();
    {
        // set up the animation path
        osg::AnimationPath* animationPath = new osg::AnimationPath;
        animationPath->insert(0.0,osg::AnimationPath::ControlPoint(bb.corner(0)));
        animationPath->insert(1.0,osg::AnimationPath::ControlPoint(bb.corner(1)));
        animationPath->insert(2.0,osg::AnimationPath::ControlPoint(bb.corner(2)));
        animationPath->insert(3.0,osg::AnimationPath::ControlPoint(bb.corner(3)));
        animationPath->insert(4.0,osg::AnimationPath::ControlPoint(bb.corner(4)));
        animationPath->insert(5.0,osg::AnimationPath::ControlPoint(bb.corner(5)));
        animationPath->insert(6.0,osg::AnimationPath::ControlPoint(bb.corner(6)));
        animationPath->insert(7.0,osg::AnimationPath::ControlPoint(bb.corner(7)));
        animationPath->insert(8.0,osg::AnimationPath::ControlPoint(bb.corner(0)));
        animationPath->setLoopMode(osg::AnimationPath::SWING);

        mt->setUpdateCallback(new osg::AnimationPathCallback(animationPath));
    }

    // create marker for point light.
    osg::Geometry* marker = new osg::Geometry;
    marker->setUseVertexBufferObjects(true);
    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(0.0,0.0,0.0));
    marker->setVertexArray(vertices);
    marker->addPrimitiveSet(new osg::DrawArrays(GL_POINTS,0,1));

    osg::StateSet* stateset = new osg::StateSet;
    osg::Point* point = new osg::Point;
    point->setSize(4.0f);
    stateset->setAttribute(point);

    marker->setStateSet(stateset);

    osg::Geode* markerGeode = new osg::Geode;
    markerGeode->addDrawable(marker);

    mt->addChild(lightS2);
    mt->addChild(markerGeode);

    lightGroup->addChild(mt);

    // Set light uniforms on the root state set
    osg::Uniform* light0Pos = NULL;
    osg::Uniform* light1Pos = NULL;
    osg::Uniform* light0SpotDir = NULL;
    osg::Uniform* light1SpotDir = NULL;
    setLightUniforms(rootStateSet, "light0", myLight1, light0Pos, light0SpotDir);
    setLightUniforms(rootStateSet, "light1", myLight2, light1Pos, light1SpotDir);

    // Store references for the update callback
    setupData.light0 = myLight1;
    setupData.light1 = myLight2;
    setupData.light1Transform = mt;
    setupData.light0PosUniform = light0Pos;
    setupData.light1PosUniform = light1Pos;
    setupData.light0SpotDirUniform = light0SpotDir;
    setupData.light1SpotDirUniform = light1SpotDir;

    return lightGroup;
}

osg::Geometry* createWall(const osg::Vec3& v1,const osg::Vec3& v2,const osg::Vec3& v3,osg::StateSet* stateset)
{

   // create a drawable for occluder.
    osg::Geometry* geom = new osg::Geometry;

    geom->setStateSet(stateset);
    geom->setUseVertexBufferObjects(true);

    unsigned int noXSteps = 100;
    unsigned int noYSteps = 100;

    osg::Vec3Array* coords = new osg::Vec3Array;
    coords->reserve(noXSteps*noYSteps);


    osg::Vec3 dx = (v2-v1)/((float)noXSteps-1.0f);
    osg::Vec3 dy = (v3-v1)/((float)noYSteps-1.0f);

    unsigned int row;
    osg::Vec3 vRowStart = v1;
    for(row=0;row<noYSteps;++row)
    {
        osg::Vec3 v = vRowStart;
        for(unsigned int col=0;col<noXSteps;++col)
        {
            coords->push_back(v);
            v += dx;
        }
        vRowStart+=dy;
    }

    geom->setVertexArray(coords);

    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);


    for(row=0;row<noYSteps-1;++row)
    {
        osg::DrawElementsUShort* tristrip = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP);
        tristrip->reserve(noXSteps*2);
        for(unsigned int col=0;col<noXSteps;++col)
        {
            tristrip->push_back((row+1)*noXSteps+col);
            tristrip->push_back(row*noXSteps+col);
        }
        geom->addPrimitiveSet(tristrip);
    }

    // create the normals.
    osgUtil::SmoothingVisitor::smooth(*geom);

    return geom;

}


osg::ref_ptr<osg::Node> createRoom(const osg::ref_ptr<osg::Node>& loadedModel, LightSetupData& setupData)
{
    // default scale for this model.
    osg::BoundingSphere bs(osg::Vec3(0.0f,0.0f,0.0f),1.0f);

    osg::Group* root = new osg::Group;

    if (loadedModel)
    {
        const osg::BoundingSphere& loaded_bs = loadedModel->getBound();

        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform();
        pat->setPivotPoint(loaded_bs.center());

        pat->setUpdateCallback(new ModelTransformCallback(loaded_bs));
        pat->addChild(loadedModel);

        bs = pat->getBound();

        root->addChild(pat);

    }

    bs.radius()*=1.5f;

    // create a bounding box, which we'll use to size the room.
    osg::BoundingBox bb;
    bb.expandBy(bs);


    // create statesets.
    osg::StateSet* rootStateSet = new osg::StateSet;
    root->setStateSet(rootStateSet);

    osg::StateSet* wall = new osg::StateSet;
    wall->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

    osg::StateSet* floor = new osg::StateSet;
    floor->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

    osg::StateSet* roof = new osg::StateSet;
    roof->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

    osg::Geode* geode = new osg::Geode;

    // create front side.
    geode->addDrawable(createWall(bb.corner(0),
                                  bb.corner(4),
                                  bb.corner(1),
                                  wall));

    // right side
    geode->addDrawable(createWall(bb.corner(1),
                                  bb.corner(5),
                                  bb.corner(3),
                                  wall));

    // left side
    geode->addDrawable(createWall(bb.corner(2),
                                  bb.corner(6),
                                  bb.corner(0),
                                  wall));
    // back side
    geode->addDrawable(createWall(bb.corner(3),
                                  bb.corner(7),
                                  bb.corner(2),
                                  wall));

    // floor
    geode->addDrawable(createWall(bb.corner(0),
                                  bb.corner(1),
                                  bb.corner(2),
                                  floor));

    // roof
    geode->addDrawable(createWall(bb.corner(6),
                                  bb.corner(7),
                                  bb.corner(4),
                                  roof));

    root->addChild(geode);

    root->addChild(createLights(bb, rootStateSet, setupData));

    // Attach shader program to root state set
    osg::Program* program = new osg::Program;
    program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource));
    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource));
    rootStateSet->setAttributeAndModes(program, osg::StateAttribute::ON);

    // Set default material uniforms
    rootStateSet->addUniform(new osg::Uniform("material.ambient", osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f)));
    rootStateSet->addUniform(new osg::Uniform("material.diffuse", osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f)));
    rootStateSet->addUniform(new osg::Uniform("material.specular", osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f)));
    rootStateSet->addUniform(new osg::Uniform("material.emission", osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f)));
    rootStateSet->addUniform(new osg::Uniform("material.shininess", 0.0f));

    // Scene ambient
    rootStateSet->addUniform(new osg::Uniform("sceneAmbient", osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f)));

    return root;

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
    if (!loadedModel) loadedModel = osgDB::readRefNodeFile("glider.osgt");

    // create a room made of four walls, a floor, a roof, and swinging light fitting.
    LightSetupData lightData;
    osg::ref_ptr<osg::Node> rootnode = createRoom(loadedModel, lightData);

    // Add update callback on root node to update light position uniforms each frame
    rootnode->addUpdateCallback(new LightUniformUpdateCallback(
        viewer.getCamera(),
        lightData.light0, lightData.light1,
        lightData.light1Transform,
        lightData.light0PosUniform, lightData.light1PosUniform,
        lightData.light0SpotDirUniform, lightData.light1SpotDirUniform));

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(rootnode);


    // create the windows and run the threads.
    viewer.realize();

    viewer.getCamera()->setCullingMode( viewer.getCamera()->getCullingMode() & ~osg::CullStack::SMALL_FEATURE_CULLING);

    return viewer.run();
}
