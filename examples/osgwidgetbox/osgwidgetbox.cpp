// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetbox.cpp 59 2008-05-15 20:55:31Z cubicool $

// NOTE: You'll find this example very similar to osgwidgetwindow. However, here we
// demonstrate a bit of subclassing of Widget so that we can respond to events
// such as mouseEnter and mouseLeave. We also demonstrate the use of padding, though
// fill and alignment should be working too.

#include <osg/core/io_utils.hpp>
#include <osg/maths/compat.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgWidget/Box>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>

const unsigned int MASK_2D = 0XF0'00'00'00;
const unsigned int MASK_3D = 0X0F'00'00'00;

struct ColorWidget : public osgWidget::Widget
{
        ColorWidget() :
            osgWidget::Widget( "",
                               256.0F,
                               256.0F )
        {
            setEventMask( osgWidget::EVENT_ALL );
        }

        bool
        mouseEnter( double,
                    double,
                    const osgWidget::WindowManager* )
        {
            addColor( -osgWidget::Color( 0.4F, 0.4F, 0.4F, 0.0F ) );

            // osgWidget::warn() << "enter: " << getColor() << std::endl;

            return true;
        }

        bool
        mouseLeave( double,
                    double,
                    const osgWidget::WindowManager* )
        {
            addColor( osgWidget::Color( 0.4F, 0.4F, 0.4F, 0.0F ) );

            // osgWidget::warn() << "leave: " << getColor() << std::endl;

            return true;
        }

        bool
        mouseOver( double x,
                   double y,
                   const osgWidget::WindowManager* )
        {

            osgWidget::Color c = getImageColorAtPointerXY( x, y );

            if( c.a < 0.001F )
            {
                // osgWidget::warn() << "Transparent Pixel: " << x << " " << y <<
                // std::endl;

                return false;
            }
            return true;
        }

        bool
        keyUp( int /*key*/,
               int /*keyMask*/,
               osgWidget::WindowManager* )
        {
            // osgWidget::warn() << "..." << key << " - " << keyMask << std::endl;

            return true;
        }
};

osgWidget::Box*
createBox( const std::string&      name,
           osgWidget::Box::BoxType bt )
{
    osgWidget::Box*    box = new osgWidget::Box( name, bt, true );
    osgWidget::Widget* widget1 =
        new osgWidget::Widget( name + "_widget1", 100.0F, 100.0F );
    osgWidget::Widget* widget2 =
        new osgWidget::Widget( name + "_widget2", 100.0F, 100.0F );
    osgWidget::Widget* widget3 = new ColorWidget();

    widget1->setColor( 0.3F, 0.3F, 0.3F, 1.0F );
    widget2->setColor( 0.6F, 0.6F, 0.6F, 1.0F );

    widget3->setImage( "osgWidget/natascha.png" );
    widget3->setTexCoord( 0.0F, 0.0F, osgWidget::Widget::LOWER_LEFT );
    widget3->setTexCoord( 1.0F, 0.0F, osgWidget::Widget::LOWER_RIGHT );
    widget3->setTexCoord( 1.0F, 1.0F, osgWidget::Widget::UPPER_RIGHT );
    widget3->setTexCoord( 0.0F, 1.0F, osgWidget::Widget::UPPER_LEFT );

    box->addWidget( widget1 );
    box->addWidget( widget2 );
    box->addWidget( widget3 );

    return box;
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
                node = osgDB::readRefNodeFile( "avocado.glb" );
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

    wm->setPointerFocusMode( osgWidget::WindowManager::PFM_SLOPPY );

    osgWidget::Window* box1 = createBox( "HBOX", osgWidget::Box::HORIZONTAL );
    osgWidget::Window* box2 = createBox( "VBOX", osgWidget::Box::VERTICAL );
    osgWidget::Window* box3 = createBox( "HBOX2", osgWidget::Box::HORIZONTAL );
    osgWidget::Window* box4 = createBox( "VBOX2", osgWidget::Box::VERTICAL );

    box1->getBackground()->setColor( 1.0F, 0.0F, 0.0F, 0.8F );
    box1->attachMoveCallback();

    box2->getBackground()->setColor( 0.0F, 1.0F, 0.0F, 0.8F );
    box2->attachMoveCallback();

    box3->getBackground()->setColor( 0.0F, 0.0F, 1.0F, 0.8F );
    box3->attachMoveCallback();

    wm->addChild( box1 );
    wm->addChild( box2 );
    wm->addChild( box3 );
    wm->addChild( box4 );

    box4->hide();

    osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile( "avocado.glb" );

    model->setNodeMask( MASK_3D );

    return osgWidget::createExample( viewer, wm, model.get() );
}
