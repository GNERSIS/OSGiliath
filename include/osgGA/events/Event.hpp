/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Base event class carrying a timestamp. Foundation for
 * GUIEventAdapter and custom application events.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osgGA/Export>

namespace osgGA
{

    // forward declare
    class GUIEventAdapter;

    /** Base Event class.*/
    class OSGGA_EXPORT Event : public osg::Inherit<osg::Object, Event>
    {
        public:

            Event();

            Event( const Event&       rhs,
                   const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgGA,
                               Event )

            virtual GUIEventAdapter*
            asGUIEventAdapter()
            {
                return 0;
            }

            virtual const GUIEventAdapter*
            asGUIEventAdapter() const
            {
                return 0;
            }

            /** Set whether this event has been handled by an event handler or not.*/
            void
            setHandled( bool handled ) const
            {
                _handled = handled;
            }

            /** Get whether this event has been handled by an event handler or not.*/
            bool
            getHandled() const
            {
                return _handled;
            }

            /** set time in seconds of event. */
            void
            setTime( double time )
            {
                _time = time;
            }

            /** get time in seconds of event. */
            double
            getTime() const
            {
                return _time;
            }

        protected:

            virtual ~Event()
            {
            }

            mutable bool _handled;
            double       _time;
    };

}
