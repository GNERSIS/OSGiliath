// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetshader.cpp 28 2008-03-26 15:26:48Z cubicool $

#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgWidget/Canvas>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>

const unsigned int MASK_2D = 0XF0'00'00'00;

osgWidget::Widget*
createWidget( const std::string&       name,
              osgWidget::color_type    col,
              osgWidget::Widget::Layer layer )
{
    osgWidget::Widget* widget = new osgWidget::Widget( name, 200.0F, 200.0F );

    widget->setColor( col, col, col, 0.2F );
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
        new osgWidget::WindowManager( &viewer, 1280.0F, 1024.0F, MASK_2D );

    osgWidget::Canvas* canvas = new osgWidget::Canvas( "canvas" );

    canvas->attachMoveCallback();
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

    wm->addChild( canvas );

    osg::Program* program = new osg::Program();

    program->addShader(
        osgDB::readRefShaderFile( osg::Shader::VERTEX,
                                  "osgWidget/osgwidgetshader-vert.glsl" )
    );
    program->addShader(
        osgDB::readRefShaderFile( osg::Shader::FRAGMENT,
                                  "osgWidget/osgwidgetshader-frag.glsl" )
    );

    canvas->getGeode()->getOrCreateStateSet()->setAttribute( program );

    return osgWidget::createExample( viewer, wm );
}
