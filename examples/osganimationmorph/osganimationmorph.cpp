/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osganimationmorph example application
 */
#include <iostream>
#include <osg/core/io_utils.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgAnimation/core/BasicAnimationManager.hpp>
#include <osgAnimation/morph/MorphGeometry.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgUtil/optimization/SmoothingVisitor.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

struct GeometryFinder : public osg::DualModeVisitor
{
        osg::ref_ptr<osg::Geometry> _geom;

        GeometryFinder() :
            osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
        {
        }

        void
        apply( osg::Geode& geode )
        {
            if( _geom.valid() )
            {
                return;
            }
            for( unsigned int i = 0; i < geode.getNumDrawables(); i++ )
            {
                osg::Geometry* geom =
                    dynamic_cast<osg::Geometry*>( geode.getDrawable( i ) );
                if( geom )
                {
                    _geom = geom;
                    return;
                }
            }
        }
};

osg::ref_ptr<osg::Geometry>
getShape( const std::string& name )
{
    osg::ref_ptr<osg::Node> shape0 = osgDB::readRefNodeFile( name );
    if( shape0 )
    {
        GeometryFinder finder;
        shape0->accept( finder );
        return finder._geom;
    }
    else
    {
        return NULL;
    }
}

int
main( int   argc,
      char* argv[] )
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

    osgViewer::Viewer                 viewer( arguments );

    osgAnimation::Animation*          animation = new osgAnimation::Animation;
    osgAnimation::FloatLinearChannel* channel0  = new osgAnimation::FloatLinearChannel;
    channel0->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(
        osgAnimation::FloatKeyframe( 0, 0.0 )
    );
    channel0->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(
        osgAnimation::FloatKeyframe( 1, 1.0 )
    );
    channel0->setTargetName( "MorphNodeCallback" );
    channel0->setName( "0" );

    animation->addChannel( channel0 );
    animation->setName( "Morph" );
    animation->computeDuration();
    animation->setPlayMode( osgAnimation::Animation::PPONG );
    osgAnimation::BasicAnimationManager* bam = new osgAnimation::BasicAnimationManager;
    bam->registerAnimation( animation );

    osg::ref_ptr<osg::Geometry> geom0 = getShape( "morphtarget_shape0.glb" );
    if( !geom0 )
    {
        std::cerr << "can't read morphtarget_shape0.glb" << std::endl;
        return 0;
    }

    osg::ref_ptr<osg::Geometry> geom1 = getShape( "morphtarget_shape1.glb" );
    if( !geom1 )
    {
        std::cerr << "can't read morphtarget_shape1.glb" << std::endl;
        return 0;
    }

    // initialize with the first shape
    osgAnimation::MorphGeometry* morph = new osgAnimation::MorphGeometry( *geom0 );
    morph->addMorphTarget( geom1.get() );

    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

    osg::Group* scene = new osg::Group;
    scene->addUpdateCallback( bam );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( morph );
    osgAnimation::UpdateMorph* morphupdate =
        new osgAnimation::UpdateMorph( "MorphNodeCallback" );
    morphupdate->addTarget( "MorphNodeCallback" );
    geode->addUpdateCallback( morphupdate );
    scene->addChild( geode );

    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler() );
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );

    // let's run !
    viewer.setSceneData( scene );
    viewer.realize();

    bam->playAnimation( animation );

    while( !viewer.done() )
    {
        viewer.frame();
    }

    // .osg format removed; debug save disabled
    // osgDB::writeNodeFile(*scene, "morph_scene.osg");

    return 0;
}
