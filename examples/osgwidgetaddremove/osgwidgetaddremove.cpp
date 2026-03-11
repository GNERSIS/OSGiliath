// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetaddremove.cpp 45 2008-04-23 16:46:11Z cubicool $

#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgWidget/Box>
#include <osgWidget/Label>
#include <osgWidget/Table>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>

const unsigned int MASK_2D = 0XF0'00'00'00;

class ABCWidget : public osgWidget::Label
{
    public:

        ABCWidget( const std::string& label ) :
            osgWidget::Label( "",
                              label )
        {
            setFont( "fonts/Vera.ttf" );
            setFontSize( 20 );
            setCanFill( true );
            setShadow( 0.08F );
            addSize( 10.0F, 10.0F );
        }
};

class Button : public osgWidget::Label
{
    public:

        Button( const std::string& label ) :
            osgWidget::Label( "",
                              label )
        {
            setFont( "fonts/Vera.ttf" );
            setFontSize( 30 );
            setColor( 0.8F, 0.2F, 0.2F, 0.8F );
            setCanFill( true );
            setShadow( 0.1F );
            setEventMask( osgWidget::EVENT_MASK_MOUSE_CLICK );
            addSize( 20.0F, 20.0F );
        }

        // NOTE! I need to make it clearer than Push/Release can happen so fast that
        // the changes you make aren't visible with your refresh rate. Throttling state
        // changes and what-have-you on mousePush/mouseRelease/etc. is going to be
        // annoying...

        virtual bool
        mousePush( double,
                   double,
                   const osgWidget::WindowManager* )
        {
            addColor( 0.2F, 0.2F, 0.2F, 0.0F );

            return true;
        }

        virtual bool
        mouseRelease( double,
                      double,
                      const osgWidget::WindowManager* )
        {
            addColor( -0.2F, -0.2F, -0.2F, 0.0F );

            return true;
        }
};

class AddRemove : public osgWidget::Box
{
        osg::ref_ptr<osgWidget::Window> _win1;

    public:

        AddRemove() :
            osgWidget::Box( "buttons",
                            osgWidget::Box::VERTICAL ),
            _win1( new osgWidget::Box( "win1",
                                       osgWidget::Box::VERTICAL ) )
        {
            addWidget( new Button( "Add Widget" ) );
            addWidget( new Button( "Remove Widget" ) );

            // Take special note here! Not only do the Button objects have their
            // own overridden methods for changing the color, but they have attached
            // callbacks for doing the work with local data.
            getByName( "Widget_1" )
                ->addCallback( new osgWidget::Callback( &AddRemove::handlePressAdd,
                                                        this,
                                                        osgWidget::EVENT_MOUSE_PUSH ) );

            getByName( "Widget_2" )
                ->addCallback( new osgWidget::Callback( &AddRemove::handlePressRemove,
                                                        this,
                                                        osgWidget::EVENT_MOUSE_PUSH ) );
        }

        virtual void
        managed( osgWidget::WindowManager* wm )
        {
            osgWidget::Box::managed( wm );

            _win1->setOrigin( 250.0F, 0.0F );

            wm->addChild( _win1.get() );
        }

        bool
        handlePressAdd( osgWidget::Event& /*ev*/ )
        {
            static unsigned int num = 0;

            std::stringstream   ss;

            ss << "a random widget " << num;

            _win1->addWidget( new ABCWidget( ss.str() ) );

            num++;

            return true;
        }

        bool
        handlePressRemove( osgWidget::Event& /*ev*/ )
        {
            // TODO: Temporary hack!
            const osgWidget::Box::Vector& v = _win1->getObjects();

            if( !v.size() )
            {
                return false;
            }

            osgWidget::Widget* w = _win1->getObjects()[v.size() - 1].get();

            _win1->removeWidget( w );

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
        new osgWidget::WindowManager( &viewer, 1280.0F, 1024.0F, MASK_2D );

    osgWidget::Box* buttons = new AddRemove();

    wm->addChild( buttons );

    return createExample( viewer, wm );
}
