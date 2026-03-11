/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Legacy glDrawPixels wrapper. Retained for API compat but
 * non-functional in Core Profile (use textured quads instead).
 */
#pragma once

#include <osg/geometry/Drawable.hpp>
#include <osg/images/Image.hpp>
#include <osg/maths/vec3.hpp>

namespace osg
{

    /** DrawPixels is an osg::Drawable subclass which encapsulates the drawing of
     * images using glDrawPixels.*/
    class OSG_EXPORT DrawPixels : public Drawable
    {
        public:

            DrawPixels();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            DrawPixels( const DrawPixels& drawimage,
                        const CopyOp&     copyop = CopyOp::SHALLOW_COPY );

            virtual Object*
            cloneType() const
            {
                return new DrawPixels();
            }

            virtual Object*
            clone( const CopyOp& copyop ) const
            {
                return new DrawPixels( *this, copyop );
            }

            virtual bool
            isSameKindAs( const Object* obj ) const
            {
                return dynamic_cast<const DrawPixels*>( obj ) != NULL;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return "DrawPixels";
            }

            void
            setPosition( const osg::vec3& position );

            osg::vec3&
            getPosition()
            {
                return _position;
            }

            const osg::vec3&
            getPosition() const
            {
                return _position;
            }

            void
            setImage( osg::Image* image )
            {
                _image = image;
            }

            osg::Image*
            getImage()
            {
                return _image.get();
            }

            const osg::Image*
            getImage() const
            {
                return _image.get();
            }

            void
            setUseSubImage( bool useSubImage )
            {
                _useSubImage = useSubImage;
            }

            bool
            getUseSubImage() const
            {
                return _useSubImage;
            }

            void
            setSubImageDimensions( unsigned int offsetX,
                                   unsigned int offsetY,
                                   unsigned int width,
                                   unsigned int height );
            void
            getSubImageDimensions( unsigned int& offsetX,
                                   unsigned int& offsetY,
                                   unsigned int& width,
                                   unsigned int& height ) const;

            virtual void
            drawImplementation( RenderInfo& renderInfo ) const;

            virtual box
            computeBoundingBox() const;

        protected:

            DrawPixels&
            operator=( const DrawPixels& )
            {
                return *this;
            }

            virtual ~DrawPixels();

            vec3           _position;
            ref_ptr<Image> _image;

            bool           _useSubImage;
            unsigned int   _offsetX, _offsetY, _width, _height;
    };

}
