/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgkeystone example application
 */
#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/ValueObject.hpp>
#include <osg/maths/compat.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/Stencil.hpp>
#include <osg/textures/TextureRectangle.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/config/SingleScreen>
#include <osgViewer/config/SingleWindow>
#include <osgViewer/config/WoWVxDisplay>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    // initialize the viewer.
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

    osgViewer::Viewer     viewer( arguments );

    osg::DisplaySettings* ds = viewer.getDisplaySettings()
                                 ? viewer.getDisplaySettings()
                                 : osg::DisplaySettings::instance().get();
    ds->readCommandLine( arguments );

    osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFiles( arguments );

    if( !model )
    {
        OSG_NOTICE << "No models loaded, please specify a model file on the command line"
                   << std::endl;
        return 1;
    }

    OSG_NOTICE << "Stereo " << ds->getStereo() << std::endl;
    OSG_NOTICE << "StereoMode " << ds->getStereoMode() << std::endl;

    viewer.setSceneData( model );

    // add the state manipulator
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );

    // add the stats handler
    viewer.addEventHandler( new osgViewer::StatsHandler );

    // add camera manipulator
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

    OSG_NOTICE << "KeystoneFileNames.size()=" << ds->getKeystoneFileNames().size()
               << std::endl;
    for( osg::DisplaySettings::FileNames::iterator itr =
             ds->getKeystoneFileNames().begin();
         itr != ds->getKeystoneFileNames().end();
         ++itr )
    {
        OSG_NOTICE << "   keystone filename = " << *itr << std::endl;
    }

    ds->setKeystoneHint( true );

    if( !ds->getKeystoneFileNames().empty() )
    {
        for( osg::DisplaySettings::Objects::iterator itr = ds->getKeystones().begin();
             itr != ds->getKeystones().end();
             ++itr )
        {
            osgViewer::Keystone* keystone =
                dynamic_cast<osgViewer::Keystone*>( itr->get() );
            if( keystone )
            {
                std::string filename;
                keystone->getUserValue( "filename", filename );
                OSG_NOTICE << "Loaded keystone " << filename << ", " << keystone
                           << std::endl;

                ds->getKeystones().push_back( keystone );
            }
        }
    }

    viewer.apply( new osgViewer::SingleScreen( 0 ) );

    viewer.realize();

    while( !viewer.done() )
    {
        viewer.advance();
        viewer.eventTraversal();
        viewer.updateTraversal();
        viewer.renderingTraversals();
    }
    return 0;
}
