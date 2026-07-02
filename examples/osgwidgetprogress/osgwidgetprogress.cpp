// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id$

#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgWidget/Canvas.hpp>
#include <osgWidget/Util.hpp>
#include <osgWidget/WindowManager.hpp>

const unsigned int MASK_2D = 0XF0'00'00'00;

struct UpdateProgressNode : public osg::NodeCallback
{
        float start;
        float done;

        UpdateProgressNode() :
            start( 0.0F ),
            done( 5.0F )
        {
        }

        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            const osg::FrameStamp* fs = nv->getFrameStamp();

            float                  t  = fs->getSimulationTime();

            if( start == 0.0F )
            {
                start = t;
            }

            float width   = ( ( t - start ) / done ) * 512.0F;
            float percent = ( width / 512.0F ) * 100.0F;

            if( width < 1.0F || width > 512.0F )
            {
                return;
            }

            osgWidget::Window* window = dynamic_cast<osgWidget::Window*>( node );

            if( !window )
            {
                return;
            }

            osgWidget::Widget* w = window->getByName( "pMeter" );
            osgWidget::Label*  l =
                dynamic_cast<osgWidget::Label*>( window->getByName( "pLabel" ) );

            if( !w || !l )
            {
                return;
            }

            w->setWidth( width );
            w->setTexCoordRegion( 0.0F, 0.0F, width, 64.0F );

            std::ostringstream ss;

            ss << std::round( percent ) << "% Done" << std::endl;

            l->setLabel( ss.str() );
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

    osgWidget::Canvas* canvas   = new osgWidget::Canvas( "canvas" );
    osgWidget::Widget* pOutline = new osgWidget::Widget( "pOutline", 512.0F, 64.0F );
    osgWidget::Widget* pMeter   = new osgWidget::Widget( "pMeter", 0.0F, 64.0F );
    osgWidget::Label*  pLabel   = new osgWidget::Label( "pLabel", "0% Done" );

    pOutline->setImage( "osgWidget/progress-outline.png", true );
    pOutline->setLayer( osgWidget::Widget::LAYER_MIDDLE, 2 );

    pMeter->setImage( "osgWidget/progress-meter.png" );
    pMeter->setColor( 0.7F, 0.1F, 0.1F, 0.7F );
    pMeter->setLayer( osgWidget::Widget::LAYER_MIDDLE, 1 );

    pLabel->setFont( "fonts/VeraMono.ttf" );
    pLabel->setFontSize( 20 );
    pLabel->setFontColor( 1.0F, 1.0F, 1.0F, 1.0F );
    pLabel->setSize( 512.0F, 64.0F );
    pLabel->setLayer( osgWidget::Widget::LAYER_MIDDLE, 3 );

    canvas->setOrigin( 300.0F, 300.0F );
    canvas->addWidget( pMeter, 0.0F, 0.0F );
    canvas->addWidget( pOutline, 0.0F, 0.0F );
    canvas->addWidget( pLabel, 0.0F, 0.0F );
    canvas->getBackground()->setColor( 0.0F, 0.0F, 0.0F, 0.0F );
    canvas->setUpdateCallback( new UpdateProgressNode() );

    wm->addChild( canvas );

    return osgWidget::createExample( viewer, wm, osgDB::readNodeFile( "duck.glb" ) );
}
