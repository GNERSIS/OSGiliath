/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Single volume tile holding a 3D image layer.
 * Generates rendering geometry from volumetric data.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/images/Image.hpp>
#include <osg/nodes/Group.hpp>
#include <osgDB/registry/ReaderWriter.hpp>
#include <osgVolume/Layer.hpp>
#include <osgVolume/VolumeTechnique.hpp>

namespace osgVolume
{

    class Volume;

    class OSGVOLUME_EXPORT TileID
    {
        public:

            TileID();

            TileID( int in_level,
                    int in_x,
                    int in_y,
                    int in_z );

            bool
            operator==( const TileID& rhs ) const
            {
                return ( level == rhs.level ) &&
                       ( x == rhs.x ) &&
                       ( y == rhs.y ) &&
                       ( z == rhs.z );
            }

            bool
            operator!=( const TileID& rhs ) const
            {
                return ( level != rhs.level ) ||
                       ( x != rhs.x ) ||
                       ( y != rhs.y ) ||
                       ( z != rhs.z );
            }

            bool
            operator<( const TileID& rhs ) const
            {
                if( level < rhs.level )
                {
                    return true;
                }
                if( level > rhs.level )
                {
                    return false;
                }
                if( x < rhs.x )
                {
                    return true;
                }
                if( x > rhs.x )
                {
                    return false;
                }
                if( y < rhs.y )
                {
                    return true;
                }
                if( y > rhs.y )
                {
                    return false;
                }
                return z < rhs.z;
            }

            bool
            valid() const
            {
                return level >= 0;
            }

            int level;
            int x;
            int y;
            int z;
    };

    /** VolumeTile provides a framework for loosely coupling 3d image data with rendering
     * algorithms. This allows TerrainTechnique's to be plugged in at runtime.*/
    class OSGVOLUME_EXPORT VolumeTile : public osg::Inherit<osg::Group, VolumeTile>
    {
        public:

            VolumeTile();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            VolumeTile( const VolumeTile&,
                        const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgVolume,
                               VolumeTile )

            virtual void
            traverse( osg::NodeVisitor& nv );

            /** Call init on any attached TerrainTechnique.*/
            void
            init();

            /** Set the Volume that this Volume tile is a member of.*/
            void
            setVolume( Volume* ts );

            /** Get the Volume that this Volume tile is a member of.*/
            Volume*
            getVolume()
            {
                return _volume;
            }

            /** Get the const Volume that this Volume tile is a member of.*/
            const Volume*
            getVolume() const
            {
                return _volume;
            }

            /** Set the TileID (layer, x,y,z) of the VolumeTile.
             * The TileID is used so it can be located by its neighbours
             * via the enclosing Volume node that manages a map of TileID to
             * VolumeTiles.*/
            void
            setTileID( const TileID& tileID );

            /** Get the TileID (layer, x,y,z) of the VolumeTile.*/
            const TileID&
            getTileID() const
            {
                return _tileID;
            }

            void
            setLocator( Locator* locator )
            {
                _locator = locator;
            }

            Locator*
            getLocator()
            {
                return _locator.get();
            }

            const Locator*
            getLocator() const
            {
                return _locator.get();
            }

            void
            setLayer( Layer* layer );

            Layer*
            getLayer()
            {
                return _layer.get();
            }

            const Layer*
            getLayer() const
            {
                return _layer.get();
            }

            /** Set the VolumeTechnique that will be used to render this tile. */
            void
            setVolumeTechnique( VolumeTechnique* VolumeTechnique );

            /** Get the VolumeTechnique that will be used to render this tile. */
            VolumeTechnique*
            getVolumeTechnique()
            {
                return _volumeTechnique.get();
            }

            /** Get the const VolumeTechnique that will be used to render this tile. */
            const VolumeTechnique*
            getVolumeTechnique() const
            {
                return _volumeTechnique.get();
            }

            /** Set the dirty flag on/off.*/
            void
            setDirty( bool dirty );

            /** return true if the tile is dirty and needs to be updated,*/
            bool
            getDirty() const
            {
                return _dirty;
            }

            virtual osg::sphere
            computeBound() const;

        protected:

            virtual ~VolumeTile();

            friend class Volume;

            Volume*                       _volume;

            bool                          _dirty;
            bool                          _hasBeenTraversal;

            TileID                        _tileID;

            osg::ref_ptr<VolumeTechnique> _volumeTechnique;

            osg::ref_ptr<Locator>         _locator;

            osg::ref_ptr<Layer>           _layer;
    };

}
