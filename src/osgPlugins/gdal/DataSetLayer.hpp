/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * DataSetLayer, derived from osgTerrain.
 * Provides: OSG_REGISTER_TYPE, isOpen, open, close, extractImageLayer, setGdalReader.
 */
#pragma once

#include <gdal_priv.h>
#include <osg/core/Inherit.hpp>
#include <osgDB/registry/ReaderWriter.hpp>
#include <osgTerrain/Layer.hpp>

namespace GDALPlugin
{

    class DataSetLayer : public osg::Inherit<osgTerrain::Layer, DataSetLayer>
    {
        public:

            OSG_REGISTER_TYPE( GDALPlugin,
                               DataSetLayer )

            DataSetLayer();

            DataSetLayer( const std::string& fileName );

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            DataSetLayer( const DataSetLayer& dataSetLayer,
                          const osg::CopyOp&  copyop = osg::CopyOp::SHALLOW_COPY );

            virtual bool
            isOpen() const
            {
                return _dataset != 0;
            }

            virtual void
            open();

            virtual void
            close();

            virtual unsigned int
            getNumColumns() const;

            virtual unsigned int
            getNumRows() const;

            virtual osgTerrain::ImageLayer*
            extractImageLayer( unsigned int sourceMinX,
                               unsigned int sourceMinY,
                               unsigned int sourceMaxX,
                               unsigned int sourceMaxY,
                               unsigned int targetWidth  = 0,
                               unsigned int targetHeight = 0 );

            void
            setGdalReader( const osgDB::ReaderWriter* rw );

        protected:

            virtual ~DataSetLayer();

            void
                                 setUpLocator();

            GDALDataset*         _dataset;

            osgDB::ReaderWriter* _gdalReader;
    };

}
