/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgparticleshader example application
 */
#include <iostream>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/Point.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgParticle/AccelOperator>
#include <osgParticle/BounceOperator>
#include <osgParticle/DampingOperator>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ModularProgram>
#include <osgParticle/ParticleSystem>
#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/SinkOperator>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

void
createFountainEffect( osgParticle::ModularEmitter* emitter,
                      osgParticle::ModularProgram* program )
{
    // Emit specific number of particles every frame
    osg::ref_ptr<osgParticle::RandomRateCounter> rrc =
        new osgParticle::RandomRateCounter;
    rrc->setRateRange( 500, 2'000 );

    // Accelerate particles in the given gravity direction.
    osg::ref_ptr<osgParticle::AccelOperator> accel = new osgParticle::AccelOperator;
    accel->setToGravity();

    // Multiply each particle's velocity by a damping constant.
    osg::ref_ptr<osgParticle::DampingOperator> damping =
        new osgParticle::DampingOperator;
    damping->setDamping( 0.9F );

    // Bounce particles off objects defined by one or more domains.
    // Supported domains include triangle, rectangle, plane, disk and sphere.
    // Since a bounce always happens instantaneously, it will not work correctly with
    // unstable delta-time. At present, even the floating error of dt (which are applied
    // to ParticleSystem and Operator separately) causes wrong bounce results. Some one
    // else may have better solutions for this.
    osg::ref_ptr<osgParticle::BounceOperator> bounce = new osgParticle::BounceOperator;
    bounce->setFriction( -0.05 );
    bounce->setResilience( 0.35 );
    bounce->addDiskDomain( osg::vec3( 0.0F, 0.0F, -2.0F ),
                           osg::vec3( 0.0F, 0.0F, 1.0F ),
                           8.0F );
    bounce->addPlaneDomain( osg::Plane( osg::dvec3( 0.0, 0.0, 1.0 ), 5.0 ) );

    // Kill particles going inside/outside of specified domains.
    osg::ref_ptr<osgParticle::SinkOperator> sink = new osgParticle::SinkOperator;
    sink->setSinkStrategy( osgParticle::SinkOperator::SINK_OUTSIDE );
    sink->addSphereDomain( osg::vec3(), 20.0F );

    emitter->setCounter( rrc.get() );
    program->addOperator( accel.get() );
    program->addOperator( damping.get() );
    program->addOperator( bounce.get() );
    program->addOperator( sink.get() );
}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    std::string         textureFile( "Images/smoke.rgb" );
    while( arguments.read( "--texture", textureFile ) )
    {
    }

    float pointSize = 20.0F;
    while( arguments.read( "--point", pointSize ) )
    {
    }

    double visibilityDistance = -1.0F;
    while( arguments.read( "--visibility", visibilityDistance ) )
    {
    }

    bool useShaders = true;
    while( arguments.read( "--disable-shaders" ) )
    {
        useShaders = false;
    }

    /***
    Customize particle template and system attributes
    ***/
    osg::ref_ptr<osgParticle::ParticleSystem> ps = new osgParticle::ParticleSystem;

    ps->getDefaultParticleTemplate().setLifeTime( 5.0F );

    // The shader only supports rendering points at present.
    ps->getDefaultParticleTemplate().setShape( osgParticle::Particle::POINT );

    // Set the visibility distance of particles, due to their Z-value in the eye
    // coordinates. Particles that are out of the distance (or behind the eye) will not
    // be rendered.
    ps->setVisibilityDistance( visibilityDistance );

    if( useShaders )
    {
        // Set using local GLSL shaders to render particles.
        // At present, this is slightly efficient than ordinary methods. The bottlenack
        // here seems to be the cull traversal time. Operators go through the particle
        // list again and again...
        ps->setDefaultAttributesUsingShaders( textureFile, true, 0 );
    }
    else
    {
        ps->setDefaultAttributes( textureFile, true, false, 0 );

        // Without the help of shaders, we have to sort particles to make the visibility
        // distance work. Sorting is also useful in rendering transparent particles in
        // back-to-front order.
        if( visibilityDistance > 0.0 )
        {
            ps->setSortMode( osgParticle::ParticleSystem::SORT_BACK_TO_FRONT );
        }
    }

    // Set the points size.
    osg::StateSet* stateset = ps->getOrCreateStateSet();
    stateset->setAttribute( new osg::Point( pointSize ) );

    /***
    Construct other particle system elements, including the emitter and program
    ***/
    osg::ref_ptr<osgParticle::ModularEmitter> emitter = new osgParticle::ModularEmitter;
    emitter->setParticleSystem( ps.get() );

    osg::ref_ptr<osgParticle::ModularProgram> program = new osgParticle::ModularProgram;
    program->setParticleSystem( ps.get() );

    createFountainEffect( emitter.get(), program.get() );

    /***
    Add the entire particle system to the scene graph
    ***/
    osg::ref_ptr<osg::MatrixTransform> parent = new osg::MatrixTransform;
    parent->addChild( emitter.get() );
    parent->addChild( program.get() );

    // The updater can receive particle systems as child drawables now. The
    // addParticleSystem() method is still usable, with which we should define another
    // geode to contain a particle system.
    osg::ref_ptr<osgParticle::ParticleSystemUpdater> updater =
        new osgParticle::ParticleSystemUpdater;
    // updater->addDrawable( ps.get() );

    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( parent.get() );
    root->addChild( updater.get() );

    // FIXME 2010.9.19: the updater can't be a drawable; otherwise the ParticleEffect
    // will not work properly. why?
    updater->addParticleSystem( ps.get() );

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( ps.get() );
    root->addChild( geode.get() );

    /***
    Start the viewer
    ***/
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
    viewer.addEventHandler(
        new osgGA::StateSetManipulator( viewer.getCamera()->getOrCreateStateSet() )
    );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( root.get() );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    // A floating error of delta-time should be explained here:
    // The particles emitter, program and updater all use a 'dt' to compute the time
    // value in every frame. Because the 'dt' is a double value, it is not suitable to
    // keep three copies of it separately, which is the previous implementation. The
    // small error makes some operators unable to work correctly, e.g. the
    // BounceOperator. Now we make use of the getDeltaTime() of ParticleSystem to
    // maintain and dispatch the delta time. But.. it is not the best solution so far,
    // since there are still very few particles acting unexpectedly.
    return viewer.run();

    // FIXME 2010.9.19: At present, getDeltaTime() is not used and the implementations in
    // the updater and processors still use a (t - _t0) as the delta time, which is of
    // course causing floating errors. ParticleEffect will not work if we replace the
    // delta time with getDeltaTime()... Need to find a solution.
}
