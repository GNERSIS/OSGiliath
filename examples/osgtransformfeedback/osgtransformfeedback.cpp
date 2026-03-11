/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgtransformfeedback example application
 */
/* file:        examples/osgtransformfeedback/osgtransformfeedback.cpp
 * author:      Julien Valentin 2013-10-01
 * copyright:   (C) 2013
 * license:     OpenSceneGraph Public License (OSGPL)
 *
 * A demo of GLSL geometry shaders using OSG transform feedback
 *
 */

#include <iostream>
#include <osg/core/Notify.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/BufferIndexBinding.hpp>
#include <osg/state/Point.hpp>
#include <osg/state/Program.hpp>
#include <osg/state/Shader.hpp>
#include <osg/state/Uniform.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/core/Viewer.hpp>

///////////////////////////////////////////////////////////////////////////

class SineAnimation : public osg::UniformCallback
{
    public:

        SineAnimation( float rate   = 1.0F,
                       float scale  = 1.0F,
                       float offset = 0.0F ) :
            _rate( rate ),
            _scale( scale ),
            _offset( offset )
        {
        }

        void
        operator()( osg::Uniform*     uniform,
                    osg::NodeVisitor* nv )
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
static const char* RendervertSource = {
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
static const char* vertSource = { "#version 460 core\n"
                                  "layout(location = 0) in vec4 osg_Vertex;\n"
                                  "uniform float u_anim1;\n"
                                  "out vec4 v_color;\n"
                                  "void main(void)\n"
                                  "{\n"
                                  "   gl_Position = osg_Vertex;\n"
                                  "	v_color = osg_Vertex;\n"
                                  "}\n" };

static const char* geomSource = { "#version 460 core\n"
                                  "layout(points) in;\n"
                                  "layout(points, max_vertices = 4) out;\n"
                                  "uniform float u_anim1;\n"
                                  "in vec4 v_color[];\n"
                                  "out vec4 out1;\n"
                                  "void main(void)\n"
                                  "{\n"
                                  "    vec4 v =vec4( gl_in[0].gl_Position.xyz,1);\n"
                                  " out1 =  v + vec4(u_anim1,0.,0.,0.);\n"
                                  "  EmitVertex();\n"
                                  "    EndPrimitive();\n"
                                  "   out1 =  v - vec4(u_anim1,0.,0.,0.);\n"
                                  " EmitVertex();\n"
                                  "    EndPrimitive();\n"
                                  "\n"
                                  "   out1=  v + vec4(0.,1.0-u_anim1,0.,0.);\n"
                                  "  EmitVertex();\n"
                                  "    EndPrimitive();\n"
                                  "   out1 =  v - vec4(0.,1.0-u_anim1,0.,0.);\n"
                                  "  EmitVertex();\n"
                                  "    EndPrimitive();\n"
                                  "}\n" };

static const char* fragSource = { "#version 460 core\n"
                                  "uniform float u_anim1;\n"
                                  "in vec4 v_color;\n"
                                  "out vec4 fragColor;\n"
                                  "void main(void)\n"
                                  "{\n"
                                  "    fragColor = vec4(1,0,0,1);//v_color;\n"
                                  "}\n" };

osg::Program*
createGeneratorShader()
{
    osg::Program* pgm = new osg::Program;
    pgm->setName( "osg transformfeedback demo" );
    pgm->addShader( new osg::Shader( osg::Shader::VERTEX, vertSource ) );
    pgm->addShader( new osg::Shader( osg::Shader::GEOMETRY, geomSource ) );
    pgm->addTransformFeedBackVarying( std::string( "out1" ) );
    pgm->setTransformFeedBackMode( GL_INTERLEAVED_ATTRIBS );
    return pgm;
}

osg::Program*
createRenderShader()
{
    osg::Program* pgm = new osg::Program;
    pgm->setName( "osg transformfeedback renderer demo" );
    pgm->addShader( new osg::Shader( osg::Shader::VERTEX, RendervertSource ) );
    pgm->addShader( new osg::Shader( osg::Shader::FRAGMENT, fragSource ) );
    return pgm;
}

//////////////////////////////////////////////////////////////////////////////////////

class SomePointsRenderer;

class SomePointsGenerator : public osg::Geometry
{
    public:

        SomePointsGenerator();
        void
        setRenderer( osg::Geometry* renderer );
        GLuint
        getNumPrimitivesGenerated() const;

    protected:

        osg::Program*                         _program;
        osg::ref_ptr<osg::VertexBufferObject> genbuffer;    // Renderer buffer
        osg::Vec4Array*                       vAry;

        virtual void
        drawImplementation( osg::RenderInfo& renderInfo ) const;
};

/////////////////////////////////////////////////////////////////////////////////////

class SomePointsRenderer : public osg::Geometry
{
    public:

        SomePointsRenderer( SomePointsGenerator* _generator )
        {

            setUseVertexBufferObjects( true );

            osg::Vec4Array* vAry2 = new osg::Vec4Array;
            vAry2->resize( _generator->getNumPrimitivesGenerated() );
            setVertexArray( vAry2 );
            addPrimitiveSet(
                new osg::DrawArrays( GL_LINES,
                                     0,
                                     _generator->getNumPrimitivesGenerated() )
            );

            osg::StateSet* sset = getOrCreateStateSet();
            /// hacking rendering order
            /*osg::BlendFunc* bf = new
            osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,
            osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
            sset->setAttributeAndModes(bf);*/

            // GL_LIGHTING removed: lighting is shader-controlled in core profile
            getOrCreateVertexBufferObject();
            sset->setAttribute( createRenderShader() );
        }
};

///////////////////////////////////////////////////////////////////////////
GLuint
SomePointsGenerator::getNumPrimitivesGenerated() const
{
    return vAry->size() * 4;
}

void
SomePointsGenerator::drawImplementation( osg::RenderInfo& renderInfo ) const
{

    // get output buffer
    unsigned int contextID = renderInfo.getState()->getContextID();

    GLuint ubuff = genbuffer->getOrCreateGLBufferObject( contextID )->getGLObjectID();

    osg::GLExtensions* ext = renderInfo.getState()->get<osg::GLExtensions>();

    ext->glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, ubuff );

    glEnable( GL_RASTERIZER_DISCARD );

    ext->glBeginTransformFeedback( GL_POINTS );

    osg::Geometry::drawImplementation( renderInfo );

    ext->glEndTransformFeedback();

    glDisable( GL_RASTERIZER_DISCARD );

    ext->glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0 );
}

SomePointsGenerator::SomePointsGenerator() :
    osg::Geometry()
{

    setUseVertexBufferObjects( true );

    osg::StateSet* sset = getOrCreateStateSet();
    // GL_LIGHTING removed: not in core profile
    vAry = new osg::Vec4Array;
    ;
    vAry->push_back( osg::vec4( 0, 0, 0, 1 ) );
    vAry->push_back( osg::vec4( 0, 1, 0, 1 ) );
    vAry->push_back( osg::vec4( 1, 0, 0, 1 ) );
    vAry->push_back( osg::vec4( 1, 1, 0, 1 ) );
    addPrimitiveSet( new osg::DrawArrays( GL_POINTS, 0, vAry->size() ) );
    setVertexArray( vAry );

    _program = createGeneratorShader();
    sset->setAttribute( _program );

    // a generic cyclic animation value
    osg::Uniform* u_anim1( new osg::Uniform( "u_anim1", 0.9F ) );
    u_anim1->setUpdateCallback( new SineAnimation( 4, 0.5, 0.5 ) );
    sset->addUniform( u_anim1 );
}

void
SomePointsGenerator::setRenderer( osg::Geometry* renderer )
{
    genbuffer = renderer->getOrCreateVertexBufferObject();
}

///////////////////////////////////////////////////////////////////////////

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser  arguments( &argc, argv );
    osg::Geode*          root( new osg::Geode );
    SomePointsGenerator* pate  = new SomePointsGenerator();
    SomePointsRenderer*  pate2 = new SomePointsRenderer( pate );
    pate->setRenderer( pate2 );
    root->addDrawable( pate );
    root->addDrawable( pate2 );
    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer;
    viewer.setSceneData( root );
    return viewer.run();
}
