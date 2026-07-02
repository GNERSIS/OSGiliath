/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Individual glyph bitmap and metrics from FreeType.
 * Stores texture sub-region, bearing, advance, and kerning data.
 */
#pragma once

#include <istream>
#include <mutex>
#include <osg/geometry/Geometry.hpp>
#include <osg/images/Image.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgText/Export.hpp>
#include <osgText/KerningType.hpp>
#include <osgText/Style.hpp>
#include <string>

namespace osgText
{

    class Font;
    class Text;
    class Glyph3D;
    class GlyphGeometry;
    class GlyphTexture;

    enum ShaderTechnique
    {
        NO_TEXT_SHADER        = 0X0,
        GREYSCALE             = 0X1,
        SIGNED_DISTANCE_FIELD = 0X2,
        ALL_FEATURES          = GREYSCALE | SIGNED_DISTANCE_FIELD
    };

    class OSGTEXT_EXPORT Glyph : public osg::Image
    {
        public:

            Glyph( Font*        font,
                   unsigned int glyphCode );

            Font*
            getFont()
            {
                return _font;
            }

            const Font*
            getFont() const
            {
                return _font;
            }

            unsigned int
            getGlyphCode() const
            {
                return _glyphCode;
            }

            void
            setFontResolution( const FontResolution& fontRes )
            {
                _fontResolution = fontRes;
            }

            const FontResolution&
            getFontResolution() const
            {
                return _fontResolution;
            }

            void
            setWidth( float width )
            {
                _width = width;
            }

            float
            getWidth() const
            {
                return _width;
            }

            void
            setHeight( float height )
            {
                _height = height;
            }

            float
            getHeight() const
            {
                return _height;
            }

            void
            setHorizontalBearing( const osg::vec2& bearing );
            const osg::vec2&
            getHorizontalBearing() const;

            void
            setHorizontalAdvance( float advance );
            float
            getHorizontalAdvance() const;

            void
            setVerticalBearing( const osg::vec2& bearing );
            const osg::vec2&
            getVerticalBearing() const;

            void
            setVerticalAdvance( float advance );
            float
            getVerticalAdvance() const;

            struct TextureInfo : public osg::Referenced
            {
                    TextureInfo() :
                        texture( 0 ),
                        texelMargin( 0.0F )
                    {
                    }

                    TextureInfo( GlyphTexture*    tex,
                                 int              x,
                                 int              y,
                                 const osg::vec2& mintc,
                                 const osg::vec2& maxtc,
                                 float            margin ) :
                        texture( tex ),
                        texturePositionX( x ),
                        texturePositionY( y ),
                        minTexCoord( mintc ),
                        maxTexCoord( maxtc ),
                        texelMargin( margin )
                    {
                    }

                    GlyphTexture* texture;
                    int           texturePositionX;
                    int           texturePositionY;
                    osg::vec2     minTexCoord;
                    osg::vec2     maxTexCoord;
                    float         texelMargin;
            };

            void
            setTextureInfo( ShaderTechnique technique,
                            TextureInfo*    info );

            const TextureInfo*
            getTextureInfo( ShaderTechnique technique ) const;

            TextureInfo*
            getOrCreateTextureInfo( ShaderTechnique technique );

        protected:

            virtual ~Glyph();

            Font*                                          _font;
            unsigned int                                   _glyphCode;

            FontResolution                                 _fontResolution;

            float                                          _width;
            float                                          _height;

            osg::vec2                                      _horizontalBearing;
            float                                          _horizontalAdvance;

            osg::vec2                                      _verticalBearing;
            float                                          _verticalAdvance;

            typedef std::vector<osg::ref_ptr<TextureInfo>> TextureInfoList;
            TextureInfoList                                _textureInfoList;

            mutable std::recursive_mutex                   _textureInfoListMutex;
    };

    class OSGTEXT_EXPORT GlyphGeometry : public osg::Referenced
    {
        public:

            GlyphGeometry();

            void
            setup( const Glyph3D* glyph,
                   const Style*   style );

            bool
            match( const Style* style ) const;

            osg::Geode*
            getGeode() const
            {
                return _geode.get();
            }

            osg::Geometry*
            getGeometry() const
            {
                return _geometry.get();
            }

            /** Set the VertexArray of the glyph. */
            void
            setVertexArray( osg::Vec3Array* va )
            {
                _vertices = va;
            }

            /** Get the VertexArray of the glyph. */
            osg::Vec3Array*
            getVertexArray() const
            {
                return _vertices.get();
            }

            /** Set the VertexArray of the glyph. */
            void
            setNormalArray( osg::Vec3Array* na )
            {
                _normals = na;
            }

            /** Get the NormalArray for the wall face. */
            osg::Vec3Array*
            getNormalArray() const
            {
                return _normals.get();
            }

            /** Get the PrimitiveSetList for the front face. */
            osg::Geometry::PrimitiveSetList&
            getFrontPrimitiveSetList()
            {
                return _frontPrimitiveSetList;
            }

            /** Get the PrimitiveSetList for the wall face. */
            osg::Geometry::PrimitiveSetList&
            getWallPrimitiveSetList()
            {
                return _wallPrimitiveSetList;
            }

            /** Get et the PrimitiveSetList for the back face. */
            osg::Geometry::PrimitiveSetList&
            getBackPrimitiveSetList()
            {
                return _backPrimitiveSetList;
            }

            /** Set whether to use a mutex to ensure ref() and unref() are thread safe.*/
            virtual void
            setThreadSafeRefUnref( bool threadSafe );

        protected:

            osg::ref_ptr<Style>             _style;
            osg::ref_ptr<osg::Geode>        _geode;
            osg::ref_ptr<osg::Geometry>     _geometry;
            osg::ref_ptr<osg::Vec3Array>    _vertices;
            osg::ref_ptr<osg::Vec3Array>    _normals;

            osg::Geometry::PrimitiveSetList _frontPrimitiveSetList;
            osg::Geometry::PrimitiveSetList _wallPrimitiveSetList;
            osg::Geometry::PrimitiveSetList _backPrimitiveSetList;
    };

    class OSGTEXT_EXPORT Glyph3D : public osg::Referenced
    {
        public:

            Glyph3D( Font*        font,
                     unsigned int glyphCode );

            Font*
            getFont()
            {
                return _font;
            }

            const Font*
            getFont() const
            {
                return _font;
            }

            unsigned int
            getGlyphCode() const
            {
                return _glyphCode;
            }

            void
            setWidth( float width )
            {
                _width = width;
            }

            float
            getWidth() const
            {
                return _width;
            }

            void
            setHeight( float height )
            {
                _height = height;
            }

            float
            getHeight() const
            {
                return _height;
            }

            void
            setHorizontalBearing( const osg::vec2& bearing )
            {
                _horizontalBearing = bearing;
            }

            const osg::vec2&
            getHorizontalBearing() const
            {
                return _horizontalBearing;
            }

            void
            setHorizontalAdvance( float advance )
            {
                _horizontalAdvance = advance;
            }

            float
            getHorizontalAdvance() const
            {
                return _horizontalAdvance;
            }

            void
            setVerticalBearing( const osg::vec2& bearing )
            {
                _verticalBearing = bearing;
            }

            const osg::vec2&
            getVerticalBearing() const
            {
                return _verticalBearing;
            }

            void
            setVerticalAdvance( float advance )
            {
                _verticalAdvance = advance;
            }

            float
            getVerticalAdvance() const
            {
                return _verticalAdvance;
            }

            void
            setBoundingBox( osg::box& bb )
            {
                _bb = bb;
            }

            const osg::box&
            getBoundingBox() const
            {
                return _bb;
            }

            /** Set whether to use a mutex to ensure ref() and unref() are thread safe.*/
            virtual void
            setThreadSafeRefUnref( bool threadSafe );

            void
            setRawVertexArray( osg::Vec3Array* vertices )
            {
                _rawVertexArray = vertices;
            }

            osg::Vec3Array*
            getRawVertexArray()
            {
                return _rawVertexArray.get();
            }

            const osg::Vec3Array*
            getRawVertexArray() const
            {
                return _rawVertexArray.get();
            }

            /** Get the PrimitiveSetList for the raw face which hasn't been tessellated.
             */
            osg::Geometry::PrimitiveSetList&
            getRawFacePrimitiveSetList()
            {
                return _rawFacePrimitiveSetList;
            }

            const osg::Geometry::PrimitiveSetList&
            getRawFacePrimitiveSetList() const
            {
                return _rawFacePrimitiveSetList;
            }

            GlyphGeometry*
            getGlyphGeometry( const Style* style );

        protected:

            virtual ~Glyph3D()
            {
            }

            Font*                                          _font;
            unsigned int                                   _glyphCode;

            float                                          _width;
            float                                          _height;

            osg::vec2                                      _horizontalBearing;
            float                                          _horizontalAdvance;

            osg::vec2                                      _verticalBearing;
            float                                          _verticalAdvance;

            osg::box                                       _bb;
            // osg::vec2                   _advance;

            osg::ref_ptr<osg::Vec3Array>                   _rawVertexArray;
            osg::Geometry::PrimitiveSetList                _rawFacePrimitiveSetList;

            typedef std::list<osg::ref_ptr<GlyphGeometry>> GlyphGeometries;
            GlyphGeometries                                _glyphGeometries;
    };

    class OSGTEXT_EXPORT GlyphTexture : public osg::Texture2D
    {
        public:

            GlyphTexture();

            const char*
            className() const
            {
                return "GlyphTexture";
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
            virtual int
            compare( const osg::StateAttribute& rhs ) const;

            void
            setShaderTechnique( ShaderTechnique technique )
            {
                _shaderTechnique = technique;
            }

            ShaderTechnique
            getShaderTechnique() const
            {
                return _shaderTechnique;
            }

            int
            getEffectMargin( const Glyph* glyph );
            int
            getTexelMargin( const Glyph* glyph );

            bool
            getSpaceForGlyph( Glyph* glyph,
                              int&   posX,
                              int&   posY );

            void
            addGlyph( Glyph* glyph,
                      int    posX,
                      int    posY );

            /** Set whether to use a mutex to ensure ref() and unref() are thread safe.*/
            virtual void
            setThreadSafeRefUnref( bool threadSafe );

            /** Resize any per context GLObject buffers to specified size. */
            virtual void
            resizeGLObjectBuffers( unsigned int maxSize );

            /** create an image that maps all the associated Glyph's onto a single image,
             * that is equivalent to what will be downloaded to the texture.*/
            osg::Image*
            createImage();

        protected:

            virtual ~GlyphTexture();

            void
                            copyGlyphImage( Glyph*              glyph,
                                            Glyph::TextureInfo* info );

            ShaderTechnique _shaderTechnique;

            int             _usedY;
            int             _partUsedX;
            int             _partUsedY;

            typedef std::vector<osg::ref_ptr<Glyph>>   GlyphRefList;
            typedef std::vector<const Glyph*>          GlyphPtrList;
            typedef osg::buffered_object<GlyphPtrList> GlyphBuffer;

            GlyphRefList                               _glyphs;
            mutable GlyphBuffer                        _glyphsToSubload;

            mutable std::mutex                         _mutex;
    };

}
