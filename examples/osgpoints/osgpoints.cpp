/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgpoints example application
 */
#include <iostream>
#include <osg/maths/compat.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/Point.hpp>
#include <osg/state/PolygonMode.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
    public:

        KeyboardEventHandler( osg::StateSet* stateset ) :
            _stateset( stateset )
        {
            _point = new osg::Point;
            _point->setDistanceAttenuation( osg::vec3( 0.0, 0.0000, 0.05F ) );
            _stateset->setAttribute( _point.get() );
        }

        virtual bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& )
        {
            switch( ea.getEventType() )
            {
                case( osgGA::GUIEventAdapter::KEYDOWN ) :
                    {
                        if( ea.getKey() ==
                            '+' ||
                            ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Add )
                        {
                            changePointSize( 1.0F );
                            return true;
                        }
                        else if( ea.getKey() ==
                                 '-' ||
                                 ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Subtract )
                        {
                            changePointSize( -1.0F );
                            return true;
                        }
                        else if( ea.getKey() == '<' )
                        {
                            changePointAttenuation( 1.1F );
                            return true;
                        }
                        else if( ea.getKey() == '>' )
                        {
                            changePointAttenuation( 1.0F / 1.1F );
                            return true;
                        }
                        break;
                    }
                default :
                    break;
            }
            return false;
        }

        float
        getPointSize() const
        {
            return _point->getSize();
        }

        void
        setPointSize( float psize )
        {
            if( psize > 0.0 )
            {
                _point->setSize( psize );
            }
            std::cout << "Point size " << psize << std::endl;
        }

        void
        changePointSize( float delta )
        {
            setPointSize( getPointSize() + delta );
        }

        void
        changePointAttenuation( float scale )
        {
            _point->setDistanceAttenuation( _point->getDistanceAttenuation() * scale );
        }

        osg::ref_ptr<osg::StateSet> _stateset;
        osg::ref_ptr<osg::Point>    _point;
};

int
main( int    argc,
      char** argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(
        arguments.getApplicationName()
    );
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " example provides an interactive viewer for visualising point clouds.."
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] filename ..."
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );
    arguments.getApplicationUsage()->addCommandLineOption( "--sprites",
                                                           "Point sprites." );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--points",
        "Sets the polygon mode to GL_POINT for front and back faces."
    );

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

    bool              shader = false;
    while( arguments.read( "--shader" ) )
    {
        shader = true;
    }

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    bool usePointSprites = false;
    while( arguments.read( "--sprites" ) )
    {
        usePointSprites = true;
    };

    bool forcePointMode = false;
    while( arguments.read( "--points" ) )
    {
        forcePointMode = true;
    };

    if( arguments.argc() <= 1 )
    {
        arguments.getApplicationUsage()->write(
            std::cout,
            osg::ApplicationUsage::COMMAND_LINE_OPTION
        );
        return 1;
    }

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );

    // if no model has been successfully loaded report failure.
    if( !loadedModel )
    {
        std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
        return 1;
    }

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize( loadedModel.get() );

    // set the scene to render
    viewer.setSceneData( loadedModel.get() );

    osg::StateSet* stateset = loadedModel->getOrCreateStateSet();
    if( usePointSprites )
    {
        /// Setup cool blending
        osg::BlendFunc* fn = new osg::BlendFunc();
        stateset->setAttributeAndModes( fn, osg::StateAttribute::ON );

        /// The texture for the sprites
        osg::Texture2D* tex = new osg::Texture2D();
        tex->setImage( osgDB::readRefImageFile( "Images/particle.rgb" ) );
        stateset->setTextureAttributeAndModes( 0, tex, osg::StateAttribute::ON );
    }

    if( forcePointMode )
    {
        /// Set polygon mode to GL_POINT
        osg::PolygonMode* pm =
            new osg::PolygonMode( osg::PolygonMode::Face::FRONT_AND_BACK,
                                  osg::PolygonMode::Mode::POINT );
        stateset->setAttributeAndModes( pm,
                                        osg::StateAttribute::ON |
                                            osg::StateAttribute::OVERRIDE );
    }

    // register the handler for modifying the point size
    viewer.addEventHandler(
        new KeyboardEventHandler( viewer.getCamera()->getOrCreateStateSet() )
    );

    if( shader )
    {
        osg::StateSet* ss = loadedModel->getOrCreateStateSet();

        ///////////////////////////////////////////////////////////////////
        // vertex shader using just vec4 coefficients
        char           vertexShaderSource[] =
            "#version 460 core\n"
            "layout(location = 0) in vec4 osg_Vertex;\n"
            "layout(location = 3) in vec4 osg_Color;\n"
            "uniform mat4 osg_ModelViewProjectionMatrix;\n"
            "out vec4 vertColor;\n"
            "void main(void) \n"
            "{ \n"
            "    vertColor = osg_Color;\n"
            "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
            "}\n";

        osg::Program* program = new osg::Program;
        ss->setAttribute( program );

        osg::Shader* vertex_shader =
            new osg::Shader( osg::Shader::VERTEX, vertexShaderSource );
        program->addShader( vertex_shader );

        //////////////////////////////////////////////////////////////////
        // fragment shader
        //
        char         fragmentShaderSource[] = "#version 460 core\n"
                                              "in vec4 vertColor;\n"
                                              "out vec4 fragColor;\n"
                                              "void main(void) \n"
                                              "{ \n"
                                              "    fragColor = vertColor; \n"
                                              "}\n";

        osg::Shader* fragment_shader =
            new osg::Shader( osg::Shader::FRAGMENT, fragmentShaderSource );
        program->addShader( fragment_shader );
    }
    return viewer.run();
}
