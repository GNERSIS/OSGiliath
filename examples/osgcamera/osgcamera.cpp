/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgcamera example application
 */
#include <iostream>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/manipulators/AnimationPathManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

class ModelHandler : public osgGA::GUIEventHandler
{
    public:

        ModelHandler() :
            _position( 0 )
        {
        }

        typedef std::vector<std::string> Filenames;
        Filenames                        _filenames;
        unsigned int                     _position;

        void
        add( const std::string& filename )
        {
            _filenames.push_back( filename );
        }

        bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter&      aa )
        {
            osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>( &aa );
            if( !viewer )
            {
                return false;
            }

            if( _filenames.empty() )
            {
                return false;
            }

            switch( ea.getEventType() )
            {
                case( osgGA::GUIEventAdapter::KEYUP ) :
                    {
                        if( ea.getKey() == 'l' )
                        {
                            osg::ref_ptr<osg::Node> model =
                                osgDB::readRefNodeFile( _filenames[_position] );
                            ++_position;
                            if( _position >= _filenames.size() )
                            {
                                _position = 0;
                            }

                            if( model.valid() )
                            {
                                viewer->setSceneData( model.get() );
                            }

                            return true;
                        }
                    }
                default :
                    break;
            }

            return false;
        }

        bool _done;
};

void
singleWindowMultipleCameras( osgViewer::Viewer& viewer )
{
    osg::GraphicsContext::WindowingSystemInterface* wsi =
        osg::GraphicsContext::getWindowingSystemInterface();
    if( !wsi )
    {
        osg::notify( osg::NOTICE )
            << "Error, no WindowSystemInterface available, cannot create windows."
            << std::endl;
        return;
    }

    unsigned int                           width, height;
    osg::GraphicsContext::ScreenIdentifier main_screen_id;

    main_screen_id.readDISPLAY();
    main_screen_id.setUndefinedScreenDetailsToDefaultScreen();
    wsi->getScreenResolution( main_screen_id, width, height );

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x                                         = 1'000;
    traits->y                                         = 100;
    traits->width                                     = 640;
    traits->height                                    = 480;
    traits->windowDecoration                          = true;
    traits->doubleBuffer                              = true;
    traits->sharedContext                             = 0;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();

    width  = traits->width;
    height = traits->height;

    osg::ref_ptr<osg::GraphicsContext> gc =
        osg::GraphicsContext::createGraphicsContext( traits.get() );
    if( gc.valid() )
    {
        osg::notify( osg::INFO )
            << "  GraphicsWindow has been created successfully." << std::endl;

        // need to ensure that the window is cleared make sure that the complete window
        // is set the correct colour rather than just the parts of the window that are
        // under the camera's viewports
        gc->setClearColor( osg::vec4( 0.2F, 0.2F, 0.6F, 1.0F ) );
        gc->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }
    else
    {
        osg::notify( osg::NOTICE )
            << "  GraphicsWindow has not been created successfully." << std::endl;
    }

    unsigned int numCameras       = 2;
    double       aspectRatioScale = 1.0;    ///(double)numCameras;
    for( unsigned int i = 0; i < numCameras; ++i )
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext( gc.get() );
        camera->setViewport( new osg::Viewport( ( i * width ) / numCameras,
                                                ( i * height ) / numCameras,
                                                width / numCameras,
                                                height / numCameras ) );
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer( buffer );
        camera->setReadBuffer( buffer );

        viewer.addSlave( camera.get(),
                         osg::dmat4(),
                         osg::scale( aspectRatioScale, 1.0, 1.0 ) );
    }
}

void
multipleWindowMultipleCameras( osgViewer::Viewer& viewer,
                               bool               multipleScreens )
{
    osg::GraphicsContext::WindowingSystemInterface* wsi =
        osg::GraphicsContext::getWindowingSystemInterface();
    if( !wsi )
    {
        osg::notify( osg::NOTICE )
            << "Error, no WindowSystemInterface available, cannot create windows."
            << std::endl;
        return;
    }

    unsigned int                           width, height;
    osg::GraphicsContext::ScreenIdentifier main_screen_id;

    main_screen_id.readDISPLAY();
    main_screen_id.setUndefinedScreenDetailsToDefaultScreen();
    wsi->getScreenResolution( main_screen_id, width, height );

    unsigned int numCameras       = 6;
    double       aspectRatioScale = ( double )numCameras;
    double       translate_x      = double( numCameras ) - 1;
    for( unsigned int i = 0; i < numCameras; ++i, translate_x -= 2.0 )
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits =
            new osg::GraphicsContext::Traits;
        traits->screenNum        = multipleScreens ? i / 3 : 0;
        traits->x                = 1'000 + ( i * 640 ) / numCameras;
        traits->y                = 100;
        traits->width            = 640 / numCameras - 1;
        traits->height           = 480;
        traits->windowDecoration = true;
        traits->doubleBuffer     = true;
        traits->sharedContext    = 0;
        traits->readDISPLAY();
        traits->setUndefinedScreenDetailsToDefaultScreen();

        osg::ref_ptr<osg::GraphicsContext> gc =
            osg::GraphicsContext::createGraphicsContext( traits.get() );
        if( gc.valid() )
        {
            osg::notify( osg::INFO )
                << "  GraphicsWindow has been created successfully." << std::endl;
        }
        else
        {
            osg::notify( osg::NOTICE )
                << "  GraphicsWindow has not been created successfully." << std::endl;
        }

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext( gc.get() );
        camera->setViewport( new osg::Viewport( 0, 0, width / numCameras, height ) );
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer( buffer );
        camera->setReadBuffer( buffer );

        viewer.addSlave( camera.get(),
                         osg::scale( aspectRatioScale, 1.0, 1.0 ) *
                             osg::translate( translate_x, 0.0, 0.0 ),
                         osg::dmat4() );
    }
}

class EnableVBOVisitor : public osg::DualModeVisitor
{
    public:

        EnableVBOVisitor() :
            osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
        {
        }

        void
        apply( osg::Geode& geode )
        {
            for( unsigned int i = 0; i < geode.getNumDrawables(); ++i )
            {
                osg::Geometry* geom = geode.getDrawable( i )->asGeometry();
                if( geom )
                {
                    osg::notify( osg::NOTICE ) << "Enabling VBO" << std::endl;
                    geom->setUseVertexBufferObjects( true );
                }
            }
        }
};

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    if( argc < 2 )
    {
        std::cout << argv[0] << ": requires filename argument." << std::endl;
        return 1;
    }

    unsigned int numRepeats = 2;
    if( arguments.read( "--repeat", numRepeats ) ||
        arguments.read( "-r", numRepeats ) ||
        arguments.read( "--repeat" ) ||
        arguments.read( "-r" ) )
    {

        bool                    sharedModel = arguments.read( "--shared" );
        bool                    enableVBO   = arguments.read( "--vbo" );

        osg::ref_ptr<osg::Node> model;
        if( sharedModel )
        {
            model = osgDB::readRefNodeFiles( arguments );
            if( !model )
            {
                return 0;
            }

            if( enableVBO )
            {
                EnableVBOVisitor enableVBOs;
                model->accept( enableVBOs );
            }
        }

        osgViewer::Viewer::ThreadingModel threadingModel =
            osgViewer::Viewer::AutomaticSelection;
        while( arguments.read( "-s" ) )
        {
            threadingModel = osgViewer::Viewer::SingleThreaded;
        }
        while( arguments.read( "-g" ) )
        {
            threadingModel = osgViewer::Viewer::CullDrawThreadPerContext;
        }
        while( arguments.read( "-d" ) )
        {
            threadingModel = osgViewer::Viewer::DrawThreadPerContext;
        }
        while( arguments.read( "-c" ) )
        {
            threadingModel = osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext;
        }

        for( unsigned int i = 0; i < numRepeats; ++i )
        {
            osg::notify( osg::NOTICE )
                << "+++++++++++++ New viewer ++++++++++++" << std::endl;

            {
                osg::ref_ptr<osg::Node> node;
                if( sharedModel )
                {
                    node = model;
                }
                else
                {
                    node = osgDB::readRefNodeFiles( arguments );
                    if( !node )
                    {
                        return 0;
                    }

                    if( enableVBO )
                    {
                        EnableVBOVisitor enableVBOs;
                        node->accept( enableVBOs );
                    }
                }

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
                        return osg::headlessCapture( node.get(),
                                                     headlessOutput,
                                                     640,
                                                     480 )
                                 ? 0
                                 : 1;
                    }
                }

                osgViewer::Viewer viewer;

                viewer.setThreadingModel( threadingModel );
                viewer.setSceneData( node.get() );
                viewer.run();
            }

            osg::notify( osg::NOTICE )
                << "------------ Viewer ended ----------" << std::endl
                << std::endl;
        }
        return 0;
    }

    std::string                                   pathfile;
    osg::ref_ptr<osgGA::AnimationPathManipulator> apm = 0;
    while( arguments.read( "-p", pathfile ) )
    {
        apm = new osgGA::AnimationPathManipulator( pathfile );
        if( !apm.valid() || !( apm->valid() ) )
        {
            apm = 0;
        }
    }

    // Headless capture: early check before viewer construction (main path)
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

    while( arguments.read( "-s" ) )
    {
        viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );
    }
    while( arguments.read( "-g" ) )
    {
        viewer.setThreadingModel( osgViewer::Viewer::CullDrawThreadPerContext );
    }
    while( arguments.read( "-d" ) )
    {
        viewer.setThreadingModel( osgViewer::Viewer::DrawThreadPerContext );
    }
    while( arguments.read( "-c" ) )
    {
        viewer.setThreadingModel(
            osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext
        );
    }

    bool         limitNumberOfFrames = false;
    unsigned int maxFrames           = 10;
    while( arguments.read( "--run-till-frame-number", maxFrames ) )
    {
        limitNumberOfFrames = true;
    }

    // alternative viewer window setups.
    while( arguments.read( "-1" ) )
    {
        singleWindowMultipleCameras( viewer );
    }
    while( arguments.read( "-2" ) )
    {
        multipleWindowMultipleCameras( viewer, false );
    }
    while( arguments.read( "-3" ) )
    {
        multipleWindowMultipleCameras( viewer, true );
    }

    if( apm.valid() )
    {
        viewer.setCameraManipulator( apm.get() );
    }
    else
    {
        viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    }

    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::ThreadingHandler );

    std::string configfile;
    while( arguments.read( "--config", configfile ) )
    {
        osg::notify( osg::NOTICE )
            << "Trying to read config file " << configfile << std::endl;
        osg::ref_ptr<osg::Object> object = osgDB::readRefObjectFile( configfile );
        osgViewer::View*          view = dynamic_cast<osgViewer::View*>( object.get() );
        if( view )
        {
            osg::notify( osg::NOTICE ) << "Read config file successfully" << std::endl;
        }
        else
        {
            osg::notify( osg::NOTICE )
                << "Failed to read config file : " << configfile << std::endl;
            return 1;
        }
    }

    while( arguments.read( "--write-config", configfile ) )
    {
        osgDB::writeObjectFile( viewer, configfile );
    }

    if( arguments.read( "-m" ) )
    {
        ModelHandler* modelHandler = new ModelHandler;
        for( int i = 1; i < arguments.argc(); ++i )
        {
            modelHandler->add( arguments[i] );
        }

        viewer.addEventHandler( modelHandler );
    }
    else
    {
        // load the scene.
        osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );

        if( !loadedModel )
        {
            loadedModel = osgDB::readRefNodeFile( "duck.glb" );
        }

        if( !loadedModel )
        {
            std::cout << argv[0] << ": No data loaded." << std::endl;
            return 1;
        }

        viewer.setSceneData( loadedModel );
    }

    viewer.realize();

    unsigned int numFrames = 0;
    while( !viewer.done() && !( limitNumberOfFrames && numFrames >= maxFrames ) )
    {
        viewer.frame();
        ++numFrames;
    }

    return 0;
}
