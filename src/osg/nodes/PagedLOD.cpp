/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * LOD node with database-paged children. Loads and unloads detail
 * levels on demand via the DatabasePager for large terrains.
 */
#include <osg/nodes/PagedLOD.hpp>

#include <algorithm>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/traversal/CullStack.hpp>

using namespace osg;

PagedLOD::PerRangeData::PerRangeData() :
    _priorityOffset( 0.0F ),
    _priorityScale( 1.0F ),
    _minExpiryTime( 0.0 ),
    _minExpiryFrames( 0 ),
    _timeStamp( 0.0F ),
    _frameNumber( 0 ),
    _frameNumberOfLastReleaseGLObjects( 0 )
{
}

PagedLOD::PerRangeData::PerRangeData( const PerRangeData& prd ) :
    _filename( prd._filename ),
    _priorityOffset( prd._priorityOffset ),
    _priorityScale( prd._priorityScale ),
    _minExpiryTime( prd._minExpiryTime ),
    _minExpiryFrames( prd._minExpiryFrames ),
    _timeStamp( prd._timeStamp ),
    _frameNumber( prd._frameNumber ),
    _frameNumberOfLastReleaseGLObjects( prd._frameNumberOfLastReleaseGLObjects ),
    _databaseRequest( prd._databaseRequest )
{
}

PagedLOD::PerRangeData&
PagedLOD::PerRangeData::operator=( const PerRangeData& prd )
{
    if( this == &prd )
    {
        return *this;
    }
    _filename                          = prd._filename;
    _priorityOffset                    = prd._priorityOffset;
    _priorityScale                     = prd._priorityScale;
    _timeStamp                         = prd._timeStamp;
    _frameNumber                       = prd._frameNumber;
    _frameNumberOfLastReleaseGLObjects = prd._frameNumberOfLastReleaseGLObjects;
    _databaseRequest                   = prd._databaseRequest;
    _minExpiryTime                     = prd._minExpiryTime;
    _minExpiryFrames                   = prd._minExpiryFrames;
    return *this;
}

PagedLOD::PagedLOD()
{
    _frameNumberOfLastTraversal     = 0;
    _centerMode                     = USER_DEFINED_CENTER;
    _radius                         = -1;
    _numChildrenThatCannotBeExpired = 0;
    _disableExternalChildrenPaging  = false;
}

PagedLOD::PagedLOD( const PagedLOD& plod,
                    const CopyOp&   copyop ) :
    Inherit( plod,
             copyop ),
    _databaseOptions( plod._databaseOptions ),
    _databasePath( plod._databasePath ),
    _frameNumberOfLastTraversal( plod._frameNumberOfLastTraversal ),
    _numChildrenThatCannotBeExpired( plod._numChildrenThatCannotBeExpired ),
    _disableExternalChildrenPaging( plod._disableExternalChildrenPaging ),
    _perRangeDataList( plod._perRangeDataList )
{
}

PagedLOD::~PagedLOD()
{
}

void
PagedLOD::setDatabasePath( const std::string& path )
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

        /*
                // make sure the last character is the appropriate slash
        #ifdef _WIN32
                if (lastCharacter==unixSlash)
                {
                    lastCharacter = winSlash;
                }
                else if (lastCharacter!=winSlash)
                {
                    _databasePath += winSlash;
                }
        #else
                if (lastCharacter==winSlash)
                {
                    lastCharacter = unixSlash;
                }
                else if (lastCharacter!=unixSlash)
                {
                    _databasePath += unixSlash;
                }
        #endif
        */
    }
}

void
PagedLOD::traverse( NodeVisitor& nv )
{
    // set the frame number of the traversal so that external nodes can find out how
    // active this node is.
    if( nv.getFrameStamp() && nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR )
    {
        setFrameNumberOfLastTraversal( nv.getFrameStamp()->getFrameNumber() );
    }

    double timeStamp = nv.getFrameStamp() ? nv.getFrameStamp()->getReferenceTime() : 0.0;
    unsigned int frameNumber =
        nv.getFrameStamp() ? nv.getFrameStamp()->getFrameNumber() : 0;
    bool updateTimeStamp = nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR;

    switch( nv.getTraversalMode() )
    {
        case( NodeVisitor::TRAVERSE_ALL_CHILDREN ) :
            std::for_each( _children.begin(), _children.end(), NodeAcceptOp( nv ) );
            break;
        case( NodeVisitor::TRAVERSE_ACTIVE_CHILDREN ) :
            {
                float required_range = 0;
                if( _rangeMode == DISTANCE_FROM_EYE_POINT )
                {
                    required_range = nv.getDistanceToViewPoint( getCenter(), true );
                }
                else
                {
                    osg::CullStack* cullStack = nv.asCullStack();
                    if( cullStack && cullStack->getLODScale() > 0.0F )
                    {
                        required_range = cullStack->clampedPixelSize( getBound() ) /
                                         cullStack->getLODScale();
                    }
                    else
                    {
                        // fallback to selecting the highest res tile by
                        // finding out the max range
                        for( unsigned int i = 0; i < _rangeList.size(); ++i )
                        {
                            required_range =
                                std::max( required_range, _rangeList[i].first );
                        }
                    }
                }

                int  lastChildTraversed = -1;
                bool needToLoadChild    = false;
                for( unsigned int i = 0; i < _rangeList.size(); ++i )
                {
                    if( _rangeList[i].first <=
                        required_range &&
                        required_range < _rangeList[i].second )
                    {
                        if( i < _children.size() )
                        {
                            if( updateTimeStamp )
                            {
                                _perRangeDataList[i]._timeStamp   = timeStamp;
                                _perRangeDataList[i]._frameNumber = frameNumber;
                            }

                            _children[i]->accept( nv );
                            lastChildTraversed = ( int )i;
                        }
                        else
                        {
                            needToLoadChild = true;
                        }
                    }
                }

                if( needToLoadChild )
                {
                    unsigned int numChildren =
                        static_cast<unsigned int>( _children.size() );

                    // select the last valid child.
                    if( numChildren >
                        0 &&
                        ( ( int )numChildren - 1 ) != lastChildTraversed )
                    {
                        if( updateTimeStamp )
                        {
                            _perRangeDataList[numChildren - 1]._timeStamp = timeStamp;
                            _perRangeDataList[numChildren - 1]._frameNumber =
                                frameNumber;
                        }
                        _children[numChildren - 1]->accept( nv );
                    }

                    // now request the loading of the next unloaded child.
                    if( !_disableExternalChildrenPaging &&
                        nv.getDatabaseRequestHandler() &&
                        numChildren < _perRangeDataList.size() )
                    {
                        // compute priority from where abouts in the required range the
                        // distance falls.
                        float priority =
                            ( _rangeList[numChildren].second - required_range ) /
                            ( _rangeList[numChildren].second -
                              _rangeList[numChildren].first );

                        // invert priority for PIXEL_SIZE_ON_SCREEN mode
                        if( _rangeMode == PIXEL_SIZE_ON_SCREEN )
                        {
                            priority = -priority;
                        }

                        // modify the priority according to the child's priority offset
                        // and scale.
                        priority = _perRangeDataList[numChildren]._priorityOffset +
                                   priority *
                                   _perRangeDataList[numChildren]._priorityScale;

                        if( _databasePath.empty() )
                        {
                            nv.getDatabaseRequestHandler()->requestNodeFile(
                                _perRangeDataList[numChildren]._filename,
                                nv.getNodePath(),
                                priority,
                                nv.getFrameStamp(),
                                _perRangeDataList[numChildren]._databaseRequest,
                                _databaseOptions.get()
                            );
                        }
                        else
                        {
                            // prepend the databasePath to the child's filename.
                            nv.getDatabaseRequestHandler()->requestNodeFile(
                                _databasePath + _perRangeDataList[numChildren]._filename,
                                nv.getNodePath(),
                                priority,
                                nv.getFrameStamp(),
                                _perRangeDataList[numChildren]._databaseRequest,
                                _databaseOptions.get()
                            );
                        }
                    }
                }

                break;
            }
        default :
            break;
    }
}

void
PagedLOD::traverse( ConstNodeVisitor& nv ) const
{
    // Const traversal: no timestamp updates, no database requests
    switch( nv.getTraversalMode() )
    {
        case( NodeVisitor::TRAVERSE_ALL_CHILDREN ) :
            for( const auto& child : _children )
            {
                child->accept( nv );
            }
            break;
        case( NodeVisitor::TRAVERSE_ACTIVE_CHILDREN ) :
            {
                float required_range = 0;
                if( _rangeMode == DISTANCE_FROM_EYE_POINT )
                {
                    // For const traversal, use 0 distance (no eye point available)
                    required_range = 0;
                }
                else
                {
                    // fallback to selecting the highest res tile
                    for( unsigned int i = 0; i < _rangeList.size(); ++i )
                    {
                        required_range = std::max( required_range, _rangeList[i].first );
                    }
                }

                int lastChildTraversed = -1;
                for( unsigned int i = 0; i < _rangeList.size(); ++i )
                {
                    if( _rangeList[i].first <=
                        required_range &&
                        required_range < _rangeList[i].second )
                    {
                        if( i < _children.size() )
                        {
                            _children[i]->accept( nv );
                            lastChildTraversed = ( int )i;
                        }
                    }
                }

                // If no matching child was found, select the last valid child
                if( lastChildTraversed < 0 && !_children.empty() )
                {
                    _children.back()->accept( nv );
                }

                break;
            }
        default :
            break;
    }
}

void
PagedLOD::expandPerRangeDataTo( unsigned int pos )
{
    if( pos >= _perRangeDataList.size() )
    {
        _perRangeDataList.resize( pos + 1 );
    }
}

bool
PagedLOD::addChild( Node* child )
{
    if( LOD::addChild( child ) )
    {
        expandPerRangeDataTo( static_cast<unsigned int>( _children.size() ) - 1 );
        return true;
    }
    return false;
}

bool
PagedLOD::addChild( Node* child,
                    float min,
                    float max )
{
    if( LOD::addChild( child, min, max ) )
    {
        expandPerRangeDataTo( static_cast<unsigned int>( _children.size() ) - 1 );
        return true;
    }
    return false;
}

bool
PagedLOD::addChild( Node*              child,
                    float              min,
                    float              max,
                    const std::string& filename,
                    float              priorityOffset,
                    float              priorityScale )
{
    if( LOD::addChild( child, min, max ) )
    {
        setFileName( static_cast<unsigned int>( _children.size() ) - 1, filename );
        setPriorityOffset( static_cast<unsigned int>( _children.size() ) - 1,
                           priorityOffset );
        setPriorityScale( static_cast<unsigned int>( _children.size() ) - 1,
                          priorityScale );
        return true;
    }
    return false;
}

bool
PagedLOD::removeChildren( unsigned int pos,
                          unsigned int numChildrenToRemove )
{
    if( pos < _rangeList.size() )
    {
        _rangeList.erase( _rangeList.begin() + pos,
                          std::min( _rangeList.begin() + ( pos + numChildrenToRemove ),
                                    _rangeList.end() ) );
    }
    if( pos < _perRangeDataList.size() )
    {
        _perRangeDataList.erase( _perRangeDataList.begin() + pos,
                                 std::min( _perRangeDataList.begin() +
                                               ( pos + numChildrenToRemove ),
                                           _perRangeDataList.end() ) );
    }

    return Group::removeChildren( pos, numChildrenToRemove );
}

bool
PagedLOD::removeExpiredChildren( double       expiryTime,
                                 unsigned int expiryFrame,
                                 NodeList&    removedChildren )
{
    if( _children.size() > _numChildrenThatCannotBeExpired )
    {
        unsigned cindex = static_cast<unsigned int>( _children.size() ) - 1;
        if( !_perRangeDataList[cindex]._filename.empty() &&
            _perRangeDataList[cindex]._timeStamp +
            _perRangeDataList[cindex]._minExpiryTime <
            expiryTime &&
            _perRangeDataList[cindex]._frameNumber +
            _perRangeDataList[cindex]._minExpiryFrames < expiryFrame )
        {
            osg::Node* nodeToRemove = _children[cindex].get();
            removedChildren.push_back( nodeToRemove );
            return Group::removeChildren( cindex, 1 );
        }
    }
    return false;
}
