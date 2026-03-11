#include <iostream>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/FirstPersonManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    // Sponza is a gltf with separate files
    auto                model = osgDB::readRefNodeFile( "sponza/Sponza.gltf" );
    if( !model )
    {
        std::cerr << "Failed to load sponza/Sponza.gltf" << std::endl;
        return 1;
    }

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
    viewer.setSceneData( model );
    viewer.setCameraManipulator( new osgGA::FirstPersonManipulator );
    // Sponza atrium: start inside looking down the long axis, Y-up
    viewer.getCameraManipulator()->setHomePosition( osg::dvec3( -5.0, 2.0, 0.0 ),
                                                    osg::dvec3( 5.0, 2.0, 0.0 ),
                                                    osg::dvec3( 0.0, 1.0, 0.0 ) );
    return viewer.run();
}
