/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * X11 offscreen pixel buffer. Creates a GLX Pbuffer for headless
 * rendering without a visible window.
 */
/* Note, elements of PixelBufferX11 have used Prodcer/RenderSurface_X11.cpp as both
 * a guide to use of X11/GLX and copiying directly in the case of setBorder().
 * These elements are license under OSGPL as above, with Copyright (C) 2001-2004  Don
 * Burns.
 */

#pragma once

#include <osg/rendering/GraphicsContext.hpp>
#include <osgViewer/api/X11/GraphicsHandleX11.hpp>

namespace osgViewer
{

    class OSGVIEWER_EXPORT PixelBufferX11 : public osg::GraphicsContext,
                                            public osgViewer::GraphicsHandleX11
    {
        public:

            PixelBufferX11( osg::GraphicsContext::Traits* traits );

            virtual bool
            isSameKindAs( const Object* object ) const
            {
                return dynamic_cast<const PixelBufferX11*>( object ) != 0;
            }

            virtual const char*
            libraryName() const
            {
                return "osgViewer";
            }

            virtual const char*
            className() const
            {
                return "PixelBufferX11";
            }

            virtual bool
            valid() const
            {
                return _valid;
            }

            /** Realise the GraphicsContext.*/
            virtual bool
            realizeImplementation();

            /** Return true if the graphics context has been realised and is ready to
             * use.*/
            virtual bool
            isRealizedImplementation() const
            {
                return _realized;
            }

            /** Close the graphics context.*/
            virtual void
            closeImplementation();

            /** Make this graphics context current.*/
            virtual bool
            makeCurrentImplementation();

            /** Make this graphics context current with specified read context
             * implementation. */
            virtual bool
            makeContextCurrentImplementation( osg::GraphicsContext* readContext );

            /** Release the graphics context.*/
            virtual bool
            releaseContextImplementation();

            /** Bind the graphics context to associated texture implementation.*/
            virtual void
            bindPBufferToTextureImplementation( GLenum buffer );

            /** Swap the front and back buffers.*/
            virtual void
            swapBuffersImplementation();

        public:

            // X11 specific aces functions

            Pbuffer&
            getPbuffer()
            {
                return _pbuffer;
            }

        protected:

            ~PixelBufferX11();

            bool
            createVisualInfo();

            void
                         init();

            bool         _valid;
            Pbuffer      _pbuffer;
            XVisualInfo* _visualInfo;

            bool         _initialized;
            bool         _realized;

            bool         _useGLX1_3;
            bool         _useSGIX;

#ifdef GLX_SGIX_pbuffer
            typedef Pbuffer ( *GLXCreateGLXPbufferSGIX_FuncPtr )( Display*    dpy,
                                                                  GLXFBConfig config,
                                                                  unsigned int,
                                                                  unsigned height,
                                                                  int*     attrib_list );
            typedef void    ( *GLXDestroyGLXPbufferSGIX_FuncPtr )( Display* dpy,
                                                                   Pbuffer  pbuf );
            typedef int     ( *GLXQueryGLXPbufferSGIX_FuncCPtr )( Display*      dpy,
                                                                  Pbuffer       pbuf,
                                                                  int           attribute,
                                                                  unsigned int* value );
            typedef GLXFBConfig ( *GLXGetFBConfigFromVisualSGIX_FuncPtr )(
                Display*     dpy,
                XVisualInfo* vis
            );

            GLXCreateGLXPbufferSGIX_FuncPtr      _glXCreateGLXPbufferSGIX;
            GLXDestroyGLXPbufferSGIX_FuncPtr     _glXDestroyGLXPbufferSGIX;
            GLXQueryGLXPbufferSGIX_FuncCPtr      _glXQueryGLXPbufferSGIX;
            GLXGetFBConfigFromVisualSGIX_FuncPtr _glXGetFBConfigFromVisualSGIX;
#endif
    };

}
