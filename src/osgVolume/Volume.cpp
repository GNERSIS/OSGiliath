/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Root node for volume rendering. Manages VolumeTiles
 * and coordinates volume data display.
 */
#include <osgVolume/Volume>

using namespace osgVolume;

Volume::Volume()
{
}

Volume::Volume( const Volume&      ts,
                const osg::CopyOp& copyop ) :
    Inherit( ts,
             copyop )
{
}

Volume::~Volume()
{
    std::lock_guard<std::mutex> lock( _mutex );

    for( VolumeTileSet::iterator itr = _volumeTileSet.begin();
         itr != _volumeTileSet.end();
         ++itr )
    {
        const_cast<VolumeTile*>( *itr )->_volume = 0;
    }

    _volumeTileSet.clear();
    _volumeTileMap.clear();
}

void
Volume::traverse( osg::NodeVisitor& nv )
{
    Group::traverse( nv );
}

VolumeTile*
Volume::getVolumeTile( const TileID& tileID )
{
    std::lock_guard<std::mutex> lock( _mutex );

    VolumeTileMap::iterator     itr = _volumeTileMap.find( tileID );
    return ( itr != _volumeTileMap.end() ) ? itr->second : 0;
}

const VolumeTile*
Volume::getVolumeTile( const TileID& tileID ) const
{
    std::lock_guard<std::mutex>   lock( _mutex );

    VolumeTileMap::const_iterator itr = _volumeTileMap.find( tileID );
    return ( itr != _volumeTileMap.end() ) ? itr->second : 0;
}

void
Volume::dirtyRegisteredVolumeTiles()
{
    std::lock_guard<std::mutex> lock( _mutex );

    for( VolumeTileSet::iterator itr = _volumeTileSet.begin();
         itr != _volumeTileSet.end();
         ++itr )
    {
        ( const_cast<VolumeTile*>( *itr ) )->setDirty( true );
    }
}

static unsigned int s_maxNumVolumeTiles = 0;

void
Volume::registerVolumeTile( VolumeTile* volumeTile )
{
    if( !volumeTile )
    {
        return;
    }

    std::lock_guard<std::mutex> lock( _mutex );

    if( volumeTile->getTileID().valid() )
    {
        _volumeTileMap[volumeTile->getTileID()] = volumeTile;
    }

    _volumeTileSet.insert( volumeTile );

    if( _volumeTileSet.size() > s_maxNumVolumeTiles )
    {
        s_maxNumVolumeTiles = static_cast<unsigned int>( _volumeTileSet.size() );
    }

    // OSG_NOTICE<<"Volume::registerVolumeTile "<<volumeTile<<" total number of
    // VolumeTile "<<_volumeTileSet.size()<<" max = "<<s_maxNumVolumeTiles<<std::endl;
}

void
Volume::unregisterVolumeTile( VolumeTile* volumeTile )
{
    if( !volumeTile )
    {
        return;
    }

    std::lock_guard<std::mutex> lock( _mutex );

    if( volumeTile->getTileID().valid() )
    {
        _volumeTileMap.erase( volumeTile->getTileID() );
    }

    _volumeTileSet.erase( volumeTile );

    // OSG_NOTICE<<"Volume::unregisterVolumeTile "<<volumeTile<<" total number of
    // VolumeTile "<<_volumeTileSet.size()<<" max = "<<s_maxNumVolumeTiles<<std::endl;
}
