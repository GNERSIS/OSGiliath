/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Scene-level volume rendering manager. Coordinates multiple
 * VolumeTiles with proper back-to-front ordering.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgUtil/culling/CullVisitor.hpp>
#include <osgVolume/VolumeTile.hpp>

namespace osgVolume
{

    /** VolumeScene provides high level support for doing multi-pass rendering of volumes
     * where the main scene to rendered to color and depth textures and then re-rendered
     * for the purposes of volume rendering.*/
    class OSGVOLUME_EXPORT VolumeScene : public osg::Inherit<osg::Group, VolumeScene>
    {
        public:

            VolumeScene();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            VolumeScene( const VolumeScene&,
                         const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgVolume,
                               VolumeScene )

            virtual void
            traverse( osg::NodeVisitor& nv );

            TileData*
            tileVisited( osgUtil::CullVisitor* cv,
                         VolumeTile*           tile );
            TileData*
            getTileData( osgUtil::CullVisitor* cv,
                         VolumeTile*           tile );

        protected:

            virtual ~VolumeScene();

            typedef std::map<VolumeTile*, osg::ref_ptr<TileData>> Tiles;

            class ViewData : public osg::Referenced
            {
                public:

                    ViewData();

                    void
                    clearTiles();
                    void
                                                 visitTile( VolumeTile* tile );

                    osg::ref_ptr<osg::Texture2D> _depthTexture;
                    osg::ref_ptr<osg::Texture2D> _colorTexture;
                    osg::ref_ptr<osg::Camera>    _rttCamera;
                    osg::ref_ptr<osg::Node>      _backdropSubgraph;
                    osg::ref_ptr<osg::Geometry>  _geometry;
                    osg::ref_ptr<osg::Vec3Array> _vertices;
                    osg::ref_ptr<osg::StateSet>  _stateset;
                    osg::ref_ptr<osg::Uniform>   _viewportDimensionsUniform;

                    Tiles                        _tiles;

                protected:

                    virtual ~ViewData()
                    {
                    }
            };

            typedef std::map<osgUtil::CullVisitor*, osg::ref_ptr<ViewData>> ViewDataMap;
            std::mutex  _viewDataMapMutex;
            ViewDataMap _viewDataMap;
    };

}
