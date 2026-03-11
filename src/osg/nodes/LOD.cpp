/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Level-of-detail node that selects children based on distance
 * from the camera. Used for performance optimization.
 */
#include <osg/nodes/LOD.hpp>

#include <algorithm>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/traversal/CullStack.hpp>

using namespace osg;

LOD::LOD() :
    _centerMode( USE_BOUNDING_SPHERE_CENTER ),
    _radius( -1.0F ),
    _rangeMode( DISTANCE_FROM_EYE_POINT )
{
}

LOD::LOD( const LOD&    lod,
          const CopyOp& copyop ) :
    Inherit<Group,
            LOD>( lod,
                  copyop ),
    _centerMode( lod._centerMode ),
    _userDefinedCenter( lod._userDefinedCenter ),
    _radius( lod._radius ),
    _rangeMode( lod._rangeMode ),
    _rangeList( lod._rangeList )
{
}

void
LOD::traverse( NodeVisitor& nv )
{
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

                unsigned int numChildren = static_cast<unsigned int>( _children.size() );
                if( _rangeList.size() < numChildren )
                {
                    numChildren = static_cast<unsigned int>( _rangeList.size() );
                }

                for( unsigned int i = 0; i < numChildren; ++i )
                {
                    if( _rangeList[i].first <=
                        required_range &&
                        required_range < _rangeList[i].second )
                    {
                        _children[i]->accept( nv );
                    }
                }
                break;
            }
        default :
            break;
    }
}

void
LOD::traverse( ConstNodeVisitor& nv ) const
{
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
                    // fallback to selecting the highest res tile by
                    // finding out the max range
                    for( unsigned int i = 0; i < _rangeList.size(); ++i )
                    {
                        required_range = std::max( required_range, _rangeList[i].first );
                    }
                }

                unsigned int numChildren = static_cast<unsigned int>( _children.size() );
                if( _rangeList.size() < numChildren )
                {
                    numChildren = static_cast<unsigned int>( _rangeList.size() );
                }

                for( unsigned int i = 0; i < numChildren; ++i )
                {
                    if( _rangeList[i].first <=
                        required_range &&
                        required_range < _rangeList[i].second )
                    {
                        _children[i]->accept( nv );
                    }
                }
                break;
            }
        default :
            break;
    }
}

sphere
LOD::computeBound() const
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

bool
LOD::addChild( Node* child )
{
    if( Group::addChild( child ) )
    {

        if( _children.size() > _rangeList.size() )
        {
            float maxRange = !_rangeList.empty() ? _rangeList.back().second : 0.0F;

            _rangeList.resize( _children.size(), MinMaxPair( maxRange, maxRange ) );
        }

        return true;
    }
    return false;
}

bool
LOD::addChild( Node* child,
               float min,
               float max )
{
    if( Group::addChild( child ) )
    {
        if( _children.size() > _rangeList.size() )
        {
            _rangeList.resize( _children.size(), MinMaxPair( min, min ) );
        }
        _rangeList[_children.size() - 1].first  = min;
        _rangeList[_children.size() - 1].second = max;
        return true;
    }
    return false;
}

bool
LOD::removeChildren( unsigned int pos,
                     unsigned int numChildrenToRemove )
{
    if( pos < _rangeList.size() )
    {
        _rangeList.erase( _rangeList.begin() + pos,
                          std::min( _rangeList.begin() + ( pos + numChildrenToRemove ),
                                    _rangeList.end() ) );
    }

    return Group::removeChildren( pos, numChildrenToRemove );
}

void
LOD::setRange( unsigned int childNo,
               float        min,
               float        max )
{
    if( childNo >= _rangeList.size() )
    {
        _rangeList.resize( childNo + 1, MinMaxPair( min, min ) );
    }
    _rangeList[childNo].first  = min;
    _rangeList[childNo].second = max;
}
