/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Transfer function mapping scalar values to RGBA colors.
 * Used in volume rendering and scientific visualization.
 */
#pragma once

#include <map>
#include <osg/core/Inherit.hpp>
#include <osg/state/Shader.hpp>
#include <osg/textures/Texture.hpp>

namespace osg
{

    /** TransferFunction is a class that provide a 1D,2D or 3D colour look up table
     * that can be used on the GPU as a 1D, 2D or 3D texture.
     * Typically uses include mapping heights to colours when contouring terrain,
     * or mapping intensities to colours when volume rendering.
     */
    class OSG_EXPORT TransferFunction
        : public osg::Inherit<osg::Object, TransferFunction>
    {
        public:

            TransferFunction();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            TransferFunction( const TransferFunction& tf,
                              const CopyOp&           copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               TransferFunction )

            /** Get the image that is used for passing the transfer function data to the
             * GPU.*/
            osg::Image*
            getImage()
            {
                return _image.get();
            }

            /** Get the const image that is used for passing the transfer function data
             * to the GPU.*/
            const osg::Image*
            getImage() const
            {
                return _image.get();
            }

        protected:

            virtual ~TransferFunction();

            osg::ref_ptr<osg::Image> _image;
    };

    /** 1D variant of TransferFunction. */
    class OSG_EXPORT TransferFunction1D
        : public osg::Inherit<osg::TransferFunction, TransferFunction1D>
    {
        public:

            TransferFunction1D();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            TransferFunction1D( const TransferFunction1D& tf,
                                const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               TransferFunction1D )

            /** Get the minimum transfer function value.*/
            float
            getMinimum() const
            {
                return _colorMap.empty() ? 0.0F : _colorMap.begin()->first;
            }

            /** Get the maximum transfer function value.*/
            float
            getMaximum() const
            {
                return _colorMap.empty() ? 0.0F : _colorMap.rbegin()->first;
            }

            /** allocate the osg::Image with specified dimension.  The Image tracks the
             * color map, and is used to represent the transfer function when download to
             * GPU.*/
            void
            allocate( unsigned int numImageCells );

            /** Clear the whole range to just represent a single color.*/
            void
            clear( const osg::vec4& color = osg::vec4( 1.0F,
                                                       1.0F,
                                                       1.0F,
                                                       1.0F ) );

            /** Get pixel value from the image. */
            osg::vec4
            getPixelValue( unsigned int i ) const
            {
                if( _image.valid() && i < static_cast<unsigned int>( _image->s() ) )
                {
                    return *reinterpret_cast<osg::vec4*>( _image->data( i ) );
                }
                else
                {
                    return osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F );
                }
            }

            /** Get the number of image cells that are assigned to the represent the
             * transfer function when download to the GPU.*/
            unsigned int
            getNumberImageCells() const
            {
                return _image.valid() ? static_cast<unsigned int>( _image->s() ) : 0U;
            }

            /** Set the color for a specified transfer function value.
             * updateImage defaults to true, and tells the setColor function to update
             * the associate osg::Image that tracks the color map.  Pass in false as the
             * updateImage parameter if you are setting up many values at once to avoid
             * recomputation of the image data, then once all setColor calls are made
             * explicitly call updateImage() to bring the osg::Image back into sync with
             * the color map. */
            void
            setColor( float            v,
                      const osg::vec4& color,
                      bool             updateImage = true );

            /** Get the color for a specified transfer function value, interpolating the
             * value if no exact match is found.*/
            osg::vec4
                                               getColor( float v ) const;

            typedef std::map<float, osg::vec4> ColorMap;

            /** set the color map and automatically update the image to make sure they
             * are in sync.*/
            void
            setColorMap( const ColorMap& vcm )
            {
                assign( vcm );
            }

            /** Get the color map that stores the mapping between the transfer function
             * value and the colour it maps to.*/
            ColorMap&
            getColorMap()
            {
                return _colorMap;
            }

            /** Get the const color map that stores the mapping between the transfer
             * function value and the colour it maps to.*/
            const ColorMap&
            getColorMap() const
            {
                return _colorMap;
            }

            /** Assign a color map and automatically update the image to make sure they
             * are in sync.*/
            void
            assign( const ColorMap& vcm );

            /** Manually update the associate osg::Image to represent the colors assigned
             * in the color map.*/
            void
            updateImage();

        protected:

            ColorMap _colorMap;

            void
            assignToImage( float            lower_v,
                           const osg::vec4& lower_c,
                           float            upper_v,
                           const osg::vec4& upper_c );
    };

}
