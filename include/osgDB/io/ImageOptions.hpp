/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Extended image loading options. Configures sub-region loading,
 * scaling, and format conversion for image plugins.
 */
#pragma once

#include <osgDB/registry/Options.hpp>

namespace osgDB
{

    class OSGDB_EXPORT ImageOptions : public osg::Inherit<osgDB::Options, ImageOptions>
    {
        public:

            ImageOptions();

            ImageOptions( const std::string& str );

            ImageOptions( const ImageOptions& options,
                          const osg::CopyOp&  copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( options,
                         copyop ),
                _sourceImageSamplingMode( options._sourceImageSamplingMode ),
                _sourceImageWindowMode( options._sourceImageWindowMode ),
                _sourceRatioWindow( options._sourceRatioWindow ),
                _sourcePixelWindow( options._sourcePixelWindow ),
                _destinationImage( options._destinationImage ),
                _destinationImageWindowMode( options._destinationImageWindowMode ),
                _destinationRatioWindow( options._destinationRatioWindow ),
                _destinationPixelWindow( options._destinationPixelWindow ),
                _destinationDataType( options._destinationDataType ),
                _destinationPixelFormat( options._destinationPixelFormat )
            {
            }

            OSG_REGISTER_TYPE( osgDB,
                               ImageOptions )

            /** RatioWindow stores the window (as ratios of 0.0 to 1.0) from the overall
             * imagery from which to extract the osg::Image*/
            struct RatioWindow
            {
                    RatioWindow() :
                        windowX( 0.0 ),
                        windowY( 0.0 ),
                        windowWidth( 1.0 ),
                        windowHeight( 1.0 )
                    {
                    }

                    void
                    set( double x,
                         double y,
                         double w,
                         double h )
                    {
                        windowX      = x;
                        windowY      = y;
                        windowWidth  = w;
                        windowHeight = h;
                    }

                    double windowX;
                    double windowY;
                    double windowWidth;
                    double windowHeight;
            };

            /** PixelWindow stores the window (in exact pixels) from the overall imagery
             * from which to extract the osg::Image*/
            struct PixelWindow
            {
                    PixelWindow() :
                        windowX( 0 ),
                        windowY( 0 ),
                        windowWidth( 0 ),
                        windowHeight( 0 )
                    {
                    }

                    void
                    set( unsigned int x,
                         unsigned int y,
                         unsigned int w,
                         unsigned int h )
                    {
                        windowX      = x;
                        windowY      = y;
                        windowWidth  = w;
                        windowHeight = h;
                    }

                    unsigned int windowX;
                    unsigned int windowY;
                    unsigned int windowWidth;
                    unsigned int windowHeight;
            };

            enum ImageWindowMode
            {
                ALL_IMAGE,
                RATIO_WINDOW,
                PIXEL_WINDOW,
            };

            enum ImageSamplingMode
            {
                NEAREST,
                LINEAR,
                CUBIC,
            };

            /** Used as UserData attached to generated osg::Image's*/
            struct TexCoordRange : public osg::Referenced
            {
                    TexCoordRange() :
                        _x( 0.0 ),
                        _y( 0.0 ),
                        _w( 1.0 ),
                        _h( 1.0 )
                    {
                    }

                    void
                    set( double x,
                         double y,
                         double w,
                         double h )
                    {
                        _x = x;
                        _y = y;
                        _w = w;
                        _h = h;
                    }

                    double _x, _y, _w, _h;

                protected:

                    virtual ~TexCoordRange()
                    {
                    }
            };

            // source
            ImageSamplingMode        _sourceImageSamplingMode;
            ImageWindowMode          _sourceImageWindowMode;
            RatioWindow              _sourceRatioWindow;
            PixelWindow              _sourcePixelWindow;

            // destination
            osg::ref_ptr<osg::Image> _destinationImage;

            ImageWindowMode          _destinationImageWindowMode;
            RatioWindow              _destinationRatioWindow;
            PixelWindow              _destinationPixelWindow;

            GLenum                   _destinationDataType;
            GLenum                   _destinationPixelFormat;

            void
            init();
    };

}
