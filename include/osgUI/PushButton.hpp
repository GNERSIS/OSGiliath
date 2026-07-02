/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Clickable push button widget. Emits pressed/released
 * events for user interaction in 3D UI.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/Switch.hpp>
#include <osgText/Text.hpp>
#include <osgUI/Widget.hpp>

namespace osgUI
{

    class OSGUI_EXPORT PushButton : public osg::Inherit<osgUI::Widget, PushButton>
    {
        public:

            PushButton();
            PushButton( const PushButton&  pb,
                        const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               PushButton )

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

            virtual bool
            handleImplementation( osgGA::EventVisitor* ev,
                                  osgGA::Event*        event );
            virtual void
            createGraphicsImplementation();
            virtual void
            enterImplementation();
            virtual void
            leaveImplementation();

            virtual void
            pressed()
            {
                if( !runCallbacks( "pressed" ) )
                {
                    pressedImplementation();
                }
            }

            virtual void
            pressedImplementation();

            virtual void
            released()
            {
                if( !runCallbacks( "released" ) )
                {
                    releasedImplementation();
                }
            }

            virtual void
            releasedImplementation();

        protected:

            virtual ~PushButton()
            {
            }

            std::string                 _text;

            // implementation detail
            osg::ref_ptr<osg::Switch>   _buttonSwitch;
            osg::ref_ptr<osgText::Text> _textDrawable;
    };

}
