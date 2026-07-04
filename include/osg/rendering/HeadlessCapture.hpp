/* Truly headless offscreen rendering — no X11 connection, no window manager.
 *
 * Uses EGL platform device + pbuffer surface to get GL 4.6 Core Profile
 * directly on the GPU without any display server involvement.
 *
 * Usage from any example (BEFORE creating a Viewer):
 *
 *   std::string output;
 *   if (arguments.read("--headless", output)) {
 *       auto scene = osgDB::readRefNodeFile("model.glb");
 *       return osg::headlessCapture(scene.get(), output) ? 0 : 1;
 *   }
 */

#pragma once

#include <cstdlib>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <iostream>
#include <osg/core/ref_ptr.hpp>
#include <osg/images/Image.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/rendering/GraphicsContext.hpp>
#include <osg/state/State.hpp>
#include <osgDB/io/WriteFile.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <string>

namespace osg
{

    namespace detail
    {

        struct CaptureCallback : public osg::Camera::DrawCallback
        {
                osg::ref_ptr<osg::Image> image;
                int                      width, height;

                CaptureCallback( osg::Image* img,
                                 int         w,
                                 int         h ) :
                    image( img ),
                    width( w ),
                    height( h )
                {
                }

                void
                operator()( osg::RenderInfo& ) const override
                {
                    glReadBuffer( GL_BACK );
                    image->readPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE );
                }
        };

        // Minimal EGL GraphicsContext that bypasses X11 entirely
        class EGLHeadlessContext : public osg::GraphicsContext
        {
            public:

                EGLHeadlessContext( int width,
                                    int height ) :
                    _eglDisplay( EGL_NO_DISPLAY ),
                    _eglContext( EGL_NO_CONTEXT ),
                    _eglSurface( EGL_NO_SURFACE ),
                    _width( width ),
                    _height( height )
                {
                    _traits               = new Traits;
                    _traits->width        = width;
                    _traits->height       = height;
                    _traits->doubleBuffer = true;

                    // Initialize State — required by GraphicsContext::makeCurrent()
                    setState( new osg::State );
                    getState()->setGraphicsContext( this );
                    getState()->setContextID(
                        osg::GraphicsContext::createNewContextID()
                    );
                }

                ~EGLHeadlessContext() override
                {
                    closeImplementation();
                }

                bool
                isSameKindAs( const osg::Object* obj ) const override
                {
                    return dynamic_cast<const EGLHeadlessContext*>( obj ) != nullptr;
                }

                const char*
                libraryName() const override
                {
                    return "osg";
                }

                const char*
                className() const override
                {
                    return "EGLHeadlessContext";
                }

                bool
                valid() const override
                {
                    return _eglContext != EGL_NO_CONTEXT;
                }

                bool
                realizeImplementation() override
                {
                    if( _realized )
                    {
                        return true;
                    }

                    // Query EGL devices
                    auto eglQueryDevicesEXT = ( PFNEGLQUERYDEVICESEXTPROC )
                        eglGetProcAddress( "eglQueryDevicesEXT" );
                    auto eglGetPlatformDisplayEXT = ( PFNEGLGETPLATFORMDISPLAYEXTPROC )
                        eglGetProcAddress( "eglGetPlatformDisplayEXT" );
                    if( eglQueryDevicesEXT && eglGetPlatformDisplayEXT )
                    {
                        EGLDeviceEXT devices[4];
                        EGLint       numDevices = 0;
                        eglQueryDevicesEXT( 4, devices, &numDevices );
                        if( numDevices > 0 &&
                            realizeDisplay( eglGetPlatformDisplayEXT(
                                EGL_PLATFORM_DEVICE_EXT, devices[0], nullptr ) ) )
                        {
                            return true;
                        }
                    }

#ifdef EGL_PLATFORM_SURFACELESS_MESA
                    if( eglGetPlatformDisplayEXT &&
                        realizeDisplay( eglGetPlatformDisplayEXT(
                            EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr ) ) )
                    {
                        return true;
                    }
#endif

                    return realizeDisplay( eglGetDisplay( EGL_DEFAULT_DISPLAY ) );
                }

                bool
                isRealizedImplementation() const override
                {
                    return _realized;
                }

                bool
                makeCurrentImplementation() override
                {
                    return eglMakeCurrent( _eglDisplay,
                                           _eglSurface,
                                           _eglSurface,
                                           _eglContext ) == EGL_TRUE;
                }

                bool
                makeContextCurrentImplementation( osg::GraphicsContext* ) override
                {
                    return makeCurrentImplementation();
                }

                bool
                releaseContextImplementation() override
                {
                    return eglMakeCurrent( _eglDisplay,
                                           EGL_NO_SURFACE,
                                           EGL_NO_SURFACE,
                                           EGL_NO_CONTEXT ) == EGL_TRUE;
                }

                void
                closeImplementation() override
                {
                    if( _eglDisplay != EGL_NO_DISPLAY )
                    {
                        if( _eglSurface != EGL_NO_SURFACE )
                        {
                            eglDestroySurface( _eglDisplay, _eglSurface );
                        }
                        if( _eglContext != EGL_NO_CONTEXT )
                        {
                            eglDestroyContext( _eglDisplay, _eglContext );
                        }
                        eglTerminate( _eglDisplay );
                    }
                    _eglDisplay = EGL_NO_DISPLAY;
                    _eglContext = EGL_NO_CONTEXT;
                    _eglSurface = EGL_NO_SURFACE;
                    _realized   = false;
                }

                void
                swapBuffersImplementation() override
                {
                    if( _eglDisplay != EGL_NO_DISPLAY && _eglSurface != EGL_NO_SURFACE )
                    {
                        eglSwapBuffers( _eglDisplay, _eglSurface );
                    }
                }

                void
                bindPBufferToTextureImplementation( GLenum ) override
                {
                }

            private:

                bool
                realizeDisplay( EGLDisplay display )
                {
                    if( display == EGL_NO_DISPLAY )
                    {
                        return false;
                    }

                    closeImplementation();
                    _eglDisplay = display;

                    EGLint major, minor;
                    if( !eglInitialize( _eglDisplay, &major, &minor ) )
                    {
                        closeImplementation();
                        return false;
                    }
                    if( !eglBindAPI( EGL_OPENGL_API ) )
                    {
                        closeImplementation();
                        return false;
                    }

                    EGLint cfgAttribs[] = {
                        EGL_SURFACE_TYPE,
                        EGL_PBUFFER_BIT,
                        EGL_RENDERABLE_TYPE,
                        EGL_OPENGL_BIT,
                        EGL_RED_SIZE,
                        8,
                        EGL_GREEN_SIZE,
                        8,
                        EGL_BLUE_SIZE,
                        8,
                        EGL_ALPHA_SIZE,
                        8,
                        EGL_DEPTH_SIZE,
                        24,
                        EGL_NONE
                    };
                    EGLConfig cfg;
                    EGLint    numCfg = 0;
                    eglChooseConfig( _eglDisplay, cfgAttribs, &cfg, 1, &numCfg );
                    if( numCfg == 0 )
                    {
                        closeImplementation();
                        return false;
                    }

                    _eglContext = createContext( cfg, 4, 6 );
                    if( _eglContext == EGL_NO_CONTEXT )
                    {
                        closeImplementation();
                        return false;
                    }

                    EGLint pbAttribs[] =
                        { EGL_WIDTH, _width, EGL_HEIGHT, _height, EGL_NONE };
                    _eglSurface = eglCreatePbufferSurface( _eglDisplay, cfg, pbAttribs );
                    if( _eglSurface == EGL_NO_SURFACE )
                    {
                        closeImplementation();
                        return false;
                    }

                    // Make context current immediately so eglGetProcAddress
                    // returns valid function pointers during GL extension init.
                    if( !eglMakeCurrent( _eglDisplay,
                                         _eglSurface,
                                         _eglSurface,
                                         _eglContext ) )
                    {
                        closeImplementation();
                        return false;
                    }

                    _realized = true;
                    return true;
                }

                EGLContext
                createContext( EGLConfig cfg,
                               EGLint    major,
                               EGLint    minor )
                {
                    EGLint ctxAttribs[] = {
                        EGL_CONTEXT_MAJOR_VERSION,
                        major,
                        EGL_CONTEXT_MINOR_VERSION,
                        minor,
                        EGL_CONTEXT_OPENGL_PROFILE_MASK,
                        EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                        EGL_NONE
                    };
                    return eglCreateContext(
                        _eglDisplay, cfg, EGL_NO_CONTEXT, ctxAttribs
                    );
                }

                EGLDisplay _eglDisplay;
                EGLContext _eglContext;
                EGLSurface _eglSurface;
                int        _width, _height;
                bool       _realized = false;
        };

    }

    /// Render a scene offscreen and save as PNG.
    /// Calls _Exit() and does not return.
    /// If eye/center/up are all zero, auto-computes from scene bounds.
    inline bool
    headlessCapture( osg::Node*         scene,
                     const std::string& outputPath,
                     int                width  = 640,
                     int                height = 480,
                     osg::dvec3         eye    = osg::dvec3( 0,
                                                             0,
                                                             0 ),
                     osg::dvec3         center = osg::dvec3( 0,
                                                             0,
                                                             0 ),
                     osg::dvec3         up     = osg::dvec3( 0,
                                                             0,
                                                             0 ) )
    {
        if( !scene )
        {
            std::cerr << "headlessCapture: null scene" << std::endl;
            _Exit( 1 );
        }

        osg::ref_ptr<osg::Image>                   image = new osg::Image;

        osgViewer::Viewer viewer;
        viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );

        {
            // X11 window with override_redirect + no XMapWindow.
            // The window exists for its GL context but is never shown.
            osg::ref_ptr<osg::GraphicsContext::Traits> traits =
                new osg::GraphicsContext::Traits;
            traits->x                = 0;
            traits->y                = 0;
            traits->width            = width;
            traits->height           = height;
            traits->doubleBuffer     = true;
            traits->windowDecoration = false;
            traits->headless         = true;
            traits->readDISPLAY();
            traits->setUndefinedScreenDetailsToDefaultScreen();

            osg::ref_ptr<osg::GraphicsContext> gc =
                osg::GraphicsContext::createGraphicsContext( traits.get() );
            if( !gc.valid() || !gc->valid() )
            {
                gc = new detail::EGLHeadlessContext( width, height );
                if( !gc->realize() || !gc->valid() )
                {
                    std::cerr << "headlessCapture: failed to create graphics context"
                              << std::endl;
                    _Exit( 1 );
                }
            }

            viewer.getCamera()->setGraphicsContext( gc.get() );
        }

        viewer.getCamera()->setViewport( new osg::Viewport( 0, 0, width, height ) );
        viewer.getCamera()->setDrawBuffer( GL_BACK );
        viewer.getCamera()->setReadBuffer( GL_BACK );
        viewer.getCamera()->setFinalDrawCallback(
            new detail::CaptureCallback( image.get(), width, height )
        );

        viewer.setSceneData( scene );
        auto* manip = new osgGA::TrackballManipulator;
        manip->setAutoComputeHomePosition( false );    // don't override our camera
        viewer.setCameraManipulator( manip );

        // Set camera position — if all zero, auto-compute a 3/4 view
        {
            const osg::sphere& bs = scene->getBound();
            double             r  = bs.radius > 0 ? bs.radius : 1.0;
            osg::dvec3         c( bs.center );

            bool               hasExplicit = ( eye.x != 0 || eye.y != 0 || eye.z != 0 );
            if( !hasExplicit )
            {
                // Auto 3/4 view: Y-up, camera from front-right elevated
                eye    = osg::dvec3( c.x + r * 2.0, c.y + r * 1.0, c.z + r * 2.0 );
                center = c;
                up     = osg::dvec3( 0, 1, 0 );
            }

            manip->setHomePosition( eye, center, up );
        }

        viewer.realize();
        manip->home( 0 );    // apply home position

        for( int i = 0; i < 3; ++i )
        {
            viewer.frame();
        }

        if( !osgDB::writeImageFile( *image, outputPath ) )
        {
            std::cerr << "headlessCapture: failed to write " << outputPath << std::endl;
            _Exit( 1 );
        }

        _Exit( 0 );
        return true;
    }

}    // namespace osg
