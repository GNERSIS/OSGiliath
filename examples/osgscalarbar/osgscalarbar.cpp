/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgscalarbar example application
 */
#include <iostream>
#include <math.h>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/ClearNode.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/nodes/Projection.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/Material.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgSim/ColorRange>
#include <osgSim/ScalarBar>
#include <osgSim/ScalarsToColors>
#include <osgUtil/culling/CullVisitor.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <sstream>

using namespace osgSim;
using osgSim::ScalarBar;

#if defined( _MSC_VER )
// not have to have this pathway for just VS6.0 as its unable to handle the full
// ScalarBar::ScalarPrinter::printScalar scoping.

// Create a custom scalar printer
struct MyScalarPrinter : public ScalarBar::ScalarPrinter
{
        std::string
        printScalar( float scalar )
        {
            std::cout << "In MyScalarPrinter::printScalar" << std::endl;
            if( scalar == 0.0F )
            {
                return ScalarPrinter::printScalar( scalar ) + " Bottom";
            }
            else if( scalar == 0.5F )
            {
                return ScalarPrinter::printScalar( scalar ) + " Middle";
            }
            else if( scalar == 1.0F )
            {
                return ScalarPrinter::printScalar( scalar ) + " Top";
            }
            else
            {
                return ScalarPrinter::printScalar( scalar );
            }
        }
};
#else
// Create a custom scalar printer
struct MyScalarPrinter : public ScalarBar::ScalarPrinter
{
        std::string
        printScalar( float scalar )
        {
            std::cout << "In MyScalarPrinter::printScalar" << std::endl;
            if( scalar == 0.0F )
            {
                return ScalarBar::ScalarPrinter::printScalar( scalar ) + " Bottom";
            }
            else if( scalar == 0.5F )
            {
                return ScalarBar::ScalarPrinter::printScalar( scalar ) + " Middle";
            }
            else if( scalar == 1.0F )
            {
                return ScalarBar::ScalarPrinter::printScalar( scalar ) + " Top";
            }
            else
            {
                return ScalarBar::ScalarPrinter::printScalar( scalar );
            }
        }
};
#endif

osg::Node*
createScalarBar( bool vertical )
{
#if 1
    // ScalarsToColors* stc = new ScalarsToColors(0.0f,1.0f);
    // ScalarBar* sb = new ScalarBar(2,3,stc,"STC_ScalarBar");

    // Create a custom color set
    std::vector<osg::vec4> cs;
    cs.push_back( osg::vec4( 1.0F, 0.0F, 0.0F, 1.0F ) );    // R
    cs.push_back( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );    // G
    cs.push_back( osg::vec4( 1.0F, 1.0F, 0.0F, 1.0F ) );    // G
    cs.push_back( osg::vec4( 0.0F, 0.0F, 1.0F, 1.0F ) );    // B
    cs.push_back( osg::vec4( 0.0F, 1.0F, 1.0F, 1.0F ) );    // R

    ColorRange* cr = new ColorRange( 0.0F, 1.0F, cs );
    ScalarBar*  sb =
        new ScalarBar( 20,
                       11,
                       cr,
                       vertical ? "Vertical" : "Horizontal",
                       vertical ? ScalarBar::VERTICAL : ScalarBar::HORIZONTAL,
                       0.1F,
                       new MyScalarPrinter );
    sb->setScalarPrinter( new MyScalarPrinter );

    if( !vertical )
    {
        sb->setPosition( osg::vec3( 0.5F, 0.5F, 0 ) );
    }

    return sb;
#else
    ScalarBar*                sb = new ScalarBar;
    ScalarBar::TextProperties tp;
    tp._fontFile = "fonts/times.ttf";

    sb->setTextProperties( tp );

    return sb;
#endif
}

osg::Node*
createScalarBar_HUD()
{
    osgSim::ScalarBar*                geode = new osgSim::ScalarBar;
    osgSim::ScalarBar::TextProperties tp;
    tp._fontFile = "fonts/times.ttf";
    geode->setTextProperties( tp );
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
    stateset->setRenderBinDetails( 11, "RenderBin" );

    osg::MatrixTransform* modelview = new osg::MatrixTransform;
    modelview->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    osg::dmat4 matrix(
        osg::scale( 1'000, 1'000, 1'000 ) * osg::translate( 120, 10, 0 )
    );    // I've played with these values a lot and it seems to work, but I have no idea
          // why
    modelview->setMatrix( matrix );
    modelview->addChild( geode );

    osg::Projection* projection = new osg::Projection;
    projection->setMatrix(
        osg::ortho2D( 0.0, ( double )1'280, 0.0, ( double )1'024 )
    );                    // or whatever the OSG window res is
    projection->addChild( modelview );

    return projection;    // make sure you delete the return sb line
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

    osgViewer::Viewer viewer;

    osg::Group*       group = new osg::Group;

    group->addChild( createScalarBar_HUD() );

    // rotate the scalar from XY plane to XZ so we see them viewing it with the default
    // camera manipulators that look along the Y axis, with Z up.
    osg::MatrixTransform* transform = new osg::MatrixTransform;
    group->addChild( transform );
    transform->setMatrix( osg::rotate( osg::radians( 90.0 ), 1.0, 0.0, 0.0 ) );
    transform->addChild( createScalarBar( true ) );
    transform->addChild( createScalarBar( false ) );

    // add model to viewer.
    viewer.setSceneData( group );
    return viewer.run();
}
