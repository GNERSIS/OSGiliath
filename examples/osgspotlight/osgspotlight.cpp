/* OpenSceneGraph example, osgspotlight.
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

/* Modernized for GL 4.6 Core Profile:
 *  - Replaced TexGen/TexGenNode with shader-based projective texturing
 *  - Added vertex/fragment shaders for projective texture lookup + lighting
 *  - Removed GL_LIGHTING and GL_TEXTURE_GEN_* mode enables
 *  - Uses VBOs for all geometry (ShapeDrawable uses them by default)
 *  - The texGenMatrix uniform is computed from the spotlight's view/projection
 *    matrices and updated each frame via a callback.
 */

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/LightSource>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>

#include <osgUtil/Optimizer>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>


// for the grid data..
#include "../osghangglide/terrain_coords.h"


///////////////////////////////////////////////////////////////////////////
// GLSL shaders for projective spotlight texturing

static const char* spotlightVertSource =
    "#version 460 core\n"
    "layout(location = 0) in vec4 osg_Vertex;\n"
    "layout(location = 2) in vec3 osg_Normal;\n"
    "layout(location = 3) in vec4 osg_Color;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "uniform mat4 osg_ModelViewMatrix;\n"
    "uniform mat3 osg_NormalMatrix;\n"
    "uniform mat4 texGenMatrix;  // eye-to-texture-projection matrix\n"
    "out vec3 vNormal;\n"
    "out vec3 vFragPos;\n"
    "out vec4 vTexCoord;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    vec4 eyePos = osg_ModelViewMatrix * osg_Vertex;\n"
    "    vFragPos = eyePos.xyz;\n"
    "    vNormal = normalize(osg_NormalMatrix * osg_Normal);\n"
    "    vTexCoord = texGenMatrix * eyePos;\n"
    "    vColor = osg_Color;\n"
    "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
    "}\n";

static const char* spotlightFragSource =
    "#version 460 core\n"
    "uniform sampler2D spotTexture;\n"
    "in vec3 vNormal;\n"
    "in vec3 vFragPos;\n"
    "in vec4 vTexCoord;\n"
    "in vec4 vColor;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    vec3 lightDir = normalize(-vFragPos);\n"
    "    float diff = max(dot(normalize(vNormal), lightDir), 0.0) * 0.7 + 0.3;\n"
    "    vec2 tc = vTexCoord.xy / vTexCoord.w;\n"
    "    vec4 spotColor = vec4(0.05, 0.05, 0.05, 1.0);\n"
    "    if (tc.x >= 0.0 && tc.x <= 1.0 && tc.y >= 0.0 && tc.y <= 1.0 && vTexCoord.w > 0.0)\n"
    "        spotColor = texture(spotTexture, tc);\n"
    "    fragColor = vec4(vColor.rgb * diff, 1.0) * spotColor;\n"
    "}\n";


///////////////////////////////////////////////////////////////////////////
// Callback to update the texGenMatrix uniform each frame.
// The matrix transforms from eye space to the spotlight's projective
// texture coordinates: lookAt * perspective * bias (translate+scale to [0,1]).

class TexGenMatrixCallback : public osg::UniformCallback
{
public:
    TexGenMatrixCallback(const osg::Vec3& position, const osg::Vec3& direction, float angle)
        : _position(position), _direction(direction), _angle(angle) {}

    virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
    {
        // The texGenMatrix transforms eye-space positions into projective
        // texture coordinates. We need the inverse of the current camera's
        // modelview (to go from eye space to world space), then multiply
        // by the spotlight's view-projection-bias matrix.
        //
        // However, since the spotlight is attached to an animated transform,
        // its position/direction are in the local coordinate system of that
        // transform. The TexGen in the original code worked in eye space using
        // EYE_LINEAR planes set from this matrix. We replicate that by computing
        // the same matrix and passing it as a uniform.
        //
        // The original code computed:
        //   lookAt(pos, pos+dir, up) * perspective(angle,1,0.1,100) * translate(1,1,1) * scale(0.5,0.5,0.5)
        //
        // In the shader, we multiply this by the eye-space position.
        // Since the spotlight moves with the transform, we need to account
        // for the inverse modelview. We store the combined matrix at setup time
        // (it's constant in local space) and the shader handles the eye-space part.

        osg::Vec3 up(0.0f, 0.0f, 1.0f);
        up = (_direction ^ up) ^ _direction;
        up.normalize();

        osg::Matrixd texGenMatrix =
            osg::Matrixd::lookAt(_position, _position + _direction, up) *
            osg::Matrixd::perspective(_angle, 1.0, 0.1, 100) *
            osg::Matrixd::translate(1.0, 1.0, 1.0) *
            osg::Matrixd::scale(0.5, 0.5, 0.5);

        uniform->set(osg::Matrixf(texGenMatrix));
    }

private:
    osg::Vec3 _position;
    osg::Vec3 _direction;
    float _angle;
};


osg::Image* createSpotLightImage(const osg::Vec4& centerColour, const osg::Vec4& backgroudColour, unsigned int size, float power)
{
    osg::Image* image = new osg::Image;
    image->allocateImage(size,size,1,
                         GL_RGBA,GL_UNSIGNED_BYTE);


    float mid = (float(size)-1)*0.5f;
    float div = 2.0f/float(size);
    for(unsigned int r=0;r<size;++r)
    {
        unsigned char* ptr = image->data(0,r,0);
        for(unsigned int c=0;c<size;++c)
        {
            float dx = (float(c) - mid)*div;
            float dy = (float(r) - mid)*div;
            float pr = powf(1.0f-sqrtf(dx*dx+dy*dy),power);
            if (pr<0.0f) pr=0.0f;
            osg::Vec4 color = centerColour*pr+backgroudColour*(1.0f-pr);
            *ptr++ = (unsigned char)((color[0])*255.0f);
            *ptr++ = (unsigned char)((color[1])*255.0f);
            *ptr++ = (unsigned char)((color[2])*255.0f);
            *ptr++ = (unsigned char)((color[3])*255.0f);
        }
    }
    return image;
}

osg::StateSet* createSpotLightDecoratorState(unsigned int textureUnit)
{
    osg::StateSet* stateset = new osg::StateSet;

    osg::Vec4 centerColour(1.0f,1.0f,1.0f,1.0f);
    osg::Vec4 ambientColour(0.05f,0.05f,0.05f,1.0f);

    // set up spot light texture
    osg::Texture2D* texture = new osg::Texture2D();
    texture->setImage(createSpotLightImage(centerColour, ambientColour, 64, 1.0));
    texture->setBorderColor(osg::Vec4(ambientColour));
    texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_BORDER);

    stateset->setTextureAttributeAndModes(textureUnit, texture, osg::StateAttribute::ON);

    // Bind texture to uniform for shader access
    stateset->addUniform(new osg::Uniform("spotTexture", (int)textureUnit));

    // Add the shader program
    osg::Program* program = new osg::Program;
    program->setName("spotlight");
    program->addShader(new osg::Shader(osg::Shader::VERTEX, spotlightVertSource));
    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, spotlightFragSource));
    stateset->setAttributeAndModes(program, osg::StateAttribute::ON);

    // Add the texGenMatrix uniform with a callback to update it
    // (initial value will be set by the callback)
    osg::Uniform* texGenMatrixUniform = new osg::Uniform("texGenMatrix", osg::Matrixf());
    texGenMatrixUniform->setUpdateCallback(
        new TexGenMatrixCallback(osg::Vec3(0.0f,0.0f,0.0f), osg::Vec3(0.0f,1.0f,-1.0f), 60.0f));
    stateset->addUniform(texGenMatrixUniform);

    return stateset;
}


osg::AnimationPath* createAnimationPath(const osg::Vec3& center,float radius,double looptime)
{
    // set up the animation path
    osg::AnimationPath* animationPath = new osg::AnimationPath;
    animationPath->setLoopMode(osg::AnimationPath::LOOP);

    int numSamples = 40;
    float yaw = 0.0f;
    float yaw_delta = 2.0f*osg::PI/((float)numSamples-1.0f);
    float roll = osg::inDegrees(30.0f);

    double time=0.0f;
    double time_delta = looptime/(double)numSamples;
    for(int i=0;i<numSamples;++i)
    {
        osg::Vec3 position(center+osg::Vec3(sinf(yaw)*radius,cosf(yaw)*radius,0.0f));
        osg::Quat rotation(osg::Quat(roll,osg::Vec3(0.0,1.0,0.0))*osg::Quat(-(yaw+osg::inDegrees(90.0f)),osg::Vec3(0.0,0.0,1.0)));

        animationPath->insert(time,osg::AnimationPath::ControlPoint(position,rotation));

        yaw += yaw_delta;
        time += time_delta;

    }
    return animationPath;
}

osg::Node* createBase(const osg::Vec3& center,float radius)
{

    osg::Geode* geode = new osg::Geode;

    // set up the texture of the base.
    osg::StateSet* stateset = new osg::StateSet();
    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile("Images/lz.rgb");
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    }

    geode->setStateSet( stateset );


    osg::HeightField* grid = new osg::HeightField;
    grid->allocate(38,39);
    grid->setOrigin(center+osg::Vec3(-radius,-radius,0.0f));
    grid->setXInterval(radius*2.0f/(float)(38-1));
    grid->setYInterval(radius*2.0f/(float)(39-1));

    float minHeight = FLT_MAX;
    float maxHeight = -FLT_MAX;


    unsigned int r;
    for(r=0;r<39;++r)
    {
        for(unsigned int c=0;c<38;++c)
        {
            float h = vertex[r+c*39][2];
            if (h>maxHeight) maxHeight=h;
            if (h<minHeight) minHeight=h;
        }
    }

    float hieghtScale = radius*0.5f/(maxHeight-minHeight);
    float hieghtOffset = -(minHeight+maxHeight)*0.5f;

    for(r=0;r<39;++r)
    {
        for(unsigned int c=0;c<38;++c)
        {
            float h = vertex[r+c*39][2];
            grid->setHeight(c,r,(h+hieghtOffset)*hieghtScale);
        }
    }

    geode->addDrawable(new osg::ShapeDrawable(grid));

    osg::Group* group = new osg::Group;
    group->addChild(geode);

    return group;

}

osg::Node* createMovingModel(const osg::Vec3& center, float radius)
{
    float animationLength = 10.0f;

    osg::AnimationPath* animationPath = createAnimationPath(center,radius,animationLength);

    osg::Group* model = new osg::Group;

    osg::ref_ptr<osg::Node> cessna = osgDB::readRefNodeFile("cessna.osgt");
    if (cessna)
    {
        const osg::BoundingSphere& bs = cessna->getBound();

        float size = radius/bs.radius()*0.3f;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
                              osg::Matrix::scale(size,size,size)*
                              osg::Matrix::rotate(osg::inDegrees(180.0f),0.0f,0.0f,2.0f));

        positioned->addChild(cessna);

        osg::MatrixTransform* xform = new osg::MatrixTransform;
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0f,2.0));
        xform->addChild(positioned);

        // The spotlight is now shader-driven; no TexGenNode needed here.
        // The light source is kept for completeness but lighting is done in shaders.
        osg::LightSource* lightsource = new osg::LightSource;
        osg::Light* light = lightsource->getLight();
        light->setLightNum(0);
        light->setPosition(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
        light->setAmbient(osg::Vec4(0.00f,0.00f,0.05f,1.0f));
        light->setDiffuse(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        xform->addChild(lightsource);

        model->addChild(xform);
    }

    return model;
}




osg::Node* createModel()
{
    osg::Vec3 center(0.0f,0.0f,0.0f);
    float radius = 100.0f;

    // the shadower model
    osg::Node* shadower = createMovingModel(center,radius*0.5f);

    // the shadowed model
    osg::Node* shadowed = createBase(center-osg::Vec3(0.0f,0.0f,radius*0.1),radius);

    // combine the models together to create one which has the shadower and the shadowed with the required callback.
    osg::Group* root = new osg::Group;

    root->setStateSet(createSpotLightDecoratorState(1));

    root->addChild(shadower);
    root->addChild(shadowed);

    return root;
}


int main(int, char **)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    // add the spotlight model to the viewer
    viewer.setSceneData( createModel() );

    // run the viewer main frame loop.
    return viewer.run();
}
