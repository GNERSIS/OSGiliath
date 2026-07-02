/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osglightpoint example application
 */
#include <iostream>
#include <osg/core/Notify.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/GL>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Billboard.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgSim/LightPointNode.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>

#define INTERPOLATE( member ) lp.member = start.member * rstart + end.member * rend;

void
addToLightPointNode( osgSim::LightPointNode& lpn,
                     osgSim::LightPoint&     start,
                     osgSim::LightPoint&     end,
                     unsigned int            noSteps )
{
    if( noSteps <= 1 )
    {
        lpn.addLightPoint( start );
        return;
    }

    float rend   = 0.0F;
    float rdelta = 1.0F / ( ( float )noSteps - 1.0F );

    lpn.getLightPointList().reserve( noSteps );

    for( unsigned int i = 0; i < noSteps; ++i, rend += rdelta )
    {
        float              rstart = 1.0F - rend;
        osgSim::LightPoint lp( start );
        INTERPOLATE( _position )
        INTERPOLATE( _intensity );
        INTERPOLATE( _color );
        INTERPOLATE( _radius );

        lpn.addLightPoint( lp );
    }
}

#undef INTERPOLATE

bool usePointSprites;

osg::Node*
createLightPointsDatabase()
{
    osgSim::LightPoint start;
    osgSim::LightPoint end;

    start._position.set( -500.0F, -500.0F, 0.0F );
    start._color.set( 1.0F, 0.0F, 0.0F, 1.0F );

    end._position.set( 500.0F, -500.0F, 0.0F );
    end._color.set( 1.0F, 1.0F, 1.0F, 1.0F );

    osg::MatrixTransform* transform = new osg::MatrixTransform;

    transform->setDataVariance( osg::Object::DataVariance::STATIC );
    transform->setMatrix( osg::scale( 0.1, 0.1, 0.1 ) );

    osg::vec3 start_delta( 0.0F, 10.0F, 0.0F );
    osg::vec3 end_delta( 0.0F, 10.0F, 1.0F );

    int       noStepsX = 100;
    int       noStepsY = 100;

    // osgSim::BlinkSequence* bs = new osgSim::BlinkSequence;
    // bs->addPulse(1.0,osg::vec4(1.0f,0.0f,0.0f,1.0f));
    // bs->addPulse(0.5,osg::vec4(0.0f,0.0f,0.0f,0.0f)); // off
    // bs->addPulse(1.5,osg::vec4(1.0f,1.0f,0.0f,1.0f));
    // bs->addPulse(0.5,osg::vec4(0.0f,0.0f,0.0f,0.0f)); // off
    // bs->addPulse(1.0,osg::vec4(1.0f,1.0f,1.0f,1.0f));
    // bs->addPulse(0.5,osg::vec4(0.0f,0.0f,0.0f,0.0f)); // off

    // osgSim::Sector* sector = new
    // osgSim::ConeSector(osg::vec3(0.0f,0.0f,1.0f),osg::radians(45.0),osg::radians(45.0));
    // osgSim::Sector* sector = new
    // osgSim::ElevationSector(-osg::radians(45.0),osg::radians(45.0),osg::radians(45.0));
    // osgSim::Sector* sector = new
    // osgSim::AzimSector(-osg::radians(45.0),osg::radians(45.0),osg::radians(90.0));
    //  osgSim::Sector* sector = new
    //  osgSim::AzimElevationSector(osg::radians(180),osg::radians(90), // azim range
    //                                                              osg::radians(0.0),osg::radians(90.0),
    //                                                              // elevation range
    //                                                              osg::radians(5.0));

    for( int i = 0; i < noStepsY; ++i )
    {

        // osgSim::BlinkSequence* local_bs = new osgSim::BlinkSequence(*bs);
        // local_bs->setSequenceGroup(new
        // osgSim::BlinkSequence::SequenceGroup((double)i*0.1)); start._blinkSequence =
        // local_bs;

        // start._sector = sector;

        osgSim::LightPointNode* lpn = new osgSim::LightPointNode;

        //
        osg::StateSet*          set = lpn->getOrCreateStateSet();

        if( usePointSprites )
        {
            lpn->setPointSprite();

            // Set point sprite texture in LightPointNode StateSet.
            osg::Texture2D* tex = new osg::Texture2D();
            tex->setImage( osgDB::readRefImageFile( "Images/particle.rgb" ) );
            set->setTextureAttributeAndModes( 0, tex, osg::StateAttribute::ON );
        }

        // set->setMode(GL_BLEND, osg::StateAttribute::ON);
        // osg::BlendFunc *fn = new osg::BlendFunc();
        // fn->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::DST_ALPHA);
        // set->setAttributeAndModes(fn, osg::StateAttribute::ON);
        //

        addToLightPointNode( *lpn, start, end, noStepsX );

        start._position += start_delta;
        end._position   += end_delta;

        transform->addChild( lpn );
    }

    osg::Group* group = new osg::Group;
    group->addChild( transform );

    return group;
}

static osg::Node*
CreateBlinkSequenceLightNode()
{
    osgSim::LightPointNode* lightPointNode = new osgSim::LightPointNode;
    ;

    osgSim::LightPointNode::LightPointList lpList;

    osg::ref_ptr<osgSim::SequenceGroup>    seq_0;
    seq_0            = new osgSim::SequenceGroup;
    seq_0->_baseTime = 0.0;

    osg::ref_ptr<osgSim::SequenceGroup> seq_1;
    seq_1                = new osgSim::SequenceGroup;
    seq_1->_baseTime     = 0.5;

    const int max_points = 32;
    for( int i = 0; i < max_points; ++i )
    {
        osgSim::LightPoint lp;
        double             x = cos( ( 2.0 * osg::PI * i ) / max_points );
        double             z = sin( ( 2.0 * osg::PI * i ) / max_points );
        lp._position.set( x, 0.0F, z + 30.0F );
        lp._blinkSequence = new osgSim::BlinkSequence;
        for( int j = 10; j > 0; --j )
        {
            float intensity = j / 10.0F;
            lp._blinkSequence->addPulse(
                1.0 / max_points,
                osg::vec4( intensity, intensity, intensity, intensity )
            );
        }
        if( max_points > 10 )
        {
            lp._blinkSequence->addPulse( 1.0 - 10.0 / max_points,
                                         osg::vec4( 0.0F, 0.0F, 0.0F, 0.0F ) );
        }

        if( i & 1 )
        {
            lp._blinkSequence->setSequenceGroup( seq_1.get() );
        }
        else
        {
            lp._blinkSequence->setSequenceGroup( seq_0.get() );
        }
        lp._blinkSequence->setPhaseShift( i / ( static_cast<double>( max_points ) ) );
        lpList.push_back( lp );
    }

    lightPointNode->setLightPointList( lpList );

    return lightPointNode;
}

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " is the example which demonstrates use high quality light point, typically "
        "used for naviagional lights."
    );
    arguments.getApplicationUsage()->setCommandLineUsage(
        arguments.getApplicationName() + " [options] filename ..."
    );
    arguments.getApplicationUsage()->addCommandLineOption( "-h or --help",
                                                           "Display this information" );
    arguments.getApplicationUsage()->addCommandLineOption( "--sprites",
                                                           "Point sprites." );

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

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    usePointSprites = false;
    while( arguments.read( "--sprites" ) )
    {
        usePointSprites = true;
    };

    osg::Group* rootnode = new osg::Group;

    // load the nodes from the commandline arguments.
    rootnode->addChild( osgDB::readRefNodeFiles( arguments ) );
    rootnode->addChild( createLightPointsDatabase() );
    rootnode->addChild( CreateBlinkSequenceLightNode() );

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize( rootnode );

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode );
    return viewer.run();
}
