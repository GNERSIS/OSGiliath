// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgettable.cpp 43 2008-04-17 03:40:05Z cubicool $

#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgWidget/Table>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>

const unsigned int MASK_2D = 0XF0'00'00'00;

// This examples demonstrates the use of an osgWidget::Table, which differs from a Box in
// many ways. First and foremost, a Table's size is initially known, whereas a Box can be
// dynamically added to. Secondly, a table is matrix Layout, with both vertical and
// horizontal placement cells. A Box, on the other hand, can only be vertical or
// horizontal.

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

    osgWidget::Table* table = new osgWidget::Table( "table", 3, 3 );

    // Here we create our "cells" manually, though it will often be convenient to
    // do so algorithmically. Also, notice how we set the text name of each widget to
    // correspond with it's "index" in the table. This is merely a convenience, which
    // we use later...
    table->addWidget( new osgWidget::Widget( "0, 0", 100.0F, 25.0F ), 0, 0 );
    table->addWidget( new osgWidget::Widget( "0, 1", 100.0F, 25.0F ), 0, 1 );
    table->addWidget( new osgWidget::Widget( "0, 2", 100.0F, 75.0F ), 0, 2 );

    table->addWidget( new osgWidget::Widget( "1, 0", 200.0F, 45.0F ), 1, 0 );
    table->addWidget( new osgWidget::Widget( "1, 1", 200.0F, 45.0F ), 1, 1 );
    table->addWidget( new osgWidget::Widget( "1, 2", 200.0F, 45.0F ), 1, 2 );

    table->addWidget( new osgWidget::Widget( "2, 0", 300.0F, 65.0F ), 2, 0 );
    table->addWidget( new osgWidget::Widget( "2, 1", 300.0F, 65.0F ), 2, 1 );
    table->addWidget( new osgWidget::Widget( "2, 2", 300.0F, 65.0F ), 2, 2 );

    table->getBackground()->setColor( 0.0F, 0.0F, 0.5F, 1.0F );
    table->attachMoveCallback();

    // Use a hackish method of setting the spacing for all Widgets.
    for( osgWidget::Table::Iterator i = table->begin(); i != table->end(); i++ )
    {
        i->get()->setPadding( 1.0F );
    }

    // Now we fetch the very first 0, 0 Widget in the table using an awkward method.
    // This is merely one way to fetch a Widget from a Window, there are many others.
    // The osgWidget::Window::getByName interface will be very handy in scripting
    // languages where users will want to retrieve handles to existing Windows using a
    // useful textual name, such as "MainGUIParent" or something.
    table->getByName( "0, 0" )->setAlignHorizontal( osgWidget::Widget::HA_LEFT );
    table->getByName( "0, 0" )->setAlignVertical( osgWidget::Widget::VA_BOTTOM );
    table->getByName( "0, 0" )->setPadLeft( 50.0F );
    table->getByName( "0, 0" )->setPadTop( 3.0F );

    // Change the colors a bit to differentiate this row from the others.
    table->getByName( "2, 0" )->setColor( 1.0F,
                                          0.0F,
                                          0.0F,
                                          1.0F,
                                          osgWidget::Widget::LOWER_LEFT );
    table->getByName( "2, 1" )->setColor( 1.0F, 0.0F, 0.0F, 0.5F );
    table->getByName( "2, 2" )->setColor( 1.0F, 0.0F, 0.0F, 0.5F );

    wm->addChild( table );

    return createExample( viewer, wm );
}
