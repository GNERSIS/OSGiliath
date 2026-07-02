#pragma once

#if defined( OSG_USE_EGL )
    #include <EGL/egl.h>
    #define EGL_EGLEXT_PROTOTYPES
    #include <EGL/eglext.h>
#endif

// Fallback if not correctly detected in CMake macro
#ifndef _WIN32_WINNT
    // #define _WIN32_WINNT    0x0A00 // Windows 10
    // #define _WIN32_WINNT    0x0603 // Windows 8.1
    // #define _WIN32_WINNT    0x0602 // Windows 8
    // #define _WIN32_WINNT    0x0601 // Windows 7
    // #define _WIN32_WINNT    0x0600 // Windows Server 2008
    // #define _WIN32_WINNT    0x0600 // Windows Vista
    // #define _WIN32_WINNT    0x0502 // Windows Server 2003 with SP1, Windows XP with
    // SP2 #define _WIN32_WINNT    0x0501 // Windows Server 2003, Windows XP
    #define _WIN32_WINNT 0X05'00    // Windows NT
#endif
#define WIN32_LEAN_AND_MEAN
#include <osg/rendering/GraphicsContext.hpp>
#include <windows.h>

namespace osgViewer
{

    template<typename T>
    class XGLAttributes
    {
        public:

            XGLAttributes()
            {
            }

            ~XGLAttributes()
            {
            }

            void
            begin()
            {
                m_parameters.clear();
            }

            void
            set( const T& id,
                 const T& value )
            {
                add( id );
                add( value );
            }

            void
            enable( const T& id )
            {
                add( id );
                add( true );
            }

            void
            disable( const T& id )
            {
                add( id );
                add( false );
            }
#if defined( OSG_USE_EGL )
            void
            end()
            {
                add( EGL_NONE );
            }
#else
            void
            end()
            {
                add( 0 );
            }
#endif

            const T*
            get() const
            {
                return &m_parameters.front();
            }

        protected:

            void
            add( const T& t )
            {
                m_parameters.push_back( t );
            }

            std::vector<T> m_parameters;    // parameters added

        private:

            // No implementation for these
            XGLAttributes( const XGLAttributes& );
            XGLAttributes&
            operator=( const XGLAttributes& );
    };

    typedef XGLAttributes<int>   XGLIntegerAttributes;
    typedef XGLAttributes<float> XGLFloatAttributes;

    void
    reportError( const std::string& msg );
    void
    reportError( const std::string& msg,
                 unsigned int       errorCode );
    void
    reportErrorForScreen( const std::string&                            msg,
                          const osg::GraphicsContext::ScreenIdentifier& si,
                          unsigned int                                  errorCode );

#if defined( OSG_USE_EGL )
    namespace EGL
    {

        struct ContextInfo
        {
                ContextInfo();
                ContextInfo( EGLContext _eglContext,
                             EGLDisplay _eglDisplay,
                             EGLSurface _eglSurface );
                ContextInfo( const ContextInfo& o );
                ContextInfo&
                operator=( const ContextInfo& o );
                void
                clear();
                bool
                           isEmpty();
                EGLContext eglContext;
                EGLDisplay eglDisplay;
                EGLSurface eglSurface;
        };

        // A class representing an OpenGL rendering context
        class OpenGLContext
        {
            public:

                OpenGLContext();
                OpenGLContext( HWND       hwnd,
                               HDC        hdc,
                               EGLContext eglContext,
                               EGLDisplay eglDisplay,
                               EGLSurface eglSurface );

                ~OpenGLContext();

                void
                set( HWND       hwnd,
                     HDC        hdc,
                     EGLContext eglContext,
                     EGLDisplay eglDisplay,
                     EGLSurface eglSurface,
                     EGLConfig  eglConfig );
                void
                clear();

                HDC
                deviceContext();
                EGLConfig
                getConfig();

                bool
                makeCurrent( HDC  restoreOnHdc,
                             bool restorePreviousOnExit );
                ContextInfo&
                contextInfo();

            protected:

                EGL::ContextInfo _eglCtx;
                EGLConfig        _eglConfig;

                HDC _previousHdc;    // previously HDC to restore rendering context on
                EGLContext _previousContext;    // previously current rendering context

                HWND       _hwnd;               // handle to OpenGL window
                HDC        _hdc;                // handle to device context

            private:

                // no implementation for these
                OpenGLContext( const OpenGLContext& );
                OpenGLContext&
                operator=( const OpenGLContext& );
        };

        bool
        createDisplaySurfaceAndContext( ContextInfo&        context,
                                        EGLConfig&          config,
                                        XGLAttributes<int>& configAttribs,
                                        HWND                hwnd,
                                        HDC                 hdc );
        bool
        createDisplaySurfaceAndContextForPBuffer( ContextInfo&        context,
                                                  EGLConfig&          config,
                                                  XGLAttributes<int>& configAttribs );
        EGLContext
        createContext( EGLDisplay       eglDisplay,
                       const EGLConfig& config );
        void
        destroyContext( ContextInfo& c );
        void
        preparePixelFormatSpecifications( const osg::GraphicsContext::Traits& traits,
                                          XGLIntegerAttributes&               attributes,
                                          bool allowSwapExchangeARB );

    }    // end of namespace EGL

#else

    namespace WGL
    {

        class OpenGLContext
        {
            public:

                OpenGLContext();
                OpenGLContext( HWND  hwnd,
                               HDC   hdc,
                               HGLRC hglrc );

                ~OpenGLContext();

                void
                set( HWND  hwnd,
                     HDC   hdc,
                     HGLRC hglrc );

                void
                clear();

                HDC
                deviceContext();

                bool
                makeCurrent( HDC  restoreOnHdc,
                             bool restorePreviousOnExit );

            protected:

                //
                // Data members
                //

                HDC   _previousHdc;    // previously HDC to restore rendering context on
                HGLRC _previousHglrc;            // previously current rendering context
                HWND  _hwnd;                     // handle to OpenGL window
                HDC   _hdc;                      // handle to device context
                HGLRC _hglrc;                    // handle to OpenGL rendering context
                bool  _restorePreviousOnExit;    // restore original context on exit

            private:

                // no implementation for these
                OpenGLContext( const OpenGLContext& );
                OpenGLContext&
                operator=( const OpenGLContext& );
        };

    }    // end of namespace WGL
#endif

}

#if !defined( OSG_USE_EGL )
    //
    // Defines from the WGL_ARB_pixel_format specification document
    // See http://www.opengl.org/registry/specs/ARB/wgl_pixel_format.txt
    //
    #ifndef WGL_ARB_pixel_format
        #define WGL_ARB_pixel_format            1
        #define WGL_NUMBER_PIXEL_FORMATS_ARB    0X20'00
        #define WGL_DRAW_TO_WINDOW_ARB          0X20'01
        #define WGL_DRAW_TO_BITMAP_ARB          0X20'02
        #define WGL_ACCELERATION_ARB            0X20'03
        #define WGL_NEED_PALETTE_ARB            0X20'04
        #define WGL_NEED_SYSTEM_PALETTE_ARB     0X20'05
        #define WGL_SWAP_LAYER_BUFFERS_ARB      0X20'06
        #define WGL_SWAP_METHOD_ARB             0X20'07
        #define WGL_NUMBER_OVERLAYS_ARB         0X20'08
        #define WGL_NUMBER_UNDERLAYS_ARB        0X20'09
        #define WGL_TRANSPARENT_ARB             0X20'0A
        #define WGL_TRANSPARENT_RED_VALUE_ARB   0X20'37
        #define WGL_TRANSPARENT_GREEN_VALUE_ARB 0X20'38
        #define WGL_TRANSPARENT_BLUE_VALUE_ARB  0X20'39
        #define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0X20'3A
        #define WGL_TRANSPARENT_INDEX_VALUE_ARB 0X20'3B
        #define WGL_SHARE_DEPTH_ARB             0X20'0C
        #define WGL_SHARE_STENCIL_ARB           0X20'0D
        #define WGL_SHARE_ACCUM_ARB             0X20'0E
        #define WGL_SUPPORT_GDI_ARB             0X20'0F
        #define WGL_SUPPORT_OPENGL_ARB          0X20'10
        #define WGL_DOUBLE_BUFFER_ARB           0X20'11
        #define WGL_STEREO_ARB                  0X20'12
        #define WGL_PIXEL_TYPE_ARB              0X20'13
        #define WGL_COLOR_BITS_ARB              0X20'14
        #define WGL_RED_BITS_ARB                0X20'15
        #define WGL_RED_SHIFT_ARB               0X20'16
        #define WGL_GREEN_BITS_ARB              0X20'17
        #define WGL_GREEN_SHIFT_ARB             0X20'18
        #define WGL_BLUE_BITS_ARB               0X20'19
        #define WGL_BLUE_SHIFT_ARB              0X20'1A
        #define WGL_ALPHA_BITS_ARB              0X20'1B
        #define WGL_ALPHA_SHIFT_ARB             0X20'1C
        #define WGL_ACCUM_BITS_ARB              0X20'1D
        #define WGL_ACCUM_RED_BITS_ARB          0X20'1E
        #define WGL_ACCUM_GREEN_BITS_ARB        0X20'1F
        #define WGL_ACCUM_BLUE_BITS_ARB         0X20'20
        #define WGL_ACCUM_ALPHA_BITS_ARB        0X20'21
        #define WGL_DEPTH_BITS_ARB              0X20'22
        #define WGL_STENCIL_BITS_ARB            0X20'23
        #define WGL_AUX_BUFFERS_ARB             0X20'24
        #define WGL_NO_ACCELERATION_ARB         0X20'25
        #define WGL_GENERIC_ACCELERATION_ARB    0X20'26
        #define WGL_FULL_ACCELERATION_ARB       0X20'27
        #define WGL_SWAP_EXCHANGE_ARB           0X20'28
        #define WGL_SWAP_COPY_ARB               0X20'29
        #define WGL_SWAP_UNDEFINED_ARB          0X20'2A
        #define WGL_TYPE_RGBA_ARB               0X20'2B
        #define WGL_TYPE_COLORINDEX_ARB         0X20'2C
        #define WGL_SAMPLE_BUFFERS_ARB          0X20'41
        #define WGL_SAMPLES_ARB                 0X20'42
    #endif

    #ifndef WGL_ARB_render_texture
        #define WGL_ARB_render_texture              1
        #define WGL_BIND_TO_TEXTURE_RGB_ARB         0X20'70
        #define WGL_BIND_TO_TEXTURE_RGBA_ARB        0X20'71
        #define WGL_TEXTURE_FORMAT_ARB              0X20'72
        #define WGL_TEXTURE_TARGET_ARB              0X20'73
        #define WGL_MIPMAP_TEXTURE_ARB              0X20'74
        #define WGL_TEXTURE_RGB_ARB                 0X20'75
        #define WGL_TEXTURE_RGBA_ARB                0X20'76
        #define WGL_NO_TEXTURE_ARB                  0X20'77
        #define WGL_TEXTURE_CUBE_MAP_ARB            0X20'78
        #define WGL_TEXTURE_1D_ARB                  0X20'79
        #define WGL_TEXTURE_2D_ARB                  0X20'7A
        #define WGL_MIPMAP_LEVEL_ARB                0X20'7B
        #define WGL_CUBE_MAP_FACE_ARB               0X20'7C
        #define WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0X20'7D
        #define WGL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0X20'7E
        #define WGL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0X20'7F
        #define WGL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0X20'80
        #define WGL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0X20'81
        #define WGL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0X20'82
        #define WGL_FRONT_LEFT_ARB                  0X20'83
        #define WGL_FRONT_RIGHT_ARB                 0X20'84
        #define WGL_BACK_LEFT_ARB                   0X20'85
        #define WGL_BACK_RIGHT_ARB                  0X20'86
        #define WGL_AUX0_ARB                        0X20'87
        #define WGL_AUX1_ARB                        0X20'88
        #define WGL_AUX2_ARB                        0X20'89
        #define WGL_AUX3_ARB                        0X20'8A
        #define WGL_AUX4_ARB                        0X20'8B
        #define WGL_AUX5_ARB                        0X20'8C
        #define WGL_AUX6_ARB                        0X20'8D
        #define WGL_AUX7_ARB                        0X20'8E
        #define WGL_AUX8_ARB                        0X20'8F
        #define WGL_AUX9_ARB                        0X20'90
    #endif

    #ifndef WGL_NV_render_depth_texture
        #define WGL_NV_render_depth_texture            1
        #define WGL_BIND_TO_TEXTURE_DEPTH_NV           0X20'A3
        #define WGL_BIND_TO_TEXTURE_RECTANGLE_DEPTH_NV 0X20'A4
        #define WGL_DEPTH_TEXTURE_FORMAT_NV            0X20'A5
        #define WGL_TEXTURE_DEPTH_COMPONENT_NV         0X20'A6
        #define WGL_DEPTH_COMPONENT_NV                 0X20'A7
    #endif

    #ifndef WGL_NV_render_texture_rectangle
        #define WGL_NV_render_texture_rectangle       1
        #define WGL_BIND_TO_TEXTURE_RECTANGLE_RGB_NV  0X20'A0
        #define WGL_BIND_TO_TEXTURE_RECTANGLE_RGBA_NV 0X20'A1
        #define WGL_TEXTURE_RECTANGLE_NV              0X20'A2
    #endif

    #ifndef WGL_SAMPLE_BUFFERS_ARB
        #define WGL_SAMPLE_BUFFERS_ARB 0X20'41
    #endif
    #ifndef WGL_SAMPLES_ARB
        #define WGL_SAMPLES_ARB 0X20'42
    #endif

    #ifndef WGL_ARB_create_context
        #define WGL_CONTEXT_DEBUG_BIT_ARB              0X00'00'00'01
        #define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0X00'00'00'02
        #define WGL_CONTEXT_MAJOR_VERSION_ARB          0X20'91
        #define WGL_CONTEXT_MINOR_VERSION_ARB          0X20'92
        #define WGL_CONTEXT_LAYER_PLANE_ARB            0X20'93
        #define WGL_CONTEXT_FLAGS_ARB                  0X20'94
        #define WGL_CONTEXT_PROFILE_MASK_ARB           0X91'26
        #define WGL_CONTEXT_CORE_PROFILE_BIT_ARB       0X00'00'00'01
        #define ERROR_INVALID_VERSION_ARB              0X20'95
    #endif

    #ifndef WGL_ARB_pbuffer
        #define WGL_ARB_pbuffer 1
DECLARE_HANDLE( HPBUFFERARB );
        #define WGL_DRAW_TO_PBUFFER_ARB    0X20'2D
        #define WGL_MAX_PBUFFER_PIXELS_ARB 0X20'2E
        #define WGL_MAX_PBUFFER_WIDTH_ARB  0X20'2F
        #define WGL_MAX_PBUFFER_HEIGHT_ARB 0X20'30
        #define WGL_PBUFFER_LARGEST_ARB    0X20'33
        #define WGL_PBUFFER_WIDTH_ARB      0X20'34
        #define WGL_PBUFFER_HEIGHT_ARB     0X20'35
        #define WGL_PBUFFER_LOST_ARB       0X20'36
    #endif

    #ifndef WGL_ARB_create_context
        #define WGL_ARB_create_context 1
        #ifdef WGL_WGLEXT_PROTOTYPES
extern HGLRC WINAPI
wglCreateContextAttribsARB( HDC,
                            HGLRC,
                            const int* );
        #endif /* WGL_WGLEXT_PROTOTYPES */
typedef HGLRC( WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC )( HDC        hDC,
                                                            HGLRC      hShareContext,
                                                            const int* attribList );
    #endif

//
// Entry points used from the WGL extensions
//
//    BOOL wglChoosePixelFormatARB(HDC hdc,
//                                 const int *piAttribIList,
//                                 const FLOAT *pfAttribFList,
//                                 UINT nMaxFormats,
//                                 int *piFormats,
//                                 UINT *nNumFormats);
//

typedef bool( WINAPI* WGLChoosePixelFormatARB )( HDC,
                                                 const int*,
                                                 const float*,
                                                 unsigned int,
                                                 int*,
                                                 unsigned int* );
#endif
