/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * AnimtkViewer example application
 */
#include "AnimtkViewerGUI.hpp"
#include "AnimtkViewerKeyHandler.hpp"

#include <iostream>
#include <osg/core/io_utils.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgAnimation/core/AnimationManagerBase.hpp>
#include <osgAnimation/skeletal/Bone.hpp>
#include <osgDB/io/FileNameUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>
#include <osgWidget/ViewerEventHandlers.hpp>

const int WIDTH  = 1'440;
const int HEIGHT = 900;

osg::Geode*
createAxis()
{
    osg::Geode*     geode    = new osg::Geode();
    osg::Geometry*  geometry = new osg::Geometry();
    osg::Vec3Array* vertices = new osg::Vec3Array();
    osg::Vec4Array* colors   = new osg::Vec4Array();

    vertices->push_back( osg::vec3( 0.0F, 0.0F, 0.0F ) );
    vertices->push_back( osg::vec3( 1.0F, 0.0F, 0.0F ) );
    vertices->push_back( osg::vec3( 0.0F, 0.0F, 0.0F ) );
    vertices->push_back( osg::vec3( 0.0F, 1.0F, 0.0F ) );
    vertices->push_back( osg::vec3( 0.0F, 0.0F, 0.0F ) );
    vertices->push_back( osg::vec3( 0.0F, 0.0F, 1.0F ) );

    colors->push_back( osg::vec4( 1.0F, 0.0F, 0.0F, 1.0F ) );
    colors->push_back( osg::vec4( 1.0F, 0.0F, 0.0F, 1.0F ) );
    colors->push_back( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    colors->push_back( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    colors->push_back( osg::vec4( 0.0F, 0.0F, 1.0F, 1.0F ) );
    colors->push_back( osg::vec4( 0.0F, 0.0F, 1.0F, 1.0F ) );

    geometry->setVertexArray( vertices );
    geometry->setColorArray( colors, osg::Array::BIND_PER_VERTEX );
    geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, 6 ) );
    // GL_LIGHTING removed: not in core profile

    geode->addDrawable( geometry );

    return geode;
}

struct AnimationManagerFinder : public osg::DualModeVisitor
{
        osg::ref_ptr<osgAnimation::BasicAnimationManager> _am;

        AnimationManagerFinder() :
            osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
        {
        }

        void
        apply( osg::Node& node )
        {
            if( _am.valid() )
            {
                return;
            }
            if( node.getUpdateCallback() )
            {
                osgAnimation::AnimationManagerBase* b =
                    dynamic_cast<osgAnimation::AnimationManagerBase*>(
                        node.getUpdateCallback()
                    );
                if( b )
                {
                    _am = new osgAnimation::BasicAnimationManager( *b );
                    return;
                }
            }
            traverse( node );
        }
};

struct AddHelperBone : public osg::DualModeVisitor
{
        AddHelperBone() :
            osg::DualModeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
        {
        }

        void
        apply( osg::Transform& node )
        {
            osgAnimation::Bone* bone = dynamic_cast<osgAnimation::Bone*>( &node );
            if( bone )
            {
                bone->addChild( createAxis() );
            }
            traverse( node );
        }
};

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    arguments.getApplicationUsage()->setApplicationName(
        arguments.getApplicationName()
    );
    arguments.getApplicationUsage()->setDescription(
        arguments.getApplicationName() +
        " is an example for viewing osgAnimation animations."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "-h or --help",
        "List command line options."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--drawbone",
        "draw helps to display bones."
    );

    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write(
            std::cout,
            osg::ApplicationUsage::COMMAND_LINE_OPTION
        );
        return 0;
    }

    if( arguments.argc() <= 1 )
    {
        arguments.getApplicationUsage()->write(
            std::cout,
            osg::ApplicationUsage::COMMAND_LINE_OPTION
        );
        return 1;
    }

    bool drawBone = false;
    if( arguments.read( "--drawbone" ) )
    {
        drawBone = true;
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

    osgViewer::Viewer        viewer( arguments );
    osg::ref_ptr<osg::Group> group       = new osg::Group();

    osg::ref_ptr<osg::Node>  loadedmodel = osgDB::readRefNodeFiles( arguments );
    osg::Group*              node = dynamic_cast<osg::Group*>( loadedmodel.get() );
    if( !node )
    {
        std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
        return 1;
    }

    // Set our Singleton's model.
    AnimationManagerFinder finder;
    node->accept( finder );
    if( finder._am.valid() )
    {

        std::string playModeOpt;
        if( arguments.read( "--play-mode", playModeOpt ) )
        {
            osgAnimation::Animation::PlayMode playMode = osgAnimation::Animation::LOOP;
            if( osgDB::equalCaseInsensitive( playModeOpt, "ONCE" ) )
            {
                playMode = osgAnimation::Animation::ONCE;
            }
            else if( osgDB::equalCaseInsensitive( playModeOpt, "STAY" ) )
            {
                playMode = osgAnimation::Animation::STAY;
            }
            else if( osgDB::equalCaseInsensitive( playModeOpt, "LOOP" ) )
            {
                playMode = osgAnimation::Animation::LOOP;
            }
            else if( osgDB::equalCaseInsensitive( playModeOpt, "PPONG" ) )
            {
                playMode = osgAnimation::Animation::PPONG;
            }

            for( osgAnimation::AnimationList::const_iterator animIter =
                     finder._am->getAnimationList().begin();
                 animIter != finder._am->getAnimationList().end();
                 ++animIter )
            {
                ( *animIter )->setPlayMode( playMode );
            }
        }

        node->setUpdateCallback( finder._am.get() );
        AnimtkViewerModelController::setModel( finder._am.get() );
    }
    else
    {
        osg::notify( osg::WARN ) << "no osgAnimation::AnimationManagerBase found in the "
                                    "subgraph, no animations available"
                                 << std::endl;
    }

    if( drawBone )
    {
        osg::notify( osg::INFO ) << "Add Bones Helper" << std::endl;
        AddHelperBone addHelper;
        node->accept( addHelper );
    }
    node->addChild( createAxis() );

    AnimtkViewerGUI* gui    = new AnimtkViewerGUI( &viewer, WIDTH, HEIGHT, 0X12'34 );
    osg::Camera*     camera = gui->createParentOrthoCamera();

    node->setNodeMask( 0X00'01 );

    group->addChild( node );
    group->addChild( camera );

    viewer.addEventHandler( new AnimtkKeyEventHandler() );
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler() );
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );
    viewer.addEventHandler( new osgWidget::MouseHandler( gui ) );
    viewer.addEventHandler( new osgWidget::KeyboardHandler( gui ) );
    viewer.addEventHandler( new osgWidget::ResizeHandler( gui, camera ) );
    viewer.setSceneData( group.get() );

    viewer.setUpViewInWindow( 1'000, 100, 640, 480 );
    return viewer.run();
}
