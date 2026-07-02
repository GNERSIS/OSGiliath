/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract rendering technique for terrain tiles.
 * Defines the interface for mesh generation from elevation data.
 */
#include <osgTerrain/TerrainTechnique.hpp>

#include <osgTerrain/TerrainTile.hpp>

using namespace osgTerrain;

/////////////////////////////////////////////////////////////////////////////////////
//
// TerrainNeighbours
//
TerrainNeighbours::TerrainNeighbours()
{
}

TerrainNeighbours::~TerrainNeighbours()
{
    clear();
}

void
TerrainNeighbours::addNeighbour( TerrainTile* tile )
{
    std::lock_guard<std::mutex> lock( _neighboursMutex );
    _neighbours.insert( tile );
}

void
TerrainNeighbours::removeNeighbour( TerrainTile* tile )
{
    std::lock_guard<std::mutex> lock( _neighboursMutex );
    _neighbours.erase( tile );
}

void
TerrainNeighbours::clear()
{
    std::lock_guard<std::mutex> lock( _neighboursMutex );
    _neighbours.clear();
}

bool
TerrainNeighbours::containsNeighbour( TerrainTile* tile ) const
{
    std::lock_guard<std::mutex> lock( _neighboursMutex );
    return _neighbours.count( tile ) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////
//
// TerrainTechnique
//
TerrainTechnique::TerrainTechnique() :
    _terrainTile( 0 )
{
    setThreadSafeRefUnref( true );
}

TerrainTechnique::TerrainTechnique( const TerrainTechnique& tt,
                                    const osg::CopyOp&      copyop ) :
    Inherit( tt,
             copyop ),
    _terrainTile( 0 )
{
}

TerrainTechnique::~TerrainTechnique()
{
}

void
TerrainTechnique::setTerrainTile( TerrainTile* tile )
{
    if( _terrainTile == tile )
    {
        return;
    }

    _neighbours.clear();

    _terrainTile = tile;
}

void
TerrainTechnique::init( int /*dirtyMask*/,
                        bool /*assumeMultiThreaded*/ )
{
    OSG_NOTICE << className() << "::init(..) not implemented yet" << std::endl;
}

void
TerrainTechnique::update( osgUtil::UpdateVisitor* uv )
{
    OSG_NOTICE << className() << "::update(..) not implemented yet" << std::endl;
    if( _terrainTile )
    {
        _terrainTile->osg::Group::traverse( *uv );
    }
}

void
TerrainTechnique::cull( osgUtil::CullVisitor* cv )
{
    OSG_NOTICE << className() << "::cull(..) not implemented yet" << std::endl;
    if( _terrainTile )
    {
        _terrainTile->osg::Group::traverse( *cv );
    }
}

void
TerrainTechnique::cleanSceneGraph()
{
    OSG_NOTICE << className() << "::cleanSceneGraph(..) not implemented yet"
               << std::endl;
}

void
TerrainTechnique::traverse( osg::NodeVisitor& nv )
{
    if( !_terrainTile )
    {
        return;
    }

    // if app traversal update the frame count.
    if( nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR )
    {
        if( _terrainTile->getDirty() )
        {
            _terrainTile->init( _terrainTile->getDirtyMask(), false );
        }

        osgUtil::UpdateVisitor* uv = nv.asUpdateVisitor();
        if( uv )
        {
            update( uv );
            return;
        }
    }
    else if( nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR )
    {
        osgUtil::CullVisitor* cv = nv.asCullVisitor();
        if( cv )
        {
            cull( cv );
            return;
        }
    }

    if( _terrainTile->getDirty() )
    {
        _terrainTile->init( _terrainTile->getDirtyMask(), false );
    }

    // otherwise fallback to the Group::traverse()
    _terrainTile->osg::Group::traverse( nv );
}
