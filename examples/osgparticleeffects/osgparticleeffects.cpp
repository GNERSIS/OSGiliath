/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgparticleeffects example application
 */
#include <osg/core/io_utils.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/nodes/PositionAttitudeTransform.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/ExplosionEffect>
#include <osgParticle/FireEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgText/Text>
#include <osgUtil/optimization/Optimizer.hpp>
#include <osgViewer/core/Viewer.hpp>

// for the grid data..
#include "../osghangglide/terrain_coords.hpp"

#include <osg/rendering/HeadlessCapture.hpp>

osg::vec3 wind( 1.0F,
                0.0F,
                0.0F );

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
        osg::quat rotation( osg::quat( -( yaw + osg::radians( 90.0F ) ),
                                       osg::vec3( 0.0, 0.0, 1.0 ) ) *
                            osg::quat( roll, osg::vec3( 0.0, 1.0, 0.0 ) ) );

        animationPath->insert( time,
                               osg::AnimationPath::ControlPoint( osg::dvec3( position ),
                                                                 rotation ) );

        yaw  += yaw_delta;
        time += time_delta;
    }
    return animationPath;
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
        float                 size       = radius / bs.radius * 0.15F;

        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance( osg::Object::DataVariance::STATIC );
        positioned->setMatrix(
            osg::dmat4( osg::rotate( osg::radians( -90.0F ), 0.0F, 0.0F, 1.0F ) *
                        osg::scale( size, size, size ) *
                        osg::translate( -bs.center ) )
        );

        positioned->addChild( glider );

        osg::PositionAttitudeTransform* xform = new osg::PositionAttitudeTransform;
        xform->setDataVariance( osg::Object::DataVariance::DYNAMIC );
        xform->setUpdateCallback(
            new osg::AnimationPathCallback( animationPath, 0.0, 0.5 )
        );
        xform->addChild( positioned );

        model->addChild( xform );
    }

    osg::ref_ptr<osg::Node> cessna = osgDB::readRefNodeFile( "damaged_helmet.glb" );
    if( cessna )
    {
        const osg::sphere&    bs         = cessna->getBound();
        float                 size       = radius / bs.radius * 0.15F;

        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance( osg::Object::DataVariance::STATIC );
        positioned->setMatrix(
            osg::dmat4( osg::rotate( osg::radians( 180.0F ), 0.0F, 0.0F, 1.0F ) *
                        osg::scale( size, size, size ) *
                        osg::translate( -bs.center ) )
        );

        // positioned->addChild(cessna);
        positioned->addChild( cessna );

        osg::MatrixTransform* xform = new osg::MatrixTransform;
        xform->setDataVariance( osg::Object::DataVariance::DYNAMIC );
        xform->setUpdateCallback(
            new osg::AnimationPathCallback( animationPath, 0.0F, 1.0 )
        );
        xform->addChild( positioned );

        model->addChild( xform );
    }

    return model.release();
}

osg::vec3
computeTerrainIntersection( osg::Node* subgraph,
                            float      x,
                            float      y )
{
    const osg::sphere&                            bs   = subgraph->getBound();
    float                                         zMax = bs.center.z + bs.radius;
    float                                         zMin = bs.center.z - bs.radius;

    osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
        new osgUtil::LineSegmentIntersector( osg::dvec3( x, y, zMin ),
                                             osg::dvec3( x, y, zMax ) );

    osgUtil::IntersectionVisitor iv( intersector.get() );

    subgraph->accept( iv );

    if( intersector->containsIntersections() )
    {
        return osg::vec3( intersector->getFirstIntersection().getWorldIntersectPoint() );
    }

    return osg::vec3( x, y, 0.0F );
}

//////////////////////////////////////////////////////////////////////////////
// MAIN SCENE GRAPH BUILDING FUNCTION
//////////////////////////////////////////////////////////////////////////////

void
build_world( osg::Group* root )
{

    osg::Geode* terrainGeode = new osg::Geode;
    // create terrain
    {
        osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();
        osg::ref_ptr<osg::Image>    image = osgDB::readRefImageFile( "Images/lz.rgb" );
        if( image )
        {
            osg::Texture2D* texture = new osg::Texture2D;
            texture->setImage( image );
            stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );
        }

        terrainGeode->setStateSet( stateset );

        float             size    = 1'000;           // 10km;
        float             scale   = size / 39.0F;    // 10km;
        float             z_scale = scale * 3.0F;

        osg::HeightField* grid    = new osg::HeightField;
        grid->allocate( 38, 39 );
        grid->setXInterval( scale );
        grid->setYInterval( scale );

        for( unsigned int r = 0; r < 39; ++r )
        {
            for( unsigned int c = 0; c < 38; ++c )
            {
                grid->setHeight( c, r, z_scale * vertex[r + c * 39][2] );
            }
        }
        terrainGeode->addDrawable( new osg::ShapeDrawable( grid ) );

        root->addChild( terrainGeode );
    }

    // create particle effects
    {
        osg::vec3 position = computeTerrainIntersection( terrainGeode, 100.0F, 100.0F );

        osgParticle::ExplosionEffect* explosion =
            new osgParticle::ExplosionEffect( position, 10.0F );
        osgParticle::ExplosionDebrisEffect* explosionDebri =
            new osgParticle::ExplosionDebrisEffect( position, 10.0F );
        osgParticle::SmokeEffect* smoke =
            new osgParticle::SmokeEffect( position, 10.0F );
        osgParticle::FireEffect* fire = new osgParticle::FireEffect( position, 10.0F );

        explosion->setWind( wind );
        explosionDebri->setWind( wind );
        smoke->setWind( wind );
        fire->setWind( wind );

        root->addChild( explosion );
        root->addChild( explosionDebri );
        root->addChild( smoke );
        root->addChild( fire );
    }

    // create particle effects
    {
        osg::vec3 position = computeTerrainIntersection( terrainGeode, 200.0F, 100.0F );

        osgParticle::ExplosionEffect* explosion =
            new osgParticle::ExplosionEffect( position, 1.0F );
        osgParticle::ExplosionDebrisEffect* explosionDebri =
            new osgParticle::ExplosionDebrisEffect( position, 1.0F );
        osgParticle::SmokeEffect* smoke = new osgParticle::SmokeEffect( position, 1.0F );
        osgParticle::FireEffect*  fire  = new osgParticle::FireEffect( position, 1.0F );

        explosion->setWind( wind );
        explosionDebri->setWind( wind );
        smoke->setWind( wind );
        fire->setWind( wind );

        root->addChild( explosion );
        root->addChild( explosionDebri );
        root->addChild( smoke );
        root->addChild( fire );
    }

    // create the moving models.
    {
        root->addChild( createMovingModel( osg::vec3( 500.0F, 500.0F, 500.0F ),
                                           300.0F ) );
    }
}

// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler
{
    public:

        PickHandler()
        {
        }

        bool
        handle( const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter&      aa )
        {
            switch( ea.getEventType() )
            {
                case( osgGA::GUIEventAdapter::PUSH ) :
                    {
                        osgViewer::Viewer* viewer =
                            dynamic_cast<osgViewer::Viewer*>( &aa );
                        pick( viewer, ea );
                    }
                    return false;

                default :
                    return false;
            }
        }

        void
        pick( osgViewer::Viewer*            viewer,
              const osgGA::GUIEventAdapter& ea )
        {
            osg::Group* root = dynamic_cast<osg::Group*>( viewer->getSceneData() );
            if( !root )
            {
                return;
            }

            osgUtil::LineSegmentIntersector::Intersections intersections;
            if( viewer->computeIntersections( ea, intersections ) )
            {
                const osgUtil::LineSegmentIntersector::Intersection& hit =
                    *intersections.begin();

                bool                 handleMovingModels = false;
                const osg::NodePath& nodePath           = hit.nodePath;
                for( osg::NodePath::const_iterator nitr = nodePath.begin();
                     nitr != nodePath.end();
                     ++nitr )
                {
                    const osg::Transform* transform =
                        dynamic_cast<const osg::Transform*>( *nitr );
                    if( transform )
                    {
                        if( transform->getDataVariance() ==
                            osg::Object::DataVariance::DYNAMIC )
                        {
                            handleMovingModels = true;
                        }
                    }
                }

                osg::vec3 position =
                    osg::vec3( handleMovingModels ? hit.getLocalIntersectPoint()
                                                  : hit.getWorldIntersectPoint() );
                float scale     = 10.0F * ( ( float )rand() / ( float )RAND_MAX );
                float intensity = 1.0F;

                osgParticle::ExplosionEffect* explosion =
                    new osgParticle::ExplosionEffect( position, scale, intensity );
                osgParticle::ExplosionDebrisEffect* explosionDebri =
                    new osgParticle::ExplosionDebrisEffect( position, scale, intensity );
                osgParticle::FireEffect* fire =
                    new osgParticle::FireEffect( position, scale, intensity );
                osgParticle::ParticleEffect* smoke = 0;
                if( handleMovingModels )
                {
                    smoke =
                        new osgParticle::SmokeTrailEffect( position, scale, intensity );
                }
                else
                {
                    smoke = new osgParticle::SmokeEffect( position, scale, intensity );
                }

                explosion->setWind( wind );
                explosionDebri->setWind( wind );
                smoke->setWind( wind );
                fire->setWind( wind );

                osg::Group* effectsGroup = new osg::Group;
                effectsGroup->addChild( explosion );
                effectsGroup->addChild( explosionDebri );
                effectsGroup->addChild( smoke );
                effectsGroup->addChild( fire );

                if( handleMovingModels )
                {
                    // insert particle effects alongside the hit node, therefore able to
                    // track that nodes movement, however, this does require us to insert
                    // the ParticleSystem itself into the root of the scene graph
                    // separately from the main particle effects group which contains the
                    // emitters and programs. the follow code block implements this, note
                    // the path for handling particle effects which aren't attached to
                    // moving models is easy - just a single line of code!

                    // tell the effects not to attach to the particle system locally for
                    // rendering, as we'll handle add it into the scene graph ourselves.
                    explosion->setUseLocalParticleSystem( false );
                    explosionDebri->setUseLocalParticleSystem( false );
                    smoke->setUseLocalParticleSystem( false );
                    fire->setUseLocalParticleSystem( false );

                    // find a place to insert the particle effects group alongside the
                    // hit node. there are two possible ways that this can be done,
                    // either insert it into a pre-existing group along side the hit
                    // node, or if no pre existing group is found then this needs to be
                    // inserted above the hit node, and then the particle effect can be
                    // inserted into this.
                    osg::ref_ptr<osg::Node> hitNode        = hit.nodePath.back();
                    osg::Node::ParentList   parents        = hitNode->getParents();
                    osg::Group*             insertGroup    = 0;
                    unsigned int            numGroupsFound = 0;
                    for( osg::Node::ParentList::iterator itr = parents.begin();
                         itr != parents.end();
                         ++itr )
                    {
                        osg::Group* parent = ( *itr );
                        if( typeid( *parent ) == typeid( osg::Group ) )
                        {
                            ++numGroupsFound;
                            insertGroup = parent;
                        }
                    }
                    if( numGroupsFound ==
                        parents.size() &&
                        numGroupsFound ==
                        1 &&
                        insertGroup )
                    {
                        osg::notify( osg::INFO )
                            << "PickHandler::pick(,) hit node's parent is a single "
                               "osg::Group so we can simple the insert the particle "
                               "effects group here."
                            << std::endl;

                        // just reuse the existing group.
                        insertGroup->addChild( effectsGroup );
                    }
                    else
                    {
                        osg::notify( osg::INFO )
                            << "PickHandler::pick(,) hit node doesn't have an "
                               "appropriate osg::Group node to insert particle effects "
                               "into, inserting a new osg::Group."
                            << std::endl;
                        insertGroup = new osg::Group;
                        for( osg::Node::ParentList::iterator itr = parents.begin();
                             itr != parents.end();
                             ++itr )
                        {
                            ( *itr )->replaceChild( hit.nodePath.back(), insertGroup );
                        }
                        insertGroup->addChild( hitNode.get() );
                        insertGroup->addChild( effectsGroup );
                    }

                    // finally insert the particle systems into a Geode and attach to the
                    // root of the scene graph so the particle system can be rendered.
                    osg::Geode* geode = new osg::Geode;
                    geode->addDrawable( explosion->getParticleSystem() );
                    geode->addDrawable( explosionDebri->getParticleSystem() );
                    geode->addDrawable( smoke->getParticleSystem() );
                    geode->addDrawable( fire->getParticleSystem() );

                    root->addChild( geode );
                }
                else
                {
                    // when we don't have moving models we can simple insert the particle
                    // effect into the root of the scene graph
                    osg::notify( osg::INFO )
                        << "PickHandler::pick(,) adding particle effects to root node."
                        << std::endl;
                    root->addChild( effectsGroup );
                }

#if 0
            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(position,scale)));
            group->addChild(geode);
#endif
            }
        }

    protected:

        virtual ~PickHandler()
        {
        }
};

// function used in debugging
void
insertParticle( osg::Group*      root,
                const osg::vec3& center,
                float            radius )
{
    bool      handleMovingModels = false;

    osg::vec3 position =
        center +
        osg::vec3( radius * ( ( ( float )rand() / ( float )RAND_MAX ) - 0.5 ) * 2.0,
                   radius * ( ( ( float )rand() / ( float )RAND_MAX ) - 0.5 ) * 2.0,
                   0.0F );

    float scale     = 10.0F * ( ( float )rand() / ( float )RAND_MAX );
    float intensity = 1.0F;

    osgParticle::ExplosionEffect* explosion =
        new osgParticle::ExplosionEffect( position, scale, intensity );
    osgParticle::ExplosionDebrisEffect* explosionDebri =
        new osgParticle::ExplosionDebrisEffect( position, scale, intensity );
    osgParticle::FireEffect* fire =
        new osgParticle::FireEffect( position, scale, intensity );
    osgParticle::ParticleEffect* smoke = 0;
    if( handleMovingModels )
    {
        smoke = new osgParticle::SmokeTrailEffect( position, scale, intensity );
    }
    else
    {
        smoke = new osgParticle::SmokeEffect( position, scale, intensity );
    }

    explosion->setWind( wind );
    explosionDebri->setWind( wind );
    smoke->setWind( wind );
    fire->setWind( wind );

    osg::Group* effectsGroup = new osg::Group;
    effectsGroup->addChild( explosion );
    effectsGroup->addChild( explosionDebri );
    effectsGroup->addChild( smoke );
    effectsGroup->addChild( fire );

    root->addChild( effectsGroup );
}

//////////////////////////////////////////////////////////////////////////////
// main()
//////////////////////////////////////////////////////////////////////////////

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    osg::Group*         root = new osg::Group;
    build_world( root );

    osgUtil::Optimizer optimizer;
    optimizer.optimize( root );

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

    // construct the viewer.
    osgViewer::Viewer viewer;

    // register the pick handler
    viewer.addEventHandler( new PickHandler() );

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( root );
    return viewer.run();
}
