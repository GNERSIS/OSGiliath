/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgsimulation example application
 */
#if defined( _WIN32 ) && !( defined( __CYGWIN__ ) || defined( __MINGW32__ ) )
/////////////////////////////////////////////////////////////////////////////
// Disable unavoidable warning messages:

// 4103: used #pragma pack to change alignment
// 4114: same type qualifier used more than once
// 4201: nonstandard extension used : nameless struct/union
// 4237: "keyword" reserved for future use
// 4251: class needs to have dll-interface to export class
// 4275: non DLL-interface class used as base for DLL-interface class
// 4290: C++ Exception Specification ignored
// 4503: decorated name length exceeded, name was truncated
// 4786: string too long - truncated to 255 characters

    #pragma warning( disable                                                         \
                     : 4'103 4'114 4'201 4'237 4'251 4'275 4'290 4'503 4'335 4'786 )

#endif    // _WIN32

#include <iostream>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/fstream.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/AnimationPathManipulator.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/KeySwitchMatrixManipulator.hpp>
#include <osgGA/manipulators/NodeTrackerManipulator.hpp>
#include <osgGA/manipulators/TerrainManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgParticle/FireEffect>
#include <osgSim/OverlayNode>
#include <osgSim/SphereSegment>
#include <osgText/Text>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

osg::Node*
createEarth()
{
    osg::TessellationHints* hints = new osg::TessellationHints;
    hints->setDetailRatio( 5.0F );

    osg::ShapeDrawable* sd =
        new osg::ShapeDrawable( new osg::Sphere( osg::vec3( 0.0, 0.0, 0.0 ),
                                                 osg::WGS_84_RADIUS_POLAR ),
                                hints );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( sd );

    std::string filename = osgDB::findDataFile( "Images/land_shallow_topo_2048.jpg" );
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(
        0,
        new osg::Texture2D( osgDB::readRefImageFile( filename ) )
    );

    osg::CoordinateSystemNode* csn = new osg::CoordinateSystemNode;
    csn->setEllipsoidModel( new osg::EllipsoidModel() );
    csn->addChild( geode );

    return csn;
}

class ModelPositionCallback : public osg::NodeCallback
{
    public:

        ModelPositionCallback( double speed ) :
            _latitude( 0.0 ),
            _longitude( 0.0 ),
            _height( 100000.0 ),
            _speed( speed )
        {
            _rotation = osg::quat( osg::radians( 90.0 ), osg::vec3( 0.0, 0.0, 1.0 ) );
        }

        void
        updateParameters()
        {
            _longitude += _speed * ( ( 2.0 * osg::PI ) / 360.0 ) / 20.0;
        }

        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            updateParameters();

            osg::NodePath         nodePath = nv->getNodePath();

            osg::MatrixTransform* mt =
                nodePath.empty()
                    ? 0
                    : dynamic_cast<osg::MatrixTransform*>( nodePath.back() );
            if( mt )
            {
                osg::CoordinateSystemNode* csn = 0;

                // find coordinate system node from our parental chain
                unsigned int               i;
                for( i = 0; i < nodePath.size() && csn == 0; ++i )
                {
                    csn = dynamic_cast<osg::CoordinateSystemNode*>( nodePath[i] );
                }

                if( csn )
                {

                    osg::EllipsoidModel* ellipsoid = csn->getEllipsoidModel();
                    if( ellipsoid )
                    {
                        osg::dmat4 inheritedMatrix;
                        for( i += 1; i < nodePath.size() - 1; ++i )
                        {
                            osg::Transform* transform = nodePath[i]->asTransform();
                            if( transform )
                            {
                                transform->computeLocalToWorldMatrix( inheritedMatrix,
                                                                      nv );
                            }
                        }

                        osg::dmat4 matrix( inheritedMatrix );

                        // osg::dmat4 matrix;
                        ellipsoid->computeLocalToWorldTransformFromLatLongHeight(
                            _latitude,
                            _longitude,
                            _height,
                            matrix
                        );
                        osg::preMultRotate( matrix, _rotation );

                        mt->setMatrix( matrix );
                    }
                }
            }

            traverse( node, nv );
        }

        double    _latitude;
        double    _longitude;
        double    _height;
        osg::quat _rotation;
        double    _speed;
};

class FindNamedNodeVisitor : public osg::DualModeVisitor
{
    public:

        FindNamedNodeVisitor( const std::string& name ) :
            osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
            _name( name )
        {
        }

        virtual void
        apply( osg::Node& node )
        {
            if( node.getName() == _name )
            {
                _foundNodes.push_back( &node );
            }
            traverse( node );
        }

        typedef std::vector<osg::ref_ptr<osg::Node>> NodeList;

        std::string                                  _name;
        NodeList                                     _foundNodes;
};

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " is the example which demonstrates use of node tracker."
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName()
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );

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

    osgViewer::Viewer viewer( arguments );

    // add the state manipulator
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );

    // add the thread model handler
    viewer.addEventHandler( new osgViewer::ThreadingHandler );

    // add the window size toggle handler
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );

    // add the stats handler
    viewer.addEventHandler( new osgViewer::StatsHandler );

    // add the record camera path  handler
    viewer.addEventHandler( new osgViewer::RecordCameraPathHandler );

    // add the help handler
    viewer.addEventHandler(
        new osgViewer::HelpHandler( arguments.getApplicationUsage() )
    );

    // set the near far ration computation up.
    viewer.getCamera()->setComputeNearFarMode(
        osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES
    );
    viewer.getCamera()->setNearFarRatio( 0.000003F );

    double speed = 1.0;
    while( arguments.read( "-f" ) || arguments.read( "--fixed" ) )
    {
        speed = 0.0;
    }

    osg::quat rotation;
    osg::vec4 vec4;
    while( arguments.read( "--rotate-model", vec4[0], vec4[1], vec4[2], vec4[3] ) )
    {
        osg::quat local_rotate;
        local_rotate =
            osg::quat( osg::radians( vec4[0] ), osg::vec3( vec4[1], vec4[2], vec4[3] ) );

        rotation = rotation * local_rotate;
    }

    osg::NodeCallback* nc = 0;
    std::string        flightpath_filename;
    while( arguments.read( "--flight-path", flightpath_filename ) )
    {
        osgDB::ifstream fin( flightpath_filename.c_str() );
        if( fin )
        {
            osg::AnimationPath* path = new osg::AnimationPath;
            path->read( fin );
            nc = new osg::AnimationPathCallback( path );
        }
    }

    osgGA::NodeTrackerManipulator::TrackerMode trackerMode =
        osgGA::NodeTrackerManipulator::NODE_CENTER_AND_ROTATION;
    std::string mode;
    while( arguments.read( "--tracker-mode", mode ) )
    {
        if( mode == "NODE_CENTER_AND_ROTATION" )
        {
            trackerMode = osgGA::NodeTrackerManipulator::NODE_CENTER_AND_ROTATION;
        }
        else if( mode == "NODE_CENTER_AND_AZIM" )
        {
            trackerMode = osgGA::NodeTrackerManipulator::NODE_CENTER_AND_AZIM;
        }
        else if( mode == "NODE_CENTER" )
        {
            trackerMode = osgGA::NodeTrackerManipulator::NODE_CENTER;
        }
        else
        {
            std::cout << "Unrecognized --tracker-mode option " << mode
                      << ", valid options are:" << std::endl;
            std::cout << "    NODE_CENTER_AND_ROTATION" << std::endl;
            std::cout << "    NODE_CENTER_AND_AZIM" << std::endl;
            std::cout << "    NODE_CENTER" << std::endl;
            return 1;
        }
    }

    osgGA::NodeTrackerManipulator::RotationMode rotationMode =
        osgGA::NodeTrackerManipulator::TRACKBALL;
    while( arguments.read( "--rotation-mode", mode ) )
    {
        if( mode == "TRACKBALL" )
        {
            rotationMode = osgGA::NodeTrackerManipulator::TRACKBALL;
        }
        else if( mode == "ELEVATION_AZIM" )
        {
            rotationMode = osgGA::NodeTrackerManipulator::ELEVATION_AZIM;
        }
        else
        {
            std::cout << "Unrecognized --rotation-mode option " << mode
                      << ", valid options are:" << std::endl;
            std::cout << "    TRACKBALL" << std::endl;
            std::cout << "    ELEVATION_AZIM" << std::endl;
            return 1;
        }
    }

    bool useOverlay = true;
    while( arguments.read( "--no-overlay" ) || arguments.read( "-n" ) )
    {
        useOverlay = false;
    }

    osgSim::OverlayNode::OverlayTechnique technique =
        osgSim::OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY;
    while( arguments.read( "--object" ) )
    {
        technique = osgSim::OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY;
    }
    while( arguments.read( "--ortho" ) || arguments.read( "--orthographic" ) )
    {
        technique = osgSim::OverlayNode::VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY;
    }
    while( arguments.read( "--persp" ) || arguments.read( "--perspective" ) )
    {
        technique = osgSim::OverlayNode::VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY;
    }

    unsigned int overlayTextureUnit = 1;
    while( arguments.read( "--unit", overlayTextureUnit ) )
    {
    }

    std::string pathfile;
    while( arguments.read( "-p", pathfile ) )
    {
    }

    bool addFireEffect = arguments.read( "--fire" );

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    osg::ref_ptr<osgGA::NodeTrackerManipulator> tm;

    std::string                                 overlayFilename;
    while( arguments.read( "--overlay", overlayFilename ) )
    {
    }

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> root = osgDB::readRefNodeFiles( arguments );

    if( !root )
    {
        root = createEarth();
    }

    if( !root )
    {
        return 0;
    }

    if( !overlayFilename.empty() )
    {
        // osg::Object *pObj = osgDB::readObjectFile("alaska_clean.shp");
        // osg::ref_ptr<osg::Geode> shapefile = dynamic_cast<osg::Geode*> (pObj);
        //
        // ConvertLatLon2EllipsoidCoordinates latlon2em;
        // shapefile->accept(latlon2em);

        osg::ref_ptr<osg::Node> shapefile = osgDB::readRefNodeFile( overlayFilename );

        if( !shapefile )
        {
            osg::notify( osg::NOTICE )
                << "File `" << overlayFilename << "` not found" << std::endl;
            return 1;
        }

        osg::CoordinateSystemNode* csn =
            dynamic_cast<osg::CoordinateSystemNode*>( root.get() );
        if( csn )
        {

            osgSim::OverlayNode* overlayNode = new osgSim::OverlayNode( technique );

            overlayNode->setOverlaySubgraph( shapefile.get() );
            overlayNode->setOverlayTextureSizeHint( 1'024 );
            overlayNode->setOverlayTextureUnit( overlayTextureUnit );

            // insert the OverlayNode between the coordinate system node and its
            // children.
            for( unsigned int i = 0; i < csn->getNumChildren(); ++i )
            {
                overlayNode->addChild( csn->getChild( i ) );
            }

            csn->removeChildren( 0, csn->getNumChildren() );
            csn->addChild( overlayNode );

            viewer.setSceneData( csn );
        }
        else
        {
            osgSim::OverlayNode* overlayNode = new osgSim::OverlayNode( technique );

            overlayNode->setOverlaySubgraph( shapefile.get() );
            overlayNode->setOverlayTextureSizeHint( 1'024 );
            overlayNode->addChild( root.get() );

            viewer.setSceneData( overlayNode );
        }
    }
    else
    {

        // add a viewport to the viewer and attach the scene graph.
        viewer.setSceneData( root.get() );

        osg::CoordinateSystemNode* csn =
            dynamic_cast<osg::CoordinateSystemNode*>( root.get() );
        if( csn )
        {

            osg::ref_ptr<osgSim::OverlayNode> overlayNode;
            if( useOverlay )
            {
                overlayNode = new osgSim::OverlayNode( technique );

                // insert the OverlayNode between the coordinate system node and its
                // children.
                for( unsigned int i = 0; i < csn->getNumChildren(); ++i )
                {
                    overlayNode->addChild( csn->getChild( i ) );
                }

                csn->removeChildren( 0, csn->getNumChildren() );
                csn->addChild( overlayNode.get() );

                // tell the overlay node to continuously update its overlay texture
                // as we know we'll be tracking a moving target.
                overlayNode->setContinuousUpdate( true );
            }

            osg::ref_ptr<osg::Node> cessna =
                osgDB::readRefNodeFile( "damaged_helmet.glb" );
            if( cessna )
            {
                double                s      = 200000.0 / cessna->getBound().radius;

                osg::MatrixTransform* scaler = new osg::MatrixTransform;
                scaler->addChild( cessna );
                scaler->setMatrix( osg::rotate( rotation ) * osg::scale( s, s, s ) );
                if( addFireEffect )
                {
                    osg::vec3                center = cessna->getBound().center;

                    osgParticle::FireEffect* fire =
                        new osgParticle::FireEffect( center, 10.0F );
                    scaler->addChild( fire );
                }

                if( false )
                {
                    osgSim::SphereSegment* ss = new osgSim::SphereSegment(
                        osg::vec3( 0.0F, 0.0F, 0.0F ),    // center
                        19.9F,                            // radius
                        osg::radians( 135.0F ),
                        osg::radians( 240.0F ),
                        osg::radians( -10.0F ),
                        osg::radians( 30.0F ),
                        60
                    );

                    scaler->addChild( ss );
                }

                osg::MatrixTransform* mt = new osg::MatrixTransform;
                mt->addChild( scaler );

                if( !nc )
                {
                    nc = new ModelPositionCallback( speed );
                }

                mt->setUpdateCallback( nc );

                csn->addChild( mt );

                // if we are using an overaly node, use the cessna subgraph as the
                // overlay subgraph
                if( overlayNode.valid() )
                {
                    overlayNode->setOverlaySubgraph( mt );
                }

                tm = new osgGA::NodeTrackerManipulator;
                tm->setTrackerMode( trackerMode );
                tm->setRotationMode( rotationMode );
                tm->setTrackNode( scaler );
            }
            else
            {
                std::cout << "Failed to read damaged_helmet.glb" << std::endl;
            }
        }
    }

    // set up camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator =
            new osgGA::KeySwitchMatrixManipulator;

        if( tm.valid() )
        {
            keyswitchManipulator->addMatrixManipulator( '0', "NodeTracker", tm.get() );
        }

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

        if( !pathfile.empty() )
        {
            osgGA::AnimationPathManipulator* apm =
                new osgGA::AnimationPathManipulator( pathfile );
            if( apm || !apm->valid() )
            {
                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( '5', "Path", apm );
                keyswitchManipulator->selectMatrixManipulator( num );
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    // viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
    return viewer.run();
}
