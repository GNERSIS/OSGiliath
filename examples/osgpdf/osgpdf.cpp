#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>
#include <osgWidget/PdfReader.hpp>

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

    osgViewer::Viewer        viewer( arguments );

    osgWidget::GeometryHints hints(
        osg::vec3( 0.0F, 0.0F, 0.0F ),
        osg::vec3( 1.0F, 0.0F, 0.0F ),
        osg::vec3( 0.0F, 0.0F, 1.0F ),
        osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ),
        osgWidget::GeometryHints::RESIZE_HEIGHT_TO_MAINTAINCE_ASPECT_RATIO
    );

    osg::ref_ptr<osg::Group> group = new osg::Group;

    for( int i = 1; i < arguments.argc(); ++i )
    {
        if( !arguments.isOption( i ) )
        {
            osg::ref_ptr<osgWidget::PdfReader> pdfReader = new osgWidget::PdfReader;
            if( pdfReader->open( arguments[i], hints ) )
            {
                group->addChild( pdfReader.get() );

                hints.position.x += 1.1F;
            }
        }
    }

    viewer.setSceneData( group.get() );

    viewer.addEventHandler( new osgViewer::StatsHandler );
    return viewer.run();
}
