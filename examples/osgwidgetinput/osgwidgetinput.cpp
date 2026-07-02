// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetinput.cpp 50 2008-05-06 05:06:36Z cubicool $

#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>
#include <osgWidget/Box.hpp>
#include <osgWidget/Input.hpp>
#include <osgWidget/ViewerEventHandlers.hpp>
#include <osgWidget/WindowManager.hpp>

const unsigned int MASK_2D = 0XF0'00'00'00;

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

    osgViewer::Viewer         viewer;

    osgWidget::WindowManager* wm =
        new osgWidget::WindowManager( &viewer,
                                      1280.0F,
                                      1024.0F,
                                      MASK_2D,
                                      osgWidget::WindowManager::WM_PICK_DEBUG );

    osgWidget::Box*   box   = new osgWidget::Box( "vbox", osgWidget::Box::VERTICAL );
    osgWidget::Input* input = new osgWidget::Input( "input", "", 50 );

    input->setFont( "fonts/VeraMono.ttf" );
    input->setFontColor( 0.0F, 0.0F, 0.0F, 1.0F );
    input->setFontSize( 15 );
    input->setYOffset( input->calculateBestYOffset( "y" ) );
    input->setSize( 400.0F, input->getText()->getCharacterHeight() );

    box->addWidget( input );
    box->setOrigin( 200.0F, 200.0F );

    wm->addChild( box );

    viewer.setUpViewInWindow( 1'000, 100, 640, 480 );

    osg::Camera* camera = wm->createParentOrthoCamera();

    viewer.addEventHandler( new osgWidget::MouseHandler( wm ) );
    viewer.addEventHandler( new osgWidget::KeyboardHandler( wm ) );
    viewer.addEventHandler( new osgWidget::ResizeHandler( wm, camera ) );
    viewer.addEventHandler( new osgWidget::CameraSwitchHandler( wm, camera ) );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler() );

    wm->resizeAllWindows();

    viewer.setSceneData( camera );
    return viewer.run();
}
