/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * X11 platform handle holder. Stores the Display*, Window, and
 * GLXContext for sharing with external X11/GLX code.
 */
#pragma once

#include <osg/GL>
#include <osgViewer/core/Export.hpp>

// osg/GL (GLEW) must be included before GL/glx.h — glew.h errors out if gl.h
// wins. The #define line below also fences include-sorting from moving glx.h up.
#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifndef GLX_VERSION_1_3
typedef XID GLXPbuffer;
#endif

namespace osgViewer
{

    /** Class to encapsulate platform-specific OpenGL context handle variables.
     * Derived osg::GraphicsContext classes can inherit from this class to
     * share OpenGL resources.*/

    class OSGVIEWER_EXPORT GraphicsHandleX11
    {
        public:

            GraphicsHandleX11() :
                _display( 0 ),
                _context( 0 )
            {
            }

            /** Set X11 display.*/
            inline void
            setDisplay( Display* display )
            {
                _display = display;
            }

            /** Get X11 display.*/
            inline Display*
            getDisplay() const
            {
                return _display;
            }

#ifdef OSG_USE_EGL
            typedef EGLContext Context;
            typedef EGLSurface Pbuffer;
#else
            typedef GLXContext Context;
            typedef GLXPbuffer Pbuffer;
#endif

            /** Set native OpenGL graphics context.*/
            inline void
            setContext( Context context )
            {
                _context = context;
            }

            /** Get native OpenGL graphics context.*/
            inline Context
            getContext() const
            {
                return _context;
            }

        protected:

            Display* _display;
            Context  _context;
    };

}
