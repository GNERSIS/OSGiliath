/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Node that selectively enables/disables children by index.
 * Used for toggling visibility of scene graph branches.
 */
#include <osg/nodes/Switch.hpp>

#include <algorithm>
#include <osg/core/Notify.hpp>
#include <osg/maths/box.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/Transform.hpp>

using namespace osg;

Switch::Switch() :
    _newChildDefaultValue( true )
{
}

Switch::Switch( const Switch& sw,
                const CopyOp& copyop ) :
    Inherit<Group,
            Switch>( sw,
                     copyop ),
    _newChildDefaultValue( sw._newChildDefaultValue ),
    _values( sw._values )
{
}

void
Switch::traverse( NodeVisitor& nv )
{
    if( nv.getTraversalMode() == NodeVisitor::TRAVERSE_ACTIVE_CHILDREN )
    {
        for( unsigned int pos = 0; pos < _children.size(); ++pos )
        {
            if( _values[pos] )
            {
                _children[pos]->accept( nv );
            }
        }
    }
    else
    {
        Group::traverse( nv );
    }
}

void
Switch::traverse( ConstNodeVisitor& nv ) const
{
    if( nv.getTraversalMode() == NodeVisitor::TRAVERSE_ACTIVE_CHILDREN )
    {
        for( unsigned int pos = 0; pos < _children.size(); ++pos )
        {
            if( _values[pos] )
            {
                _children[pos]->accept( nv );
            }
        }
    }
    else
    {
        Group::traverse( nv );
    }
}

bool
Switch::addChild( Node* child )
{
    if( Group::addChild( child ) )
    {
        if( _children.size() > _values.size() )
        {
            _values.resize( _children.size(), _newChildDefaultValue );
        }
        // note, we don't override any pre-existing _values[childPosition] setting
        // like in addChild(child,value) below.
        return true;
    }
    return false;
}

bool
Switch::addChild( Node* child,
                  bool  value )
{
    unsigned int childPosition = static_cast<unsigned int>( _children.size() );
    if( Group::addChild( child ) )
    {
        if( _children.size() > _values.size() )
        {
            _values.resize( _children.size(), _newChildDefaultValue );
        }
        _values[childPosition] = value;
        return true;
    }
    return false;
}

bool
Switch::insertChild( unsigned int index,
                     Node*        child )
{
    return insertChild( index, child, _newChildDefaultValue );
}

bool
Switch::insertChild( unsigned int index,
                     Node*        child,
                     bool         value )
{
    if( Group::insertChild( index, child ) )
    {
        if( index >= _values.size() )
        {
            _values.push_back( value );
        }
        else
        {
            _values.insert( _values.begin() + index, value );
        }

        return true;
    }
    return false;
}

bool
Switch::removeChildren( unsigned int pos,
                        unsigned int numChildrenToRemove )
{
    if( pos < _values.size() )
    {
        _values.erase( _values.begin() + pos,
                       std::min( _values.begin() + ( pos + numChildrenToRemove ),
                                 _values.end() ) );
    }

    return Group::removeChildren( pos, numChildrenToRemove );
}

void
Switch::setValue( unsigned int pos,
                  bool         value )
{
    if( pos >= _values.size() )
    {
        _values.resize( pos + 1, _newChildDefaultValue );
    }
    _values[pos] = value;
    dirtyBound();
}

void
Switch::setChildValue( const Node* child,
                       bool        value )
{
    // find the child's position.
    unsigned int pos = getChildIndex( child );
    if( pos == _children.size() )
    {
        return;
    }

    _values[pos] = value;
    dirtyBound();
}

bool
Switch::getValue( unsigned int pos ) const
{
    if( pos >= _values.size() )
    {
        return false;
    }
    return _values[pos];
}

bool
Switch::getChildValue( const Node* child ) const
{
    // find the child's position.
    unsigned int pos = getChildIndex( child );
    if( pos == _children.size() )
    {
        return false;
    }

    return _values[pos];
}

bool
Switch::setAllChildrenOff()
{
    _newChildDefaultValue = false;
    for( ValueList::iterator itr = _values.begin(); itr != _values.end(); ++itr )
    {
        *itr = false;
    }
    dirtyBound();
    return true;
}

bool
Switch::setAllChildrenOn()
{
    _newChildDefaultValue = true;
    for( ValueList::iterator itr = _values.begin(); itr != _values.end(); ++itr )
    {
        *itr = true;
    }
    dirtyBound();
    return true;
}

bool
Switch::setSingleChildOn( unsigned int pos )
{
    for( ValueList::iterator itr = _values.begin(); itr != _values.end(); ++itr )
    {
        *itr = false;
    }
    setValue( pos, true );
    return true;
}

sphere
Switch::computeBound() const
{
    sphere bsphere;
    if( _children.empty() )
    {
        return bsphere;
    }

    // note, special handling of the case when a child is an Transform,
    // such that only Transforms which are relative to their parents coordinates frame
    // (i.e this group) are handled, Transform relative to and absolute reference frame
    // are ignored.

    box bb;
    bb.init();
    for( unsigned int pos = 0; pos < _children.size(); ++pos )
    {
        const osg::Transform* transform = _children[pos]->asTransform();
        if( !transform || transform->getReferenceFrame() == osg::Transform::RELATIVE_RF )
        {
            if( _values[pos] == true )
            {
                const osg::sphere& bs = _children[pos]->getBound();
                bb.expandBy( bs );
            }
        }
    }

    if( !bb.valid() )
    {
        return bsphere;
    }

    bsphere.center = bb.center();
    bsphere.radius = 0.0F;
    for( unsigned int pos = 0; pos < _children.size(); ++pos )
    {
        const osg::Transform* transform = _children[pos]->asTransform();
        if( !transform || transform->getReferenceFrame() == osg::Transform::RELATIVE_RF )
        {
            if( _values[pos] == true )
            {
                const osg::sphere& bs = _children[pos]->getBound();
                bsphere.expandRadiusBy( bs );
            }
        }
    }
    return bsphere;
}
