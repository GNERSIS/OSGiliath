// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetframe.cpp 40 2008-04-11 14:05:11Z cubicool $

#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgWidget/Box.hpp>
#include <osgWidget/Frame.hpp>
#include <osgWidget/Util.hpp>
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

    osgWidget::Frame* frame =
        osgWidget::Frame::createSimpleFrame( "frame", 32.0F, 32.0F, 300.0F, 300.0F );

    osgWidget::Frame* frame2 = osgWidget::Frame::createSimpleFrameFromTheme(
        "frameTheme",
        osgDB::readRefImageFile( "osgWidget/theme-1.png" ),
        300.0F,
        300.0F,
        osgWidget::Frame::FRAME_ALL
    );
    frame2->setPosition( 300, 100, 0 );
    frame2->getBackground()->setColor( 1.0F, 1.0F, 1.0F, 0.0F );

    osgWidget::Frame* frame22 = osgWidget::Frame::createSimpleFrameFromTheme(
        "frameTheme",
        osgDB::readRefImageFile( "osgWidget/theme-2.png" ),
        300.0F,
        300.0F,
        osgWidget::Frame::FRAME_ALL
    );
    frame22->setPosition( 300, 100, 0 );
    frame22->getBackground()->setColor( 1.0F, 1.0F, 1.0F, 0.0F );

    osgWidget::Frame* frame3 = osgWidget::Frame::createSimpleFrameFromTheme(
        "frameTheme",
        osgDB::readRefImageFile( "osgWidget/theme-2.png" ),
        300.0F,
        300.0F,
        osgWidget::Frame::FRAME_ALL
    );
    frame3->setPosition( 300, 100, 0 );
    frame3->getBackground()->setColor( 0.0F, 0.0F, 0.0F, 1.0F );

    osgWidget::Table* table  = new osgWidget::Table( "table", 2, 2 );
    osgWidget::Box*   bottom = new osgWidget::Box( "panel", osgWidget::Box::HORIZONTAL );

    table->addWidget( new osgWidget::Widget( "red", 300.0F, 300.0F ), 0, 0 );
    table->addWidget( new osgWidget::Widget( "white", 300.0F, 300.0F ), 0, 1 );
    table->addWidget( new osgWidget::Widget( "yellow", 300.0F, 300.0F ), 1, 0 );
    table->addWidget( new osgWidget::Widget( "purple", 300.0F, 300.0F ), 1, 1 );
    table->getByRowCol( 0, 0 )->setColor( 1.0F, 0.0F, 0.0F, 1.0F );
    table->getByRowCol( 0, 1 )->setColor( 1.0F, 1.0F, 1.0F, 1.0F );
    table->getByRowCol( 1, 0 )->setColor( 1.0F, 1.0F, 0.0F, 1.0F );
    table->getByRowCol( 1, 1 )->setColor( 1.0F, 0.0F, 1.0F, 1.0F );
    table->getByRowCol( 0, 0 )->setMinimumSize( 100.0F, 100.0F );
    table->getByRowCol( 0, 1 )->setMinimumSize( 100.0F, 100.0F );
    table->getByRowCol( 1, 0 )->setMinimumSize( 100.0F, 100.0F );
    table->getByRowCol( 1, 1 )->setMinimumSize( 100.0F, 100.0F );

    frame->setWindow( table );

    // Give frame some nice textures.
    // TODO: This has to be done after setWindow(); wtf?
    frame->getBackground()->setColor( 0.0F, 0.0F, 0.0F, 0.0F );

    osgWidget::Widget* l = frame->getBorder( osgWidget::Frame::BORDER_LEFT );
    osgWidget::Widget* r = frame->getBorder( osgWidget::Frame::BORDER_RIGHT );
    osgWidget::Widget* t = frame->getBorder( osgWidget::Frame::BORDER_TOP );
    osgWidget::Widget* b = frame->getBorder( osgWidget::Frame::BORDER_BOTTOM );

    l->setImage( "osgWidget/border-left.tga", true );
    r->setImage( "osgWidget/border-right.tga", true );
    t->setImage( "osgWidget/border-top.tga", true );
    b->setImage( "osgWidget/border-bottom.tga", true );

    l->setTexCoordWrapVertical();
    r->setTexCoordWrapVertical();
    t->setTexCoordWrapHorizontal();
    b->setTexCoordWrapHorizontal();

    // Create the bottom, XArt panel.
    osgWidget::Widget* left   = new osgWidget::Widget( "left", 512.0F, 256.0F );
    osgWidget::Widget* center = new osgWidget::Widget( "center", 256.0F, 256.0F );
    osgWidget::Widget* right  = new osgWidget::Widget( "right", 512.0F, 256.0F );

    left->setImage( "osgWidget/panel-left.tga", true );
    center->setImage( "osgWidget/panel-center.tga", true );
    right->setImage( "osgWidget/panel-right.tga", true );

    center->setTexCoordWrapHorizontal();

    bottom->addWidget( left );
    bottom->addWidget( center );
    bottom->addWidget( right );
    bottom->getBackground()->setColor( 0.0F, 0.0F, 0.0F, 0.0F );
    bottom->setOrigin( 0.0F, 1024.0F - 256.0F );

    // Add everything to the WindowManager.
    wm->addChild( frame );
    wm->addChild( frame2 );
    wm->addChild( frame22 );
    wm->addChild( frame3 );
    wm->addChild( bottom );

    return osgWidget::createExample( viewer, wm );
}
