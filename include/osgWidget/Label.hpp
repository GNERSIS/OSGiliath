/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Text label widget using osgText::Text. Displays styled
 * text within a widget bounds.
 */
// Code by: Jeremy Moles (cubicool) 2007-2008

#pragma once

#include <osg/core/Inherit.hpp>
#include <osgText/Text.hpp>
#include <osgWidget/Widget.hpp>
#include <osgWidget/Window.hpp>

namespace osgWidget
{

    class OSGWIDGET_EXPORT Label : public osg::Inherit<Widget, Label>
    {
        public:

            OSG_REGISTER_TYPE( osgWidget,
                               Label )

            Label( const std::string& = "",
                   const std::string& = "" );
            Label( const Label&,
                   const osg::CopyOp& );

            virtual void
            parented( Window* );
            virtual void
            unparented( Window* );
            virtual void
            positioned();

            void
            setLabel( const std::string& );
            void
            setLabel( const osgText::String& );
            void
            setFont( const std::string& );
            void
            setFontSize( unsigned int );
            void
                 setFontColor( const Color& );
            void setShadow( point_type );

            XYCoord
            getTextSize() const;

            std::string
            getLabel() const
            {
                return _text->getText().createUTF8EncodedString();
            }

            void
            setFontColor( point_type r,
                          point_type g,
                          point_type b,
                          point_type a )
            {
                setFontColor( Color( r, g, b, a ) );
            }

            osgText::Text*
            getText()
            {
                return _text.get();
            }

            const osgText::Text*
            getText() const
            {
                return _text.get();
            }

        protected:

            osg::ref_ptr<osgText::Text> _text;
            unsigned int                _textIndex;

            virtual void
            _calculateSize( const XYCoord& );
    };

}
