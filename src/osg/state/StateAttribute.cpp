/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for OpenGL state attributes (textures, programs,
 * blend funcs, etc.). Provides type identity, comparison, and
 * shader composition defines.
 */
#include <osg/state/StateAttribute.hpp>

#include <algorithm>
#include <osg/core/Notify.hpp>
#include <osg/state/State.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/traversal/NodeVisitor.hpp>

using namespace osg;

StateAttribute::StateAttribute() :
    Object( true )
{
}

void
StateAttribute::addParent( osg::StateSet* object )
{
    OSG_DEBUG_FP << "Adding parent" << getRefMutex() << std::endl;
    std::unique_lock<std::mutex> lock( *getRefMutex() );

    _parents.push_back( object );
}

void
StateAttribute::removeParent( osg::StateSet* object )
{
    std::unique_lock<std::mutex> lock( *getRefMutex() );

    ParentList::iterator pitr = std::find( _parents.begin(), _parents.end(), object );
    if( pitr != _parents.end() )
    {
        _parents.erase( pitr );
    }
}

void
StateAttribute::setUpdateCallback( StateAttributeCallback* uc )
{
    OSG_DEBUG << "StateAttribute::Setting Update callbacks" << std::endl;

    if( _updateCallback == uc )
    {
        return;
    }

    int delta = 0;
    if( _updateCallback.valid() )
    {
        --delta;
    }
    if( uc )
    {
        ++delta;
    }

    _updateCallback = uc;

    if( delta != 0 )
    {
        for( ParentList::iterator itr = _parents.begin(); itr != _parents.end(); ++itr )
        {
            ( *itr )->setNumChildrenRequiringUpdateTraversal( static_cast<unsigned int>(
                static_cast<int>( ( *itr )->getNumChildrenRequiringUpdateTraversal() ) +
                delta
            ) );
        }
    }
}

void
StateAttribute::setEventCallback( StateAttributeCallback* ec )
{
    OSG_DEBUG << "StateAttribute::Setting Event callbacks" << std::endl;

    if( _eventCallback == ec )
    {
        return;
    }

    int delta = 0;
    if( _eventCallback.valid() )
    {
        --delta;
    }
    if( ec )
    {
        ++delta;
    }

    _eventCallback = ec;

    if( delta != 0 )
    {
        for( ParentList::iterator itr = _parents.begin(); itr != _parents.end(); ++itr )
        {
            ( *itr )->setNumChildrenRequiringEventTraversal( static_cast<unsigned int>(
                static_cast<int>( ( *itr )->getNumChildrenRequiringEventTraversal() ) +
                delta
            ) );
        }
    }
}

StateAttribute::ReassignToParents::ReassignToParents( osg::StateAttribute* attr )
{
    if( !attr->isTextureAttribute() && !attr->getParents().empty() )
    {
        // take a reference to this clip plane to prevent it from going out of scope
        // when we remove it temporarily from its parents.
        attribute = attr;

        // copy the parents as they _parents list will be changed by the subsequent
        // removeAttributes.
        parents = attr->getParents();

        // remove this attribute from its parents as its position is being changed
        // and would no longer be valid.
        for( ParentList::iterator itr = parents.begin(); itr != parents.end(); ++itr )
        {
            osg::StateSet* stateset = *itr;
            stateset->removeAttribute( attr );

            OSG_NOTICE << "  Removed from parent " << stateset << std::endl;
        }
    }
}

StateAttribute::ReassignToParents::~ReassignToParents()
{
    // add attribute back into its original parents with its new position
    for( ParentList::iterator itr = parents.begin(); itr != parents.end(); ++itr )
    {
        osg::StateSet* stateset = *itr;
        stateset->setAttribute( attribute.get() );
        OSG_NOTICE << "   Added back to parent " << stateset << std::endl;
    }
}
