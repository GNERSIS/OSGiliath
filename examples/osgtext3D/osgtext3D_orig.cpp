/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgtext3D_orig example application
 */
#include <iostream>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/Shape.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/Material.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgText/Text3D>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>
#include <sstream>

// create text which sits in 3D space such as would be inserted into a normal model
osg::Group*
create3DText( const osg::vec3& center,
              float            radius )
{

    osg::Geode*      geode = new osg::Geode;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Examples of how to set up axis/orientation alignments
    //

    float            characterSize  = radius * 0.2F;
    float            characterDepth = characterSize * 0.2F;

    osg::vec3        pos( center.x - radius * .5F,
                          center.y - radius * .5F,
                          center.z - radius * .5F );

    osgText::Text3D* text1 = new osgText::Text3D;
    text1->setFont( "fonts/arial.ttf" );
    text1->setCharacterSize( characterSize );
    text1->setCharacterDepth( characterDepth );
    text1->setPosition( pos );
    text1->setDrawMode( osgText::Text3D::TEXT | osgText::Text3D::BOUNDINGBOX );
    text1->setAxisAlignment( osgText::Text3D::XY_PLANE );
    text1->setText( "XY_PLANE" );
    geode->addDrawable( text1 );

    osgText::Text3D* text2 = new osgText::Text3D;
    text2->setFont( "fonts/times.ttf" );
    text2->setCharacterSize( characterSize );
    text2->setCharacterDepth( characterDepth );
    text2->setPosition( pos );
    text2->setDrawMode( osgText::Text3D::TEXT | osgText::Text3D::BOUNDINGBOX );
    text2->setAxisAlignment( osgText::Text3D::YZ_PLANE );
    text2->setText( "YZ_PLANE" );
    geode->addDrawable( text2 );

    osgText::Text3D* text3 = new osgText::Text3D;
    text3->setFont( "fonts/dirtydoz.ttf" );
    text3->setCharacterSize( characterSize );
    text3->setCharacterDepth( characterDepth );
    text3->setPosition( pos );
    text3->setDrawMode( osgText::Text3D::TEXT | osgText::Text3D::BOUNDINGBOX );
    text3->setAxisAlignment( osgText::Text3D::XZ_PLANE );
    text3->setText( "XZ_PLANE" );
    geode->addDrawable( text3 );

    osg::ref_ptr<osgText::Style> style = new osgText::Style;
    osg::ref_ptr<osgText::Bevel> bevel = new osgText::Bevel;
    bevel->roundedBevel2( 0.25 );
    style->setBevel( bevel.get() );
    style->setWidthRatio( 0.4F );

    osgText::Text3D* text7 = new osgText::Text3D;
    text7->setFont( "fonts/times.ttf" );
    text7->setStyle( style.get() );
    text7->setCharacterSize( characterSize );
    text7->setCharacterDepth( characterSize * 0.2F );
    text7->setPosition( center - osg::vec3( 0.0, 0.0, 0.6 ) );
    text7->setDrawMode( osgText::Text3D::TEXT | osgText::Text3D::BOUNDINGBOX );
    text7->setAxisAlignment( osgText::Text3D::SCREEN );
    text7->setCharacterSizeMode( osgText::Text3D::OBJECT_COORDS );
    text7->setText( "CharacterSizeMode OBJECT_COORDS (default)" );
    geode->addDrawable( text7 );

    osg::ShapeDrawable* shape =
        new osg::ShapeDrawable( new osg::Sphere( center, characterSize * 0.2F ) );
    // GL_LIGHTING removed: not in core profile
    geode->addDrawable( shape );

    osg::Group* rootNode = new osg::Group;
    rootNode->addChild( geode );

    osg::Material* front = new osg::Material;
    front->setAlpha( osg::Material::FRONT_AND_BACK, 1 );
    front->setAmbient( osg::Material::FRONT_AND_BACK, osg::vec4( 0.2, 0.2, 0.2, 1.0 ) );
    front->setDiffuse( osg::Material::FRONT_AND_BACK, osg::vec4( .0, .0, 1.0, 1.0 ) );
    rootNode->getOrCreateStateSet()->setAttributeAndModes( front );

    return rootNode;
}

int
main_orig( int    argc,
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

    osg::vec3         center( 0.0F, 0.0F, 0.0F );
    float             radius = 1.0F;

    osg::Group*       root   = new osg::Group;
    root->addChild( create3DText( center, radius ) );

    viewer.setSceneData( root );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );

    viewer.addEventHandler( new osgViewer::ThreadingHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( new osgViewer::StatsHandler );

    viewer.run();

    return 0;
}
