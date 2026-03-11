/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Visitor that connects animation channels to their target
 * nodes. Resolves named targets in the animation graph.
 */
#include <osgAnimation/skeletal/LinkVisitor.hpp>

#include <osg/core/Notify.hpp>
#include <osg/nodes/Geode.hpp>
#include <osgAnimation/core/AnimationUpdateCallback.hpp>

using namespace osgAnimation;

LinkVisitor::LinkVisitor() :
    osg::DualModeVisitor( osg::DualModeVisitor::TRAVERSE_ALL_CHILDREN )
{
    _nbLinkedTarget = 0;
}

void
LinkVisitor::reset()
{
    _nbLinkedTarget = 0;
}

AnimationList&
LinkVisitor::getAnimationList()
{
    return _animations;
}

void
LinkVisitor::link( AnimationUpdateCallbackBase* cb )
{
    int result = 0;
    for( std::size_t i = 0; i < _animations.size(); i++ )
    {
        result          += cb->link( _animations[i].get() );
        _nbLinkedTarget += static_cast<unsigned int>( result );
    }
    OSG_DEBUG << "LinkVisitor links " << result << " for \"" << cb->getName() << '"'
              << std::endl;
}

void
LinkVisitor::handle_stateset( osg::StateSet* stateset )
{
    if( !stateset )
    {
        return;
    }
    const osg::StateSet::AttributeList& attr = stateset->getAttributeList();
    for( osg::StateSet::AttributeList::const_iterator it = attr.begin();
         it != attr.end();
         ++it )
    {
        osg::StateAttribute*                       sattr = it->second.first.get();
        osgAnimation::AnimationUpdateCallbackBase* cb =
            dynamic_cast<osgAnimation::AnimationUpdateCallbackBase*>(
                sattr->getUpdateCallback()
            );
        if( cb )
        {
            link( cb );
        }
    }
}

void
LinkVisitor::apply( osg::Node& node )
{
    osg::StateSet* st = node.getStateSet();
    if( st )
    {
        handle_stateset( st );
    }

    osg::Callback* cb = node.getUpdateCallback();
    while( cb )
    {
        osgAnimation::AnimationUpdateCallbackBase* cba =
            dynamic_cast<osgAnimation::AnimationUpdateCallbackBase*>( cb );
        if( cba )
        {
            link( cba );
        }
        cb = cb->getNestedCallback();
    }
    traverse( node );
}

void
LinkVisitor::apply( osg::Geode& node )
{
    for( unsigned int i = 0; i < node.getNumDrawables(); i++ )
    {
        osg::Drawable* drawable = node.getDrawable( i );
        if( drawable && drawable->getStateSet() )
        {
            handle_stateset( drawable->getStateSet() );
        }
    }
    apply( static_cast<osg::Node&>( node ) );
}
