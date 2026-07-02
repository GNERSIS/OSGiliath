/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Viewer configuration that creates a single window on one screen.
 * The most common setup for desktop applications.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgViewer/core/View.hpp>

namespace osgViewer
{

    /** single camera on a single window.*/
    class OSGVIEWER_EXPORT SingleWindow : public osg::Inherit<ViewConfig, SingleWindow>
    {
        public:

            SingleWindow() :
                _x( 0 ),
                _y( 0 ),
                _width( -1 ),
                _height( -1 ),
                _screenNum( 0 ),
                _windowDecoration( true ),
                _overrideRedirect( false )
            {
            }

            SingleWindow( int          x,
                          int          y,
                          int          width,
                          int          height,
                          unsigned int screenNum = 0 ) :
                _x( x ),
                _y( y ),
                _width( width ),
                _height( height ),
                _screenNum( screenNum ),
                _windowDecoration( true ),
                _overrideRedirect( false )
            {
            }

            SingleWindow( const SingleWindow& rhs,
                          const osg::CopyOp&  copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( rhs,
                         copyop ),
                _x( rhs._x ),
                _y( rhs._y ),
                _width( rhs._width ),
                _height( rhs._height ),
                _screenNum( rhs._screenNum ),
                _windowDecoration( rhs._windowDecoration ),
                _overrideRedirect( rhs._overrideRedirect )
            {
            }

            OSG_REGISTER_TYPE( osgViewer,
                               SingleWindow )

            virtual void
            configure( osgViewer::View& view ) const;

            void
            setX( int x )
            {
                _x = x;
            }

            int
            getX() const
            {
                return _x;
            }

            void
            setY( int y )
            {
                _y = y;
            }

            int
            getY() const
            {
                return _y;
            }

            void
            setWidth( int w )
            {
                _width = w;
            }

            int
            getWidth() const
            {
                return _width;
            }

            void
            setHeight( int h )
            {
                _height = h;
            }

            int
            getHeight() const
            {
                return _height;
            }

            void
            setScreenNum( unsigned int sn )
            {
                _screenNum = sn;
            }

            unsigned int
            getScreenNum() const
            {
                return _screenNum;
            }

            void
            setWindowDecoration( bool wd )
            {
                _windowDecoration = wd;
            }

            bool
            getWindowDecoration() const
            {
                return _windowDecoration;
            }

            void
            setOverrideRedirect( bool override )
            {
                _overrideRedirect = override;
            }

            bool
            getOverrideRedirect() const
            {
                return _overrideRedirect;
            }

        protected:

            int          _x, _y, _width, _height;
            unsigned int _screenNum;
            bool         _windowDecoration;
            bool         _overrideRedirect;
    };

}
