/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract event handler base. Provides handle() method receiving
 * Event and NodeVisitor for scene-graph-attached event processing.
 */
#pragma once

#include <osg/core/ApplicationUsage.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/geometry/Drawable.hpp>
#include <osgGA/events/GUIActionAdapter.hpp>
#include <osgGA/events/GUIEventAdapter.hpp>
#include <osgGA/Export.hpp>
#include <vector>

namespace osgGA
{

    /**
    EventHandler is base class for adding handling of events, either as node event
    callback, drawable event callback or an event handler attached directly to the
    view(er)
    */

    class OSGGA_EXPORT EventHandler
        : public osg::Inherit<osg::NodeCallback, EventHandler>,
          public osg::DrawableEventCallback
    {
        public:

            EventHandler()
            {
            }

            EventHandler( const EventHandler& eh,
                          const osg::CopyOp&  copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( eh,
                         copyop ),
                osg::DrawableEventCallback( eh,
                                            copyop )
            {
            }

            OSG_REGISTER_TYPE( osgGA,
                               EventHandler )

            // Explicitly resolve ambiguity from Inherit<NodeCallback,EventHandler> vs
            // DrawableEventCallback
            osg::Object*
            cloneType() const override
            {
                return new EventHandler();
            }

            osg::Object*
            clone( const osg::CopyOp& copyop ) const override
            {
                return new EventHandler( *this, copyop );
            }

            bool
            isSameKindAs( const osg::Object* obj ) const override
            {
                return dynamic_cast<const EventHandler*>( obj ) != NULL;
            }

            const char*
            libraryName() const override
            {
                return _s_libraryName();
            }

            const char*
            className() const override
            {
                return _s_className();
            }

            NodeCallback*
            asNodeCallback() override
            {
                return osg::NodeCallback::asNodeCallback();
            }

            const NodeCallback*
            asNodeCallback() const override
            {
                return osg::NodeCallback::asNodeCallback();
            }

            DrawableEventCallback*
            asDrawableEventCallback() override
            {
                return osg::DrawableEventCallback::asDrawableEventCallback();
            }

            const DrawableEventCallback*
            asDrawableEventCallback() const override
            {
                return osg::DrawableEventCallback::asDrawableEventCallback();
            }

            EventHandler*
            asEventHandler() override
            {
                return this;
            }

            const EventHandler*
            asEventHandler() const override
            {
                return this;
            }

            bool
            run( osg::Object* object,
                 osg::Object* data ) override
            {
                osg::Node*        node = object->asNode();
                osg::NodeVisitor* nv   = data->asNodeVisitor();
                operator()( node, nv );
                return true;
            }

            /** Event traversal node callback method. There is no need to override this
             * method in subclasses of EventHandler as this implementation calls
             * handle(..) for you. */
            void
            operator()( osg::Node*        node,
                        osg::NodeVisitor* nv ) override;

            /** Event traversal drawable callback method. There is no need to override
             * this method in subclasses of EventHandler as this implementation calls
             * handle(..) for you. */
            void
            event( osg::NodeVisitor* nv,
                   osg::Drawable*    drawable ) override;

            /** Handle event. Override the handle(..) method in your event handlers to
             * respond to events. */
            virtual bool
            handle( osgGA::Event*     event,
                    osg::Object*      object,
                    osg::NodeVisitor* nv );

            /** Get the user interface usage of this event handler, i.e. keyboard and
             * mouse bindings.*/
            virtual void
            getUsage( osg::ApplicationUsage& ) const
            {
            }

        protected:
    };

}
