/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Callback interfaces for UI widget events.
 * Provides slots for pressed, released, and value-changed.
 */
#pragma once

#include <osg/core/Callback.hpp>
#include <osg/core/Inherit.hpp>
#include <osgGA/events/EventVisitor.hpp>
#include <osgUI/Export.hpp>
#include <osgUI/Widget.hpp>

namespace osgUI
{

    class OSGUI_EXPORT CloseCallback
        : public osg::Inherit<osg::CallbackObject, CloseCallback>
    {
        public:

            CloseCallback( const std::string& callbackName = std::string( "close" ),
                           osgUI::Widget*     closeWidget  = 0 );
            CloseCallback( const CloseCallback& hc,
                           const osg::CopyOp&   copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               CloseCallback )

            void
            setCloseWidget( osgUI::Widget* widget )
            {
                _closeWidget = widget;
            }

            osgUI::Widget*
            getCloseWidget()
            {
                return _closeWidget.get();
            }

            const osgUI::Widget*
            getCloseWidget() const
            {
                return _closeWidget.get();
            }

            virtual bool
            run( osg::Object*     object,
                 osg::Parameters& inputParameters,
                 osg::Parameters& outputParameters ) const;

        protected:

            virtual ~CloseCallback()
            {
            }

            osg::observer_ptr<osgUI::Widget> _closeWidget;
    };

    class OSGUI_EXPORT HandleCallback
        : public osg::Inherit<osg::CallbackObject, HandleCallback>
    {
        public:

            HandleCallback();
            HandleCallback( const HandleCallback& hc,
                            const osg::CopyOp&    copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               HandleCallback )

            virtual bool
            run( osg::Object*     object,
                 osg::Parameters& inputParameters,
                 osg::Parameters& outputParameters ) const;
            virtual bool
            handle( osgGA::EventVisitor* ev,
                    osgGA::Event*        event ) const;

        protected:

            virtual ~HandleCallback()
            {
            }
    };

    class OSGUI_EXPORT DragCallback : public osg::Inherit<HandleCallback, DragCallback>
    {
        public:

            DragCallback();
            DragCallback( const DragCallback& dc,
                          const osg::CopyOp&  copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgUI,
                               DragCallback )

            void
            setDragging( bool v )
            {
                _dragging = v;
            }

            bool
            getDragging() const
            {
                return _dragging;
            }

            void
            setPreviousPosition( const osg::dvec3& position )
            {
                _previousPosition = position;
            }

            const osg::dvec3&
            getPreviousPosition() const
            {
                return _previousPosition;
            }

            virtual bool
            handle( osgGA::EventVisitor* ev,
                    osgGA::Event*        event ) const;

        protected:

            virtual ~DragCallback()
            {
            }

            bool       _dragging;
            osg::dvec3 _previousPosition;
    };

}
