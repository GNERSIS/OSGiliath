/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgviewerSDL example application
 */
// (C) 2005 Mike Weiblen http://mew.cx/ released under the OSGPL.
// Simple example using GLUT to create an OpenGL window and OSG for rendering.
// Derived from osgGLUTsimple.cpp and osgkeyboardmouse.cpp

#include "SDL.h"

#include <iostream>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

bool
convertEvent( SDL_Event&         event,
              osgGA::EventQueue& eventQueue )
{
    switch( event.type )
    {

        case SDL_MOUSEMOTION :
            eventQueue.mouseMotion( event.motion.x, event.motion.y );
            return true;

        case SDL_MOUSEBUTTONDOWN :
            eventQueue.mouseButtonPress( event.button.x,
                                         event.button.y,
                                         event.button.button );
            return true;

        case SDL_MOUSEBUTTONUP :
            eventQueue.mouseButtonRelease( event.button.x,
                                           event.button.y,
                                           event.button.button );
            return true;

        case SDL_KEYUP :
            eventQueue.keyRelease(
                ( osgGA::GUIEventAdapter::KeySymbol )event.key.keysym.unicode
            );
            return true;

        case SDL_KEYDOWN :
            eventQueue.keyPress(
                ( osgGA::GUIEventAdapter::KeySymbol )event.key.keysym.unicode
            );
            return true;

        case SDL_VIDEORESIZE :
            eventQueue.windowResize( 0, 0, event.resize.w, event.resize.h );
            return true;

        default :
            break;
    }
    return false;
}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    if( argc < 2 )
    {
        std::cout << argv[0] << ": requires filename argument." << std::endl;
        return 1;
    }

    // init SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        fprintf( stderr, "Unable to init SDL: %s\n", SDL_GetError() );
        exit( 1 );
    }
    atexit( SDL_Quit );

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFile( argv[1] );
    if( !loadedModel )
    {
        std::cout << argv[0] << ": No data loaded." << std::endl;
        return 1;
    }

    // Starting with SDL 1.2.10, passing in 0 will use the system's current resolution.
    unsigned int       windowWidth  = 0;
    unsigned int       windowHeight = 0;

    // Passing in 0 for bitdepth also uses the system's current bitdepth. This works
    // before 1.2.10 too.
    unsigned int       bitDepth = 0;

    // If not linked to SDL 1.2.10+, then we must use hardcoded values
    const SDL_version* linked_version = SDL_Linked_Version();
    if( linked_version->major == 1 && linked_version->minor == 2 )
    {
        if( linked_version->patch < 10 )
        {
            windowWidth  = 1'280;
            windowHeight = 1'024;
        }
    }

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    // set up the surface to render to
    SDL_Surface* screen =
        SDL_SetVideoMode( windowWidth,
                          windowHeight,
                          bitDepth,
                          SDL_OPENGL | SDL_FULLSCREEN | SDL_RESIZABLE );
    if( screen == NULL )
    {
        std::cerr << "Unable to set " << windowWidth << "x" << windowHeight
                  << " video: %s\n"
                  << SDL_GetError() << std::endl;
        exit( 1 );
    }

    SDL_EnableUNICODE( 1 );

    // If we used 0 to set the fields, query the values so we can pass it to osgViewer
    windowWidth  = screen->w;
    windowHeight = screen->h;

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

    osgViewer::Viewer                               viewer;
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> gw =
        viewer.setUpViewerAsEmbeddedInWindow( 0, 0, windowWidth, windowHeight );

    // set the draw and read buffers up for a double buffered window with rendering going
    // to back buffer
    viewer.getCamera()->setDrawBuffer( GL_BACK );
    viewer.getCamera()->setReadBuffer( GL_BACK );

    viewer.setSceneData( loadedModel.get() );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.realize();

    bool done = false;
    while( !done )
    {
        SDL_Event event;

        while( SDL_PollEvent( &event ) )
        {
            // pass the SDL event into the viewers event queue
            convertEvent( event, *( gw->getEventQueue() ) );

            switch( event.type )
            {

                case SDL_VIDEORESIZE :
                    SDL_SetVideoMode( event.resize.w,
                                      event.resize.h,
                                      bitDepth,
                                      SDL_OPENGL | SDL_RESIZABLE );
                    gw->resized( 0, 0, event.resize.w, event.resize.h );
                    break;

                case SDL_KEYUP :

                    if( event.key.keysym.sym == SDLK_ESCAPE )
                    {
                        done = true;
                    }
                    if( event.key.keysym.sym == 'f' )
                    {
                        SDL_WM_ToggleFullScreen( screen );
                        gw->resized( 0, 0, screen->w, screen->h );
                    }

                    break;

                case SDL_QUIT :
                    done = true;
            }
        }

        if( done )
        {
            continue;
        }

        // draw the new frame
        viewer.frame();

        // Swap Buffers
        SDL_GL_SwapBuffers();
    }

    return 0;
}

/*EOF*/
