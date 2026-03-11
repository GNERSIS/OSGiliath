/* OpenSceneGraph example, osgminimalglsl.
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

/* file:        examples/osgsimpleshaders/osgsimpleshaders.cpp
 *
 * A minimal demo of OpenGL 4.6 Core Profile shaders using OSG.
 * Uses gl_* built-in names which OSG auto-replaces with osg_* equivalents
 * and inserts proper declarations for Core Profile.
*/

#include <osgViewer/Viewer>

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Vec3>
#include <osg/Vec4>

#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>

#include <iostream>

using namespace osg;

///////////////////////////////////////////////////////////////////////////
// in-line GLSL source code for GL 4.6 Core Profile
// OSG's State::convertShaderSourceToOsgBuiltIns() will:
//   - Replace gl_ModelViewMatrix -> osg_ModelViewMatrix (adds uniform declaration)
//   - Replace gl_ModelViewProjectionMatrix -> osg_ModelViewProjectionMatrix
//   - Replace gl_NormalMatrix -> osg_NormalMatrix
//   - Replace gl_Vertex -> osg_Vertex (adds 'in vec4' declaration)
//   - Replace gl_Normal -> osg_Normal (adds 'in vec3' declaration)
// The #version 460 tells OSG to use 'in' qualifier (not 'attribute')

static const char *blockyVertSource = {
    "#version 460 core\n"
    "// blocky.vert - a GLSL vertex shader with animation\n"
    "uniform float Sine;\n"
    "layout(location = 0) in vec4 osg_Vertex;\n"
    "layout(location = 2) in vec3 osg_Normal;\n"
    "uniform mat4 osg_ModelViewMatrix;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "uniform mat3 osg_NormalMatrix;\n"
    "const vec3 LightPosition = vec3(0.0, 0.0, 4.0);\n"
    "const float BlockScale = 0.30;\n"
    "out float LightIntensity;\n"
    "out vec2  BlockPosition;\n"
    "void main(void)\n"
    "{\n"
    "    // per-vertex diffuse lighting\n"
    "    vec4 ecPosition    = osg_ModelViewMatrix * osg_Vertex;\n"
    "    vec3 tnorm         = normalize(osg_NormalMatrix * osg_Normal);\n"
    "    vec3 lightVec      = normalize(LightPosition - vec3(ecPosition));\n"
    "    LightIntensity     = max(dot(lightVec, tnorm), 0.0);\n"
    "    // blocks will be determined by fragment's position on the XZ plane.\n"
    "    BlockPosition  = osg_Vertex.xz / BlockScale;\n"
    "    // scale the geometry based on an animation variable.\n"
    "    vec4 vertex    = osg_Vertex;\n"
    "    vertex.w       = 1.0 + 0.4 * (Sine + 1.0);\n"
    "    gl_Position    = osg_ModelViewProjectionMatrix * vertex;\n"
    "}\n"
};

static const char *blockyFragSource = {
    "#version 460 core\n"
    "// blocky.frag - a GLSL fragment shader with animation\n"
    "uniform float Sine;\n"
    "const vec3 Color1 = vec3(1.0, 1.0, 1.0);\n"
    "const vec3 Color2 = vec3(0.0, 0.0, 0.0);\n"
    "in vec2  BlockPosition;\n"
    "in float LightIntensity;\n"
    "out vec4 fragColor;\n"
    "void main(void)\n"
    "{\n"
    "    vec3 color;\n"
    "    float ss, tt, w, h;\n"
    "    ss = BlockPosition.x;\n"
    "    tt = BlockPosition.y;\n"
    "    if (fract(tt * 0.5) > 0.5)\n"
    "        ss += 0.5;\n"
    "    ss = fract(ss);\n"
    "    tt = fract(tt);\n"
    "    // animate the proportion of block to mortar\n"
    "    float blockFract = (Sine + 1.1) * 0.4;\n"
    "    w = step(ss, blockFract);\n"
    "    h = step(tt, blockFract);\n"
    "    color = mix(Color2, Color1, w * h) * LightIntensity;\n"
    "    fragColor = vec4(color, 1.0);\n"
    "}\n"
};

///////////////////////////////////////////////////////////////////////////
// callback for animating the Sine uniform

class AnimateCallback: public osg::UniformCallback
{
    public:
        enum Operation { SIN };
        AnimateCallback(Operation op) : _operation(op) {}
        virtual void operator() ( osg::Uniform* uniform, osg::NodeVisitor* nv )
        {
            float angle = 2.0 * nv->getFrameStamp()->getSimulationTime();
            float sine = sinf( angle );            // -1 -> 1
            switch(_operation) {
                case SIN : uniform->set( sine ); break;
            }
        }
    private:
        Operation _operation;
};

///////////////////////////////////////////////////////////////////////////
// Create a simple colored cube using osg::Geometry with VBOs

osg::Geometry* createCube()
{
    osg::Geometry* geom = new osg::Geometry;
    geom->setUseVertexBufferObjects(true);

    // 24 vertices (4 per face, 6 faces)
    osg::Vec3Array* vertices = new osg::Vec3Array(24);
    osg::Vec3Array* normals = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX, 24);

    float s = 0.5f;

    // Front face
    (*vertices)[ 0].set(-s, -s,  s); (*normals)[ 0].set( 0, 0, 1);
    (*vertices)[ 1].set( s, -s,  s); (*normals)[ 1].set( 0, 0, 1);
    (*vertices)[ 2].set( s,  s,  s); (*normals)[ 2].set( 0, 0, 1);
    (*vertices)[ 3].set(-s,  s,  s); (*normals)[ 3].set( 0, 0, 1);
    // Back face
    (*vertices)[ 4].set( s, -s, -s); (*normals)[ 4].set( 0, 0,-1);
    (*vertices)[ 5].set(-s, -s, -s); (*normals)[ 5].set( 0, 0,-1);
    (*vertices)[ 6].set(-s,  s, -s); (*normals)[ 6].set( 0, 0,-1);
    (*vertices)[ 7].set( s,  s, -s); (*normals)[ 7].set( 0, 0,-1);
    // Top face
    (*vertices)[ 8].set(-s,  s,  s); (*normals)[ 8].set( 0, 1, 0);
    (*vertices)[ 9].set( s,  s,  s); (*normals)[ 9].set( 0, 1, 0);
    (*vertices)[10].set( s,  s, -s); (*normals)[10].set( 0, 1, 0);
    (*vertices)[11].set(-s,  s, -s); (*normals)[11].set( 0, 1, 0);
    // Bottom face
    (*vertices)[12].set(-s, -s, -s); (*normals)[12].set( 0,-1, 0);
    (*vertices)[13].set( s, -s, -s); (*normals)[13].set( 0,-1, 0);
    (*vertices)[14].set( s, -s,  s); (*normals)[14].set( 0,-1, 0);
    (*vertices)[15].set(-s, -s,  s); (*normals)[15].set( 0,-1, 0);
    // Right face
    (*vertices)[16].set( s, -s,  s); (*normals)[16].set( 1, 0, 0);
    (*vertices)[17].set( s, -s, -s); (*normals)[17].set( 1, 0, 0);
    (*vertices)[18].set( s,  s, -s); (*normals)[18].set( 1, 0, 0);
    (*vertices)[19].set( s,  s,  s); (*normals)[19].set( 1, 0, 0);
    // Left face
    (*vertices)[20].set(-s, -s, -s); (*normals)[20].set(-1, 0, 0);
    (*vertices)[21].set(-s, -s,  s); (*normals)[21].set(-1, 0, 0);
    (*vertices)[22].set(-s,  s,  s); (*normals)[22].set(-1, 0, 0);
    (*vertices)[23].set(-s,  s, -s); (*normals)[23].set(-1, 0, 0);

    geom->setVertexArray(vertices);
    geom->setNormalArray(normals);

    // Index buffer: 2 triangles per face, 6 faces = 36 indices
    osg::DrawElementsUShort* indices = new osg::DrawElementsUShort(GL_TRIANGLES, 36);
    for (int face = 0; face < 6; ++face)
    {
        int base = face * 4;
        (*indices)[face*6 + 0] = base + 0;
        (*indices)[face*6 + 1] = base + 1;
        (*indices)[face*6 + 2] = base + 2;
        (*indices)[face*6 + 3] = base + 0;
        (*indices)[face*6 + 4] = base + 2;
        (*indices)[face*6 + 5] = base + 3;
    }
    geom->addPrimitiveSet(indices);

    return geom;
}

int main(int, char **)
{
    std::cerr << "osgsimpleshaders: Starting GL 4.6 Core Profile test..." << std::endl;

    // construct the viewer.
    osgViewer::Viewer viewer;

    // use a geode with a manually-built cube (no ShapeDrawable)
    osg::Geode* basicModel = new osg::Geode();
    basicModel->addDrawable(createCube());

    std::cerr << "osgsimpleshaders: Geometry created." << std::endl;

    // create the "blocky" shader, a simple animation test
    osg::StateSet *ss = basicModel->getOrCreateStateSet();
    osg::Program* program = new osg::Program;
    program->setName( "blocky" );
    program->addShader( new osg::Shader( osg::Shader::VERTEX, blockyVertSource ) );
    program->addShader( new osg::Shader( osg::Shader::FRAGMENT, blockyFragSource ) );
    ss->setAttributeAndModes(program, osg::StateAttribute::ON);

    std::cerr << "osgsimpleshaders: Shaders attached." << std::endl;

    // attach some animated Uniform variable to the state set
    osg::Uniform* SineUniform   = new osg::Uniform( "Sine", 0.0f );
    ss->addUniform( SineUniform );
    SineUniform->setUpdateCallback(new AnimateCallback(AnimateCallback::SIN));

    // run the osg::Viewer using our model
    viewer.setSceneData( basicModel );

    std::cerr << "osgsimpleshaders: Calling viewer.run()..." << std::endl;

    return viewer.run();
}

/*EOF*/
