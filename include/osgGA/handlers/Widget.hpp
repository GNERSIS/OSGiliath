/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 3D widget base for in-scene UI elements. Receives events
 * via the EventVisitor for interactive scene graph objects.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/box.hpp>
#include <osg/nodes/Group.hpp>
#include <osgGA/events/Event.hpp>
#include <osgGA/events/EventVisitor.hpp>

namespace osgGA
{

    class OSGGA_EXPORT Widget : public osg::Inherit<osg::Group, Widget>
    {
        public:

            Widget();
            Widget( const Widget&      tfw,
                    const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osgGA,
                               Widget )

            virtual void
            traverse( osg::NodeVisitor& nv );
            virtual void
            traverseImplementation( osg::NodeVisitor& nv );

            virtual bool
            handle( osgGA::EventVisitor* ev,
                    osgGA::Event*        event );
            virtual bool
            handleImplementation( osgGA::EventVisitor* ev,
                                  osgGA::Event*        event );

            virtual bool
            computePositionInLocalCoordinates( osgGA::EventVisitor*    ev,
                                               osgGA::GUIEventAdapter* event,
                                               osg::vec3& localPosition ) const;

            virtual void
            createGraphics();
            virtual void
            createGraphicsImplementation();

            virtual void
            setExtents( const osg::box& bb );

            const osg::box&
            getExtents() const
            {
                return _extents;
            }

            enum FocusBehaviour
            {
                CLICK_TO_FOCUS,
                FOCUS_FOLLOWS_POINTER,
                EVENT_DRIVEN_FOCUS_DISABLED,
            };

            void
            setFocusBehaviour( FocusBehaviour behaviour )
            {
                _focusBehaviour = behaviour;
            }

            FocusBehaviour
            getFocusBehaviour() const
            {
                return _focusBehaviour;
            }

            /** update the focus according to events.*/
            virtual void
            updateFocus( osg::NodeVisitor& nv );

            /** set whether the widget has focus or not.*/
            virtual void
            setHasEventFocus( bool focus );

            /** get whether the widget has focus or not.*/
            virtual bool
            getHasEventFocus() const;

            virtual osg::sphere
            computeBound() const;

            /** update any focus related graphics+state to the focused state.*/
            virtual void
            enter();
            virtual void
            enterImplementation();

            /** update any focus related graphics+state to the unfocused state.*/
            virtual void
            leave();
            virtual void
            leaveImplementation();

        protected:

            virtual ~Widget()
            {
            }

            FocusBehaviour _focusBehaviour;
            bool           _hasEventFocus;
            bool           _graphicsInitialized;

            osg::box       _extents;
    };

}
