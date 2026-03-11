/* OpenSceneGraph example, osgshaders2
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

/* file:        examples/osgshaders2/osgshaders2.cpp
 * author:      Mike Weiblen 2008-01-03
 * copyright:   (C) 2008 Zebra Imaging
 * license:     OpenSceneGraph Public License (OSGPL)
 *
 * A demo of GLSL geometry shaders using OSG
 * Tested on Dell Precision M4300 w/ NVIDIA Quadro FX 360M
*/


#include <osg/Notify>
#include <osg/ref_ptr>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Point>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>
#include <osgViewer/Viewer>

// play with these #defines to see their effect
#define ENABLE_GLSL
#define ENABLE_GEOMETRY_SHADER

///////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_GLSL

class SineAnimation: public osg::UniformCallback
{
public:
    SineAnimation( float rate = 1.0f, float scale = 1.0f, float offset = 0.0f ) :
            _rate(rate), _scale(scale), _offset(offset)
    {}

    void operator()( osg::Uniform* uniform, osg::NodeVisitor* nv )
    {
        float angle = _rate * nv->getFrameStamp()->getSimulationTime();
        float value = sinf( angle ) * _scale + _offset;
        uniform->set( value );
    }

private:
    const float _rate;
    const float _scale;
    const float _offset;
};

///////////////////////////////////////////////////////////////////////////

static const char* vertSource = {
"#version 460 core\n"
"layout(location = 0) in vec4 osg_Vertex;\n"
"uniform mat4 osg_ModelViewProjectionMatrix;\n"
"uniform float u_anim1;\n"
"out vec4 v_color;\n"
"void main(void)\n"
"{\n"
"    v_color = osg_Vertex;\n"
"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
"}\n"
};

static const char* geomSource = {
"#version 460 core\n"
"layout(points) in;\n"
"layout(line_strip, max_vertices = 4) out;\n"
"uniform float u_anim1;\n"
"in vec4 v_color[];\n"
"out vec4 v_color_out;\n"
"void main(void)\n"
"{\n"
"    vec4 v = gl_in[0].gl_Position;\n"
"    v_color_out = v + v_color[0];\n"
"\n"
"    gl_Position = v + vec4(u_anim1,0.,0.,0.);  EmitVertex();\n"
"    gl_Position = v - vec4(u_anim1,0.,0.,0.);  EmitVertex();\n"
"    EndPrimitive();\n"
"\n"
"    gl_Position = v + vec4(0.,1.0-u_anim1,0.,0.);  EmitVertex();\n"
"    gl_Position = v - vec4(0.,1.0-u_anim1,0.,0.);  EmitVertex();\n"
"    EndPrimitive();\n"
"}\n"
};


static const char* fragSource = {
"#version 460 core\n"
"uniform float u_anim1;\n"
"in vec4 v_color_out;\n"
"out vec4 fragColor;\n"
"void main(void)\n"
"{\n"
"    fragColor = v_color_out;\n"
"}\n"
};

osg::Program* createShader()
{
    osg::Program* pgm = new osg::Program;
    pgm->setName( "osgshader2 demo" );

    pgm->addShader( new osg::Shader( osg::Shader::VERTEX,   vertSource ) );
    pgm->addShader( new osg::Shader( osg::Shader::FRAGMENT, fragSource ) );

#ifdef ENABLE_GEOMETRY_SHADER
    pgm->addShader( new osg::Shader( osg::Shader::GEOMETRY, geomSource ) );
#endif

    return pgm;
}

#endif

///////////////////////////////////////////////////////////////////////////

class SomePoints : public osg::Geometry
{
public:
    SomePoints()
    {
        osg::Vec4Array* cAry = new osg::Vec4Array;
        setColorArray( cAry, osg::Array::BIND_OVERALL );
        cAry->push_back( osg::Vec4(1,1,1,1) );

        osg::Vec3Array* vAry = new osg::Vec3Array;
        setVertexArray( vAry );
        vAry->push_back( osg::Vec3(0,0,0) );
        vAry->push_back( osg::Vec3(0,1,0) );
        vAry->push_back( osg::Vec3(1,0,0) );
        vAry->push_back( osg::Vec3(1,1,0) );
        vAry->push_back( osg::Vec3(0,0,1) );
        vAry->push_back( osg::Vec3(0,1,1) );
        vAry->push_back( osg::Vec3(1,0,1) );
        vAry->push_back( osg::Vec3(1,1,1) );

        addPrimitiveSet( new osg::DrawArrays( GL_POINTS, 0, vAry->size() ) );

        osg::StateSet* sset = getOrCreateStateSet();
        // GL_LIGHTING removed: lighting is shader-controlled in core profile

        // if things go wrong, fall back to big points
        osg::Point* p = new osg::Point;
        p->setSize(6);
        sset->setAttribute( p );

#ifdef ENABLE_GLSL
        sset->setAttribute( createShader() );

        // a generic cyclic animation value
        osg::Uniform* u_anim1( new osg::Uniform( "u_anim1", 0.0f ) );
        u_anim1->setUpdateCallback( new SineAnimation( 4, 0.5, 0.5 ) );
        sset->addUniform( u_anim1 );
#endif
    }
};

///////////////////////////////////////////////////////////////////////////

int main( int , char** )
{
    osg::Geode* root( new osg::Geode );
    root->addDrawable( new SomePoints );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root );
    return viewer.run();
}

// vim: set sw=4 ts=8 et ic ai:
