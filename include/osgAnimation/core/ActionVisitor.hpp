/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Visitor for traversing Action hierarchies in the timeline.
 * Used internally for timeline evaluation.
 */
#pragma once

#include <osg/core/Referenced.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/core/FrameAction.hpp>
#include <vector>

namespace osgAnimation
{

    class Timeline;
    class Action;
    class ActionBlendIn;
    class ActionBlendOut;
    class ActionAnimation;
    class ActionStripAnimation;

#define META_ActionVisitor( library, name ) \
    virtual const char* libraryName() const \
    {                                       \
        return #library;                    \
    }                                       \
    virtual const char* className() const   \
    {                                       \
        return #name;                       \
    }

    class OSGANIMATION_EXPORT ActionVisitor : public osg::Referenced
    {
        public:

            META_ActionVisitor( osgAnimation,
                                ActionVisitor );
            ActionVisitor();
            void
            traverse( Action& visitor );

            void
            pushFrameActionOnStack( const FrameAction& fa );
            void
            popFrameAction();

            void
            pushTimelineOnStack( Timeline* tm );
            void
            popTimeline();

            Timeline*
            getCurrentTimeline();

            void
            setCurrentLayer( int layer )
            {
                _currentLayer = layer;
            }

            int
            getCurrentLayer() const
            {
                return _currentLayer;
            }

            const std::vector<FrameAction>&
            getStackedFrameAction() const
            {
                return _stackFrameAction;
            }

            virtual void
            apply( Action& action );
            virtual void
            apply( Timeline& tm );
            virtual void
            apply( ActionBlendIn& action );
            virtual void
            apply( ActionBlendOut& action );
            virtual void
            apply( ActionAnimation& action );
            virtual void
            apply( ActionStripAnimation& action );

        protected:

            std::vector<FrameAction> _stackFrameAction;
            std::vector<Timeline*>   _stackTimeline;
            int                      _currentLayer;
    };

    class OSGANIMATION_EXPORT UpdateActionVisitor : public osgAnimation::ActionVisitor
    {
        protected:

            unsigned int _frame;
            unsigned int _currentAnimationPriority;

        public:

            META_ActionVisitor( osgAnimation,
                                UpdateActionVisitor );
            UpdateActionVisitor();

            void
            setFrame( unsigned int frame )
            {
                _frame = frame;
            }

            bool
            isActive( Action& action ) const;
            unsigned int
            getLocalFrame() const;

            void
            apply( Timeline& action );
            void
            apply( Action& action );
            void
            apply( ActionBlendIn& action );
            void
            apply( ActionBlendOut& action );
            void
            apply( ActionAnimation& action );
            void
            apply( ActionStripAnimation& action );
    };

    class OSGANIMATION_EXPORT ClearActionVisitor : public osgAnimation::ActionVisitor
    {
        public:

            enum ClearType
            {
                BEFORE_FRAME,
                AFTER_FRAME,
            };

            META_ActionVisitor( osgAnimation,
                                ClearActionVisitor );
            ClearActionVisitor( ClearType type = BEFORE_FRAME );

            void
            setFrame( unsigned int frame )
            {
                _frame = frame;
            }

            void
            apply( Timeline& action );
            void
            apply( Action& action );

        protected:

            unsigned int                      _frame;
            std::vector<osg::ref_ptr<Action>> _remove;
            ClearType                         _clearType;
    };

}
