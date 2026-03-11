#ifndef EVENTPROPERTY_H
#define EVENTPROPERTY_H

#include "UpdateProperty.hpp"

#include <osg/core/Inherit.hpp>
#include <osg/maths/compat.hpp>
#include <osgGA/events/GUIEventAdapter.hpp>

namespace gsc
{

    class EventProperty : public osg::Inherit<gsc::UpdateProperty, EventProperty>
    {
        public:

            OSG_REGISTER_TYPE( gsc,
                               EventProperty )

            EventProperty()
            {
            }

            EventProperty( osgGA::GUIEventAdapter* event ) :
                _event( event )
            {
            }

            EventProperty( const EventProperty& cpp,
                           const osg::CopyOp&   copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( cpp,
                         copyop )
            {
            }

            void
            setEvent( osgGA::GUIEventAdapter* ea )
            {
                _event = ea;
            }

            osgGA::GUIEventAdapter*
            getEvent()
            {
                return _event.get();
            }

            const osgGA::GUIEventAdapter*
            getEvent() const
            {
                return _event.get();
            }

            virtual void
            update( osgViewer::View* view );

        protected:

            virtual ~EventProperty()
            {
            }

            double                               _previousFrameTime;
            osg::ref_ptr<osgGA::GUIEventAdapter> _event;
    };

}

#endif
