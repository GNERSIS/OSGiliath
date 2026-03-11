/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Visitor collecting animation statistics (channel counts,
 * active animations, bone counts) from the scene.
 */
#pragma once

#include <osg/core/Stats.hpp>
#include <osgAnimation/core/ActionVisitor.hpp>
#include <osgAnimation/core/Export.hpp>
#include <vector>

namespace osgAnimation
{

    class OSGANIMATION_EXPORT StatsActionVisitor
        : public osgAnimation::UpdateActionVisitor
    {
        protected:

            osg::ref_ptr<osg::Stats> _stats;
            std::vector<std::string> _channels;

        public:

            META_ActionVisitor( osgAnimation,
                                StatsActionVisitor );

            StatsActionVisitor();
            StatsActionVisitor( osg::Stats*  stats,
                                unsigned int frame );
            void
            reset();

            const std::vector<std::string>&
            getChannels() const
            {
                return _channels;
            }

            osg::Stats*
            getStats()
            {
                return _stats.get();
            }

            void
            setStats( osg::Stats* stats )
            {
                _stats = stats;
            }

            void
            setFrame( unsigned int frame )
            {
                _frame = frame;
            }

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

}
