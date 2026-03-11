/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract image processing interface. Provides texture compression,
 * mipmap generation, and format conversion services.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>

namespace osgDB
{

    class ImageProcessor : public osg::Inherit<osg::Object, ImageProcessor>
    {
        public:

            ImageProcessor() :
                Inherit( true )
            {
            }

            ImageProcessor( const ImageProcessor& rw,
                            const osg::CopyOp&    copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( rw,
                         copyop )
            {
            }

            virtual ~ImageProcessor()
            {
            }

            OSG_REGISTER_TYPE( osgDB,
                               ImageProcessor )

            enum CompressionMethod
            {
                USE_CPU,    /// Use CPU for compression even when GPU compression is
                            /// available
                USE_GPU,    /// Use GPU for compression when available (i.e CUDA),
                            /// otherwise fallback to CPU
            };

            enum CompressionQuality
            {
                FASTEST,
                NORMAL,
                PRODUCTION,
                HIGHEST,
            };

            virtual void
            compress( osg::Image& /*image*/,
                      osg::Texture::InternalFormatMode /*compressedFormat*/,
                      bool /*generateMipMap*/,
                      bool /*resizeToPowerOfTwo*/,
                      CompressionMethod /*method*/,
                      CompressionQuality /*quality*/ )
            {
            }

            virtual void
            generateMipMap( osg::Image& /*image*/,
                            bool /*resizeToPowerOfTwo*/,
                            CompressionMethod /*method*/ )
            {
            }
    };

}
