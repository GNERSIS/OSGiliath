/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgmultitexture example application
 */
#include <iostream>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "damaged_helmet.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer       viewer;

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Node> rootnode = osgDB::readRefNodeFiles( arguments );

    // if not loaded assume no arguments passed in, try use default mode instead.
    if( !rootnode )
    {
        rootnode = osgDB::readRefNodeFile( "damaged_helmet.glb" );
    }

    if( !rootnode )
    {
        osg::notify( osg::NOTICE )
            << "Please specify a model filename on the command line." << std::endl;
        return 1;
    }

    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile( "Images/reflect.rgb" );
    if( image )
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage( image );

        osg::StateSet* stateset = new osg::StateSet;
        stateset->setTextureAttributeAndModes( 1, texture, osg::StateAttribute::ON );

        rootnode->setStateSet( stateset );
    }
    else
    {
        osg::notify( osg::NOTICE )
            << "unable to load reflect map, model will not be mutlitextured"
            << std::endl;
    }

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize( rootnode );

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode );

    // create the windows and run the threads.
    viewer.realize();

    for( unsigned int contextID = 0;
         contextID < osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts();
         ++contextID )
    {
        osg::GLExtensions* textExt = osg::GLExtensions::Get( contextID, false );
        if( textExt )
        {
            if( !textExt->isMultiTexturingSupported )
            {
                std::cout << "Warning: multi-texturing not supported by OpenGL drivers, "
                             "unable to run application."
                          << std::endl;
                return 1;
            }
        }
    }
    return viewer.run();
}
