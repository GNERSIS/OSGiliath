/**
 * TODO:
 * 1) Change example to use offscreen rendering (pbuffer) so that it becomes a true
 * commandline tool with now windows 2) Make example work with other threading models
 * than SingleThreaded 3) Add support for autocapture to movies
 *
 */
#include <iostream>
#include <osg/core/ArgumentParser.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/KeySwitchMatrixManipulator.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgTerrain/GeometryTechnique>
#include <osgTerrain/Terrain>
#include <osgUtil/intersection/IntersectionVisitor.hpp>
#include <osgUtil/utils/GLObjectsVisitor.hpp>
#include <osgViewer/core/Renderer.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>
#include <sstream>

/** Helper class*/
template<class T>
class FindTopMostNodeOfTypeVisitor : public osg::DualModeVisitor
{
    public:

        FindTopMostNodeOfTypeVisitor() :
            osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
            _foundNode( 0 )
        {
        }

        void
        apply( osg::Node& node )
        {
            T* result = dynamic_cast<T*>( &node );
            if( result )
            {
                _foundNode = result;
            }
            else
            {
                traverse( node );
            }
        }

        T* _foundNode;
};

/** Convenience function*/
template<class T>
T*
findTopMostNodeOfType( osg::Node* node )
{
    if( !node )
    {
        return 0;
    }

    FindTopMostNodeOfTypeVisitor<T> fnotv;
    node->accept( fnotv );

    return fnotv._foundNode;
}

/** Capture the frame buffer and write image to disk*/
class WindowCaptureCallback : public osg::Camera::DrawCallback
{
    public:

        WindowCaptureCallback( GLenum             readBuffer,
                               const std::string& name ) :
            _readBuffer( readBuffer ),
            _fileName( name )
        {
            _image = new osg::Image;
        }

        virtual void
        operator()( osg::RenderInfo& renderInfo ) const
        {
            glReadBuffer( _readBuffer );

            std::lock_guard<std::mutex> lock( _mutex );
            osg::GraphicsContext*       gc = renderInfo.getState()->getGraphicsContext();
            if( gc->getTraits() )
            {
                GLenum pixelFormat;

                if( gc->getTraits()->alpha )
                {
                    pixelFormat = GL_RGBA;
                }
                else
                {
                    pixelFormat = GL_RGB;
                }

                int width  = gc->getTraits()->width;
                int height = gc->getTraits()->height;

                std::cout << "Capture: size=" << width << "x" << height << ", format="
                          << ( pixelFormat == GL_RGBA ? "GL_RGBA" : "GL_RGB" )
                          << std::endl;

                _image->readPixels( 0, 0, width, height, pixelFormat, GL_UNSIGNED_BYTE );
            }

            if( !_fileName.empty() )
            {
                std::cout << "Writing to: " << _fileName << std::endl;
                osgDB::writeImageFile( *_image, _fileName );
            }
        }

    protected:

        GLenum                   _readBuffer;
        std::string              _fileName;
        osg::ref_ptr<osg::Image> _image;
        mutable std::mutex       _mutex;
};

/** Do Culling only while loading PagedLODs*/
class CustomRenderer : public osgViewer::Renderer
{
    public:

        CustomRenderer( osg::Camera* camera ) :
            osgViewer::Renderer( camera ),
            _cullOnly( true )
        {
        }

        /** Set flag to omit drawing in renderingTraversals */
        void
        setCullOnly( bool on )
        {
            _cullOnly = on;
        }

        virtual void
        operator()( osg::GraphicsContext* /*context*/ )
        {
            if( _graphicsThreadDoesCull )
            {
                if( _cullOnly )
                {
                    cull();
                }
                else
                {
                    cull_draw();
                }
            }
        }

        virtual void
        cull()
        {
            osgUtil::SceneView* sceneView = _sceneView[0].get();
            if( !sceneView || _done )
            {
                return;
            }

            updateSceneView( sceneView );

            osgViewer::View* view = dynamic_cast<osgViewer::View*>( _camera->getView() );
            if( view )
            {
                sceneView->setFusionDistance( view->getFusionDistanceMode(),
                                              view->getFusionDistanceValue() );
            }

            sceneView->inheritCullSettings( *( sceneView->getCamera() ) );
            sceneView->cull();
        }

        bool _cullOnly;
};

//===============================================================
// MAIN
//
int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser    arguments( &argc, argv );
    osg::ApplicationUsage* usage = arguments.getApplicationUsage();

    usage->setApplicationName( arguments.getApplicationName() );
    usage->setDescription( arguments.getApplicationName() +
                           " loads a model, sets a camera position and automatically "
                           "captures screenshot to disk" );
    usage->setCommandLineUsage( arguments.getApplicationName() +
                                " [options] filename ..." );
    usage->addCommandLineOption(
        "--camera <lat> <lon> <alt> <heading> <incline> <roll>",
        "Specify camera position for image capture. Angles are specified in degrees and "
        "altitude in meters above sealevel (e.g. --camera 55 10 300000 0 30 0)"
    );
    usage->addCommandLineOption( "--filename",
                                 "Filename for the captured image",
                                 "autocapture.jpg" );
    usage->addCommandLineOption( "--db-threads",
                                 "Number of DatabasePager threads to use",
                                 "2" );
    usage->addCommandLineOption(
        "--active",
        "Use active rendering instead of passive / lazy rendering"
    );
    usage->addCommandLineOption( "--pbuffer",
                                 "Render into a pbuffer, not into a window" );

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

    if( arguments.argc() <= 1 )
    {
        arguments.getApplicationUsage()->write(
            std::cout,
            osg::ApplicationUsage::COMMAND_LINE_OPTION
        );
        return 1;
    }

    // Get user specified number of DatabaseThreads
    int dbThreads = 2;
    arguments.read( "--db-threads", dbThreads );
    if( dbThreads < 1 )
    {
        dbThreads = 1;
    }

    osg::DisplaySettings::instance()->setNumOfDatabaseThreadsHint( dbThreads );

    // Get user specified file name
    std::string fileName( "autocapture.jpg" );
    arguments.read( "--filename", fileName );

    // Rendering mode is passive by default
    bool activeMode = false;
    if( arguments.read( "--active" ) )
    {
        activeMode = true;
    }

    bool use_pbuffer = false;
    if( arguments.read( "--pbuffer" ) )
    {
        if( !activeMode )
        {
            use_pbuffer = true;
        }
        else
        {
            osg::notify( osg::NOTICE )
                << "ignoring --pbuffer because --active specified on commandline"
                << std::endl;
        }
    }
    if( use_pbuffer )
    {
        osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
        osg::ref_ptr<osg::GraphicsContext::Traits> traits =
            new osg::GraphicsContext::Traits( ds );

        if( viewer.getCamera()->getGraphicsContext() &&
            viewer.getCamera()->getGraphicsContext()->getTraits() )
        {
            // use viewer settings for window size
            osg::ref_ptr<const osg::GraphicsContext::Traits> src_traits =
                viewer.getCamera()->getGraphicsContext()->getTraits();
            traits->screenNum  = src_traits->screenNum;
            traits->displayNum = src_traits->displayNum;
            traits->hostName   = src_traits->hostName;
            traits->width      = src_traits->width;
            traits->height     = src_traits->height;
            traits->red        = src_traits->red;
            traits->green      = src_traits->green;
            traits->blue       = src_traits->blue;
            traits->alpha      = src_traits->alpha;
            traits->depth      = src_traits->depth;
            traits->pbuffer    = true;
        }
        else
        {
            // viewer would use fullscreen size (unknown here) pbuffer will use 4096
            // x4096 (or best available)
            traits->width   = 1 << 12;
            traits->height  = 1 << 12;
            traits->pbuffer = true;
            traits->readDISPLAY();
            traits->setUndefinedScreenDetailsToDefaultScreen();
        }
        osg::ref_ptr<osg::GraphicsContext> pbuffer =
            osg::GraphicsContext::createGraphicsContext( traits.get() );
        if( pbuffer.valid() )
        {
            osg::notify( osg::NOTICE )
                << "Pixel buffer has been created successfully." << std::endl;
            osg::ref_ptr<osg::Camera> camera = new osg::Camera( *viewer.getCamera() );
            camera->setGraphicsContext( pbuffer.get() );
            camera->setViewport(
                new osg::Viewport( 0, 0, traits->width, traits->height )
            );
            GLenum buffer = pbuffer->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT;
            camera->setDrawBuffer( buffer );
            camera->setReadBuffer( buffer );
            viewer.setCamera( camera.get() );
        }
        else
        {
            osg::notify( osg::NOTICE )
                << "Pixel buffer has not been created successfully." << std::endl;
        }
    }

    // Read camera settings for screenshot
    double lat              = 50;
    double lon              = 10;
    double alt              = 2'000;
    double heading          = 0;
    double incline          = 45;
    double roll             = 0;
    bool   camera_specified = false;
    if( arguments.read( "--camera", lat, lon, alt, heading, incline, roll ) )
    {
        camera_specified = true;
        lat              = osg::radians( lat );
        lon              = osg::radians( lon );
        heading          = osg::radians( heading );
        incline          = osg::radians( incline );
        roll             = osg::radians( roll );
    }

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );
    if( !loadedModel )
    {
        std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if( arguments.errors() )
    {
        arguments.writeErrorMessages( std::cout );
        return 1;
    }

    // Setup specified camera
    if( camera_specified )
    {
        osg::CoordinateSystemNode* csn =
            findTopMostNodeOfType<osg::CoordinateSystemNode>( loadedModel.get() );
        if( !csn )
        {
            return 1;
        }

        // Compute eye point in world coordinates
        osg::dvec3 eye;
        csn->getEllipsoidModel()
            ->convertLatLongHeightToXYZ( lat, lon, alt, eye.x, eye.y, eye.z );

        // Build matrix for computing target vector
        osg::dmat4 target_matrix = osg::rotate( -heading,
                                                osg::dvec3( 1, 0, 0 ),
                                                -lat,
                                                osg::dvec3( 0, 1, 0 ),
                                                lon,
                                                osg::dvec3( 0, 0, 1 ) );

        // Compute tangent vector ...
        osg::dvec3 tangent = target_matrix * osg::dvec3( 0, 0, 1 );

        // Compute non-inclined, non-rolled up vector ...
        osg::dvec3 up( eye );
        up = osg::normalize( up );

        // Incline by rotating the target- and up vector around the tangent/up-vector
        // cross-product ...
        osg::dvec3 up_cross_tangent = up ^ tangent;
        osg::dmat4 incline_matrix   = osg::rotate( incline, up_cross_tangent );
        osg::dvec3 target           = incline_matrix * tangent;

        // Roll by rotating the up vector around the target vector ...
        osg::dmat4 roll_matrix = incline_matrix * osg::rotate( roll, target );
        up                     = roll_matrix * up;

        viewer.getCamera()->setViewMatrixAsLookAt( eye, eye + target, up );
    }
    else
    {
        // Only add camera manipulators if camera is not specified
        camera_specified = false;
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator =
            new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1',
                                                    "Trackball",
                                                    new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2',
                                                    "Flight",
                                                    new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3',
                                                    "Drive",
                                                    new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4',
                                                    "Terrain",
                                                    new osgGA::TerrainManipulator() );

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    // Optimize DatabasePager for auto-capture
    osgDB::DatabasePager* pager = viewer.getDatabasePager();
    pager->setDoPreCompile( false );

    // Install custom renderer
    osg::ref_ptr<CustomRenderer> customRenderer =
        new CustomRenderer( viewer.getCamera() );
    viewer.getCamera()->setRenderer( customRenderer.get() );

    // Override threading model
    viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );

    // Set the final SceneData to show
    viewer.setSceneData( loadedModel.get() );

    // Realize GUI
    viewer.realize();

    //--- Load PageLOD tiles ---

    // Initiate the first PagedLOD request
    viewer.frame();

    osg::Timer_t beforeLoadTick = osg::Timer::instance()->tick();

    // Keep updating and culling until full level of detail is reached
    while( !viewer.done() && pager->getRequestsInProgress() )
    {
        // std::cout <<pager->getRequestsInProgress()<<" ";
        viewer.updateTraversal();
        viewer.renderingTraversals();
    }
    // std::cout<<std::endl;

    osg::Timer_t afterLoadTick = osg::Timer::instance()->tick();
    std::cout << "Load and Compile time = "
              << osg::Timer::instance()->delta_s( beforeLoadTick, afterLoadTick )
              << " seconds" << std::endl;

    // Do cull and draw to render the scene correctly
    customRenderer->setCullOnly( false );

    //--- Capture the image!!! ---
    if( !activeMode )
    {
        // Add the WindowCaptureCallback now that we have full resolution
        GLenum buffer =
            viewer.getCamera()->getGraphicsContext()->getTraits()->doubleBuffer
                ? GL_BACK
                : GL_FRONT;
        viewer.getCamera()->setFinalDrawCallback(
            new WindowCaptureCallback( buffer, fileName )
        );

        osg::Timer_t beforeRenderTick = osg::Timer::instance()->tick();

        // Do rendering with capture callback
        viewer.renderingTraversals();

        osg::Timer_t afterRenderTick = osg::Timer::instance()->tick();
        std::cout << "Rendering time = "
                  << osg::Timer::instance()->delta_s( beforeRenderTick, afterRenderTick )
                  << " seconds" << std::endl;

        return 0;
    }
    else
    {
        return viewer.run();
    }
}
