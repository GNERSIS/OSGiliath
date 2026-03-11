/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgoit example application
 */
#include "DepthPeeling.hpp"
#include "HeatMap.hpp"

#include <iostream>
#include <limits>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/Material.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );
    arguments.getApplicationUsage()->addKeyboardMouseBinding(
        "m",
        "Increase the number of depth peeling layers"
    );
    arguments.getApplicationUsage()->addKeyboardMouseBinding(
        "n",
        "Decrease the number of depth peeling layers"
    );
    arguments.getApplicationUsage()->addKeyboardMouseBinding(
        "l",
        "Toggle display of the individual or composed layer textures"
    );
    arguments.getApplicationUsage()->addKeyboardMouseBinding(
        "p",
        "Increase the layer offset"
    );
    arguments.getApplicationUsage()->addKeyboardMouseBinding(
        "o",
        "Decrease the layer offset"
    );

    // Have the usual viewer
    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "milk_truck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer     viewer( arguments );

    osg::DisplaySettings* displaySettings = new osg::DisplaySettings;
    viewer.setDisplaySettings( displaySettings );

    // Add the stats handler
    viewer.addEventHandler( new osgViewer::StatsHandler );

    // add the help handler
    viewer.addEventHandler(
        new osgViewer::HelpHandler( arguments.getApplicationUsage() )
    );

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // read the dump truck, we will need it twice
    osg::ref_ptr<osg::Node> dt = osgDB::readRefNodeFile( "milk_truck.glb" );

    // display a solid version of the dump truck
    osg::ref_ptr<osg::PositionAttitudeTransform> solidModel =
        new osg::PositionAttitudeTransform;
    solidModel->setPosition( osg::dvec3( 7.0, -2.0, 7.0 ) );
    solidModel->addChild( dt );

    // generate the 3D heatmap surface to display
    osg::ref_ptr<Heatmap> hm = new Heatmap( 30, 30, 10, 30, 30, 1.0, 0.25 );
    float                 data[30][30];
    for( int x = 0; x < 30; ++x )
    {
        for( int y = 0; y < 30; ++y )
        {
            data[y][x] = ( double )rand() / RAND_MAX;
        }
    }
    hm->setData( ( float* )data, 10.0, 1.0, 0.25 );

    // add a transparent version of the truck to the scene also
    osg::ref_ptr<osg::PositionAttitudeTransform> transparentTruck =
        new osg::PositionAttitudeTransform;
    transparentTruck->setPosition( osg::dvec3( 7.0, -25.0, 7.0 ) );

    // set the states of the truck so that it actually appears transparently and nicely
    // lit.
    osg::StateSet* state = transparentTruck->getOrCreateStateSet();
    state->setMode( GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    state->setAttribute( new osg::BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ),
                         osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    osg::Material* material = new osg::Material;
    material->setAmbient( osg::Material::FRONT_AND_BACK,
                          osg::vec4( 0.2F, 0.2F, 0.2F, 0.3F ) );
    material->setDiffuse( osg::Material::FRONT_AND_BACK,
                          osg::vec4( 0.8F, 0.8F, 0.8F, 0.3F ) );
    state->setAttribute( material,
                         osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    ( transparentTruck.get() )->addChild( dt.get() );

    // place the heatmap and a transparent dump truck in the transparent geometry group
    osg::ref_ptr<osg::Group> transparentModel = new osg::Group;
    ( transparentModel.get() )->addChild( hm.get() );
    ( transparentModel.get() )->addChild( transparentTruck.get() );

    // The initial size set to 0, 0. We get a resize event for the right size...
    DepthPeeling* depthPeeling = new DepthPeeling( 0, 0 );
    // the heat map already uses two textures bound to unit 0 and 1, so we can use
    // TexUnit 2 for the peeling
    depthPeeling->setTexUnit( 2 );
    depthPeeling->setSolidScene( solidModel.get() );
    depthPeeling->setTransparentScene( transparentModel.get() );
    viewer.setSceneData( depthPeeling->getRoot() );

    // Add the event handler for the depth peeling stuff
    viewer.addEventHandler( new DepthPeeling::EventHandler( depthPeeling ) );

    // force a resize event, so the DepthPeeling object updates _texWidth and _texHeight
    viewer.realize();
    int                            x, y, width, height;
    osgViewer::ViewerBase::Windows windows;
    viewer.getWindows( windows );
    windows.front()->getWindowRectangle( x, y, width, height );
    viewer.getEventQueue()->windowResize( x, y, width, height );
    return viewer.run();
}
