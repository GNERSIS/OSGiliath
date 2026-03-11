/* OpenSceneGraph example, osgtexture1D.
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

#include <osg/Notify>
#include <osg/Texture1D>
#include <osg/Material>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>

#include <iostream>

///////////////////////////////////////////////////////////////////////////
// in-line GLSL source code for GL 4.6 core profile

static const char* tex1dVertexShader = {
    "#version 460 core\n"
    "layout(location = 0) in vec4 osg_Vertex;\n"
    "layout(location = 2) in vec3 osg_Normal;\n"
    "layout(location = 3) in vec4 osg_Color;\n"
    "uniform mat4 osg_ModelViewMatrix;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "uniform mat3 osg_NormalMatrix;\n"
    "uniform vec4 texGenPlaneS;\n"
    "uniform bool useEyeLinear;\n"
    "out float vTexCoord;\n"
    "out vec3 vNormal;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    vec4 eyePos = osg_ModelViewMatrix * osg_Vertex;\n"
    "    vNormal = normalize(osg_NormalMatrix * osg_Normal);\n"
    "    vColor = osg_Color;\n"
    "    if (useEyeLinear) {\n"
    "        vTexCoord = dot(eyePos, texGenPlaneS);\n"
    "    } else {\n"
    "        vTexCoord = dot(osg_Vertex, texGenPlaneS);\n"
    "    }\n"
    "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
    "}\n"
};

static const char* tex1dFragmentShader = {
    "#version 460 core\n"
    "uniform sampler1D contourTex;\n"
    "in float vTexCoord;\n"
    "in vec3 vNormal;\n"
    "in vec4 vColor;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    vec4 texColor = texture(contourTex, vTexCoord);\n"
    "    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.8));\n"
    "    float diff = max(dot(normalize(vNormal), lightDir), 0.0) * 0.7 + 0.3;\n"
    "    fragColor = vec4(texColor.rgb * diff, texColor.a);\n"
    "}\n"
};

// Creates a stateset which contains a 1D texture which is populated by contour banded color,
// and allows tex gen to override the S texture coordinate
osg::StateSet* create1DTextureStateToDecorate(osg::Node* /*loadedModel*/)
{
    osg::Image* image = new osg::Image;

    int noPixels = 1024;

    // allocate the image data, noPixels x 1 x 1 with 4 rgba floats - equivalent to a Vec4!
    image->allocateImage(noPixels,1,1,GL_RGBA,GL_FLOAT);
    image->setInternalTextureFormat(GL_RGBA);

    typedef std::vector<osg::Vec4> ColorBands;
    ColorBands colorbands;
    colorbands.push_back(osg::Vec4(0.0f,0.0,0.0,1.0f));
    colorbands.push_back(osg::Vec4(1.0f,0.0,0.0,1.0f));
    colorbands.push_back(osg::Vec4(1.0f,1.0,0.0,1.0f));
    colorbands.push_back(osg::Vec4(0.0f,1.0,0.0,1.0f));
    colorbands.push_back(osg::Vec4(0.0f,1.0,1.0,1.0f));
    colorbands.push_back(osg::Vec4(0.0f,0.0,1.0,1.0f));
    colorbands.push_back(osg::Vec4(1.0f,0.0,1.0,1.0f));
    colorbands.push_back(osg::Vec4(1.0f,1.0,1.0,1.0f));

    float nobands = colorbands.size();
    float delta = nobands/(float)noPixels;
    float pos = 0.0f;

    // fill in the image data.
    osg::Vec4* dataPtr = (osg::Vec4*)image->data();
    for(int i=0;i<noPixels;++i,pos+=delta)
    {
        //float p = floorf(pos);
        //float r = pos-p;
        //osg::Vec4 color = colorbands[(int)p]*(1.0f-r);
        //if (p+1<colorbands.size()) color += colorbands[(int)p+1]*r;
        osg::Vec4 color = colorbands[(int)pos];
        *dataPtr++ = color;
    }

    osg::Texture1D* texture = new osg::Texture1D;
    texture->setWrap(osg::Texture1D::WRAP_S,osg::Texture1D::MIRROR);
    texture->setFilter(osg::Texture1D::MIN_FILTER,osg::Texture1D::LINEAR);
    texture->setImage(image);

    osg::StateSet* stateset = new osg::StateSet;

    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);

    // Add GLSL shaders for GL 4.6 core profile
    osg::Program* program = new osg::Program;
    program->setName("texture1d");
    program->addShader(new osg::Shader(osg::Shader::VERTEX, tex1dVertexShader));
    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, tex1dFragmentShader));
    stateset->setAttributeAndModes(program, osg::StateAttribute::ON);

    // Sampler uniform for the 1D contour texture
    osg::Uniform* texUniform = new osg::Uniform(osg::Uniform::SAMPLER_1D, "contourTex");
    texUniform->set(0);
    stateset->addUniform(texUniform);

    return stateset;
}


// An app callback which alternates the tex gen mode between object linear and eye linear
// and updates the corresponding shader uniforms.
class AnimateTexGenCallback : public osg::NodeCallback
{
    public:
        AnimateTexGenCallback(osg::Uniform* planeUniform, osg::Uniform* modeUniform, const osg::Plane& plane)
            : _planeUniform(planeUniform), _modeUniform(modeUniform), _plane(plane) {}

        void animate(double time)
        {
            const double timeInterval = 2.0f;

            static double previousTime = time;
            static bool state = false;
            while (time>previousTime+timeInterval)
            {
                previousTime+=timeInterval;
                state = !state;
            }

            _modeUniform->set(!state);

            // Update the plane uniform
            _planeUniform->set(osg::Vec4(_plane[0], _plane[1], _plane[2], _plane[3]));
        }

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            if (nv->getFrameStamp())
            {
                animate(nv->getFrameStamp()->getSimulationTime());
            }

            traverse(node,nv);
        }

    protected:
        osg::ref_ptr<osg::Uniform> _planeUniform;
        osg::ref_ptr<osg::Uniform> _modeUniform;
        osg::Plane _plane;
};


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;

    // load the images specified on command line
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readRefNodeFile("dumptruck.osgt");

    if (!loadedModel)
    {
        osg::notify(osg::NOTICE)<<arguments.getApplicationUsage()->getCommandLineUsage()<<std::endl;
        return 0;
    }

    osg::ref_ptr<osg::StateSet> stateset = create1DTextureStateToDecorate(loadedModel.get());
    if (!stateset)
    {
        std::cout<<"Error: failed to create 1D texture state."<<std::endl;
        return 1;
    }


    loadedModel->setStateSet(stateset.get());

    osg::ref_ptr<osg::Group> root = new osg:: Group;
    root -> addChild( loadedModel );

    // Compute the tex gen plane from model bounds
    const osg::BoundingSphere& bs = loadedModel->getBound();
    float zBase = bs.center().z()-bs.radius();
    float zScale = 2.0f/bs.radius();
    osg::Plane sPlane(0.0f, 0.0f, zScale, -zBase);

    // Create uniforms for the shader-based tex gen
    osg::Uniform* planeUniform = new osg::Uniform("texGenPlaneS",
        osg::Vec4(sPlane[0], sPlane[1], sPlane[2], sPlane[3]));
    osg::Uniform* modeUniform = new osg::Uniform("useEyeLinear", false);
    stateset->addUniform(planeUniform);
    stateset->addUniform(modeUniform);

    // The contour banded color texture is used with shader-based tex gen
    // to create contoured models, either in object linear coords - like
    // contours on a map, or eye linear which contour the distance from
    // the eye. An app callback toggles between the two tex gen modes.
    osg::Group* texgenGroup = new osg::Group;
    texgenGroup->setUpdateCallback(new AnimateTexGenCallback(planeUniform, modeUniform, sPlane));

    root -> addChild( texgenGroup );


    viewer.setSceneData( root );

    return viewer.run();
}
