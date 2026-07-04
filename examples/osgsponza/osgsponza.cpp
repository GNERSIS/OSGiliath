#include <iostream>
#include <osg/core/ArgumentParser.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/FirstPersonManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <string>

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    std::string headlessOutput;
    const bool  headless = arguments.read( "--headless", headlessOutput );

    std::string modelPath = "NewSponza_Main_glTF_003.gltf";
    for( int i = 1; i < arguments.argc(); ++i )
    {
        if( !arguments.isOption( i ) )
        {
            modelPath = arguments[i];
            break;
        }
    }

    osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile( modelPath );
    if( !model )
    {
        std::cerr << "Failed to load " << modelPath << std::endl;
        return 1;
    }

    const osg::dvec3 eye( -8.80743, 1.59221947, -0.85825783 );
    const osg::dvec3 center( -7.84353, 1.78652, -0.67626 );
    const osg::dvec3 up( -0.1909, 0.9809, -0.0360 );

    if( headless )
    {
        return osg::headlessCapture(
                   model.get(), headlessOutput, 1920, 1080, eye, center, up
               )
                 ? 0
                 : 1;
    }

    osgViewer::Viewer viewer;
    viewer.setSceneData( model.get() );
    viewer.setCameraManipulator( new osgGA::FirstPersonManipulator );
    viewer.getCameraManipulator()->setHomePosition( eye, center, up );

    return viewer.run();
}
