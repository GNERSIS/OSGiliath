/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgdrawinstanced example application
 */
//
// This code is copyright (c) 2008 Skew dmat4 Software LLC. You may use
// the code under the licensing terms described above.
//

#include <iostream>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgViewer/core/Viewer.hpp>

void
createDAIGeometry( osg::Geometry& geom,
                   int            nInstances = 1 )
{
    const float     halfDimX( .5 );
    const float     halfDimZ( .5 );

    osg::Vec3Array* v = new osg::Vec3Array;
    v->resize( 4 );
    geom.setVertexArray( v );

    // Geometry for a single quad.
    ( *v )[0] = osg::vec3( -halfDimX, 0., -halfDimZ );
    ( *v )[1] = osg::vec3( halfDimX, 0., -halfDimZ );
    ( *v )[2] = osg::vec3( halfDimX, 0., halfDimZ );
    ( *v )[3] = osg::vec3( -halfDimX, 0., halfDimZ );

    // Create an index buffer to draw two triangles (replacing GL_QUADS for core
    // profile).
    osg::DrawElementsUShort* elements = new osg::DrawElementsUShort( GL_TRIANGLES, 6 );
    elements->setNumInstances( nInstances );
    ( *elements )[0] = 0;
    ( *elements )[1] = 1;
    ( *elements )[2] = 2;
    ( *elements )[3] = 0;
    ( *elements )[4] = 2;
    ( *elements )[5] = 3;
    geom.addPrimitiveSet( elements );
}

osg::StateSet*
createStateSet()
{
    osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;

    // Create a vertex program that references the gl_InstanceID to
    // render each instance uniquely. gl_InstanceID will be in the range
    // 0 to numInstances-1 (1023 in our case).
    std::string                 vertexSource =
        "#version 460 core\n"
        "layout(location = 0) in vec4 osg_Vertex;\n"
        "uniform mat4 osg_ModelViewProjectionMatrix;\n"
        "uniform sampler2D osgLogo; \n"
        "uniform float osg_SimulationTime; \n"
        "out vec4 vertColor; \n"

        "void main() \n"
        "{ \n"
        // Using the instance ID, generate "texture coords" for this instance.
        "vec2 tC; \n"
        "float r = float(gl_InstanceID) / 32.; \n"
        "tC.s = fract( r ); tC.t = floor( r ) / 32.; \n"
        // Get the color from the OSG logo.
        "vertColor = texture( osgLogo, tC ); \n"

        // Use the (scaled) tex coord to translate the position of the vertices.
        "vec4 pos = vec4( tC.s * 48., 0., tC.t * 48., 1. ); \n"

        // Compute a rotation angle from the instanceID and elapsed time.
        "float timeOffset = gl_InstanceID / (32. * 32.); \n"
        "float angle = ( osg_SimulationTime - timeOffset ) * 6.283; \n"
        "float sa = sin( angle ); \n"
        "float ca = cos( angle ); \n"
        // New orientation, rotate around z axis.
        "vec4 newX = vec4( ca, sa, 0., 0. ); \n"
        "vec4 newY = vec4( sa, ca, 0., 0. ); \n"
        "vec4 newZ = vec4( 0., 0., 1., 0. ); \n"
        "mat4 mV = mat4( newX, newY, newZ, pos ); \n"
        "gl_Position = ( osg_ModelViewProjectionMatrix * mV * osg_Vertex ); \n"
        "} \n";

    std::string               fragSource   = "#version 460 core\n"
                                             "in vec4 vertColor;\n"
                                             "out vec4 fragColor;\n"
                                             "void main(void)\n"
                                             "{\n"
                                             "    fragColor = vertColor;\n"
                                             "}\n";

    osg::ref_ptr<osg::Shader> vertexShader = new osg::Shader();
    vertexShader->setType( osg::Shader::VERTEX );
    vertexShader->setShaderSource( vertexSource );

    osg::ref_ptr<osg::Shader> fragmentShader = new osg::Shader();
    fragmentShader->setType( osg::Shader::FRAGMENT );
    fragmentShader->setShaderSource( fragSource );

    osg::ref_ptr<osg::Program> program = new osg::Program();
    program->addShader( vertexShader.get() );
    program->addShader( fragmentShader.get() );

    ss->setAttribute( program.get(),
                      osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    osg::ref_ptr<osg::Image> iLogo = osgDB::readRefImageFile( "Images/osg128.png" );
    if( !iLogo.valid() )
    {
        osg::notify( osg::ALWAYS ) << "Can't open image file osg128.png" << std::endl;
        return ( NULL );
    }
    osg::Texture2D* texLogo = new osg::Texture2D( iLogo.get() );
    texLogo->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    texLogo->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );

    ss->setTextureAttribute( 0, texLogo );

    osg::ref_ptr<osg::Uniform> texLogoUniform = new osg::Uniform( "osgLogo", 0 );
    ss->addUniform( texLogoUniform.get() );

    return ( ss.release() );
}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser         arguments( &argc, argv );

    // Make a scene graph consisting of a single Geode, containing
    // a single Geometry, and a single PrimitiveSet.
    osg::ref_ptr<osg::Geode>    geode = new osg::Geode;

    osg::ref_ptr<osg::Geometry> geom  = new osg::Geometry;
    // Configure the Geometry for use with EXT_draw_arrays:
    // DL off and buffer objects on.
    geom->setUseVertexBufferObjects( true );
    // OSG has no clue where out vertex shader will place the geometric data,
    // so specify an initial bound to allow proper culling and near/far computation.
    osg::box bb( -1., -.1, -1., 49., 1., 49. );
    geom->setInitialBound( bb );
    // Add geometric data and the PrimitiveSet. Specify numInstances as 32*32 or 1024.
    createDAIGeometry( *geom, 32 * 32 );
    geode->addDrawable( geom.get() );

    // Create a StateSet to render the instanced Geometry.
    osg::ref_ptr<osg::StateSet> ss = createStateSet();
    geode->setStateSet( ss.get() );

    // osgDB::writeNodeFile(*geode, "instanced.osgt");

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

    osgViewer::Viewer viewer( arguments );
    viewer.setSceneData( geode.get() );
    return viewer.run();
}
