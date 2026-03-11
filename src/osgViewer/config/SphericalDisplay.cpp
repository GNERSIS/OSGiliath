/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgViewer: center, eye, xAxis, yAxis, screenCenter.
 */
#include <osgViewer/config/SphericalDisplay>

#include <osg/core/io_utils.hpp>
#include <osg/core/ValueObject.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/state/Stencil.hpp>
#include <osg/textures/Texture1D.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osg/textures/TextureCubeMap.hpp>
#include <osg/textures/TextureRectangle.hpp>
#include <osgViewer/core/Renderer.hpp>
#include <osgViewer/core/View.hpp>
#include <osgViewer/platform/GraphicsWindow.hpp>

using namespace osgViewer;

osg::Geometry*
SphericalDisplay::create3DSphericalDisplayDistortionMesh(
    const osg::vec3&  origin,
    const osg::vec3&  widthVector,
    const osg::vec3&  heightVector,
    double            sphere_radius,
    double            collar_radius,
    osg::Image*       intensityMap,
    const osg::dmat4& projectorMatrix
) const
{
    osg::dvec3 eye( 0.0, 0.0, 0.0 );

    double     distance =
        sqrt( sphere_radius * sphere_radius - collar_radius * collar_radius );

    bool       centerProjection = false;

    osg::dvec3 projector        = eye - osg::dvec3( 0.0, 0.0, distance );

    OSG_INFO << "create3DSphericalDisplayDistortionMesh : Projector position = "
             << projector << std::endl;
    OSG_INFO << "create3DSphericalDisplayDistortionMesh : distance = " << distance
             << std::endl;

    // create the quad to visualize.
    osg::Geometry* geometry = new osg::Geometry();

    osg::vec3      xAxis( widthVector );
    float          width  = osg::length( widthVector );
    xAxis                /= width;

    osg::vec3 yAxis( heightVector );
    float     height            = osg::length( heightVector );
    yAxis                      /= height;

    int             noSteps     = 50;

    osg::Vec3Array* vertices    = new osg::Vec3Array;
    osg::Vec3Array* texcoords0  = new osg::Vec3Array;
    osg::Vec2Array* texcoords1  = intensityMap == 0 ? new osg::Vec2Array : 0;
    osg::Vec4Array* colors      = new osg::Vec4Array;

    osg::vec3       bottom      = origin;
    osg::vec3       dx          = xAxis * ( width / ( ( float )( noSteps - 1 ) ) );
    osg::vec3       dy          = yAxis * ( height / ( ( float )( noSteps - 1 ) ) );

    osg::dvec3      screenCenter( origin + widthVector * 0.5F + heightVector * 0.5F );
    float           screenRadius = osg::length( heightVector ) * 0.5F;

    int             i, j;

    if( centerProjection )
    {
        for( i = 0; i < noSteps; ++i )
        {
            osg::vec3 cursor = bottom + dy * ( float )i;
            for( j = 0; j < noSteps; ++j )
            {
                osg::vec2 delta( cursor.x - static_cast<float>( screenCenter.x ),
                                 cursor.y - static_cast<float>( screenCenter.y ) );
                double    theta = atan2( -delta.y, delta.x );
                double    phi   = osg::PI_2 * osg::length( delta ) / screenRadius;
                if( phi > osg::PI_2 )
                {
                    phi = osg::PI_2;
                }

                phi *= 2.0;

                if( theta < 0.0 )
                {
                    theta += 2.0 * osg::PI;
                }

                // OSG_NOTICE<<"theta = "<<theta<< "phi="<<phi<<std::endl;

                osg::vec3 texcoord( static_cast<float>( sin( phi ) * cos( theta ) ),
                                    static_cast<float>( sin( phi ) * sin( theta ) ),
                                    static_cast<float>( cos( phi ) ) );

                vertices->push_back( cursor );
                texcoords0->push_back( osg::vec3( osg::dvec3( texcoord ) *
                                                  projectorMatrix ) );

                osg::vec2 texcoord1( static_cast<float>( theta / ( 2.0 * osg::PI ) ),
                                     static_cast<float>( 1.0 - phi / osg::PI_2 ) );
                if( intensityMap )
                {
                    colors->push_back( intensityMap->getColor( texcoord1 ) );
                }
                else
                {
                    colors->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );
                    if( texcoords1 )
                    {
                        texcoords1->push_back( texcoord1 );
                    }
                }

                cursor += dx;
            }
            // OSG_NOTICE<<std::endl;
        }
    }
    else
    {
        for( i = 0; i < noSteps; ++i )
        {
            osg::vec3 cursor = bottom + dy * ( float )i;
            for( j = 0; j < noSteps; ++j )
            {
                osg::vec2 delta( cursor.x - ( float )screenCenter.x,
                                 cursor.y - ( float )screenCenter.y );
                double    theta = atan2( -delta.y, delta.x );
                double    phi   = osg::PI_2 * osg::length( delta ) / screenRadius;
                if( phi > osg::PI_2 )
                {
                    phi = osg::PI_2;
                }
                if( theta < 0.0 )
                {
                    theta += 2.0 * osg::PI;
                }

                // OSG_NOTICE<<"theta = "<<theta<< "phi="<<phi<<std::endl;

                double    f = distance * sin( phi );
                double    e = distance *
                              cos( phi ) +
                              sqrt( sphere_radius * sphere_radius - f * f );
                double    l = e * cos( phi );
                double    h = e * sin( phi );
                double    z = l - distance;

                osg::vec3 texcoord(
                    static_cast<float>( h * cos( theta ) / sphere_radius ),
                    static_cast<float>( h * sin( theta ) / sphere_radius ),
                    static_cast<float>( z / sphere_radius )
                );

                vertices->push_back( cursor );
                texcoords0->push_back( osg::vec3( osg::dvec3( texcoord ) *
                                                  projectorMatrix ) );

                osg::vec2 texcoord1( static_cast<float>( theta / ( 2.0 * osg::PI ) ),
                                     static_cast<float>( 1.0 - phi / osg::PI_2 ) );
                if( intensityMap )
                {
                    colors->push_back( intensityMap->getColor( texcoord1 ) );
                }
                else
                {
                    colors->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );
                    if( texcoords1 )
                    {
                        texcoords1->push_back( texcoord1 );
                    }
                }

                cursor += dx;
            }
            // OSG_NOTICE<<std::endl;
        }
    }

    // pass the created vertex array to the points geometry object.
    geometry->setVertexArray( vertices );

    geometry->setColorArray( colors, osg::Array::BIND_PER_VERTEX );

    geometry->setTexCoordArray( 0, texcoords0 );
    if( texcoords1 )
    {
        geometry->setTexCoordArray( 1, texcoords1 );
    }

    for( i = 0; i < noSteps - 1; ++i )
    {
        osg::DrawElementsUShort* elements =
            new osg::DrawElementsUShort( osg::PrimitiveSet::QUAD_STRIP );
        for( j = 0; j < noSteps; ++j )
        {
            elements->push_back(
                static_cast<unsigned short>( j + ( i + 1 ) * noSteps )
            );
            elements->push_back( static_cast<unsigned short>( j + ( i )*noSteps ) );
        }
        geometry->addPrimitiveSet( elements );
    }

    return geometry;
}

void
SphericalDisplay::configure( osgViewer::View& view ) const
{
    OSG_INFO << "SphericalDisplay::configure(rad=" << _radius << ", cllr=" << _collar
             << ", sn=" << _screenNum << ", im=" << _intensityMap.get() << ")"
             << std::endl;
    osg::GraphicsContext::WindowingSystemInterface* wsi =
        osg::GraphicsContext::getWindowingSystemInterface();
    if( !wsi )
    {
        OSG_NOTICE << "Error, no WindowSystemInterface available, cannot create windows."
                   << std::endl;
        return;
    }

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();

    // displayNum has not been set so reset it to 0.
    if( si.displayNum < 0 )
    {
        si.displayNum = 0;
    }

    si.screenNum = static_cast<int>( _screenNum );

    unsigned int width, height;
    wsi->getScreenResolution( si, width, height );

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->hostName                                  = si.hostName;
    traits->displayNum                                = si.displayNum;
    traits->screenNum                                 = si.screenNum;
    traits->x                                         = 0;
    traits->y                                         = 0;
    traits->width                                     = static_cast<int>( width );
    traits->height                                    = static_cast<int>( height );
    traits->windowDecoration                          = false;
    traits->doubleBuffer                              = true;
    traits->sharedContext                             = 0;

    osg::ref_ptr<osg::GraphicsContext> gc =
        osg::GraphicsContext::createGraphicsContext( traits.get() );
    if( !gc )
    {
        OSG_NOTICE << "GraphicsWindow has not been created successfully." << std::endl;
        return;
    }

    bool                 applyIntensityMapAsColours = true;

    int                  tex_width                  = 512;
    int                  tex_height                 = 512;

    int                  camera_width               = tex_width;
    int                  camera_height              = tex_height;

    osg::TextureCubeMap* texture                    = new osg::TextureCubeMap;

    texture->setTextureSize( tex_width, tex_height );
    texture->setInternalFormat( GL_RGB );
    texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR );
    texture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    texture->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
    texture->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
    texture->setWrap( osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE );

#if 0
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::SEPERATE_WINDOW;
    GLenum buffer = GL_FRONT;
#else
    osg::Camera::RenderTargetImplementation renderTargetImplementation =
        osg::Camera::FRAME_BUFFER_OBJECT;
    GLenum buffer = GL_FRONT;
#endif

    // front face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName( "Front face camera" );
        camera->setGraphicsContext( gc.get() );
        camera->setViewport( new osg::Viewport( 0, 0, camera_width, camera_height ) );
        camera->setDrawBuffer( buffer );
        camera->setReadBuffer( buffer );
        camera->setAllowEventFocus( false );
        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation( renderTargetImplementation );

        // attach the texture and use it as the color buffer.
        camera->attach( osg::Camera::COLOR_BUFFER,
                        texture,
                        0,
                        osg::TextureCubeMap::POSITIVE_Y );

        view.addSlave( camera.get(), osg::dmat4(), osg::dmat4() );
    }

    // top face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName( "Top face camera" );
        camera->setGraphicsContext( gc.get() );
        camera->setViewport( new osg::Viewport( 0, 0, camera_width, camera_height ) );
        camera->setDrawBuffer( buffer );
        camera->setReadBuffer( buffer );
        camera->setAllowEventFocus( false );

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation( renderTargetImplementation );

        // attach the texture and use it as the color buffer.
        camera->attach( osg::Camera::COLOR_BUFFER,
                        texture,
                        0,
                        osg::TextureCubeMap::POSITIVE_Z );

        view.addSlave( camera.get(),
                       osg::dmat4(),
                       osg::rotate( osg::radians( -90.0 ), 1.0, 0.0, 0.0 ) );
    }

    // left face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName( "Left face camera" );
        camera->setGraphicsContext( gc.get() );
        camera->setViewport( new osg::Viewport( 0, 0, camera_width, camera_height ) );
        camera->setDrawBuffer( buffer );
        camera->setReadBuffer( buffer );
        camera->setAllowEventFocus( false );

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation( renderTargetImplementation );

        // attach the texture and use it as the color buffer.
        camera->attach( osg::Camera::COLOR_BUFFER,
                        texture,
                        0,
                        osg::TextureCubeMap::NEGATIVE_X );

        view.addSlave( camera.get(),
                       osg::dmat4(),
                       osg::rotate( osg::radians( -90.0 ), 0.0, 1.0, 0.0 ) *
                           osg::rotate( osg::radians( -90.0 ), 0.0, 0.0, 1.0 ) );
    }

    // right face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName( "Right face camera" );
        camera->setGraphicsContext( gc.get() );
        camera->setViewport( new osg::Viewport( 0, 0, camera_width, camera_height ) );
        camera->setDrawBuffer( buffer );
        camera->setReadBuffer( buffer );
        camera->setAllowEventFocus( false );

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation( renderTargetImplementation );

        // attach the texture and use it as the color buffer.
        camera->attach( osg::Camera::COLOR_BUFFER,
                        texture,
                        0,
                        osg::TextureCubeMap::POSITIVE_X );

        view.addSlave( camera.get(),
                       osg::dmat4(),
                       osg::rotate( osg::radians( 90.0 ), 0.0, 1.0, 0.0 ) *
                           osg::rotate( osg::radians( 90.0 ), 0.0, 0.0, 1.0 ) );
    }

    // bottom face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext( gc.get() );
        camera->setName( "Bottom face camera" );
        camera->setViewport( new osg::Viewport( 0, 0, camera_width, camera_height ) );
        camera->setDrawBuffer( buffer );
        camera->setReadBuffer( buffer );
        camera->setAllowEventFocus( false );

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation( renderTargetImplementation );

        // attach the texture and use it as the color buffer.
        camera->attach( osg::Camera::COLOR_BUFFER,
                        texture,
                        0,
                        osg::TextureCubeMap::NEGATIVE_Z );

        view.addSlave( camera.get(),
                       osg::dmat4(),
                       osg::rotate( osg::radians( 90.0 ), 1.0, 0.0, 0.0 ) *
                           osg::rotate( osg::radians( 180.0 ), 0.0, 0.0, 1.0 ) );
    }

    // back face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName( "Back face camera" );
        camera->setGraphicsContext( gc.get() );
        camera->setViewport( new osg::Viewport( 0, 0, camera_width, camera_height ) );
        camera->setDrawBuffer( buffer );
        camera->setReadBuffer( buffer );
        camera->setAllowEventFocus( false );

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation( renderTargetImplementation );

        // attach the texture and use it as the color buffer.
        camera->attach( osg::Camera::COLOR_BUFFER,
                        texture,
                        0,
                        osg::TextureCubeMap::NEGATIVE_Y );

        view.addSlave( camera.get(),
                       osg::dmat4(),
                       osg::rotate( osg::radians( 180.0 ), 1.0, 0.0, 0.0 ) );
    }

    view.getCamera()->setProjectionMatrixAsPerspective( 90.0F, 1.0, 1, 1000.0 );

    // distortion correction set up.
    {
        osg::Geode* geode = new osg::Geode();
        geode->addDrawable( create3DSphericalDisplayDistortionMesh(
            osg::vec3( 0.0F, 0.0F, 0.0F ),
            osg::vec3( static_cast<float>( width ), 0.0F, 0.0F ),
            osg::vec3( 0.0F, static_cast<float>( height ), 0.0F ),
            _radius,
            _collar,
            applyIntensityMapAsColours ? _intensityMap.get() : 0,
            _projectorMatrix
        ) );

        // new we need to add the texture to the mesh, we do so by creating a
        // StateSet to contain the Texture StateAttribute.
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );
        stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        if( !applyIntensityMapAsColours && _intensityMap.valid() )
        {
            stateset->setTextureAttributeAndModes(
                1,
                new osg::Texture2D( _intensityMap.get() ),
                osg::StateAttribute::ON
            );
        }

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext( gc.get() );
        camera->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
        camera->setClearColor( osg::vec4( 0.0, 0.0, 0.0, 1.0 ) );
        camera->setViewport( new osg::Viewport( 0, 0, width, height ) );

        GLenum window_buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer( window_buffer );
        camera->setReadBuffer( window_buffer );
        camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
        camera->setAllowEventFocus( true );
        camera->setInheritanceMask( camera->getInheritanceMask() &
                                    ~osg::CullSettings::CLEAR_COLOR &
                                    ~osg::CullSettings::COMPUTE_NEAR_FAR_MODE );
        // camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

        camera->setProjectionMatrixAsOrtho2D( 0, width, 0, height );
        camera->setViewMatrix( osg::dmat4() );

        // add subgraph to render
        camera->addChild( geode );

        camera->setName( "DistortionCorrectionCamera" );

        view.addSlave( camera.get(), osg::dmat4(), osg::dmat4(), false );
    }

    view.getCamera()->setNearFarRatio( 0.0001F );

    if( view.getLightingMode() == osg::View::HEADLIGHT )
    {
        // set a local light source for headlight to ensure that lighting is consistent
        // across sides of cube.
        view.getLight()->setPosition( osg::vec4( 0.0F, 0.0F, 0.0F, 1.0F ) );
    }
}
