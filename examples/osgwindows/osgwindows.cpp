/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgwindows example application
 */
#include <iostream>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser     arguments( &argc, argv );

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles( arguments );

    // if not loaded assume no arguments passed in, try use default mode instead.
    if( !loadedModel )
    {
        loadedModel = osgDB::readRefNodeFile( "duck.glb" );
    }

    // if no model has been successfully loaded report failure.
    if( !loadedModel )
    {
        std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
        return 1;
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
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer;

    int               xoffset = 40;
    int               yoffset = 40;

    // left window + left slave camera
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits =
            new osg::GraphicsContext::Traits;

        traits->x                = 1'000;
        traits->y                = 100;
        traits->width            = 640;
        traits->height           = 480;
        traits->windowDecoration = true;
        traits->doubleBuffer     = true;
        traits->sharedContext    = 0;
        traits->readDISPLAY();
        traits->setUndefinedScreenDetailsToDefaultScreen();

        osg::ref_ptr<osg::GraphicsContext> gc =
            osg::GraphicsContext::createGraphicsContext( traits.get() );

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext( gc.get() );
        camera->setViewport( new osg::Viewport( 0, 0, traits->width, traits->height ) );
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer( buffer );
        camera->setReadBuffer( buffer );

        // add this slave camera to the viewer, with a shift left of the projection
        // matrix
        viewer.addSlave( camera.get(), osg::translate( 1.0, 0.0, 0.0 ), osg::dmat4() );
    }

    // right window + right slave camera
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits =
            new osg::GraphicsContext::Traits;
        traits->x                = 1'000 + 640;
        traits->y                = 100;
        traits->width            = 640;
        traits->height           = 480;
        traits->windowDecoration = true;
        traits->doubleBuffer     = true;
        traits->sharedContext    = 0;
        traits->readDISPLAY();
        traits->setUndefinedScreenDetailsToDefaultScreen();

        osg::ref_ptr<osg::GraphicsContext> gc =
            osg::GraphicsContext::createGraphicsContext( traits.get() );

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext( gc.get() );
        camera->setViewport( new osg::Viewport( 0, 0, traits->width, traits->height ) );
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer( buffer );
        camera->setReadBuffer( buffer );

        // add this slave camera to the viewer, with a shift right of the projection
        // matrix
        viewer.addSlave( camera.get(), osg::translate( -1.0, 0.0, 0.0 ), osg::dmat4() );
    }

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize( loadedModel );

    // set the scene to render
    viewer.setSceneData( loadedModel );
    return viewer.run();
}
