/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osganimationskinning example application
 */
#include <iostream>
#include <osg/core/io_utils.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgAnimation/core/BasicAnimationManager.hpp>
#include <osgAnimation/skeletal/Bone.hpp>
#include <osgAnimation/skeletal/RigGeometry.hpp>
#include <osgAnimation/skeletal/Skeleton.hpp>
#include <osgAnimation/skeletal/UpdateBone.hpp>
#include <osgAnimation/transform/StackedRotateAxisElement.hpp>
#include <osgAnimation/transform/StackedTransform.hpp>
#include <osgAnimation/transform/StackedTranslateElement.hpp>
#include <osgAnimation/transform/UpdateMatrixTransform.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgUtil/optimization/SmoothingVisitor.hpp>
#include <osgViewer/core/Viewer.hpp>

osg::Geode*
createAxis()
{
    osg::Geode*     geode( new osg::Geode() );
    osg::Geometry*  geometry( new osg::Geometry() );

    osg::Vec3Array* vertices( new osg::Vec3Array() );
    vertices->push_back( osg::vec3( 0.0, 0.0, 0.0 ) );
    vertices->push_back( osg::vec3( 1.0, 0.0, 0.0 ) );
    vertices->push_back( osg::vec3( 0.0, 0.0, 0.0 ) );
    vertices->push_back( osg::vec3( 0.0, 1.0, 0.0 ) );
    vertices->push_back( osg::vec3( 0.0, 0.0, 0.0 ) );
    vertices->push_back( osg::vec3( 0.0, 0.0, 1.0 ) );
    geometry->setVertexArray( vertices );

    osg::Vec4Array* colors( new osg::Vec4Array() );
    colors->push_back( osg::vec4( 1.0F, 0.0F, 0.0F, 1.0F ) );
    colors->push_back( osg::vec4( 1.0F, 0.0F, 0.0F, 1.0F ) );
    colors->push_back( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    colors->push_back( osg::vec4( 0.0F, 1.0F, 0.0F, 1.0F ) );
    colors->push_back( osg::vec4( 0.0F, 0.0F, 1.0F, 1.0F ) );
    colors->push_back( osg::vec4( 0.0F, 0.0F, 1.0F, 1.0F ) );
    geometry->setColorArray( colors, osg::Array::BIND_PER_VERTEX );
    geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, 6 ) );

    geode->addDrawable( geometry );
    return geode;
}

osgAnimation::RigGeometry*
createTesselatedBox( int   nsplit,
                     float size )
{
    osgAnimation::RigGeometry*   riggeometry = new osgAnimation::RigGeometry;

    osg::Geometry*               geometry    = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> vertices( new osg::Vec3Array() );
    osg::ref_ptr<osg::Vec3Array> colors( new osg::Vec3Array() );
    geometry->setVertexArray( vertices.get() );
    geometry->setColorArray( colors.get(), osg::Array::BIND_PER_VERTEX );

    float step = size / static_cast<float>( nsplit );
    float s    = 0.5F / 4.0F;
    for( int i = 0; i < nsplit; i++ )
    {
        float x = -1.0F + static_cast<float>( i ) * step;
        std::cout << x << std::endl;
        vertices->push_back( osg::vec3( x, s, s ) );
        vertices->push_back( osg::vec3( x, -s, s ) );
        vertices->push_back( osg::vec3( x, -s, -s ) );
        vertices->push_back( osg::vec3( x, s, -s ) );
        osg::vec3 c( 0.0F, 0.0F, 0.0F );
        c[i % 3] = 1.0F;
        colors->push_back( c );
        colors->push_back( c );
        colors->push_back( c );
        colors->push_back( c );
    }

    osg::ref_ptr<osg::UIntArray> array = new osg::UIntArray;
    for( int i = 0; i < nsplit - 1; i++ )
    {
        int base = i * 4;
        array->push_back( base );
        array->push_back( base + 1 );
        array->push_back( base + 4 );
        array->push_back( base + 1 );
        array->push_back( base + 5 );
        array->push_back( base + 4 );

        array->push_back( base + 3 );
        array->push_back( base );
        array->push_back( base + 4 );
        array->push_back( base + 7 );
        array->push_back( base + 3 );
        array->push_back( base + 4 );

        array->push_back( base + 5 );
        array->push_back( base + 1 );
        array->push_back( base + 2 );
        array->push_back( base + 2 );
        array->push_back( base + 6 );
        array->push_back( base + 5 );

        array->push_back( base + 2 );
        array->push_back( base + 3 );
        array->push_back( base + 7 );
        array->push_back( base + 6 );
        array->push_back( base + 2 );
        array->push_back( base + 7 );
    }

    geometry->addPrimitiveSet( new osg::DrawElementsUInt( osg::PrimitiveSet::TRIANGLES,
                                                          array->size(),
                                                          &array->front() ) );
    riggeometry->setSourceGeometry( geometry );
    return riggeometry;
}

void
initVertexMap( osgAnimation::Bone*        b0,
               osgAnimation::Bone*        b1,
               osgAnimation::Bone*        b2,
               osgAnimation::RigGeometry* geom,
               osg::Vec3Array*            array )
{
    osgAnimation::VertexInfluenceMap* vim = new osgAnimation::VertexInfluenceMap;
    ( *vim )[b0->getName()].setName( b0->getName() );
    ( *vim )[b1->getName()].setName( b1->getName() );
    ( *vim )[b2->getName()].setName( b2->getName() );
    for( int i = 0; i < ( int )array->size(); i++ )
    {
        float val = ( *array )[i][0];
        std::cout << val << std::endl;
        if( val >= -1.0F && val <= 0.0F )
        {
            ( *vim )[b0->getName()].push_back( osgAnimation::VertexIndexWeight( i,
                                                                                1.0F ) );
        }
        else if( val > 0.0F && val <= 1.0F )
        {
            ( *vim )[b1->getName()].push_back( osgAnimation::VertexIndexWeight( i,
                                                                                1.0F ) );
        }
        else if( val > 1.0F )
        {
            ( *vim )[b2->getName()].push_back( osgAnimation::VertexIndexWeight( i,
                                                                                1.0F ) );
        }
    }

    geom->setInfluenceMap( vim );
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

    osg::ref_ptr<osgAnimation::Skeleton> skelroot = new osgAnimation::Skeleton;
    skelroot->setDefaultUpdateCallback();
    osg::ref_ptr<osgAnimation::Bone> root = new osgAnimation::Bone;
    root->setInvBindMatrixInSkeletonSpace(
        osg::inverse( osg::translate( -1.0, 0.0, 0.0 ) )
    );
    root->setName( "root" );
    osgAnimation::UpdateBone* pRootUpdate = new osgAnimation::UpdateBone( "root" );
    pRootUpdate->getStackedTransforms().push_back(
        new osgAnimation::StackedTranslateElement( "translate",
                                                   osg::vec3( -1.0F, 0.0F, 0.0F ) )
    );
    root->setUpdateCallback( pRootUpdate );

    osg::ref_ptr<osgAnimation::Bone> right0 = new osgAnimation::Bone;
    right0->setInvBindMatrixInSkeletonSpace(
        osg::inverse( osg::translate( 0.0, 0.0, 0.0 ) )
    );
    right0->setName( "right0" );
    osgAnimation::UpdateBone* pRight0Update = new osgAnimation::UpdateBone( "right0" );
    pRight0Update->getStackedTransforms().push_back(
        new osgAnimation::StackedTranslateElement( "translate",
                                                   osg::vec3( 1.0F, 0.0F, 0.0F ) )
    );
    pRight0Update->getStackedTransforms().push_back(
        new osgAnimation::StackedRotateAxisElement( "rotate",
                                                    osg::vec3( 0.0F, 0.0F, 1.0F ),
                                                    0.0 )
    );
    right0->setUpdateCallback( pRight0Update );

    osg::ref_ptr<osgAnimation::Bone> right1 = new osgAnimation::Bone;
    right1->setInvBindMatrixInSkeletonSpace(
        osg::inverse( osg::translate( 1.0, 0.0, 0.0 ) )
    );
    right1->setName( "right1" );
    osgAnimation::UpdateBone* pRight1Update = new osgAnimation::UpdateBone( "right1" );
    pRight1Update->getStackedTransforms().push_back(
        new osgAnimation::StackedTranslateElement( "translate",
                                                   osg::vec3( 1.0F, 0.0F, 0.0F ) )
    );
    pRight1Update->getStackedTransforms().push_back(
        new osgAnimation::StackedRotateAxisElement( "rotate",
                                                    osg::vec3( 0.0F, 0.0F, 1.0F ),
                                                    0.0 )
    );
    right1->setUpdateCallback( pRight1Update );

    root->addChild( right0.get() );
    right0->addChild( right1.get() );
    skelroot->addChild( root.get() );

    osg::Group*                                       scene = new osg::Group;
    osg::ref_ptr<osgAnimation::BasicAnimationManager> manager =
        new osgAnimation::BasicAnimationManager;
    scene->setUpdateCallback( manager.get() );

    osgAnimation::Animation* anim = new osgAnimation::Animation;
    {
        osgAnimation::FloatKeyframeContainer* keys0 =
            new osgAnimation::FloatKeyframeContainer;
        keys0->push_back( osgAnimation::FloatKeyframe( 0.0, 0.0F ) );
        keys0->push_back( osgAnimation::FloatKeyframe( 3.0, osg::PI_2 ) );
        keys0->push_back( osgAnimation::FloatKeyframe( 6.0, osg::PI_2 ) );
        osgAnimation::FloatLinearSampler* sampler = new osgAnimation::FloatLinearSampler;
        sampler->setKeyframeContainer( keys0 );
        osgAnimation::FloatLinearChannel* channel =
            new osgAnimation::FloatLinearChannel( sampler );
        channel->setName( "rotate" );
        channel->setTargetName( "right0" );
        anim->addChannel( channel );
    }

    {
        osgAnimation::FloatKeyframeContainer* keys1 =
            new osgAnimation::FloatKeyframeContainer;
        keys1->push_back( osgAnimation::FloatKeyframe( 0.0, 0.0F ) );
        keys1->push_back( osgAnimation::FloatKeyframe( 3.0, 0.0F ) );
        keys1->push_back( osgAnimation::FloatKeyframe( 6.0, osg::PI_2 ) );
        osgAnimation::FloatLinearSampler* sampler = new osgAnimation::FloatLinearSampler;
        sampler->setKeyframeContainer( keys1 );
        osgAnimation::FloatLinearChannel* channel =
            new osgAnimation::FloatLinearChannel( sampler );
        channel->setName( "rotate" );
        channel->setTargetName( "right1" );
        anim->addChannel( channel );
    }
    manager->registerAnimation( anim );
    manager->buildTargetReference();

    // let's start !
    manager->playAnimation( anim );

    // we will use local data from the skeleton
    osg::MatrixTransform* rootTransform = new osg::MatrixTransform;
    rootTransform->setMatrix( osg::rotate( osg::PI_2, 1.0, 0.0, 0.0 ) );
    right0->addChild( createAxis() );
    right0->setDataVariance( osg::Object::DataVariance::DYNAMIC );
    right1->addChild( createAxis() );
    right1->setDataVariance( osg::Object::DataVariance::DYNAMIC );
    osg::MatrixTransform* trueroot = new osg::MatrixTransform;
    trueroot->setMatrix( osg::dmat4( root->getMatrixInBoneSpace().data() ) );
    trueroot->addChild( createAxis() );
    trueroot->addChild( skelroot.get() );
    trueroot->setDataVariance( osg::Object::DataVariance::DYNAMIC );
    rootTransform->addChild( trueroot );
    scene->addChild( rootTransform );

    osgAnimation::RigGeometry* geom  = createTesselatedBox( 4, 4.0F );
    osg::Geode*                geode = new osg::Geode;
    geode->addDrawable( geom );
    skelroot->addChild( geode );
    osg::ref_ptr<osg::Vec3Array> src =
        dynamic_cast<osg::Vec3Array*>( geom->getSourceGeometry()->getVertexArray() );
    geom->setDataVariance( osg::Object::DataVariance::DYNAMIC );

    initVertexMap( root.get(), right0.get(), right1.get(), geom, src.get() );

    // let's run !
    viewer.setSceneData( scene );
    viewer.realize();

    while( !viewer.done() )
    {
        viewer.frame();
    }

    // .osg format removed; debug save disabled
    // osgDB::writeNodeFile(*scene, "skinning.osg");
    return 0;
}
