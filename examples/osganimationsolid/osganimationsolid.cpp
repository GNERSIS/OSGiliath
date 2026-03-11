/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osganimationsolid example application
 */
#include <iostream>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/Shape.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgAnimation/core/BasicAnimationManager.hpp>
#include <osgAnimation/core/Channel.hpp>
#include <osgAnimation/transform/StackedRotateAxisElement.hpp>
#include <osgAnimation/transform/StackedTranslateElement.hpp>
#include <osgAnimation/transform/UpdateMatrixTransform.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>

using namespace osgAnimation;

osg::ref_ptr<osg::Geode>
createAxis()
{
    osg::ref_ptr<osg::Geode>     geode( new osg::Geode() );
    osg::ref_ptr<osg::Geometry>  geometry( new osg::Geometry() );

    osg::ref_ptr<osg::Vec3Array> vertices( new osg::Vec3Array() );
    vertices->push_back( osg::vec3( 0.0, 0.0, 0.0 ) );
    vertices->push_back( osg::vec3( 10.0, 0.0, 0.0 ) );
    vertices->push_back( osg::vec3( 0.0, 0.0, 0.0 ) );
    vertices->push_back( osg::vec3( 0.0, 10.0, 0.0 ) );
    vertices->push_back( osg::vec3( 0.0, 0.0, 0.0 ) );
    vertices->push_back( osg::vec3( 0.0, 0.0, 10.0 ) );
    geometry->setVertexArray( vertices.get() );

    osg::ref_ptr<osg::Vec4Array> colors( new osg::Vec4Array() );
    colors->push_back( osg::vec4( 1.0F, 0.0F, 0.0F, 1.0F ) );
    colors->push_back( osg::vec4( 1.0F, 0.0F, 0.0F, 1.0F ) );
    colors->push_back( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    colors->push_back( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    colors->push_back( osg::vec4( 0.0F, 0.0F, 1.0F, 1.0F ) );
    colors->push_back( osg::vec4( 0.0F, 0.0F, 1.0F, 1.0F ) );
    geometry->setColorArray( colors.get(), osg::Array::BIND_PER_VERTEX );
    geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, 6 ) );

    geode->addDrawable( geometry.get() );
    return geode;
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

    osgViewer::Viewer viewer( arguments );

    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

    osg::Group*              root  = new osg::Group;

    osg::ref_ptr<osg::Geode> axe   = createAxis();
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(
        new osg::ShapeDrawable( new osg::Box( osg::vec3( 0.0F, 0.0F, 0.0F ), 0.5 ) )
    );

    // Transformation to be manipulated by the animation
    osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform();
    trans->setName( "AnimatedNode" );
    // Dynamic object, has to be updated during update traversal
    trans->setDataVariance( osg::Object::DataVariance::DYNAMIC );
    // Animation callback for dmat4 transforms, name is targetName for Channels
    osgAnimation::UpdateMatrixTransform* updatecb =
        new osgAnimation::UpdateMatrixTransform( "AnimatedCallback" );
    // add manipulator Stack, names must match with channel names
    // elements are applied in LIFO order
    // The first element modifies the position component of the matrix
    // The second element modifies the rotation around x-axis
    updatecb->getStackedTransforms().push_back(
        new osgAnimation::StackedTranslateElement( "position" )
    );
    updatecb->getStackedTransforms().push_back(
        new osgAnimation::StackedRotateAxisElement( "euler", osg::vec3( 1, 0, 0 ), 0 )
    );
    // connect the UpdateMatrixTransform callback to the MatrixTransform
    trans->setUpdateCallback( updatecb );
    // initialize MatrixTranform
    trans->setMatrix( osg::dmat4() );
    // append geometry node
    trans->addChild( geode.get() );

    root->addChild( axe.get() );
    root->addChild( trans.get() );

    // Define a scheduler for our animations
    osg::Group*                          grp = new osg::Group;
    // add the animation manager to the scene graph to get it called during update
    // traversals
    osgAnimation::BasicAnimationManager* mng = new osgAnimation::BasicAnimationManager();
    grp->setUpdateCallback( mng );
    // add the rest of the scene to the grp node
    grp->addChild( root );

    // And we finally define our channel for linear Vector interpolation
    osgAnimation::Vec3LinearChannel* channelAnimation1 =
        new osgAnimation::Vec3LinearChannel;
    // name of the AnimationUpdateCallback
    channelAnimation1->setTargetName( "AnimatedCallback" );
    // name of the StackedElementTransform for position modification
    channelAnimation1->setName( "position" );
    // Create keyframes for (in this case linear) interpolation of a osg::vec3
    channelAnimation1->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(
        osgAnimation::Vec3Keyframe( 0, osg::vec3( 0, 0, 0 ) )
    );
    channelAnimation1->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(
        osgAnimation::Vec3Keyframe( 2, osg::vec3( 1, 1, 0 ) )
    );
    osgAnimation::Animation* anim1 = new osgAnimation::Animation;
    anim1->addChannel( channelAnimation1 );
    anim1->setPlayMode( osgAnimation::Animation::PPONG );

    // define the channel for interpolation of a float angle value
    osgAnimation::FloatLinearChannel* channelAnimation2 =
        new osgAnimation::FloatLinearChannel;
    // name of the AnimationUpdateCallback
    channelAnimation2->setTargetName( "AnimatedCallback" );
    // name of the StackedElementTransform for position modification
    channelAnimation2->setName( "euler" );
    // Create keyframes for (in this case linear) interpolation of a osg::vec3
    channelAnimation2->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(
        osgAnimation::FloatKeyframe( 0, 0 )
    );
    channelAnimation2->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(
        osgAnimation::FloatKeyframe( 1.5, 2 * osg::PI )
    );
    osgAnimation::Animation* anim2 = new osgAnimation::Animation;
    anim2->addChannel( channelAnimation2 );
    anim2->setPlayMode( osgAnimation::Animation::LOOP );

    // We register all animation inside the scheduler
    mng->registerAnimation( anim1 );
    mng->registerAnimation( anim2 );

    // start the animation
    mng->playAnimation( anim1 );
    mng->playAnimation( anim2 );

    // set the grp-Group with the scene and the AnimationManager as viewer's scene data
    viewer.setSceneData( grp );
    return viewer.run();
}
