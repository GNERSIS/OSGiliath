/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Text input widget for keyboard entry. Provides cursor,
 * selection, and editable text within a widget.
 */
// Code by: Jeremy Moles (cubicool) 2007-2008

#pragma once

#include <osgWidget/Label.hpp>

namespace osgWidget
{

    // This is a string of values we use to try and determine the best Y
    // descent value (yoffset); you're welcome to use what works best for
    // your font.
    const std::string DESCENT_STRING( "qpl" );

    class OSGWIDGET_EXPORT Input : public Label
    {
        public:

            Input( const std::string& = "",
                   const std::string& = "",
                   unsigned int       = 20 );

            virtual void
            parented( Window* );
            virtual void
            positioned();

            virtual bool
            focus( const WindowManager* );
            virtual bool
            unfocus( const WindowManager* );
            virtual bool
            keyUp( int,
                   int,
                   const WindowManager* );
            virtual bool
            keyDown( int,
                     int,
                     const WindowManager* );
            virtual bool
            mouseDrag( double,
                       double,
                       const WindowManager* );
            virtual bool
            mousePush( double x,
                       double y,
                       const WindowManager* );
            virtual bool
            mouseRelease( double,
                          double,
                          const WindowManager* );

            void
            setCursor( Widget* );
            unsigned int
            calculateBestYOffset( const std::string& = "qgl" );
            void
            clear();

            void
            setXOffset( point_type xo )
            {
                _xoff = xo;
            }

            void
            setYOffset( point_type yo )
            {
                _yoff = yo;
            }

            void
            setXYOffset( point_type xo,
                         point_type yo )
            {
                _xoff = xo;
                _yoff = yo;
            }

            osg::Drawable*
            getCursor()
            {
                return _cursor.get();
            }

            const osg::Drawable*
            getCursor() const
            {
                return _cursor.get();
            }

            point_type
            getXOffset() const
            {
                return _xoff;
            }

            point_type
            getYOffset() const
            {
                return _yoff;
            }

            XYCoord
            getXYOffset() const
            {
                return XYCoord( _xoff, _yoff );
            }

        protected:

            virtual void
            _calculateSize( const XYCoord& );

            void
                                      _calculateCursorOffsets();

            point_type                _xoff;
            point_type                _yoff;

            unsigned int              _index;
            unsigned int              _size;
            unsigned int              _cursorIndex;
            unsigned int              _maxSize;

            std::vector<point_type>   _offsets;
            std::vector<unsigned int> _wordsOffsets;
            std::vector<point_type>   _widths;
            osg::ref_ptr<Widget>      _cursor;

            bool _insertMode;    // Insert was pressed --> true --> typing will overwrite
                                 // existing text

            osg::ref_ptr<Widget> _selection;
            unsigned int         _selectionStartIndex;
            unsigned int         _selectionEndIndex;
            unsigned int         _selectionIndex;

            point_type           _mouseClickX;
    };

}
