/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Thread-safe weak reference to a path of nodes. Automatically
 * invalidated when any node in the path is deleted.
 */
#include <osg/core/ObserverNodePath.hpp>

#include <osg/core/Notify.hpp>

using namespace osg;

ObserverNodePath::ObserverNodePath()
{
}

ObserverNodePath::ObserverNodePath( const ObserverNodePath& rhs )
{
    std::lock_guard<std::mutex> lock_rhs( _mutex );
    _nodePath = rhs._nodePath;
}

ObserverNodePath::ObserverNodePath( const osg::NodePath& nodePath )
{
    setNodePath( nodePath );
}

ObserverNodePath::~ObserverNodePath()
{
    clearNodePath();
}

ObserverNodePath&
ObserverNodePath::operator=( const ObserverNodePath& rhs )
{
    if( &rhs == this )
    {
        return *this;
    }

    std::lock_guard<std::mutex> lock_rhs( rhs._mutex );
    std::lock_guard<std::mutex> lock_lhs( _mutex );
    _nodePath = rhs._nodePath;
    return *this;
}

void
ObserverNodePath::setNodePathTo( osg::Node* node )
{
    if( node )
    {
        NodePathList nodePathList = node->getParentalNodePaths();
        if( nodePathList.empty() )
        {
            NodePath nodePath;
            nodePath.push_back( node );
            setNodePath( nodePath );
        }
        else
        {
            if( nodePathList[0].empty() )
            {
                nodePathList[0].push_back( node );
            }
            setNodePath( nodePathList[0] );
        }
    }
    else
    {
        clearNodePath();
    }
}

void
ObserverNodePath::setNodePath( const osg::NodePath& nodePath )
{
    std::lock_guard<std::mutex> lock( _mutex );
    _setNodePath( nodePath );
}

void
ObserverNodePath::setNodePath( const osg::RefNodePath& refNodePath )
{
    osg::NodePath nodePath;
    for( RefNodePath::const_iterator itr = refNodePath.begin(); itr != refNodePath.end();
         ++itr )
    {
        nodePath.push_back( itr->get() );
    }
    setNodePath( nodePath );
}

void
ObserverNodePath::clearNodePath()
{
    std::lock_guard<std::mutex> lock( _mutex );
    _clearNodePath();
}

bool
ObserverNodePath::getRefNodePath( RefNodePath& refNodePath ) const
{
    std::lock_guard<std::mutex> lock( _mutex );
    refNodePath.resize( _nodePath.size() );
    for( unsigned int i = 0; i < _nodePath.size(); ++i )
    {
        if( !_nodePath[i].lock( refNodePath[i] ) )
        {
            OSG_INFO << "ObserverNodePath::getRefNodePath() node has been invalidated"
                     << std::endl;
            refNodePath.clear();
            return false;
        }
    }
    return true;
}

bool
ObserverNodePath::getNodePath( NodePath& nodePath ) const
{
    std::lock_guard<std::mutex> lock( _mutex );
    nodePath.resize( _nodePath.size() );
    for( unsigned int i = 0; i < _nodePath.size(); ++i )
    {
        if( _nodePath[i].valid() )
        {
            nodePath[i] = _nodePath[i].get();
        }
        else
        {
            OSG_NOTICE << "ObserverNodePath::getNodePath() node has been invalidated"
                       << std::endl;
            nodePath.clear();
            return false;
        }
    }
    return true;
}

void
ObserverNodePath::_setNodePath( const osg::NodePath& nodePath )
{
    _clearNodePath();

    // OSG_NOTICE<<"ObserverNodePath["<<this<<"]::_setNodePath()
    // nodePath.size()="<<nodePath.size()<<std::endl;

    _nodePath.resize( nodePath.size() );
    for( unsigned int i = 0; i < nodePath.size(); ++i )
    {
        _nodePath[i] = nodePath[i];
    }
}

void
ObserverNodePath::_clearNodePath()
{
    // OSG_NOTICE<<"ObserverNodePath["<<this<<"]::_clearNodePath()
    // _nodePath.size()="<<_nodePath.size()<<std::endl;
    _nodePath.clear();
}
