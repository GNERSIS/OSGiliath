/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgparametric example application
 */
#include <iostream>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

// for the grid data..
#include "../osghangglide/terrain_coords.hpp"

#include <osg/rendering/HeadlessCapture.hpp>

///////////////////////////////////////////////////////////////////
// vertex shader using just vec4 coefficients
char vertexShaderSource_simple[] =
    "#version 460 core\n"
    "layout(location = 0) in vec4 osg_Vertex;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "out vec4 texCoord; \n"
    "uniform vec4 coeff; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "\n"
    "    texCoord = osg_Vertex; \n"
    "    vec4 vert = osg_Vertex; \n"
    "    vert.z = osg_Vertex.x*coeff[0] + osg_Vertex.x*osg_Vertex.x* coeff[1] + \n"
    "             osg_Vertex.y*coeff[2] + osg_Vertex.y*osg_Vertex.y* coeff[3]; \n"
    "    gl_Position = osg_ModelViewProjectionMatrix * vert;\n"
    "}\n";

//////////////////////////////////////////////////////////////////
// vertex shader using full Matrix4 coefficients
char vertexShaderSource_matrix[] =
    "#version 460 core\n"
    "layout(location = 0) in vec4 osg_Vertex;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "out vec4 texCoord; \n"
    "uniform vec4  origin; \n"
    "uniform mat4  coeffMatrix; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "\n"
    "    texCoord = osg_Vertex; \n"
    "    vec4 v = vec4(osg_Vertex.x, osg_Vertex.x*osg_Vertex.x, osg_Vertex.y, "
    "osg_Vertex.y*osg_Vertex.y ); \n"
    "    gl_Position = osg_ModelViewProjectionMatrix * (origin + coeffMatrix * v);\n"
    "}\n";

//////////////////////////////////////////////////////////////////
// vertex shader using texture read
char vertexShaderSource_texture[] =
    "#version 460 core\n"
    "layout(location = 0) in vec4 osg_Vertex;\n"
    "uniform mat4 osg_ModelViewProjectionMatrix;\n"
    "out vec4 texCoord; \n"
    "uniform sampler2D vertexTexture; \n"
    "\n"
    "void main(void) \n"
    "{ \n"
    "\n"
    "    texCoord = osg_Vertex; \n"
    "    vec4 vert = osg_Vertex; \n"
    "    vert.z = texture( vertexTexture, texCoord.xy).x*0.0001; \n"
    "    gl_Position = osg_ModelViewProjectionMatrix * vert;\n"
    "}\n";

//////////////////////////////////////////////////////////////////
// fragment shader
//
char fragmentShaderSource[] = "#version 460 core\n"
                              "in vec4 texCoord; \n"
                              "out vec4 fragColor; \n"
                              "uniform sampler2D baseTexture; \n"
                              "\n"
                              "void main(void) \n"
                              "{ \n"
                              "    fragColor = texture( baseTexture, texCoord.xy); \n"
                              "}\n";

class UniformVarying : public osg::UniformCallback
{
        virtual void
        operator()( osg::Uniform*     uniform,
                    osg::NodeVisitor* nv )
        {
            const osg::FrameStamp* fs    = nv->getFrameStamp();
            float                  value = sinf( fs->getSimulationTime() );
            uniform->set( osg::vec4( value, -value, -value, value ) );
        }
};

osg::Node*
createModel( const std::string& shader,
             const std::string& textureFileName,
             const std::string& terrainFileName,
             bool               dynamic,
             bool               useVBO )
{
    osg::Geode*    geode = new osg::Geode;

    osg::Geometry* geom  = new osg::Geometry;
    geode->addDrawable( geom );

    // dimensions for ~one million triangles :-)
    unsigned int num_x = 708;
    unsigned int num_y = 708;

    // set up state
    {

        osg::StateSet* stateset = geom->getOrCreateStateSet();

        osg::Program*  program  = new osg::Program;
        stateset->setAttribute( program );

        if( shader == "simple" )
        {
            osg::Shader* vertex_shader =
                new osg::Shader( osg::Shader::VERTEX, vertexShaderSource_simple );
            program->addShader( vertex_shader );

            osg::Uniform* coeff =
                new osg::Uniform( "coeff", osg::vec4( 1.0, -1.0F, -1.0F, 1.0F ) );

            stateset->addUniform( coeff );

            if( dynamic )
            {
                coeff->setUpdateCallback( new UniformVarying );
                coeff->setDataVariance( osg::Object::DataVariance::DYNAMIC );
                stateset->setDataVariance( osg::Object::DataVariance::DYNAMIC );
            }
        }
        else if( shader == "matrix" )
        {
            osg::Shader* vertex_shader =
                new osg::Shader( osg::Shader::VERTEX, vertexShaderSource_matrix );
            program->addShader( vertex_shader );

            osg::Uniform* origin =
                new osg::Uniform( "origin", osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
            stateset->addUniform( origin );

            osg::Uniform* coeffMatrix = new osg::Uniform( "coeffMatrix",
                                                          osg::dmat4( 1.0F,
                                                                      0.0F,
                                                                      1.0F,
                                                                      0.0F,
                                                                      0.0F,
                                                                      0.0F,
                                                                      -1.0F,
                                                                      0.0F,
                                                                      0.0F,
                                                                      1.0F,
                                                                      -1.0F,
                                                                      0.0F,
                                                                      0.0F,
                                                                      0.0F,
                                                                      1.0F,
                                                                      0.0F ) );

            stateset->addUniform( coeffMatrix );
        }
        else if( shader == "texture" )
        {
            osg::Shader* vertex_shader =
                new osg::Shader( osg::Shader::VERTEX, vertexShaderSource_texture );
            program->addShader( vertex_shader );

            osg::ref_ptr<osg::Image> image;

            if( terrainFileName.empty() )
            {
                image           = new osg::Image;
                unsigned int tx = 38;
                unsigned int ty = 39;
                image->allocateImage( tx, ty, 1, GL_RED, GL_FLOAT, 1 );
                for( unsigned int r = 0; r < ty; ++r )
                {
                    for( unsigned int c = 0; c < tx; ++c )
                    {
                        *( ( float* )image->data( c, r ) ) = vertex[r + c * 39][2] * 0.1;
                    }
                }

                num_x = tx;
                num_y = tx;
            }
            else
            {
                image = osgDB::readRefImageFile( terrainFileName );

                num_x = image->s();
                num_y = image->t();
            }

            osg::Texture2D* vertexTexture = new osg::Texture2D( image );

            vertexTexture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
            vertexTexture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
            vertexTexture->setInternalFormat( GL_R32F );
            stateset->setTextureAttributeAndModes( 1, vertexTexture );

            osg::Uniform* vertexTextureSampler = new osg::Uniform( "vertexTexture", 1 );
            stateset->addUniform( vertexTextureSampler );
        }

        osg::Shader* fragment_shader =
            new osg::Shader( osg::Shader::FRAGMENT, fragmentShaderSource );
        program->addShader( fragment_shader );

        osg::Texture2D* texture =
            new osg::Texture2D( osgDB::readRefImageFile( textureFileName ) );
        stateset->setTextureAttributeAndModes( 0, texture );

        osg::Uniform* baseTextureSampler = new osg::Uniform( "baseTexture", 0 );
        stateset->addUniform( baseTextureSampler );
    }

    // set up geometry data.

    osg::Vec3Array* vertices = new osg::Vec3Array( num_x * num_y );

    float           dx       = 1.0F / ( float )( num_x - 1 );
    float           dy       = 1.0F / ( float )( num_y - 1 );
    osg::vec3       row( 0.0F, 0.0F, 0.0 );

    unsigned int    vert_no = 0;
    unsigned int    iy;
    for( iy = 0; iy < num_y; ++iy )
    {
        osg::vec3 column = row;
        for( unsigned int ix = 0; ix < num_x; ++ix )
        {
            ( *vertices )[vert_no++]  = column;
            column.x                 += dx;
        }
        row.y += dy;
    }

    geom->setVertexArray( vertices );

    // Always use VBOs for core profile compatibility
    osg::VertexBufferObject* vbo = new osg::VertexBufferObject;
    vertices->setVertexBufferObject( vbo );

    osg::ElementBufferObject* ebo = new osg::ElementBufferObject;

    for( iy = 0; iy < num_y - 1; ++iy )
    {
        unsigned int           element_no = 0;
        osg::DrawElementsUInt* elements =
            new osg::DrawElementsUInt( GL_TRIANGLE_STRIP, num_x * 2 );
        unsigned int index = iy * num_x;
        for( unsigned int ix = 0; ix < num_x; ++ix )
        {
            ( *elements )[element_no++] = index + num_x;
            ( *elements )[element_no++] = index++;
        }
        geom->addPrimitiveSet( elements );

        elements->setElementBufferObject( ebo );
    }

    geom->setUseVertexBufferObjects( true );

    return geode;
}

int
main( int   argc,
      char* argv[] )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " is the example which demonstrate support for ARB_vertex_program."
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] filename ..."
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );

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

    // add the stats handler
    viewer.addEventHandler( new osgViewer::StatsHandler );

    std::string shader( "simple" );
    while( arguments.read( "-s", shader ) )
    {
    }

    std::string textureFileName( "Images/lz.rgb" );
    while( arguments.read( "-t", textureFileName ) )
    {
    }

    std::string terrainFileName( "" );
    while( arguments.read( "-d", terrainFileName ) )
    {
    }

    bool dynamic = true;
    while( arguments.read( "--static" ) )
    {
        dynamic = false;
    }

    bool vbo = false;
    while( arguments.read( "--vbo" ) )
    {
        vbo = true;
    }

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    // load the nodes from the commandline arguments.
    osg::Node* model =
        createModel( shader, textureFileName, terrainFileName, dynamic, vbo );
    if( !model )
    {
        return 1;
    }

    viewer.setSceneData( model );
    return viewer.run();
}
