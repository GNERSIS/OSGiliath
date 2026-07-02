/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Text label widget. Displays non-editable text with
 * configurable font, size, and alignment.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgText/Text.hpp>
#include <osgUI/Widget.hpp>

namespace osgUI
{

    class OSGUI_EXPORT Label : public osg::Inherit<osgUI::Widget, Label>
    {
        public:

            Label();
            Label( const Label&       label,
                   const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               Label )

            void
            setText( const std::string& text )
            {
                _text = text;
                dirty();
            }

            std::string&
            getText()
            {
                return _text;
            }

            const std::string&
            getText() const
            {
                return _text;
            }

            virtual void
            createGraphicsImplementation();

        protected:

            virtual ~Label()
            {
            }

            std::string                 _text;

            // implementation detail
            osg::ref_ptr<osgText::Text> _textDrawable;
    };

}
