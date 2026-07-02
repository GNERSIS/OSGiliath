/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgimpostor example application
 */
#include "TestManipulator.hpp"

#include <iostream>
#include <list>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/quat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/Material.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/AnimationPathManipulator.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/KeySwitchMatrixManipulator.hpp>
#include <osgGA/manipulators/SphericalManipulator.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgSim/Impostor.hpp>
#include <osgSim/InsertImpostorsVisitor.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

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
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer viewer;

    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator =
            new osgGA::KeySwitchMatrixManipulator;

        // add local test manipulator more suitable for testing impostors.
        keyswitchManipulator->addMatrixManipulator( '0', "Test", new TestManipulator );

        keyswitchManipulator->addMatrixManipulator( '1',
                                                    "Trackball",
                                                    new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2',
                                                    "Flight",
                                                    new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3',
                                                    "Drive",
                                                    new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4',
                                                    "Terrain",
                                                    new osgGA::TerrainManipulator() );
        keyswitchManipulator->addMatrixManipulator( '5',
                                                    "Orbit",
                                                    new osgGA::OrbitManipulator() );
        keyswitchManipulator->addMatrixManipulator(
            '6',
            "FirstPerson",
            new osgGA::FirstPersonManipulator()
        );
        keyswitchManipulator->addMatrixManipulator( '7',
                                                    "Spherical",
                                                    new osgGA::SphericalManipulator() );

        std::string pathfile;
        double      animationSpeed = 1.0;
        while( arguments.read( "--speed", animationSpeed ) )
        {
        }
        char keyForAnimationPath = '8';
        while( arguments.read( "-p", pathfile ) )
        {
            osgGA::AnimationPathManipulator* apm =
                new osgGA::AnimationPathManipulator( pathfile );
            if( apm || !apm->valid() )
            {
                apm->setTimeScale( animationSpeed );

                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath,
                                                            "Path",
                                                            apm );
                keyswitchManipulator->selectMatrixManipulator( num );
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );

    std::string output_filename;
    arguments.read( "-o", output_filename );

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFiles( arguments );
    if( !model )
    {
        OSG_NOTICE << "No model loaded, please specify and model on the the command line"
                   << std::endl;
        return 0;
    }

    // the osgSim::InsertImpostorsVisitor used lower down to insert impostors
    // only operators on subclass of Group's, if the model top node is not
    // a group then it won't be able to insert an impostor.  We therefore
    // manually insert an impostor above the model.
    if( dynamic_cast<osg::Group*>( model.get() ) == 0 )
    {
        const osg::sphere& bs = model->getBound();
        if( bs.valid() )
        {

            osgSim::Impostor* impostor = new osgSim::Impostor;

            // standard LOD settings
            impostor->addChild( model.get() );
            impostor->setRange( 0, 0.0F, 1E7F );
            impostor->setCenter( bs.center );

            // impostor specific settings.
            impostor->setImpostorThresholdToBound( 5.0F );

            model = impostor;
        }
    }

    // we insert an impostor node above the model, so we keep a handle
    // on the rootnode of the model, the is required since the
    // InsertImpostorsVisitor can add a new root in automatically and
    // we would know about it, other than by following the parent path
    // up from model.  This is really what should be done, but I'll pass
    // on it right now as it requires a getRoots() method to be added to
    // osg::Node, and we're about to make a release so no new features!
    osg::ref_ptr<osg::Group> rootnode = new osg::Group;
    rootnode->addChild( model );

    // now insert impostors in the model using the InsertImpostorsVisitor.
    osgSim::InsertImpostorsVisitor ov;

    // traverse the model and collect all osg::Group's and osg::LOD's.
    // however, don't traverse the rootnode since we want to keep it as
    // the start of traversal, otherwise the insertImpostor could insert
    // and Impostor above the current root, making it nolonger a root!
    model->accept( ov );

    // insert the Impostors above groups and LOD's
    ov.insertImpostors();

    if( !output_filename.empty() )
    {
        osgDB::writeNodeFile( *rootnode, output_filename );
        return 1;
    }

    // add model to viewer.
    viewer.setSceneData( rootnode );
    return viewer.run();
}
