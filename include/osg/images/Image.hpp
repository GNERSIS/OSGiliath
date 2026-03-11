/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * In-memory image container for texture data. Supports readPixels,
 * format conversion, sub-image loading, and plugin-based file I/O.
 */
#pragma once

#include <osg/core/FrameStamp.hpp>
#include <osg/geometry/BufferObject.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/state/StateAttribute.hpp>
#include <string>
#include <vector>

namespace osg
{

    // forward declare
    class NodeVisitor;

    /** Image class for encapsulating the storage texture image data. */
    class OSG_EXPORT Image : public BufferData
    {

        public:

            Image();

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Image( const Image&  image,
                   const CopyOp& copyop = CopyOp::SHALLOW_COPY );

            virtual Object*
            cloneType() const
            {
                return new Image();
            }

            virtual Object*
            clone( const CopyOp& copyop ) const
            {
                return new Image( *this, copyop );
            }

            virtual bool
            isSameKindAs( const Object* obj ) const
            {
                return dynamic_cast<const Image*>( obj ) != 0;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return "Image";
            }

            virtual osg::Image*
            asImage()
            {
                return this;
            }

            virtual const osg::Image*
            asImage() const
            {
                return this;
            }

            virtual const GLvoid*
            getDataPointer() const
            {
                return data();
            }

            virtual unsigned int
            getTotalDataSize() const
            {
                return getTotalSizeInBytesIncludingMipmaps();
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            virtual int
            compare( const Image& rhs ) const;

            void
            setFileName( const std::string& fileName );

            inline const std::string&
            getFileName() const
            {
                return _fileName;
            }

            enum WriteHint
            {
                NO_PREFERENCE,
                STORE_INLINE,
                EXTERNAL_FILE,
            };

            void
            setWriteHint( WriteHint writeHint )
            {
                _writeHint = writeHint;
            }

            WriteHint
            getWriteHint() const
            {
                return _writeHint;
            }

            enum AllocationMode
            {
                NO_DELETE,
                USE_NEW_DELETE,
                USE_MALLOC_FREE,
            };

            /** Set the method used for deleting data once it goes out of scope. */
            void
            setAllocationMode( AllocationMode mode )
            {
                _allocationMode = mode;
            }

            /** Get the method used for deleting data once it goes out of scope. */
            AllocationMode
            getAllocationMode() const
            {
                return _allocationMode;
            }

            /** Allocate a pixel block of specified size and type. */
            virtual void
            allocateImage( int    s,
                           int    t,
                           int    r,
                           GLenum pixelFormat,
                           GLenum type,
                           int    packing = 1 );

            /** Set the image dimensions, format and data. */
            virtual void
            setImage( int            s,
                      int            t,
                      int            r,
                      GLint          internalTextureformat,
                      GLenum         pixelFormat,
                      GLenum         type,
                      unsigned char* data,
                      AllocationMode mode,
                      int            packing   = 1,
                      int            rowLength = 0 );

            /** Read pixels from current frame buffer at specified position and size,
             * using glReadPixels. Create memory for storage if required, reuse existing
             * pixel coords if possible.
             */
            virtual void
            readPixels( int    x,
                        int    y,
                        int    width,
                        int    height,
                        GLenum pixelFormat,
                        GLenum type,
                        int    packing = 1 );

            /** Read the contents of the current bound texture, handling compressed
             * pixelFormats if present. Create memory for storage if required, reuse
             * existing pixel coords if possible.
             */
            virtual void
            readImageFromCurrentTexture( unsigned int contextID,
                                         bool         copyMipMapsIfAvailable,
                                         GLenum       type = GL_UNSIGNED_BYTE,
                                         unsigned int face = 0 );

            /** swap the data and settings between two image objects.*/
            void
            swap( osg::Image& rhs );

            /** Scale image to specified size. */
            void
            scaleImage( int s,
                        int t,
                        int r )
            {
                scaleImage( s, t, r, getDataType() );
            }

            /** Scale image to specified size and with specified data type. */
            virtual void
            scaleImage( int    s,
                        int    t,
                        int    r,
                        GLenum newDataType );

            /** Copy a source Image into a subpart of this Image at specified position.
             * Typically used to copy to an already allocated image, such as creating
             * a 3D image from a stack 2D images.
             * If this Image is empty then image data is created to
             * accommodate the source image in its offset position.
             * If source is NULL then no operation happens, this Image is left unchanged.
             */
            virtual void
            copySubImage( int               s_offset,
                          int               t_offset,
                          int               r_offset,
                          const osg::Image* source );

            enum Origin
            {
                BOTTOM_LEFT,
                TOP_LEFT,
            };

            /** Set the origin of the image.
             * The default value is BOTTOM_LEFT and is consistent with OpenGL.
             * TOP_LEFT is used for imagery that follows standard Imagery convention,
             * such as movies, and hasn't been flipped yet.  For such images one much
             * flip the t axis of the tex coords. to handle this origin position. */
            void
            setOrigin( Origin origin )
            {
                _origin = origin;
            }

            /** Get the origin of the image.*/
            Origin
            getOrigin() const
            {
                return _origin;
            }

            /** Width of image. */
            inline int
            s() const
            {
                return _s;
            }

            /** Height of image. */
            inline int
            t() const
            {
                return _t;
            }

            /** Depth of image. */
            inline int
            r() const
            {
                return _r;
            }

            void
            setRowLength( int length );

            inline int
            getRowLength() const
            {
                return _rowLength;
            }

            void
            setInternalTextureFormat( GLint internalFormat );

            inline GLint
            getInternalTextureFormat() const
            {
                return _internalTextureFormat;
            }

            void
            setPixelFormat( GLenum pixelFormat );

            inline GLenum
            getPixelFormat() const
            {
                return _pixelFormat;
            }

            void
            setDataType( GLenum dataType );

            inline GLenum
            getDataType() const
            {
                return _dataType;
            }

            void
            setPacking( unsigned int packing )
            {
                _packing = packing;
            }

            inline unsigned int
            getPacking() const
            {
                return _packing;
            }

            /** Return true of the pixel format is an OpenGL compressed pixel format.*/
            bool
            isCompressed() const;

            /** Set the pixel aspect ratio, defined as the pixel width divided by the
             * pixel height.*/
            inline void
            setPixelAspectRatio( float pixelAspectRatio )
            {
                _pixelAspectRatio = pixelAspectRatio;
            }

            /** Get the pixel aspect ratio.*/
            inline float
            getPixelAspectRatio() const
            {
                return _pixelAspectRatio;
            }

            /** Return the number of bits required for each pixel. */
            inline unsigned int
            getPixelSizeInBits() const
            {
                return computePixelSizeInBits( _pixelFormat, _dataType );
            }

            /** Return the number of bytes each row of pixels occupies once it has been
             * packed. */
            inline unsigned int
            getRowSizeInBytes() const
            {
                return computeRowWidthInBytes( _s,
                                               _pixelFormat,
                                               _dataType,
                                               static_cast<int>( _packing ) );
            }

            /** Return the number of bytes between each successive row.
             * Note, getRowSizeInBytes() will only equal getRowStepInBytes() when
             * isDataContiguous() return true. */
            inline unsigned int
            getRowStepInBytes() const
            {
                return computeRowWidthInBytes( _rowLength == 0 ? _s : _rowLength,
                                               _pixelFormat,
                                               _dataType,
                                               static_cast<int>( _packing ) );
            }

            /** Return the number of bytes each image (_s*_t) of pixels occupies. */
            inline unsigned int
            getImageSizeInBytes() const
            {
                return getRowSizeInBytes() * static_cast<unsigned int>( _t );
            }

            /** Return the number of bytes between each successive image.
             * Note, getImageSizeInBytes() will only equal getImageStepInBytes() when
             * isDataContiguous() return true. */
            inline unsigned int
            getImageStepInBytes() const
            {
                return getRowStepInBytes() * static_cast<unsigned int>( _t );
            }

            /** Return the number of bytes the whole row/image/volume of pixels occupies.
             */
            inline unsigned int
            getTotalSizeInBytes() const
            {
                return getImageSizeInBytes() * static_cast<unsigned int>( _r );
            }

            /** Return the number of bytes the whole row/image/volume of pixels occupies,
             * including all mip maps if included. */
            unsigned int
            getTotalSizeInBytesIncludingMipmaps() const;

            /** Return true if the Image represent a valid and usable imagery.*/
            bool
            valid() const
            {
                return _s != 0 && _t != 0 && _r != 0 && _data != 0 && _dataType != 0;
            }

            /** Raw image data.
             * Note, data in successive rows may not be contiguous, isDataContiguous()
             * return false then you should take care to access the data per row rather
             * than treating the whole data as a single block. */
            inline unsigned char*
            data()
            {
                return _data;
            }

            /** Raw const image data.
             * Note, data in successive rows may not be contiguous, isDataContiguous()
             * return false then you should take care to access the data per row rather
             * than treating the whole data as a single block. */
            inline const unsigned char*
            data() const
            {
                return _data;
            }

            inline unsigned char*
            data( unsigned int column,
                  unsigned int row   = 0,
                  unsigned int image = 0 )
            {
                if( !_data )
                {
                    return NULL;
                }
                return _data +
                       ( column * getPixelSizeInBits() ) /
                       8 +
                       row *
                       getRowStepInBytes() +
                       image *
                       getImageSizeInBytes();
            }

            inline const unsigned char*
            data( unsigned int column,
                  unsigned int row   = 0,
                  unsigned int image = 0 ) const
            {
                if( !_data )
                {
                    return NULL;
                }
                return _data +
                       ( column * getPixelSizeInBits() ) /
                       8 +
                       row *
                       getRowStepInBytes() +
                       image *
                       getImageSizeInBytes();
            }

            /** return true if the data stored in the image is a contiguous block of
             * data.*/
            bool
            isDataContiguous() const
            {
                return _rowLength == 0 || _rowLength == _s;
            }

            /** Convenience class for assisting the copying of image data when the image
             * data isn't contiguous.*/
            class OSG_EXPORT DataIterator
            {
                public:

                    DataIterator( const Image* image );
                    DataIterator( const DataIterator& ri );

                    ~DataIterator()
                    {
                    }

                    /** advance iterator to next block of data.*/
                    void
                    operator++();

                    /** is iterator valid.*/
                    bool
                    valid() const
                    {
                        return _currentPtr != 0;
                    }

                    /** data pointer of current block to copy.*/
                    const unsigned char*
                    data() const
                    {
                        return _currentPtr;
                    }

                    /** Size of current block to copy.*/
                    unsigned int
                    size() const
                    {
                        return _currentSize;
                    }

                protected:

                    void
                                         assign();

                    const osg::Image*    _image;
                    int                  _rowNum;
                    int                  _imageNum;
                    unsigned int         _mipmapNum;
                    const unsigned char* _currentPtr;
                    unsigned int         _currentSize;
            };

            /** Get the color value for specified texcoord.*/
            vec4
            getColor( unsigned int s,
                      unsigned     t = 0,
                      unsigned     r = 0 ) const;

            /** Get the color value for specified texcoord.*/
            vec4
            getColor( const vec2& texcoord ) const
            {
                return getColor( vec3( texcoord.x, texcoord.y, 0.0F ) );
            }

            /** Get the color value for specified texcoord.*/
            vec4
            getColor( const vec3& texcoord ) const;

            /** Set the color value for specified texcoord.*/
            void
            setColor( const osg::vec4& color,
                      unsigned int     s,
                      unsigned int     t = 0,
                      unsigned int     r = 0 );

            /** Set the color value for specified texcoord. Note texcoord is clamped to
             * edge.*/
            void
            setColor( const osg::vec4& color,
                      const osg::vec2& texcoord )
            {
                setColor( color, osg::vec3( texcoord, 0.0F ) );
            }

            /** Set the color value for specified texcoord. Note texcoord is clamped to
             * edge.*/
            void
            setColor( const osg::vec4& color,
                      const osg::vec3& texcoord );

            /** Flip the image horizontally, around s dimension. */
            void
            flipHorizontal();

            /** Flip the image vertically, around t dimension. */
            void
            flipVertical();

            /** Flip the image around the r dimension. Only relevant for 3D textures. */
            void
            flipDepth();

            /** Ensure image dimensions are a power of two.
             * Mipmapped textures require the image dimensions to be
             * power of two and are within the maximum texture size for
             * the host machine.
             */
            void
            ensureValidSizeForTexturing( GLint maxTextureSize );

            static bool
            isPackedType( GLenum type );
            static GLenum
            computePixelFormat( GLenum pixelFormat );
            static GLenum
            computeFormatDataType( GLenum pixelFormat );

            /** return the dimensions of a block of compressed pixels */
            static osg::ivec3
            computeBlockFootprint( GLenum pixelFormat );

            /** return the size in bytes of a block of compressed pixels */
            static unsigned int
            computeBlockSize( GLenum pixelFormat,
                              GLenum packing );
            static unsigned int
            computeNumComponents( GLenum pixelFormat );
            static unsigned int
            computePixelSizeInBits( GLenum pixelFormat,
                                    GLenum type );
            static unsigned int
            computeRowWidthInBytes( int    width,
                                    GLenum pixelFormat,
                                    GLenum type,
                                    int    packing );
            static unsigned int
            computeImageSizeInBytes( int    width,
                                     int    height,
                                     int    depth,
                                     GLenum pixelFormat,
                                     GLenum type,
                                     int    packing       = 1,
                                     int    slice_packing = 1,
                                     int    image_packing = 1 );
            static int
            roudUpToMultiple( int s,
                              int pack );
            static int
            computeNearestPowerOfTwo( int   s,
                                      float bias = 0.5F );
            static int
                                              computeNumberOfMipmapLevels( int s,
                                                                           int t = 1,
                                                                           int r = 1 );

            /** Precomputed mipmaps stuff. */
            typedef std::vector<unsigned int> MipmapDataType;

            inline bool
            isMipmap() const
            {
                return !_mipmapData.empty();
            };

            unsigned int
            getNumMipmapLevels() const
            {
                return static_cast<unsigned int>( _mipmapData.size() ) + 1;
            };

            /** Send offsets into data. It is assumed that first mipmap offset (index 0)
             * is 0.*/
            inline void
            setMipmapLevels( const MipmapDataType& mipmapDataVector )
            {
                _mipmapData = mipmapDataVector;
            }

            inline const MipmapDataType&
            getMipmapLevels() const
            {
                return _mipmapData;
            }

            inline unsigned int
            getMipmapOffset( unsigned int mipmapLevel ) const
            {
                if( mipmapLevel == 0 )
                {
                    return 0;
                }
                else if( mipmapLevel < getNumMipmapLevels() )
                {
                    return _mipmapData[mipmapLevel - 1];
                }
                return 0;
            };

            inline unsigned char*
            getMipmapData( unsigned int mipmapLevel )
            {
                return _data + getMipmapOffset( mipmapLevel );
            }

            inline const unsigned char*
            getMipmapData( unsigned int mipmapLevel ) const
            {
                return _data + getMipmapOffset( mipmapLevel );
            }

            /** returns false for texture formats that do not support texture subloading
             */
            bool
            supportsTextureSubloading() const;

            /** Return true if this image is translucent - i.e. it has alpha values that
             * are less 1.0 (when normalized). */
            virtual bool
            isImageTranslucent() const;

            /** Set the optional PixelBufferObject used to map the image memory
             * efficiently to graphics memory. */
            void
            setPixelBufferObject( PixelBufferObject* buffer )
            {
                setBufferObject( buffer );
            }

            /** Get the PixelBufferObject.*/
            PixelBufferObject*
            getPixelBufferObject()
            {
                return dynamic_cast<PixelBufferObject*>( getBufferObject() );
            }

            /** Get the const PixelBufferObject.*/
            const PixelBufferObject*
            getPixelBufferObject() const
            {
                return dynamic_cast<const PixelBufferObject*>( getBufferObject() );
            }

            /** Return whether the update(NodeVisitor* nv) should be required on each
             * frame to enable proper working of osg::Image.*/
            virtual bool
            requiresUpdateCall() const
            {
                return false;
            }

            /** update method for osg::Image subclasses that update themselves during the
             * update traversal.*/
            virtual void
            update( NodeVisitor* /*nv*/ )
            {
            }

            /** Convenience update callback class that can be attached to a
             * StateAttribute (such as Textures) to ensure that the
             * Image::update(NodeVisitor*) method is called during the update traversal.
             * This callback is automatically attached when Image::requiresUpdateCall()
             * is true (it's false by default.)
             */
            struct OSG_EXPORT UpdateCallback : public osg::StateAttributeCallback
            {
                    virtual void
                    operator()( osg::StateAttribute* attr,
                                osg::NodeVisitor*    nv );
            };

            /** Hint whether to enable or disable focus to images acting as front ends to
             * interactive surfaces such as a vnc or browser window.  Return true if
             * handled. */
            virtual bool
            sendFocusHint( bool /*focus*/ )
            {
                return false;
            }

            /** Send pointer events to images that are acting as front ends to
             * interactive surfaces such as a vnc or browser window.  Return true if
             * handled. */
            virtual bool
            sendPointerEvent( int /*x*/,
                              int /*y*/,
                              int /*buttonMask*/ )
            {
                return false;
            }

            /** Send key events to images that are acting as front ends to interactive
             * surfaces such as a vnc or browser window.  Return true if handled.*/
            virtual bool
            sendKeyEvent( int /*key*/,
                          bool /*keyDown*/ )
            {
                return false;
            }

            /** Pass frame information to the custom Image classes, to be called only
             * when objects associated with imagery are not culled.*/
            virtual void
            setFrameLastRendered( const osg::FrameStamp* /*frameStamp*/ )
            {
            }

            class DimensionsChangedCallback : public osg::Referenced
            {
                public:

                    DimensionsChangedCallback() :
                        osg::Referenced()
                    {
                    }

                    virtual void
                    operator()( osg::Image* image ) = 0;
            };

            typedef std::vector<osg::ref_ptr<DimensionsChangedCallback>>
                DimensionsChangedCallbackVector;

            void
            addDimensionsChangedCallback( DimensionsChangedCallback* cb );
            void
            removeDimensionsChangedCallback( DimensionsChangedCallback* cb );

        protected:

            virtual ~Image();

            Image&
            operator=( const Image& )
            {
                return *this;
            }

            void
            handleDimensionsChangedCallbacks()
            {
                for( DimensionsChangedCallbackVector::iterator i =
                         _dimensionsChangedCallbacks.begin();
                     i != _dimensionsChangedCallbacks.end();
                     ++i )
                {
                    ( *i )->operator()( this );
                }
            }

            std::string    _fileName;
            WriteHint      _writeHint;

            Origin         _origin;

            int            _s, _t, _r;
            int            _rowLength;
            GLint          _internalTextureFormat;
            GLenum         _pixelFormat;
            GLenum         _dataType;
            unsigned int   _packing;
            float          _pixelAspectRatio;

            AllocationMode _allocationMode;
            unsigned char* _data;

            void
            deallocateData();

            void
                                            setData( unsigned char* data,
                                                     AllocationMode allocationMode );

            MipmapDataType                  _mipmapData;

            DimensionsChangedCallbackVector _dimensionsChangedCallbacks;
    };

    class Geode;

    /** Convenience function to be used by image loaders to generate a valid geode
     * to return for readNode().
     * Use the image's s and t values to scale the dimensions of the image.
     */
    extern OSG_EXPORT Geode*
    createGeodeForImage( Image* image );

    template<class T>
    Geode*
    createGeodeForImage( const ref_ptr<T>& image )
    {
        return createGeodeForImage( image.get() );
    }

    /** Convenience function to be used by image loaders to generate a valid geode
     * to return for readNode().
     * Use the specified s and t values to scale the dimensions of the image.
     */
    extern OSG_EXPORT Geode*
    createGeodeForImage( Image* image,
                         float  s,
                         float  t );

    template<class T>
    Geode*
    createGeodeForImage( const ref_ptr<T>& image,
                         float             s,
                         float             t )
    {
        return createGeodeForImage( image.get(), s, t );
    }

}
