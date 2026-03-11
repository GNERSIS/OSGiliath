/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Ordered sequence of Actions for animation scheduling.
 * Manages playback, seeking, and action sequencing.
 */
#pragma once

#include <map>
#include <osg/core/Notify.hpp>
#include <osg/core/observer_ptr.hpp>
#include <osg/core/Stats.hpp>
#include <osgAnimation/core/Action.hpp>
#include <osgAnimation/core/AnimationManagerBase.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/core/FrameAction.hpp>
#include <osgAnimation/transform/StatsVisitor.hpp>
#include <vector>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT Timeline : public Action
    {
        public:

            Timeline();
            Timeline( const Timeline&    nc,
                      const osg::CopyOp& op = osg::CopyOp::SHALLOW_COPY );

            META_Action( osgAnimation,
                         Timeline );

            enum TimelineStatus
            {
                Play,
                Stop,
            };

            TimelineStatus
            getStatus() const
            {
                return _state;
            }

            typedef std::vector<FrameAction>  ActionList;
            typedef std::map<int, ActionList> ActionLayers;

            const ActionList&
            getActionLayer( int i )
            {
                return _actions[i];
            }

            unsigned int
            getCurrentFrame() const
            {
                return _currentFrame;
            }

            double
            getCurrentTime() const
            {
                return _currentFrame * 1.0 / _fps;
            }

            void
            play()
            {
                _state = Play;
            }

            void
            gotoFrame( unsigned int frame )
            {
                _currentFrame = frame;
            }

            void
            stop()
            {
                _state = Stop;
            }

            bool
            getEvaluating() const
            {
                return _evaluating;
            }

            bool
            isActive( Action* activeAction );

            void
            removeAction( Action* action );
            virtual void
            addActionAt( unsigned int frame,
                         Action*      action,
                         int          priority = 0 );
            virtual void
            addActionAt( double  t,
                         Action* action,
                         int     priority = 0 );
            void
            addActionNow( Action* action,
                          int     priority = 0 );

            void
            clearActions();

            virtual void
            update( double simulationTime );

            void
            setLastFrameEvaluated( unsigned int frame )
            {
                _previousFrameEvaluated = frame;
            }

            void
            setEvaluating( bool state )
            {
                _evaluating = state;
            }

            void
            traverse( ActionVisitor& visitor );

            void
            setStats( osg::Stats* stats );
            osg::Stats*
            getStats();
            void
            collectStats( bool state );
            osgAnimation::StatsActionVisitor*
            getStatsVisitor();

            const ActionLayers&
            getActionLayers() const
            {
                return _actions;
            }

            void
            processPendingOperation();
            void
            setAnimationManager( AnimationManagerBase* );

        protected:

            osg::observer_ptr<AnimationManagerBase>        _animationManager;
            ActionLayers                                   _actions;
            double                                         _lastUpdate;
            double                                         _speed;
            unsigned int                                   _currentFrame;
            unsigned int                                   _previousFrameEvaluated;
            bool                                           _initFirstFrame;
            TimelineStatus                                 _state;

            bool                                           _collectStats;
            osg::ref_ptr<osg::Stats>                       _stats;
            osg::ref_ptr<osgAnimation::StatsActionVisitor> _statsVisitor;

            // to manage pending operation
            bool                                           _evaluating;

            struct Command
            {
                    Command() :
                        _priority( 0 )
                    {
                    }

                    Command( int                priority,
                             const FrameAction& action ) :
                        _priority( priority ),
                        _action( action )
                    {
                    }

                    int         _priority;
                    FrameAction _action;
            };

            typedef std::vector<Command> CommandList;
            CommandList                  _addActionOperations;
            ActionList                   _removeActionOperations;

            void
            internalRemoveAction( Action* action );
            void
            internalAddAction( int                priority,
                               const FrameAction& ftl );
    };

}
