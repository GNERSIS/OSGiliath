/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgdeferred example application
 */
#include "osgdeferred.hpp"

#include <osg/maths/compat.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/PolygonMode.hpp>
#include <osg/traversal/AnimationPath.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgShadow/SoftShadowMap>
#include <osgUtil/optimization/TangentSpaceGenerator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

#ifdef OSG_LIBRARY_STATIC
// in case of a static build...
USE_OSGPLUGIN( osg2 )
USE_OSGPLUGIN( png )
USE_OSGPLUGIN( jpeg )
USE_OSGPLUGIN( glsl )
USE_SERIALIZER_WRAPPER_LIBRARY( osg )
USE_GRAPHICSWINDOW()
#endif

osg::Texture2D*
createFloatTexture2D( int textureSize )
{
    osg::ref_ptr<osg::Texture2D> tex2D = new osg::Texture2D;
    tex2D->setTextureSize( textureSize, textureSize );
    tex2D->setInternalFormat( GL_RGBA16F_ARB );
    tex2D->setSourceFormat( GL_RGBA );
    tex2D->setSourceType( GL_FLOAT );
    tex2D->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    tex2D->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
    return tex2D.release();
}

osg::Camera*
createHUDCamera( double left,
                 double right,
                 double bottom,
                 double top )
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setClearMask( GL_DEPTH_BUFFER_BIT );
    camera->setRenderOrder( osg::Camera::POST_RENDER );
    camera->setAllowEventFocus( false );
    camera->setProjectionMatrix( osg::ortho2D( left, right, bottom, top ) );
    // GL_LIGHTING removed for Core Profile compatibility
    return camera.release();
}

osg::ref_ptr<osg::LightSource>
createLight( const osg::vec3& pos )
{
    osg::ref_ptr<osg::LightSource> light = new osg::LightSource;
    light->getLight()->setPosition( osg::vec4( pos.x, pos.y, pos.z, 1 ) );
    light->getLight()->setAmbient( osg::vec4( 0.2, 0.2, 0.2, 1 ) );
    light->getLight()->setDiffuse( osg::vec4( 0.8, 0.8, 0.8, 1 ) );
    return light;
}

class CreateTangentSpace : public osg::DualModeVisitor
{
    public:

        CreateTangentSpace() :
            DualModeVisitor( NodeVisitor::TRAVERSE_ALL_CHILDREN ),
            tsg( new osgUtil::TangentSpaceGenerator )
        {
        }

        virtual void
        apply( osg::Geode& geode )
        {
            for( unsigned int i = 0; i < geode.getNumDrawables(); ++i )
            {
                osg::Geometry* geo =
                    dynamic_cast<osg::Geometry*>( geode.getDrawable( i ) );
                if( geo != NULL )
                {
                    // assume the texture coordinate for normal maps is stored in unit #0
                    tsg->generate( geo, 0 );
                    // pass2.vert expects the tangent array to be stored inside
                    // gl_MultiTexCoord1
                    geo->setTexCoordArray( 1, tsg->getTangentArray() );
                }
            }
            traverse( geode );
        }

    private:

        osg::ref_ptr<osgUtil::TangentSpaceGenerator> tsg;
};

Pipeline
createPipelinePlainOSG( osg::ref_ptr<osg::Group>               scene,
                        osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene,
                        const osg::vec3                        lightPos )
{
    Pipeline p;
    p.graph       = new osg::Group;
    p.textureSize = 1'024;

    // Pass 1 (shadow).
    p.pass1Shadows = createFloatTexture2D( p.textureSize );
    osg::ref_ptr<osg::Camera> pass1 =
        createRTTCamera( osg::Camera::COLOR_BUFFER, p.pass1Shadows );
    pass1->addChild( shadowedScene.get() );

    // pass2 shades expects tangent vectors to be available as texcoord array for texture
    // #1 we use osgUtil::TangentSpaceGenerator to generate these
    CreateTangentSpace cts;
    scene->accept( cts );

    // Pass 2 (positions, normals, colors).
    p.pass2Positions = createFloatTexture2D( p.textureSize );
    p.pass2Normals   = createFloatTexture2D( p.textureSize );
    p.pass2Colors    = createFloatTexture2D( p.textureSize );
    osg::ref_ptr<osg::Camera> pass2 =
        createRTTCamera( osg::Camera::COLOR_BUFFER0, p.pass2Positions );
    pass2->attach( osg::Camera::COLOR_BUFFER1, p.pass2Normals );
    pass2->attach( osg::Camera::COLOR_BUFFER2, p.pass2Colors );
    pass2->addChild( scene.get() );
    osg::ref_ptr<osg::StateSet> ss =
        setShaderProgram( pass2, "shaders/pass2.vert", "shaders/pass2.frag" );
    ss->setTextureAttributeAndModes( 0,
                                     createTexture( "Images/whitemetal_diffuse.jpg" ) );
    ss->setTextureAttributeAndModes( 1,
                                     createTexture( "Images/whitemetal_normal.jpg" ) );
    ss->addUniform( new osg::Uniform( "diffMap", 0 ) );
    ss->addUniform( new osg::Uniform( "bumpMap", 1 ) );
    ss->addUniform( new osg::Uniform( "useBumpMap", 1 ) );

    // Pass 3 (final).
    p.pass3Final = createFloatTexture2D( p.textureSize );
    osg::ref_ptr<osg::Camera> pass3 =
        createRTTCamera( osg::Camera::COLOR_BUFFER, p.pass3Final, true );
    ss = setShaderProgram( pass3, "shaders/pass3.vert", "shaders/pass3.frag" );
    ss->setTextureAttributeAndModes( 0, p.pass2Positions );
    ss->setTextureAttributeAndModes( 1, p.pass2Normals );
    ss->setTextureAttributeAndModes( 2, p.pass2Colors );
    ss->setTextureAttributeAndModes( 3, p.pass1Shadows );
    ss->addUniform( new osg::Uniform( "posMap", 0 ) );
    ss->addUniform( new osg::Uniform( "normalMap", 1 ) );
    ss->addUniform( new osg::Uniform( "colorMap", 2 ) );
    ss->addUniform( new osg::Uniform( "shadowMap", 3 ) );
    // Light position.
    ss->addUniform( new osg::Uniform( "lightPos", lightPos ) );
    // Texture size for coordinate normalization (Texture2D uses [0,1] coords).
    ss->addUniform( new osg::Uniform( "texSize", ( float )p.textureSize ) );
    // Graph.
    p.graph->addChild( pass1 );
    p.graph->addChild( pass2 );
    p.graph->addChild( pass3 );
    return p;
}

osg::Camera*
createRTTCamera( osg::Camera::BufferComponent buffer,
                 osg::Texture*                tex,
                 bool                         isAbsolute )
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setClearColor( osg::vec4() );
    camera->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    camera->setRenderOrder( osg::Camera::PRE_RENDER );
    if( tex )
    {
        tex->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
        tex->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
        camera->setViewport( 0, 0, tex->getTextureWidth(), tex->getTextureHeight() );
        camera->attach( buffer, tex );
    }
    if( isAbsolute )
    {
        camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
        camera->setProjectionMatrix( osg::ortho2D( 0.0, 1.0, 0.0, 1.0 ) );
        camera->setViewMatrix( osg::dmat4() );
        camera->addChild( createScreenQuad( 1.0F, 1.0F ) );
    }
    return camera.release();
}

osg::ref_ptr<osg::Group>
createSceneRoom()
{
    // Room.
    osg::ref_ptr<osg::MatrixTransform> room      = new osg::MatrixTransform;
    osg::ref_ptr<osg::Node>            roomModel = osgDB::readRefNodeFile( "duck.glb" );
    room->addChild( roomModel );
    room->setMatrix( osg::translate( 0.0, 0.0, 1.0 ) );
    // Torus.
    osg::ref_ptr<osg::MatrixTransform> torus      = new osg::MatrixTransform;
    osg::ref_ptr<osg::Node>            torusModel = osgDB::readRefNodeFile( "duck.glb" );
    torus->addChild( torusModel );
    setAnimationPath( torus, osg::vec3( 0, 0, 15 ), 6, 16 );
    // Torus2.
    osg::ref_ptr<osg::MatrixTransform> torus2 = new osg::MatrixTransform;
    torus2->addChild( torusModel );
    setAnimationPath( torus2, osg::vec3( -20, 0, 10 ), 20, 0 );
    // Torus3.
    osg::ref_ptr<osg::MatrixTransform> torus3 = new osg::MatrixTransform;
    torus3->addChild( torusModel );
    setAnimationPath( torus3, osg::vec3( 0, 0, 40 ), 3, 25 );
    // Scene.
    osg::ref_ptr<osg::Group> scene = new osg::Group;
    scene->addChild( room );
    scene->addChild( torus );
    scene->addChild( torus2 );
    scene->addChild( torus3 );
    return scene;
}

osg::Geode*
createScreenQuad( float     width,
                  float     height,
                  float     scale,
                  osg::vec3 corner )
{
    osg::Geometry* geom = osg::createTexturedQuadGeometry( corner,
                                                           osg::vec3( width, 0, 0 ),
                                                           osg::vec3( 0, height, 0 ),
                                                           0,
                                                           0,
                                                           scale,
                                                           scale );
    osg::ref_ptr<osg::Geode> quad = new osg::Geode;
    quad->addDrawable( geom );
    int values = osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED;
    quad->getOrCreateStateSet()->setAttribute(
        new osg::PolygonMode( osg::PolygonMode::Face::FRONT_AND_BACK,
                              osg::PolygonMode::Mode::FILL ),
        values
    );
    // GL_LIGHTING removed for Core Profile compatibility
    return quad.release();
}

osg::Texture2D*
createTexture( const std::string& fileName )
{
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( osgDB::readRefImageFile( fileName ) );
    texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
    texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );
    texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
    texture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    texture->setMaxAnisotropy( 16.0F );
    return texture.release();
}

osg::ref_ptr<osg::Camera>
createTextureDisplayQuad( const osg::vec3&     pos,
                          osg::StateAttribute* tex,
                          float                scale,
                          float                width,
                          float                height )
{
    osg::ref_ptr<osg::Camera> hc = createHUDCamera();
    hc->addChild( createScreenQuad( width, height, scale, pos ) );
    hc->getOrCreateStateSet()->setTextureAttributeAndModes( 0, tex );
    return hc;
}

void
setAnimationPath( osg::ref_ptr<osg::MatrixTransform> node,
                  const osg::vec3&                   center,
                  float                              time,
                  float                              radius )
{
    // Create animation.
    osg::ref_ptr<osg::AnimationPath> path = new osg::AnimationPath;
    path->setLoopMode( osg::AnimationPath::LOOP );
    unsigned int numSamples = 32;
    float delta_yaw  = 2.0F * osg::PI / ( static_cast<float>( numSamples ) - 1.0F );
    float delta_time = time / static_cast<float>( numSamples );
    for( unsigned int i = 0; i < numSamples; ++i )
    {
        float     yaw = delta_yaw * static_cast<float>( i );
        osg::vec3 pos( center.x + sinf( yaw ) * radius,
                       center.y + cosf( yaw ) * radius,
                       center.z );
        osg::quat rot( -yaw, osg::vec3( 0.0F, 0.0F, 1.0F ) );
        path->insert( delta_time * static_cast<float>( i ),
                      osg::AnimationPath::ControlPoint( osg::dvec3( pos ), rot ) );
    }
    // Assign it.
    node->setUpdateCallback( new osg::AnimationPathCallback( path.get() ) );
}

osg::ref_ptr<osg::StateSet>
setShaderProgram( osg::ref_ptr<osg::Camera> pass,
                  const std::string&        vert,
                  const std::string&        frag )
{
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( osgDB::readRefShaderFile( vert ) );
    program->addShader( osgDB::readRefShaderFile( frag ) );
    osg::ref_ptr<osg::StateSet> ss = pass->getOrCreateStateSet();
    ss->setAttributeAndModes( program.get(),
                              osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    return ss;
}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser            arguments( &argc, argv );
    // Useful declaration.
    osg::ref_ptr<osg::StateSet>    ss;
    // Scene.
    osg::vec3                      lightPos( 0, 0, 80 );
    osg::ref_ptr<osg::Group>       scene = createSceneRoom();
    osg::ref_ptr<osg::LightSource> light = createLight( lightPos );
    scene->addChild( light.get() );
    // Shadowed scene.
    osg::ref_ptr<osgShadow::SoftShadowMap> shadowMap = new osgShadow::SoftShadowMap;
    shadowMap->setJitteringScale( 16 );
    shadowMap->addShader( osgDB::readRefShaderFile( "shaders/pass1Shadow.frag" ) );
    shadowMap->setLight( light.get() );
    osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
    shadowedScene->setShadowTechnique( shadowMap.get() );
    shadowedScene->addChild( scene.get() );
    Pipeline p = createPipelinePlainOSG( scene, shadowedScene, lightPos );
    // Quads to display 1 pass textures.
    osg::ref_ptr<osg::Camera> qTexN = createTextureDisplayQuad( osg::vec3( 0, 0.7, 0 ),
                                                                p.pass2Normals,
                                                                p.textureSize );
    osg::ref_ptr<osg::Camera> qTexP = createTextureDisplayQuad( osg::vec3( 0, 0.35, 0 ),
                                                                p.pass2Positions,
                                                                p.textureSize );
    osg::ref_ptr<osg::Camera> qTexC =
        createTextureDisplayQuad( osg::vec3( 0, 0, 0 ), p.pass2Colors, p.textureSize );
    // Qaud to display 2 pass shadow texture.
    osg::ref_ptr<osg::Camera> qTexS = createTextureDisplayQuad( osg::vec3( 0.7, 0.7, 0 ),
                                                                p.pass1Shadows,
                                                                p.textureSize );
    // Quad to display 3 pass final (screen) texture.
    osg::ref_ptr<osg::Camera> qTexFinal = createTextureDisplayQuad( osg::vec3( 0, 0, 0 ),
                                                                    p.pass3Final,
                                                                    p.textureSize,
                                                                    1,
                                                                    1 );
    // Must be processed before the first pass takes
    // the result into pass1Shadows texture.
    p.graph->insertChild( 0, shadowedScene.get() );
    // Quads are displayed in order, so the biggest one (final) must be first,
    // otherwise other quads won't be visible.
    p.graph->addChild( qTexFinal.get() );
    p.graph->addChild( qTexN.get() );
    p.graph->addChild( qTexP.get() );
    p.graph->addChild( qTexC.get() );
    p.graph->addChild( qTexS.get() );

    // Display everything.
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

    // add the stats handler
    viewer.addEventHandler( new osgViewer::StatsHandler );

    // Make screenshots with 'c'.
    viewer.addEventHandler( new osgViewer::ScreenCaptureHandler(
        new osgViewer::ScreenCaptureHandler::WriteToFile(
            "screenshot",
            "png",
            osgViewer::ScreenCaptureHandler::WriteToFile::OVERWRITE
        )
    ) );

    viewer.getCamera()->setComputeNearFarMode(
        osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR
    );
    viewer.setSceneData( p.graph.get() );
    return viewer.run();
}
