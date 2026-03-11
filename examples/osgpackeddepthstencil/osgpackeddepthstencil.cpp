/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osgpackeddepthstencil example application
 */
#include <iostream>
#include <osg/core/Notify.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/rendering/FrameBufferObject.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/ColorMask.hpp>
#include <osg/state/Stencil.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

osg::Geode*
createMask()
{
    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back( osg::vec3( -0.5, -0.5, 0.0 ) );
    vertices->push_back( osg::vec3( 0.5, -0.5, 0.0 ) );
    vertices->push_back( osg::vec3( 0.5, 0.5, 0.0 ) );
    vertices->push_back( osg::vec3( -0.5, 0.5, 0.0 ) );

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray( vertices );
    osg::DrawElementsUShort* maskIndices = new osg::DrawElementsUShort( GL_TRIANGLES );
    maskIndices->push_back( 0 );
    maskIndices->push_back( 1 );
    maskIndices->push_back( 2 );
    maskIndices->push_back( 0 );
    maskIndices->push_back( 2 );
    maskIndices->push_back( 3 );
    geom->addPrimitiveSet( maskIndices );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( geom );

    osg::Stencil* stencil = new osg::Stencil;
    stencil->setFunction( osg::Stencil::Function::ALWAYS, 1, ~0U );
    stencil->setOperation( osg::Stencil::Operation::KEEP,
                           osg::Stencil::Operation::KEEP,
                           osg::Stencil::Operation::REPLACE );

    osg::StateSet* ss = geode->getOrCreateStateSet();
    ss->setAttributeAndModes( stencil,
                              osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    ss->setAttribute( new osg::ColorMask( false, false, false, false ),
                      osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

    return geode;
}

osg::Geode*
createGeometry()
{
    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back( osg::vec3( -1.0, -1.0, 0.0 ) );
    vertices->push_back( osg::vec3( 1.0, -1.0, 0.0 ) );
    vertices->push_back( osg::vec3( 1.0, 1.0, 0.0 ) );
    vertices->push_back( osg::vec3( -1.0, 1.0, 0.0 ) );

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray( vertices );
    osg::DrawElementsUShort* geomIndices = new osg::DrawElementsUShort( GL_TRIANGLES );
    geomIndices->push_back( 0 );
    geomIndices->push_back( 1 );
    geomIndices->push_back( 2 );
    geomIndices->push_back( 0 );
    geomIndices->push_back( 2 );
    geomIndices->push_back( 3 );
    geom->addPrimitiveSet( geomIndices );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( geom );

    osg::Stencil* stencil = new osg::Stencil;
    stencil->setFunction( osg::Stencil::Function::NOTEQUAL, 1, ~0U );
    stencil->setOperation( osg::Stencil::Operation::KEEP,
                           osg::Stencil::Operation::KEEP,
                           osg::Stencil::Operation::KEEP );

    osg::StateSet* ss = geode->getOrCreateStateSet();
    ss->setAttributeAndModes( stencil,
                              osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

    return geode;
}

osg::Geode*
createTextureQuad( osg::Texture2D* texture )
{
    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back( osg::vec3( -0.8, 0.0, -0.8 ) );
    vertices->push_back( osg::vec3( 0.8, 0.0, -0.8 ) );
    vertices->push_back( osg::vec3( 0.8, 0.0, 0.8 ) );
    vertices->push_back( osg::vec3( -0.8, 0.0, 0.8 ) );

    osg::Vec2Array* texcoord = new osg::Vec2Array;
    texcoord->push_back( osg::vec2( 0.0, 0.0 ) );
    texcoord->push_back( osg::vec2( 1.0, 0.0 ) );
    texcoord->push_back( osg::vec2( 1.0, 1.0 ) );
    texcoord->push_back( osg::vec2( 0.0, 1.0 ) );

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray( vertices );
    geom->setTexCoordArray( 0, texcoord );
    osg::DrawElementsUShort* texQuadIndices =
        new osg::DrawElementsUShort( GL_TRIANGLES );
    texQuadIndices->push_back( 0 );
    texQuadIndices->push_back( 1 );
    texQuadIndices->push_back( 2 );
    texQuadIndices->push_back( 0 );
    texQuadIndices->push_back( 2 );
    texQuadIndices->push_back( 3 );
    geom->addPrimitiveSet( texQuadIndices );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( geom );
    geode->getOrCreateStateSet()->setTextureAttributeAndModes( 0,
                                                               texture,
                                                               osg::StateAttribute::ON );

    return geode;
}

int
main( int    argc,
      char** argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc, argv );

    arguments.getApplicationUsage()->addCommandLineOption(
        "--fbo",
        "Use Frame Buffer Object for render to texture, where supported."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--pbuffer-rtt",
        "Use Pixel Buffer for render to texture, where supported."
    );
    arguments.getApplicationUsage()->addCommandLineOption(
        "--nopds",
        "Don't use packed depth stencil."
    );
    arguments.getApplicationUsage()->addCommandLineOption( "--fbo-samples", "" );
    arguments.getApplicationUsage()->addCommandLineOption( "--color-samples", "" );

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

    // add stats
    viewer.addEventHandler( new osgViewer::StatsHandler() );

    // if user request help write it out to cout.
    if( arguments.read( "-h" ) || arguments.read( "--help" ) )
    {
        arguments.getApplicationUsage()->write( std::cout );
        return 1;
    }

    osg::Camera::RenderTargetImplementation renderImplementation =
        osg::Camera::FRAME_BUFFER_OBJECT;
    int  colorSamples = 0, samples = 0;
    bool usePDS = true;

    while( arguments.read( "--fbo" ) )
    {
        renderImplementation = osg::Camera::FRAME_BUFFER_OBJECT;
    }
    while( arguments.read( "--pbuffer-rtt" ) )
    {
        renderImplementation = osg::Camera::PIXEL_BUFFER_RTT;
    }
    while( arguments.read( "--nopds" ) )
    {
        usePDS = false;
    }
    while( arguments.read( "--fbo-samples", samples ) )
    {
    }
    while( arguments.read( "--color-samples", colorSamples ) )
    {
    }

    osg::Group*     rootNode = new osg::Group;

    // creates texture to be rendered
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setTextureSize( 1'024, 1'024 );
    texture->setInternalFormat( GL_RGBA );
    texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
    texture->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
    texture->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
    texture->setBorderColor( osg::dvec4( 0, 0, 0, 0 ) );

    // creates rtt camera
    osg::ref_ptr<osg::Camera> rttCamera = new osg::Camera;
    rttCamera->setClearMask( GL_COLOR_BUFFER_BIT |
                             GL_DEPTH_BUFFER_BIT |
                             GL_STENCIL_BUFFER_BIT );
    rttCamera->setClearColor( osg::vec4( 0.0, 0.4, 0.5, 0.0 ) );
    rttCamera->setClearStencil( 0 );
    rttCamera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    rttCamera->setViewMatrix( osg::dmat4() );
    rttCamera->setViewport( 0, 0, 1'024, 1'024 );
    rttCamera->setRenderOrder( osg::Camera::PRE_RENDER );
    rttCamera->setRenderTargetImplementation( renderImplementation );

    if( usePDS )
    {
        rttCamera->attach( osg::Camera::PACKED_DEPTH_STENCIL_BUFFER,
                           GL_DEPTH_STENCIL_EXT );
    }
    else
    {
        // this doesn't work on NVIDIA/Vista 64bit
        // FBO status = 0x8cd6 (FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT)
        rttCamera->attach( osg::Camera::DEPTH_BUFFER, GL_DEPTH_COMPONENT );
        rttCamera->attach( osg::Camera::STENCIL_BUFFER, GL_STENCIL_INDEX8_EXT );
    }

    rttCamera->attach( osg::Camera::COLOR_BUFFER,
                       texture,
                       0,
                       0,
                       false,
                       samples,
                       colorSamples );
    rttCamera->setCullingMode( osg::Camera::VIEW_FRUSTUM_SIDES_CULLING );

    // creates rtt subtree
    osg::Group* g0 = new osg::Group;
    g0->addChild( createMask() );
    g0->addChild( createGeometry() );
    rttCamera->addChild( g0 );
    rootNode->addChild( rttCamera.get() );

    // creates textured quad with result
    rootNode->addChild( createTextureQuad( texture ) );

    // add model to the viewer.
    viewer.setSceneData( rootNode );
    return viewer.run();
}
