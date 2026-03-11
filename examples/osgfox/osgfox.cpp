#include <iostream>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    // Static pose — animation requires skeletal skinning, not yet supported
    auto                model = osgDB::readRefNodeFile( "fox.glb" );
    if( !model )
    {
        std::cerr << "Failed to load fox.glb" << std::endl;
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
                node = osgDB::readRefNodeFile( "fox.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer;
    viewer.setSceneData( model );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    viewer.getCameraManipulator()->setHomePosition( osg::dvec3( 150.0, 80.0, 150.0 ),
                                                    osg::dvec3( 0.0, 40.0, 0.0 ),
                                                    osg::dvec3( 0.0, 1.0, 0.0 ) );
    return viewer.run();
}
