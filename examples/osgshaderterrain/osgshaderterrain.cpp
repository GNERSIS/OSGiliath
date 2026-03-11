/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgshaderterrain example application
 */
#include <iostream>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/Billboard.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/Projection.hpp>
#include <osg/nodes/Switch.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/Depth.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/PolygonOffset.hpp>
#include <osg/state/Program.hpp>
#include <osg/state/Shader.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgText/Text>
#include <osgUtil/optimization/SmoothingVisitor.hpp>
#include <osgViewer/core/Viewer.hpp>

// for the grid data..
#include "../osghangglide/terrain_coords.hpp"

#include <osg/rendering/HeadlessCapture.hpp>

osg::Node*
createScene()
{
    osg::Group*    scene      = new osg::Group;

    unsigned int   numColumns = 38;
    unsigned int   numRows    = 39;
    unsigned int   r;
    unsigned int   c;

    osg::vec3      origin( 0.0F, 0.0F, 0.0F );
    osg::vec3      size( 1000.0F, 1000.0F, 250.0F );
    osg::vec3      scaleDown( 1.0F / size.x, 1.0F / size.y, 1.0F / size.z );

    // ---------------------------------------
    // Set up a StateSet to texture the objects
    // ---------------------------------------
    osg::StateSet* stateset      = new osg::StateSet();

    osg::Uniform*  originUniform = new osg::Uniform( "terrainOrigin", origin );
    stateset->addUniform( originUniform );

    osg::Uniform* sizeUniform = new osg::Uniform( "terrainSize", size );
    stateset->addUniform( sizeUniform );

    osg::Uniform* scaleDownUniform = new osg::Uniform( "terrainScaleDown", scaleDown );
    stateset->addUniform( scaleDownUniform );

    osg::Uniform* terrainTextureSampler = new osg::Uniform( "terrainTexture", 0 );
    stateset->addUniform( terrainTextureSampler );

    osg::Uniform* baseTextureSampler = new osg::Uniform( "baseTexture", 1 );
    stateset->addUniform( baseTextureSampler );

    osg::Uniform* treeTextureSampler = new osg::Uniform( "treeTexture", 1 );
    stateset->addUniform( treeTextureSampler );

    // compute z range of z values of grid data so we can scale it.
    float min_z = FLT_MAX;
    float max_z = -FLT_MAX;
    for( r = 0; r < numRows; ++r )
    {
        for( c = 0; c < numColumns; ++c )
        {
            min_z = std::min( min_z, vertex[r + c * numRows][2] );
            max_z = std::max( max_z, vertex[r + c * numRows][2] );
        }
    }

    float       scale_z      = size.z / ( max_z - min_z );

    osg::Image* terrainImage = new osg::Image;
    terrainImage->allocateImage( numColumns, numRows, 1, GL_RED, GL_FLOAT );
    terrainImage->setInternalTextureFormat( GL_R32F );
    for( r = 0; r < numRows; ++r )
    {
        for( c = 0; c < numColumns; ++c )
        {
            *( ( float* )( terrainImage->data( c, r ) ) ) =
                ( vertex[r + c * numRows][2] - min_z ) * scale_z;
        }
    }

    osg::Texture2D* terrainTexture = new osg::Texture2D;
    terrainTexture->setImage( terrainImage );
    terrainTexture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
    terrainTexture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
    terrainTexture->setResizeNonPowerOfTwoHint( false );
    stateset->setTextureAttributeAndModes( 0, terrainTexture, osg::StateAttribute::ON );

    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile( "Images/lz.rgb" );
    if( image )
    {
        osg::Texture2D* texture = new osg::Texture2D;

        texture->setImage( image );
        stateset->setTextureAttributeAndModes( 1, texture, osg::StateAttribute::ON );
    }

    {
        std::cout << "Creating terrain...";

        osg::Geode* geode = new osg::Geode();
        geode->setStateSet( stateset );

        {
            osg::Program* program = new osg::Program;
            stateset->setAttribute( program );

#if 1
            // use inline shaders

            ///////////////////////////////////////////////////////////////////
            // vertex shader using just vec4 coefficients
            char vertexShaderSource[] =
                "#version 460 core\n"
                "\n"
                "uniform sampler2D terrainTexture;\n"
                "uniform vec3 terrainOrigin;\n"
                "uniform vec3 terrainScaleDown;\n"
                "\n"
                "layout(location = 0) in vec4 osg_Vertex;\n"
                "uniform mat4 osg_ModelViewProjectionMatrix;\n"
                "\n"
                "out vec2 texcoord;\n"
                "out vec4 vColor;\n"
                "\n"
                "void main(void)\n"
                "{\n"
                "    texcoord = osg_Vertex.xy - terrainOrigin.xy;\n"
                "    texcoord.x *= terrainScaleDown.x;\n"
                "    texcoord.y *= terrainScaleDown.y;\n"
                "\n"
                "    vec4 position;\n"
                "    position.x = osg_Vertex.x;\n"
                "    position.y = osg_Vertex.y;\n"
                "    position.z = texture(terrainTexture, texcoord).r;\n"
                "    position.w = 1.0;\n"
                " \n"
                "    gl_Position     = osg_ModelViewProjectionMatrix * position;\n"
                "    vColor = vec4(1.0,1.0,1.0,1.0);\n"
                "}\n";

            //////////////////////////////////////////////////////////////////
            // fragment shader
            //
            char fragmentShaderSource[] =
                "#version 460 core\n"
                "\n"
                "uniform sampler2D baseTexture; \n"
                "in vec2 texcoord;\n"
                "in vec4 vColor;\n"
                "\n"
                "out vec4 fragColor;\n"
                "\n"
                "void main(void) \n"
                "{\n"
                "    fragColor = texture( baseTexture, texcoord); \n"
                "}\n";

            program->addShader( new osg::Shader( osg::Shader::VERTEX,
                                                 vertexShaderSource ) );
            program->addShader( new osg::Shader( osg::Shader::FRAGMENT,
                                                 fragmentShaderSource ) );

#else

            // get shaders from source
            program->addShader( osg::Shader::readShaderFile(
                osg::Shader::VERTEX,
                osgDB::findDataFile( "shaders/terrain.vert" )
            ) );
            program->addShader( osg::Shader::readShaderFile(
                osg::Shader::FRAGMENT,
                osgDB::findDataFile( "shaders/terrain.frag" )
            ) );

#endif

            // get shaders from source
        }

        {
            osg::Geometry* geometry = new osg::Geometry;
            geometry->setUseVertexBufferObjects( true );

            osg::Vec3Array&   v     = *( new osg::Vec3Array( numColumns * numRows ) );
            osg::Vec4ubArray& color = *( new osg::Vec4ubArray( 1 ) );

            color[0].set( 255, 255, 255, 255 );

            float     rowCoordDelta    = size.y / ( float )( numRows - 1 );
            float     columnCoordDelta = size.x / ( float )( numColumns - 1 );

            float     rowTexDelta      = 1.0F / ( float )( numRows - 1 );
            float     columnTexDelta   = 1.0F / ( float )( numColumns - 1 );

            osg::vec3 pos              = origin;
            osg::vec2 tex( 0.0F, 0.0F );
            int       vi = 0;
            for( r = 0; r < numRows; ++r )
            {
                pos.x = origin.x;
                tex.x = 0.0F;
                for( c = 0; c < numColumns; ++c )
                {
                    v[vi].set( pos.x, pos.y, pos.z );
                    pos.x += columnCoordDelta;
                    tex.x += columnTexDelta;
                    ++vi;
                }
                pos.y += rowCoordDelta;
                tex.y += rowTexDelta;
            }

            geometry->setVertexArray( &v );
            geometry->setColorArray( &color, osg::Array::BIND_OVERALL );

            for( r = 0; r < numRows - 1; ++r )
            {
                osg::DrawElementsUShort& drawElements =
                    *( new osg::DrawElementsUShort( GL_TRIANGLE_STRIP,
                                                    2 * numColumns ) );
                geometry->addPrimitiveSet( &drawElements );
                int ei = 0;
                for( c = 0; c < numColumns; ++c )
                {
                    drawElements[ei++] = ( r + 1 ) * numColumns + c;
                    drawElements[ei++] = ( r )*numColumns + c;
                }
            }

            geometry->setInitialBound( osg::box( origin, origin + size ) );

            geode->addDrawable( geometry );

            scene->addChild( geode );
        }
    }

    std::cout << "done." << std::endl;

    return scene;
}

class TestSupportOperation : public osg::GraphicsOperation
{
    public:

        TestSupportOperation() :
            osg::Referenced( true ),
            osg::GraphicsOperation( "TestSupportOperation",
                                    false ),
            _supported( true ),
            _errorMessage()
        {
        }

        virtual void
        operator()( osg::GraphicsContext* gc )
        {
            std::lock_guard<std::mutex> lock( _mutex );

            osg::GLExtensions* gl2ext = gc->getState()->get<osg::GLExtensions>();
            if( gl2ext )
            {
                if( !gl2ext->isGlslSupported )
                {
                    _supported    = false;
                    _errorMessage = "ERROR: GLSL not supported by OpenGL driver.";
                }

                GLint numVertexTexUnits = 0;
                glGetIntegerv( GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &numVertexTexUnits );
                if( numVertexTexUnits <= 0 )
                {
                    _supported = false;
                    _errorMessage =
                        "ERROR: vertex texturing not supported by OpenGL driver.";
                }
            }
            else
            {
                _supported    = false;
                _errorMessage = "ERROR: GLSL not supported.";
            }
        }

        std::mutex  _mutex;
        bool        _supported;
        std::string _errorMessage;
};

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
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

    osg::Node*        node = createScene();

    // add model to viewer.
    viewer.setSceneData( node );

    viewer.setUpViewInWindow( 1'000, 100, 640, 480 );

    osg::ref_ptr<TestSupportOperation> testSupportOperation = new TestSupportOperation;
#if 0
    // temporarily commenting out as its causing the viewer to crash... no clue yet to why
    viewer.setRealizeOperation(testSupportOperation.get());
#endif
    // create the windows and run the threads.
    viewer.realize();

    if( !testSupportOperation->_supported )
    {
        osg::notify( osg::WARN ) << testSupportOperation->_errorMessage << std::endl;

        return 1;
    }
    return viewer.run();
}
