// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetcanvas.cpp 33 2008-04-04 19:03:12Z cubicool $

#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgWidget/Canvas.hpp>
#include <osgWidget/Util.hpp>
#include <osgWidget/WindowManager.hpp>

const unsigned int MASK_2D = 0XF0'00'00'00;

bool
colorWidgetEnter( osgWidget::Event& event )
{
    event.getWidget()->addColor( 0.5F, 0.2F, 0.3F, 0.0F );

    // osgWidget::warn() << "WIDGET mouseEnter " << event.getWidget()->getName() <<
    // std::endl;

    return false;
}

bool
colorWidgetLeave( osgWidget::Event& event )
{
    event.getWidget()->addColor( -0.5F, -0.2F, -0.3F, 0.0F );

    // osgWidget::warn() << "WIDGET mouseLeave" << std::endl;

    return true;
}

bool
windowMouseOver( osgWidget::Event& /*event*/ )
{
    // osgWidget::XYCoord xy = event.getWindow()->localXY(event.x, event.y);
    //  osgWidget::warn() << "WINDOW " << xy.x << " - " << xy.y << std::endl;

    return true;
}

bool
widgetMouseOver( osgWidget::Event& /*event*/ )
{
    // osgWidget::XYCoord xy = event.getWidget()->localXY(event.x, event.y);
    // osgWidget::warn() << "WIDGET mouseOver " << xy.x << " - " << xy.y << std::endl;

    return true;
}

osgWidget::Widget*
createWidget( const std::string&       name,
              osgWidget::color_type    col,
              osgWidget::Widget::Layer layer )
{
    osgWidget::Widget* widget = new osgWidget::Widget( name, 200.0F, 200.0F );

    widget->setEventMask( osgWidget::EVENT_ALL );
    widget->addCallback( new osgWidget::Callback( &colorWidgetEnter,
                                                  osgWidget::EVENT_MOUSE_PUSH ) );
    widget->addCallback( new osgWidget::Callback( &colorWidgetLeave,
                                                  osgWidget::EVENT_MOUSE_RELEASE ) );
    widget->addCallback( new osgWidget::Callback( &colorWidgetEnter,
                                                  osgWidget::EVENT_MOUSE_ENTER ) );
    widget->addCallback( new osgWidget::Callback( &colorWidgetLeave,
                                                  osgWidget::EVENT_MOUSE_LEAVE ) );
    widget->addCallback( new osgWidget::Callback( &widgetMouseOver,
                                                  osgWidget::EVENT_MOUSE_OVER ) );
    widget->setColor( col, col, col, 0.5F );
    widget->setLayer( layer );

    return widget;
}

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

    osgWidget::Canvas* canvas = new osgWidget::Canvas( "canvas" );

    canvas->addCallback( new osgWidget::Callback( &windowMouseOver,
                                                  osgWidget::EVENT_MOUSE_OVER ) );
    canvas->attachMoveCallback();
    canvas->attachRotateCallback();
    canvas->attachScaleCallback();

    canvas->addWidget( createWidget( "w1", 0.2F, osgWidget::Widget::LAYER_LOW ),
                       0.0F,
                       0.0F );

    canvas->addWidget( createWidget( "w2", 0.4F, osgWidget::Widget::LAYER_MIDDLE ),
                       200.0F,
                       0.0F );

    canvas->addWidget( createWidget( "w3", 0.6F, osgWidget::Widget::LAYER_HIGH ),
                       400.0F,
                       0.0F );

    // Add a child and then resize it relatively to the size of the parent Window.
    osgWidget::Widget* relWidget = new osgWidget::Widget( "relative" );

    relWidget->setLayer( osgWidget::Widget::LAYER_LOW, 1 );
    relWidget->setCoordinateMode( osgWidget::Widget::CM_RELATIVE );
    relWidget->setSize( 0.2F, 0.2F );
    relWidget->setColor( 0.5F, 0.5F, 0.1F, 0.9F );

    osgWidget::warn() << canvas->getWidth() << std::endl;

    canvas->addWidget( relWidget, 0.4F, 0.4F );

    relWidget->addOrigin( 0.1F, 0.1F );
    relWidget->addSize( 0.2F, 0.2F );

    canvas->resize();

    // Finally, add the whole thing to the WindowManager.
    wm->addChild( canvas );

    return osgWidget::createExample( viewer, wm );
}

/*
int main(int argc, char** argv) {
    osgViewer::Viewer viewer;

    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        1024.0f,
        MASK_2D,
        osgWidget::WindowManager::WM_PICK_DEBUG
    );

    osgWidget::Canvas* canvas = new osgWidget::Canvas("canvas");

    canvas->addWidget(new osgWidget::Widget("spacer", 2.0f, 300.0f), 1280.0f, 0.0f);

    canvas->setOrigin(0.0f, 300.0f);

    wm->addChild(canvas);

    return osgWidget::createExample(viewer, wm);
}
*/
