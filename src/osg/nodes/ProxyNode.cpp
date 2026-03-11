/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Node whose children are loaded on demand from external files
 * via the DatabasePager. Used for deferred scene loading.
 */
#include <osg/nodes/ProxyNode.hpp>

#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/traversal/CullStack.hpp>

using namespace osg;

ProxyNode::ProxyNode() :
    _loadingExtReference( LOAD_IMMEDIATELY ),
    _centerMode( USER_DEFINED_CENTER ),
    _radius( -1 )
{
}

ProxyNode::ProxyNode( const ProxyNode& proxynode,
                      const CopyOp&    copyop ) :
    Inherit( proxynode,
             copyop ),
    _filenameList( proxynode._filenameList ),
    _databaseOptions( proxynode._databaseOptions ),
    _databasePath( proxynode._databasePath ),
    _loadingExtReference( proxynode._loadingExtReference ),
    _centerMode( proxynode._centerMode ),
    _userDefinedCenter( proxynode._userDefinedCenter ),
    _radius( proxynode._radius )
{
}

void
ProxyNode::setDatabasePath( const std::string& path )
{
    _databasePath = path;
    if( !_databasePath.empty() )
    {
        char&      lastCharacter = _databasePath[_databasePath.size() - 1];
        const char unixSlash     = '/';
        const char winSlash      = '\\';

        if( lastCharacter == winSlash )
        {
            lastCharacter = unixSlash;
        }
        else if( lastCharacter != unixSlash )
        {
            _databasePath += unixSlash;
        }
    }
}

void
ProxyNode::traverse( NodeVisitor& nv )
{
    if( nv.getDatabaseRequestHandler() &&
        _filenameList.size() >
        _children.size() &&
        _loadingExtReference != NO_AUTOMATIC_LOADING )
    {
        for( unsigned int i = static_cast<unsigned int>( _children.size() );
             i < _filenameList.size();
             ++i )
        {
            nv.getDatabaseRequestHandler()->requestNodeFile( _databasePath +
                                                                 _filenameList[i].first,
                                                             nv.getNodePath(),
                                                             1.0F,
                                                             nv.getFrameStamp(),
                                                             _filenameList[i].second,
                                                             _databaseOptions.get() );
        }
    }
    else
    {
        Group::traverse( nv );
    }
}

void
ProxyNode::traverse( ConstNodeVisitor& nv ) const
{
    // Const traversal: visit loaded children only, no database requests
    Group::traverse( nv );
}

void
ProxyNode::expandFileNameListTo( unsigned int pos )
{
    if( pos >= _filenameList.size() )
    {
        _filenameList.resize( pos + 1 );
    }
}

bool
ProxyNode::addChild( Node* child )
{
    if( Group::addChild( child ) )
    {
        expandFileNameListTo( static_cast<unsigned int>( _children.size() ) - 1 );
        return true;
    }
    return false;
}

bool
ProxyNode::addChild( Node*              child,
                     const std::string& filename )
{
    if( Group::addChild( child ) )
    {
        setFileName( static_cast<unsigned int>( _children.size() ) - 1, filename );
        return true;
    }
    return false;
}

bool
ProxyNode::removeChildren( unsigned int pos,
                           unsigned int numChildrenToRemove )
{
    if( pos < _filenameList.size() )
    {
        _filenameList.erase( _filenameList.begin() + pos,
                             std::min( _filenameList.begin() +
                                           ( pos + numChildrenToRemove ),
                                       _filenameList.end() ) );
    }

    return Group::removeChildren( pos, numChildrenToRemove );
}

sphere
ProxyNode::computeBound() const
{
    if( _centerMode == USER_DEFINED_CENTER && _radius >= 0.0F )
    {
        return sphere( _userDefinedCenter, _radius );
    }
    else if( _centerMode ==
             UNION_OF_BOUNDING_SPHERE_AND_USER_DEFINED &&
             _radius >= 0.0F )
    {
        sphere bs = sphere( _userDefinedCenter, _radius );
        bs.expandBy( Group::computeBound() );
        // alternative (used in TxpPagedLOD)
        //  bs.expandRadiusBy(Group::computeBound());
        return bs;
    }
    else
    {
        return Group::computeBound();
    }
}
