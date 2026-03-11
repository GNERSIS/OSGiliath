/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Update callback that drives animation target values.
 * Reads blended channel outputs and applies to nodes.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osgAnimation/core/Animation.hpp>
#include <osgAnimation/core/Channel.hpp>
#include <string>

namespace osgAnimation
{

    class AnimationUpdateCallbackBase : public virtual osg::Object
    {
        public:

            virtual bool
            link( Channel* channel ) = 0;
            virtual int
            link( Animation* animation ) = 0;
    };

    template<class T>
    class AnimationUpdateCallback : public AnimationUpdateCallbackBase,
                                    public T
    {
        public:

            AnimationUpdateCallback()
            {
            }

            AnimationUpdateCallback( const std::string& name )
            {
                T::setName( name );
            }

            AnimationUpdateCallback( const AnimationUpdateCallback& apc,
                                     const osg::CopyOp&             copyop ) :
                T( apc,
                   copyop )
            {
            }

            OSG_REGISTER_TYPE( osgAnimation,
                               AnimationUpdateCallback<T> )

            virtual osg::Callback*
            asCallback()
            {
                return T::asCallback();
            }

            virtual const osg::Callback*
            asCallback() const
            {
                return T::asCallback();
            }

            virtual osg::CallbackObject*
            asCallbackObject()
            {
                return T::asCallbackObject();
            }

            virtual const osg::CallbackObject*
            asCallbackObject() const
            {
                return T::asCallbackObject();
            }

            const std::string&
            getName() const
            {
                return T::getName();
            }

            bool
            link( Channel* /*channel*/ )
            {
                return 0;
            }

            int
            link( Animation* animation )
            {
                if( T::getName().empty() )
                {
                    osg::notify( osg::WARN )
                        << "An update callback has no name, it means it could link only "
                           "with \"\" named Target, often an error, discard"
                        << std::endl;
                    return 0;
                }
                int nbLinks = 0;
                for( ChannelList::iterator it = animation->getChannels().begin();
                     it != animation->getChannels().end();
                     ++it )
                {
                    std::string targetName = ( *it )->getTargetName();
                    if( targetName == T::getName() )
                    {
                        AnimationUpdateCallbackBase* a = this;
                        a->link( ( *it ).get() );
                        nbLinks++;
                    }
                }
                return nbLinks;
            }
    };

}
