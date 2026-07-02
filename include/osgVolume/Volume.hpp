/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Root node for volume rendering. Manages VolumeTiles
 * and coordinates volume data display.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osg/nodes/Group.hpp>
#include <osgVolume/VolumeTile.hpp>

namespace osgVolume
{

    /** Volume provides a framework for loosely coupling 3d image VolumeTile's with
     * volume algorithms. This allows VolumeTechnique's to be plugged in at runtime.*/
    class OSGVOLUME_EXPORT Volume : public osg::Inherit<osg::Group, Volume>
    {
        public:

            Volume();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            Volume( const Volume&,
                    const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgVolume,
                               Volume )

            virtual void
            traverse( osg::NodeVisitor& nv );

            /** Get the VolumeTile for a given VolumeTileID.*/
            VolumeTile*
            getVolumeTile( const TileID& tileID );

            /** Get the const VolumeTile for a given VolumeTileID.*/
            const VolumeTile*
            getVolumeTile( const TileID& tileID ) const;

            /** Set the VolumeTechnique prototype that nested VolumeTile should clone if
             * they haven't already been assigned a volume rendering technique. */
            void
            setVolumeTechniquePrototype( VolumeTechnique* volumeTechnique )
            {
                _volumeTechnique = volumeTechnique;
            }

            /** Get the VolumeTechnique prototype. */
            VolumeTechnique*
            getVolumeTechniquePrototype()
            {
                return _volumeTechnique.get();
            }

            /** Get the const VolumeTechnique prototype. */
            const VolumeTechnique*
            getVolumeTechniquePrototype() const
            {
                return _volumeTechnique.get();
            }

        protected:

            virtual ~Volume();

            friend class VolumeTile;

            void
            dirtyRegisteredVolumeTiles();

            void
            registerVolumeTile( VolumeTile* tile );
            void
            unregisterVolumeTile( VolumeTile* tile );

            typedef std::map<TileID, VolumeTile*> VolumeTileMap;
            typedef std::set<VolumeTile*>         VolumeTileSet;

            mutable std::mutex                    _mutex;
            VolumeTileSet                         _volumeTileSet;
            VolumeTileMap                         _volumeTileMap;

            osg::ref_ptr<VolumeTechnique>         _volumeTechnique;
    };

}
