// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetmenu.cpp 66 2008-07-14 21:54:09Z cubicool $

#include <iostream>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgWidget/Box>
#include <osgWidget/Label>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>

// For now this is just an example, but osgWidget::Menu will later be it's own Window.
// I just wanted to get this out there so that people could see it was possible.

const unsigned int MASK_2D = 0XF0'00'00'00;
const unsigned int MASK_3D = 0X0F'00'00'00;

struct ColorLabel : public osgWidget::Label
{
        ColorLabel( const char* label ) :
            osgWidget::Label( "",
                              "" )
        {
            setFont( "fonts/Vera.ttf" );
            setFontSize( 14 );
            setFontColor( 1.0F, 1.0F, 1.0F, 1.0F );
            setColor( 0.3F, 0.3F, 0.3F, 1.0F );
            addHeight( 18.0F );
            setCanFill( true );
            setLabel( label );
            setEventMask( static_cast<int>( osgWidget::EVENT_MOUSE_PUSH ) |
                          osgWidget::EVENT_MASK_MOUSE_MOVE );
        }

        bool
        mousePush( double,
                   double,
                   const osgWidget::WindowManager* )
        {
            return true;
        }

        bool
        mouseEnter( double,
                    double,
                    const osgWidget::WindowManager* )
        {
            setColor( 0.6F, 0.6F, 0.6F, 1.0F );

            return true;
        }

        bool
        mouseLeave( double,
                    double,
                    const osgWidget::WindowManager* )
        {
            setColor( 0.3F, 0.3F, 0.3F, 1.0F );

            return true;
        }
};

class ColorLabelMenu : public ColorLabel
{
        osg::ref_ptr<osgWidget::Window> _window;

    public:

        ColorLabelMenu( const char* label ) :
            ColorLabel( label )
        {
            _window = new osgWidget::Box( std::string( "Menu_" ) + label,
                                          osgWidget::Box::VERTICAL,
                                          true );

            _window->addWidget( new ColorLabel( "Open Some Stuff" ) );
            _window->addWidget( new ColorLabel( "Do It Now" ) );
            _window->addWidget( new ColorLabel( "Hello, How Are U?" ) );
            _window->addWidget( new ColorLabel( "Hmmm..." ) );
            _window->addWidget( new ColorLabel( "Option 5" ) );

            _window->resize();

            setColor( 0.8F, 0.8F, 0.8F, 0.8F );
        }

        void
        managed( osgWidget::WindowManager* wm )
        {
            osgWidget::Label::managed( wm );

            wm->addChild( _window.get() );

            _window->hide();
        }

        void
        positioned()
        {
            osgWidget::Label::positioned();

            _window->setOrigin( getX(), getHeight() );
            _window->resize( getWidth() );
        }

        bool
        mousePush( double,
                   double,
                   const osgWidget::WindowManager* )
        {
            if( !_window->isVisible() )
            {
                _window->show();
            }

            else
            {
                _window->hide();
            }

            return true;
        }

        bool
        mouseLeave( double,
                    double,
                    const osgWidget::WindowManager* )
        {
            if( !_window->isVisible() )
            {
                setColor( 0.8F, 0.8F, 0.8F, 0.8F );
            }

            return true;
        }
};

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

    osgWidget::Window* menu = new osgWidget::Box( "menu", osgWidget::Box::HORIZONTAL );

    menu->addWidget( new ColorLabelMenu( "Pick me!" ) );
    menu->addWidget( new ColorLabelMenu( "No, wait, pick me!" ) );
    menu->addWidget( new ColorLabelMenu( "Don't pick them..." ) );
    menu->addWidget( new ColorLabelMenu( "Grarar!?!" ) );

    wm->addChild( menu );

    menu->getBackground()->setColor( 1.0F, 1.0F, 1.0F, 0.0F );
    menu->resizePercent( 100.0F );

    osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile( "duck.glb" );

    if( model )
    {
        model->setNodeMask( MASK_3D );
    }

    return osgWidget::createExample( viewer, wm, model.get() );
}
