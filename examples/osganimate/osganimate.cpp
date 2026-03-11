/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osganimate example application
 */
#include <iostream>
#include <osg/core/Notify.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgDB/registry/Registry.hpp>
#include <osgGA/manipulators/DriveManipulator.hpp>
#include <osgGA/manipulators/FlightManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgSim/OverlayNode>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>

osg::AnimationPath*
createAnimationPath( const osg::vec3& center,
                     float            radius,
                     double           looptime )
{
    // set up the animation path
    osg::AnimationPath* animationPath = new osg::AnimationPath;
    animationPath->setLoopMode( osg::AnimationPath::LOOP );

    int    numSamples = 40;
    float  yaw        = 0.0F;
    float  yaw_delta  = 2.0F * osg::PI / ( ( float )numSamples - 1.0F );
    float  roll       = osg::radians( 30.0F );

    double time       = 0.0F;
    double time_delta = looptime / ( double )numSamples;
    for( int i = 0; i < numSamples; ++i )
    {
        osg::vec3 position(
            center + osg::vec3( sinf( yaw ) * radius, cosf( yaw ) * radius, 0.0F )
        );
        osg::quat rotation( osg::quat( roll, osg::vec3( 0.0, 1.0, 0.0 ) ) *
                            osg::quat( -( yaw + osg::radians( 90.0F ) ),
                                       osg::vec3( 0.0, 0.0, 1.0 ) ) );

        animationPath->insert( time,
                               osg::AnimationPath::ControlPoint( osg::dvec3( position ),
                                                                 rotation ) );

        yaw  += yaw_delta;
        time += time_delta;
    }
    return animationPath;
}

osg::Node*
createBase( const osg::vec3& center,
            float            radius )
{

    int             numTilesX = 10;
    int             numTilesY = 10;

    float           width     = 2 * radius;
    float           height    = 2 * radius;

    osg::vec3       v000( center - osg::vec3( width * 0.5F, height * 0.5F, 0.0F ) );
    osg::vec3       dx( osg::vec3( width / ( ( float )numTilesX ), 0.0, 0.0F ) );
    osg::vec3       dy( osg::vec3( 0.0F, height / ( ( float )numTilesY ), 0.0F ) );

    // fill in vertices for grid, note numTilesX+1 * numTilesY+1...
    osg::Vec3Array* coords = new osg::Vec3Array;
    int             iy;
    for( iy = 0; iy <= numTilesY; ++iy )
    {
        for( int ix = 0; ix <= numTilesX; ++ix )
        {
            coords->push_back( v000 + dx * ( float )ix + dy * ( float )iy );
        }
    }

    // Just two colours - black and white.
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );    // white
    colors->push_back( osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );    // black

    osg::ref_ptr<osg::DrawElementsUShort> whitePrimitives =
        new osg::DrawElementsUShort( GL_TRIANGLES );
    osg::ref_ptr<osg::DrawElementsUShort> blackPrimitives =
        new osg::DrawElementsUShort( GL_TRIANGLES );

    int numIndicesPerRow = numTilesX + 1;
    for( iy = 0; iy < numTilesY; ++iy )
    {
        for( int ix = 0; ix < numTilesX; ++ix )
        {
            osg::DrawElementsUShort* primitives =
                ( ( iy + ix ) % 2 == 0 ) ? whitePrimitives.get() : blackPrimitives.get();
            // First triangle
            primitives->push_back( ix + ( iy + 1 ) * numIndicesPerRow );
            primitives->push_back( ix + iy * numIndicesPerRow );
            primitives->push_back( ( ix + 1 ) + iy * numIndicesPerRow );
            // Second triangle
            primitives->push_back( ix + ( iy + 1 ) * numIndicesPerRow );
            primitives->push_back( ( ix + 1 ) + iy * numIndicesPerRow );
            primitives->push_back( ( ix + 1 ) + ( iy + 1 ) * numIndicesPerRow );
        }
    }

    // set up a single normal
    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back( osg::vec3( 0.0F, 0.0F, 1.0F ) );

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray( coords );

    geom->setColorArray( colors, osg::Array::BIND_PER_PRIMITIVE_SET );

    geom->setNormalArray( normals, osg::Array::BIND_OVERALL );

    geom->addPrimitiveSet( whitePrimitives.get() );
    geom->addPrimitiveSet( blackPrimitives.get() );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( geom );

    return geode;
}

osg::Node*
createMovingModel( const osg::vec3& center,
                   float            radius )
{
    float               animationLength = 10.0F;

    osg::AnimationPath* animationPath =
        createAnimationPath( center, radius, animationLength );

    osg::ref_ptr<osg::Group> model  = new osg::Group;

    osg::ref_ptr<osg::Node>  glider = osgDB::readRefNodeFile( "fox.glb" );
    if( glider )
    {
        const osg::sphere&    bs         = glider->getBound();

        float                 size       = radius / bs.radius * 0.3F;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance( osg::Object::DataVariance::STATIC );
        positioned->setMatrix(
            osg::rotate( osg::radians( -90.0 ), 0.0, 0.0, 1.0 ) *
            osg::scale( ( double )size, ( double )size, ( double )size ) *
            osg::translate( osg::dvec3( -bs.center ) )
        );

        positioned->addChild( glider );

        osg::PositionAttitudeTransform* xform = new osg::PositionAttitudeTransform;
        xform->setUpdateCallback(
            new osg::AnimationPathCallback( animationPath, 0.0, 1.0 )
        );
        xform->addChild( positioned );

        model->addChild( xform );
    }

    osg::ref_ptr<osg::Node> cessna = osgDB::readRefNodeFile( "damaged_helmet.glb" );
    if( cessna )
    {
        const osg::sphere&    bs         = cessna->getBound();

        float                 size       = radius / bs.radius * 0.3F;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance( osg::Object::DataVariance::STATIC );
        positioned->setMatrix(
            osg::rotate( osg::radians( 180.0 ), 0.0, 0.0, 1.0 ) *
            osg::scale( ( double )size, ( double )size, ( double )size ) *
            osg::translate( osg::dvec3( -bs.center ) )
        );

        positioned->addChild( cessna );

        osg::ref_ptr<osg::MatrixTransform> xform = new osg::MatrixTransform;
        xform->setUpdateCallback(
            new osg::AnimationPathCallback( animationPath, 0.0F, 2.0 )
        );
        xform->addChild( positioned );

        model->addChild( xform );
    }

    return model.release();
}

osg::ref_ptr<osg::Group>
createModel( bool                                  overlay,
             osgSim::OverlayNode::OverlayTechnique technique )
{
    osg::vec3                center( 0.0F, 0.0F, 0.0F );
    float                    radius     = 100.0F;

    osg::ref_ptr<osg::Group> root       = new osg::Group;

    float                    baseHeight = center.z - radius * 0.5;
    osg::ref_ptr<osg::Node>  baseModel =
        createBase( osg::vec3( center.x, center.y, baseHeight ), radius );
    osg::ref_ptr<osg::Node> movingModel = createMovingModel( center, radius * 0.8F );

    if( overlay )
    {
        osgSim::OverlayNode* overlayNode = new osgSim::OverlayNode( technique );
        overlayNode->setContinuousUpdate( true );
        overlayNode->setOverlaySubgraph( movingModel );
        overlayNode->setOverlayBaseHeight( baseHeight - 0.01 );
        overlayNode->addChild( baseModel );
        root->addChild( overlayNode );
    }
    else
    {

        root->addChild( baseModel );
    }

    root->addChild( movingModel );

    return root;
}

int
main( int    argc,
      char** argv )
{

    bool                overlay = false;
    osg::ArgumentParser arguments( &argc, argv );
    while( arguments.read( "--overlay" ) )
    {
        overlay = true;
    }

    osgSim::OverlayNode::OverlayTechnique technique =
        osgSim::OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY;
    while( arguments.read( "--object" ) )
    {
        technique = osgSim::OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY;
        overlay   = true;
    }
    while( arguments.read( "--ortho" ) || arguments.read( "--orthographic" ) )
    {
        technique = osgSim::OverlayNode::VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY;
        overlay   = true;
    }
    while( arguments.read( "--persp" ) || arguments.read( "--perspective" ) )
    {
        technique = osgSim::OverlayNode::VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY;
        overlay   = true;
    }

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "fox.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer        viewer;

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Group> model = createModel( overlay, technique );
    if( !model )
    {
        return 1;
    }

    // tilt the scene so the default eye position is looking down on the model.
    osg::ref_ptr<osg::MatrixTransform> rootnode = new osg::MatrixTransform;
    rootnode->setMatrix(
        osg::dmat4( osg::rotate( osg::radians( 30.0F ), 1.0F, 0.0F, 0.0F ) )
    );
    rootnode->addChild( model );

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize( rootnode );

    std::string filename;
    if( arguments.read( "-o", filename ) )
    {
        osgDB::writeNodeFile( *rootnode, filename );
        return 1;
    }

    // set the scene to render
    viewer.setSceneData( rootnode );

    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

    // viewer.setUpViewOnSingleScreen(1);

#if 0

    // use of custom simulation time.

    viewer.realize();

    double simulationTime = 0.0;

    while (!viewer.done())
    {
        viewer.frame(simulationTime);
        simulationTime += 0.001;
    }

    return 0;
#else

    // normal viewer usage.
    return viewer.run();

#endif
}
