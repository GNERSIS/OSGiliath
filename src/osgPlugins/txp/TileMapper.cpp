/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: distance, getLODScale, parent_tid, delta.
 */
/*    Dec 2010 - TileMapper was fixed and simplified
    Nick
*/

#include "TileMapper.h"
#include "TXPPagedLOD.h"

#include <osg/maths/compat.h>
#include <osg/maths/Math.hpp>
#include <osg/state/Material.hpp>

using namespace txp;

float
TileMapper::getDistanceToEyePoint( const osg::vec3& pos,
                                   bool             withLODScale ) const
{
    if( withLODScale )
    {
        return osg::length( pos - getEyeLocal() ) * getLODScale();
    }
    else
    {
        return osg::length( pos - getEyeLocal() );
    }
}

inline TileMapper::value_type
distance( const osg::vec3&  coord,
          const osg::dmat4& matrix )
{

    return -( ( TileMapper::value_type )coord[0] *
              ( TileMapper::value_type )matrix( 2, 0 ) +
              ( TileMapper::value_type )coord[1] *
              ( TileMapper::value_type )matrix( 2, 1 ) +
              ( TileMapper::value_type )coord[2] *
              ( TileMapper::value_type )matrix( 2, 2 ) +
              matrix( 2, 3 ) );
}

float
TileMapper::getDistanceFromEyePoint( const osg::vec3& pos,
                                     bool             withLODScale ) const
{
    const osg::dmat4& matrix = *_modelviewStack.back();
    float             dist   = distance( pos, matrix );

    if( withLODScale )
    {
        return dist * getLODScale();
    }
    else
    {
        return dist;
    }
}

void
TileMapper::apply( osg::Node& node )
{
    if( node.getName() == "TileContent" )
    {
        _containsGeode = true;
        return;
    }

    if( isCulled( node ) )
    {
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    traverse( node );

    // pop the culling mode.
    popCurrentMask();
}

void
TileMapper::apply( osg::Group& node )
{
    if( node.getName() == "TileContent" )
    {
        _containsGeode = true;
        return;
    }

    if( isCulled( node ) )
    {
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    TileIdentifier* tid = dynamic_cast<TileIdentifier*>( node.getUserData() );

    if( tid )
    {
        _containsGeode = false;
    }

    traverse( node );

    if( tid )
    {
        if( _containsGeode )
        {
            insertTile( *tid );

            _containsGeode = false;
        }
    }

    // pop the culling mode.
    popCurrentMask();
}

void
TileMapper::apply( osg::Geode& )
{
    _containsGeode = true;
}

void
TileMapper::apply( osg::PagedLOD& node )
{
    if( isCulled( node ) )
    {
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    TXPPagedLOD* txpPagedLOD = dynamic_cast<TXPPagedLOD*>( &node );
    if( txpPagedLOD )
    {
        _containsGeode = false;
    }

    traverse( node );

    if( txpPagedLOD )
    {
        if( _containsGeode )
        {
            insertTile( txpPagedLOD->_tileIdentifier );

            _containsGeode = false;
        }
    }

    // pop the culling mode.
    popCurrentMask();
}

void
TileMapper::insertTile( const TileIdentifier& tid )
{
    _tileMap.insert( TileMap::value_type( tid, 1 ) );
}

bool
TileMapper::isTileNeighbourALowerLODLevel( const TileIdentifier& tid,
                                           int                   dx,
                                           int                   dy ) const
{
    if( _tileMap.count( TileIdentifier( tid.x + dx, tid.y + dy, tid.lod ) ) != 0 )
    {
        // we have a neighbour at the same lod level.
        return false;
    }

    // find the tiles parents.
    TileMap::const_iterator itr = _tileMap.find( tid );
    if( itr == _tileMap.end() )
    {
        // not found tile in _tileMap, what should we do??
        // return true as a fallback right now.
#if 0
        std::cout << "TileMapper::isTileNeighbourALowerLODLevel() Not found tile in map," << std::endl;
        std::cout << "    LOD=" << tid.lod << "  X=" << tid.x << "  Y=" << tid.y << std::endl;
#endif
        return true;
    }
    TileIdentifier parent_tid( tid.x / 2, tid.y / 2, tid.lod - 1 );

    bool           parentHasNorthNeighour =
        _tileMap.count(
            TileIdentifier( parent_tid.x, parent_tid.y + 1, parent_tid.lod )
        ) != 0;
    bool parentHasEastNeighour = _tileMap.count( TileIdentifier( parent_tid.x + 1,
                                                                 parent_tid.y,
                                                                 parent_tid.lod ) ) != 0;
    bool parentHasSouthNeighour =
        _tileMap.count(
            TileIdentifier( parent_tid.x, parent_tid.y - 1, parent_tid.lod )
        ) != 0;
    bool parentHasWestNeighour = _tileMap.count( TileIdentifier( parent_tid.x - 1,
                                                                 parent_tid.y,
                                                                 parent_tid.lod ) ) != 0;

    // identify whether the tile is a NE/SE/SW/NW tile relative to its parent.
    osg::vec3 delta( tid.x % 2, tid.y % 2, 0 );

    if( delta.y > 0.0F )    // north side
    {
        if( delta.x > 0.0F )
        {
            // NE
            if( dy == 1 )
            {
                return parentHasNorthNeighour;
            }
            else if( dx == 1 )
            {
                return parentHasEastNeighour;
            }
        }
        else
        {
            // NW
            if( dy == 1 )
            {
                return parentHasNorthNeighour;
            }
            else if( dx == -1 )
            {
                return parentHasWestNeighour;
            }
        }
    }
    else    // south side
    {
        if( delta.x > 0.0F )
        {
            // SE
            if( dy == -1 )
            {
                return parentHasSouthNeighour;
            }
            else if( dx == 1 )
            {
                return parentHasEastNeighour;
            }
        }
        else
        {
            // SW
            if( dy == -1 )
            {
                return parentHasSouthNeighour;
            }
            else if( dx == -1 )
            {
                return parentHasWestNeighour;
            }
        }
    }

    return false;
}
