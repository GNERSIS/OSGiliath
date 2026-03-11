// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetshader.cpp 28 2008-03-26 15:26:48Z cubicool $

#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgWidget/Box>
#include <osgWidget/StyleManager>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>

const unsigned int MASK_2D = 0XF0'00'00'00;

const std::string& STYLE1  = "color 0 0 0 128\n"
                             "padding 5\n";

const std::string& STYLE2  = "color 1.0 0.5 0.0\n";

const std::string& STYLE3  = "fill true\n";

const std::string& STYLE4  = "pos 100.0 100.0\n"
                             "size 600 600\n";

class CustomStyled : public osgWidget::Widget
{
};

class CustomStyle : public osgWidget::Style
{
        virtual bool
        applyStyle( osgWidget::Widget* w,
                    osgWidget::Reader /*r*/ )
        {
            CustomStyled* cs = dynamic_cast<CustomStyled*>( w );

            if( !cs )
            {
                return false;
            }

            osgWidget::warn() << "Here, okay." << std::endl;

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

    osgWidget::Box*    box     = new osgWidget::Box( "box", osgWidget::Box::VERTICAL );

    osgWidget::Widget* widget1 = new osgWidget::Widget( "w1", 200.0F, 200.0F );
    osgWidget::Widget* widget2 = new osgWidget::Widget( "w2", 100.0F, 100.0F );
    osgWidget::Widget* widget3 = new osgWidget::Widget( "w3", 0.0F, 0.0F );
    // CustomStyled*      cs      = new CustomStyled();

    // Yep.
    wm->getStyleManager()->addStyle( new osgWidget::Style( "widget.style1", STYLE1 ) );
    wm->getStyleManager()->addStyle( new osgWidget::Style( "widget.style2", STYLE2 ) );
    wm->getStyleManager()->addStyle( new osgWidget::Style( "spacer", STYLE3 ) );
    wm->getStyleManager()->addStyle( new osgWidget::Style( "window", STYLE4 ) );
    // wm->getStyleManager()->addStyle(new CustomStyle("widget", ""));

    widget1->setStyle( "widget.style1" );
    widget2->setStyle( "widget.style2" );
    widget3->setStyle( "spacer" );

    box->setStyle( "window" );

    box->addWidget( widget1 );
    box->addWidget( widget2 );
    box->addWidget( widget3 );

    wm->addChild( box );

    // box->resizePercent(0.0f, 100.0f);

    return osgWidget::createExample( viewer, wm );
}
