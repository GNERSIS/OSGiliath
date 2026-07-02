/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Modal or modeless dialog container for UI widgets.
 * Provides title bar and layout management.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgText/Text.hpp>
#include <osgUI/Widget.hpp>

namespace osgUI
{

    class OSGUI_EXPORT Dialog : public osg::Inherit<osgUI::Widget, Dialog>
    {
        public:

            Dialog();
            Dialog( const Dialog&      dialog,
                    const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               Dialog )

            void
            setTitle( const std::string& text )
            {
                _title = text;
                dirty();
            }

            std::string&
            getTitle()
            {
                return _title;
            }

            const std::string&
            getTitle() const
            {
                return _title;
            }

            bool
            handleImplementation( osgGA::EventVisitor* ev,
                                  osgGA::Event*        event );

            virtual void
            createGraphicsImplementation();

        protected:

            virtual ~Dialog()
            {
            }

            std::string                 _title;

            osg::ref_ptr<osg::Group>    _group;

            // implementation detail
            osg::ref_ptr<osgText::Text> _titleDrawable;
    };

}
