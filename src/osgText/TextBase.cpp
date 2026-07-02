/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for text drawables. Provides position, alignment,
 * rotation, font, and character size properties.
 */
#include <osgText/TextBase.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/GL>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/state/PolygonOffset.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgText/Font.hpp>
#include <osgUtil/culling/CullVisitor.hpp>

using namespace osg;
using namespace osgText;

// #define TREES_CODE_FOR_MAKING_SPACES_EDITABLE

TextBase::TextBase() :
    _color( 1.0F,
            1.0F,
            1.0F,
            1.0F ),
    _fontSize( 32,
               32 ),
    _characterHeight( 32 ),
    _characterSizeMode( OBJECT_COORDS ),
    _maximumWidth( 0.0F ),
    _maximumHeight( 0.0F ),
    _lineSpacing( 0.0F ),
    _alignment( BASE_LINE ),
    _axisAlignment( XY_PLANE ),
    _autoRotateToScreen( false ),
    _layout( LEFT_TO_RIGHT ),
    _drawMode( TEXT ),
    _textBBMargin( 0.0F ),
    _textBBColor( 0.0,
                  0.0,
                  0.0,
                  0.5 ),
    _kerningType( KERNING_DEFAULT ),
    _lineCount( 0 ),
    _glyphNormalized( false )
{

    initArraysAndBuffers();
}

TextBase::TextBase( const TextBase&    textBase,
                    const osg::CopyOp& copyop ) :
    osg::Drawable( textBase,
                   copyop ),
    _color( textBase._color ),
    _font( textBase._font ),
    _style( textBase._style ),
    _fontSize( textBase._fontSize ),
    _characterHeight( textBase._characterHeight ),
    _characterSizeMode( textBase._characterSizeMode ),
    _maximumWidth( textBase._maximumWidth ),
    _maximumHeight( textBase._maximumHeight ),
    _lineSpacing( textBase._lineSpacing ),
    _text( textBase._text ),
    _position( textBase._position ),
    _alignment( textBase._alignment ),
    _axisAlignment( textBase._axisAlignment ),
    _rotation( textBase._rotation ),
    _autoRotateToScreen( textBase._autoRotateToScreen ),
    _layout( textBase._layout ),
    _drawMode( textBase._drawMode ),
    _textBBMargin( textBase._textBBMargin ),
    _textBBColor( textBase._textBBColor ),
    _kerningType( textBase._kerningType ),
    _lineCount( textBase._lineCount ),
    _glyphNormalized( textBase._glyphNormalized )
{
    initArraysAndBuffers();
}

TextBase::~TextBase()
{
}

osg::StateSet*
TextBase::createStateSet()
{
    return 0;
}

void
TextBase::initArraysAndBuffers()
{
    _vbo         = new osg::VertexBufferObject;
    _ebo         = new osg::ElementBufferObject;

    _coords      = new osg::Vec3Array( osg::Array::BIND_PER_VERTEX );
    _normals     = new osg::Vec3Array( osg::Array::BIND_PER_VERTEX );
    _colorCoords = new osg::Vec4Array( osg::Array::BIND_PER_VERTEX );
    _texcoords   = new osg::Vec2Array( osg::Array::BIND_PER_VERTEX );

    _coords->setBufferObject( _vbo.get() );
    _normals->setBufferObject( _vbo.get() );
    _colorCoords->setBufferObject( _vbo.get() );
    _texcoords->setBufferObject( _vbo.get() );
}

osg::VertexArrayState*
TextBase::createVertexArrayStateImplementation( osg::RenderInfo& renderInfo ) const
{
    State&            state = *renderInfo.getState();

    VertexArrayState* vas   = new osg::VertexArrayState( &state );

    // OSG_NOTICE<<"Creating new osg::VertexArrayState "<< vas<<std::endl;

    if( _coords.valid() )
    {
        vas->assignVertexArrayDispatcher();
    }
    if( _colorCoords.valid() )
    {
        vas->assignColorArrayDispatcher();
    }
    if( _normals.valid() )
    {
        vas->assignNormalArrayDispatcher();
    }

    if( _texcoords.valid() )
    {
        vas->assignTexCoordArrayDispatcher( 1 );
    }

    if( state.useVertexArrayObject( _useVertexArrayObject ) )
    {
        OSG_INFO
            << "TextBase::createVertexArrayState() Setup VertexArrayState to use VAO "
            << vas << std::endl;

        vas->generateVertexArrayObject();
    }
    else
    {
        OSG_INFO << "TextBase::createVertexArrayState() Setup VertexArrayState to "
                    "without using VAO "
                 << vas << std::endl;
    }

    return vas;
}

void
TextBase::compileGLObjects( osg::RenderInfo& renderInfo ) const
{
    State& state = *renderInfo.getState();
    if( renderInfo.getState()->useVertexBufferObject( _supportsVertexBufferObjects &&
                                                      _useVertexBufferObjects ) )
    {
        unsigned int  contextID  = state.getContextID();
        GLExtensions* extensions = state.get<GLExtensions>();

        // Compile VBO data
        if( _vbo.valid() )
        {
            GLBufferObject* glBuf = _vbo->getOrCreateGLBufferObject( contextID );
            if( glBuf && glBuf->isDirty() )
            {
                glBuf->compileBuffer();
            }
        }

        // Compile EBO data
        if( _ebo.valid() )
        {
            GLBufferObject* glBuf = _ebo->getOrCreateGLBufferObject( contextID );
            if( glBuf && glBuf->isDirty() )
            {
                glBuf->compileBuffer();
            }
        }

        if( state.useVertexArrayObject( _useVertexArrayObject ) )
        {
            VertexArrayState* vas            = 0;

            _vertexArrayStateList[contextID] = vas =
                createVertexArrayState( renderInfo );

            State::SetCurrentVertexArrayStateProxy setVASProxy( state, vas );

            state.bindVertexArrayObject( vas );

            // Set up VAO vertex attribute bindings without issuing draw calls.
            // In Core Profile, drawing without a shader program bound crashes.
            vas->lazyDisablingOfVertexAttributes();
            if( _coords.valid() )
            {
                vas->setVertexArray( state, _coords.get() );
            }
            if( _normals.valid() )
            {
                vas->setNormalArray( state, _normals.get() );
            }
            if( _colorCoords.valid() )
            {
                vas->setColorArray( state, _colorCoords.get() );
            }
            if( _texcoords.valid() )
            {
                vas->setTexCoordArray( state, 0, _texcoords.get() );
            }
            vas->applyDisablingOfVertexAttributes( state );

            // Reset buffer caches so rendering re-binds everything.
            // GL_ARRAY_BUFFER is not part of VAO state, so after unbind/rebind
            // the cached _currentVBO would be stale.
            vas->resetBufferObjectPointers();

            state.unbindVertexArrayObject();
        }

        // unbind the BufferObjects
        extensions->glBindBuffer( GL_ARRAY_BUFFER_ARB, 0 );
        extensions->glBindBuffer( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
    }
}

void
TextBase::resizeGLObjectBuffers( unsigned int maxSize )
{
    if( _font.valid() )
    {
        _font->resizeGLObjectBuffers( maxSize );
    }

    if( _coords.valid() )
    {
        _coords->resizeGLObjectBuffers( maxSize );
    }
    if( _normals.valid() )
    {
        _normals->resizeGLObjectBuffers( maxSize );
    }
    if( _colorCoords.valid() )
    {
        _colorCoords->resizeGLObjectBuffers( maxSize );
    }
    if( _texcoords.valid() )
    {
        _texcoords->resizeGLObjectBuffers( maxSize );
    }

    for( Primitives::const_iterator itr = _decorationPrimitives.begin();
         itr != _decorationPrimitives.end();
         ++itr )
    {
        ( *itr )->resizeGLObjectBuffers( maxSize );
    }

    Drawable::resizeGLObjectBuffers( maxSize );
}

void
TextBase::releaseGLObjects( osg::State* state ) const
{
    if( _font.valid() )
    {
        _font->releaseGLObjects( state );
    }

    if( _coords.valid() )
    {
        _coords->releaseGLObjects( state );
    }
    if( _normals.valid() )
    {
        _normals->releaseGLObjects( state );
    }
    if( _colorCoords.valid() )
    {
        _colorCoords->releaseGLObjects( state );
    }
    if( _texcoords.valid() )
    {
        _texcoords->releaseGLObjects( state );
    }

    for( Primitives::const_iterator itr = _decorationPrimitives.begin();
         itr != _decorationPrimitives.end();
         ++itr )
    {
        ( *itr )->releaseGLObjects( state );
    }

    Drawable::releaseGLObjects( state );
}

void
TextBase::setColor( const osg::vec4& color )
{
    _color = color;
}

void
TextBase::assignStateSet()
{
    setStateSet( createStateSet() );
}

void
TextBase::setFont( osg::ref_ptr<Font> font )
{
    if( _font == font )
    {
        return;
    }

    _font = font;

    assignStateSet();

    computeGlyphRepresentation();
}

void
TextBase::setFont( const std::string& fontfile )
{
    setFont( readRefFontFile( fontfile ) );
}

void
TextBase::setFontResolution( unsigned int width,
                             unsigned int height )
{
    FontResolution size( width, height );
    if( _fontSize == size )
    {
        return;
    }

    _fontSize = size;

    assignStateSet();

    computeGlyphRepresentation();
}

void
TextBase::setCharacterSize( float height )
{
    if( _characterHeight == height )
    {
        return;
    }

    _characterHeight = height;
    computeGlyphRepresentation();
}

void
TextBase::setCharacterSize( float height,
                            float aspectRatio )
{
    if( getCharacterAspectRatio() != aspectRatio )
    {
        getOrCreateStyle()->setWidthRatio( aspectRatio );
    }
    setCharacterSize( height );
}

void
TextBase::setMaximumWidth( float maximumWidth )
{
    if( _maximumWidth == maximumWidth )
    {
        return;
    }

    _maximumWidth = maximumWidth;
    computeGlyphRepresentation();
}

void
TextBase::setMaximumHeight( float maximumHeight )
{
    if( _maximumHeight == maximumHeight )
    {
        return;
    }

    _maximumHeight = maximumHeight;
    computeGlyphRepresentation();
}

void
TextBase::setLineSpacing( float lineSpacing )
{
    if( _lineSpacing == lineSpacing )
    {
        return;
    }

    _lineSpacing = lineSpacing;
    computeGlyphRepresentation();
}

void
TextBase::setText( const String& text )
{
    if( _text == text )
    {
        return;
    }

    _text = text;
    computeGlyphRepresentation();
}

void
TextBase::setText( const std::string& text )
{
    setText( String( text ) );
}

void
TextBase::setText( const std::string& text,
                   String::Encoding   encoding )
{
    setText( String( text, encoding ) );
}

void
TextBase::setText( const wchar_t* text )
{
    setText( String( text ) );
}

void
TextBase::setPosition( const osg::vec3& pos )
{
    if( _position == pos )
    {
        return;
    }

    _position = pos;
    computePositions();
}

void
TextBase::setAlignment( AlignmentType alignment )
{
    if( _alignment == alignment )
    {
        return;
    }

    _alignment = alignment;
    // computePositions();
    computeGlyphRepresentation();
}

void
TextBase::setAxisAlignment( AxisAlignment axis )
{
    _axisAlignment = axis;

    switch( axis )
    {
        case XZ_PLANE :
            setAutoRotateToScreen( false );
            setRotation( osg::quat( osg::radians( 90.0F ),
                                    osg::vec3( 1.0F, 0.0F, 0.0F ) ) );
            break;
        case REVERSED_XZ_PLANE :
            setAutoRotateToScreen( false );
            setRotation(
                osg::quat( osg::radians( 180.0F ), osg::vec3( 0.0F, 1.0F, 0.0F ) ) *
                osg::quat( osg::radians( 90.0F ), osg::vec3( 1.0F, 0.0F, 0.0F ) )
            );
            break;
        case YZ_PLANE :
            setAutoRotateToScreen( false );
            setRotation(
                osg::quat( osg::radians( 90.0F ), osg::vec3( 1.0F, 0.0F, 0.0F ) ) *
                osg::quat( osg::radians( 90.0F ), osg::vec3( 0.0F, 0.0F, 1.0F ) )
            );
            break;
        case REVERSED_YZ_PLANE :
            setAutoRotateToScreen( false );
            setRotation(
                osg::quat( osg::radians( 180.0F ), osg::vec3( 0.0F, 1.0F, 0.0F ) ) *
                osg::quat( osg::radians( 90.0F ), osg::vec3( 1.0F, 0.0F, 0.0F ) ) *
                osg::quat( osg::radians( 90.0F ), osg::vec3( 0.0F, 0.0F, 1.0F ) )
            );
            break;
        case XY_PLANE :
            setAutoRotateToScreen( false );
            setRotation( osg::quat() );    // nop - already on XY plane.
            break;
        case REVERSED_XY_PLANE :
            setAutoRotateToScreen( false );
            setRotation( osg::quat( osg::radians( 180.0F ),
                                    osg::vec3( 0.0F, 1.0F, 0.0F ) ) );
            break;
        case SCREEN :
            setAutoRotateToScreen( true );
            setRotation( osg::quat() );    // nop - already on XY plane.
            break;
        default :
            break;
    }
}

void
TextBase::setRotation( const osg::quat& quat )
{
    _rotation = quat;
    computePositions();
}

void
TextBase::setAutoRotateToScreen( bool autoRotateToScreen )
{
    if( _autoRotateToScreen == autoRotateToScreen )
    {
        return;
    }

    _autoRotateToScreen = autoRotateToScreen;
    computePositions();
}

void
TextBase::setLayout( Layout layout )
{
    if( _layout == layout )
    {
        return;
    }

    _layout = layout;
    computeGlyphRepresentation();
}

void
TextBase::setDrawMode( unsigned int mode )
{
    if( _drawMode == mode )
    {
        return;
    }

    _drawMode = mode;
}

void
TextBase::setBoundingBoxMargin( float margin )
{
    if( _textBBMargin == margin )
    {
        return;
    }

    _textBBMargin = margin;

    computePositions();
}

osg::box
TextBase::computeBoundingBox() const
{
    osg::box bbox;
#if 0
    return bbox;
#endif

    if( _textBBWithMargin.valid() )
    {
        bbox.expandBy( osg::vec3( _matrix *
                                  osg::dvec3( _textBBWithMargin.corner( 0 ) ) ) );
        bbox.expandBy( osg::vec3( _matrix *
                                  osg::dvec3( _textBBWithMargin.corner( 1 ) ) ) );
        bbox.expandBy( osg::vec3( _matrix *
                                  osg::dvec3( _textBBWithMargin.corner( 2 ) ) ) );
        bbox.expandBy( osg::vec3( _matrix *
                                  osg::dvec3( _textBBWithMargin.corner( 3 ) ) ) );
        bbox.expandBy( osg::vec3( _matrix *
                                  osg::dvec3( _textBBWithMargin.corner( 4 ) ) ) );
        bbox.expandBy( osg::vec3( _matrix *
                                  osg::dvec3( _textBBWithMargin.corner( 5 ) ) ) );
        bbox.expandBy( osg::vec3( _matrix *
                                  osg::dvec3( _textBBWithMargin.corner( 6 ) ) ) );
        bbox.expandBy( osg::vec3( _matrix *
                                  osg::dvec3( _textBBWithMargin.corner( 7 ) ) ) );

#if 0
        if (!bbox.valid())
        {
            // Provide a fallback in cases where no bounding box has been been setup so far.
            // Note, assume a scaling of 1.0 for _characterSizeMode!=OBJECT_COORDS as the
            // for screen space coordinates size modes we don't know what scale will be used until
            // the text is actually rendered, but we haven't to assume something otherwise the
            // text label will be culled by view or small feature culling on first frame.
            if (_autoRotateToScreen)
            {
                // use bounding sphere encompassing the maximum size of the text centered on the _position
                double radius = _textBB.radius();
                osg::vec3 diagonal(radius, radius, radius);
                bbox.set(_position-diagonal, _position+diagonal);
            }
            else
            {
                osg::dmat4 matrix;
                matrix = osg::translate(-_offset);
                matrix = matrix * osg::rotate(_rotation);
                matrix = matrix * osg::translate(_position);
                bbox.expandBy(osg::vec3(_matrix * osg::dvec3(_textBB.corner(0))));
                bbox.expandBy(osg::vec3(_matrix * osg::dvec3(_textBB.corner(1))));
                bbox.expandBy(osg::vec3(_matrix * osg::dvec3(_textBB.corner(2))));
                bbox.expandBy(osg::vec3(_matrix * osg::dvec3(_textBB.corner(3))));
                bbox.expandBy(osg::vec3(_matrix * osg::dvec3(_textBB.corner(4))));
                bbox.expandBy(osg::vec3(_matrix * osg::dvec3(_textBB.corner(5))));
                bbox.expandBy(osg::vec3(_matrix * osg::dvec3(_textBB.corner(6))));
                bbox.expandBy(osg::vec3(_matrix * osg::dvec3(_textBB.corner(7))));
            }
        }
#endif
    }

    return bbox;
}

void
TextBase::computePositions()
{
    _textBBWithMargin = _textBB;

    computePositionsImplementation();

    osg::dmat4 matrix;
    computeMatrix( matrix, 0 );

    const_cast<TextBase*>( this )->dirtyBound();
}

void
TextBase::computePositionsImplementation()
{
    _normal = osg::vec3( 0.0F, 0.0F, 1.0F );

    if( _text.empty() )
    {
        _offset = vec3();
        return;
    }

    switch( _alignment )
    {
        case LEFT_TOP :
            _offset.set( _textBB.xMin(), _textBB.yMax(), _textBB.zMin() );
            break;
        case LEFT_CENTER :
            _offset.set( _textBB.xMin(),
                         ( _textBB.yMax() + _textBB.yMin() ) * 0.5F,
                         _textBB.zMin() );
            break;
        case LEFT_BOTTOM :
            _offset.set( _textBB.xMin(), _textBB.yMin(), _textBB.zMin() );
            break;

        case CENTER_TOP :
            _offset.set( ( _textBB.xMax() + _textBB.xMin() ) * 0.5F,
                         _textBB.yMax(),
                         _textBB.zMin() );
            break;
        case CENTER_CENTER :
            _offset.set( ( _textBB.xMax() + _textBB.xMin() ) * 0.5F,
                         ( _textBB.yMax() + _textBB.yMin() ) * 0.5F,
                         _textBB.zMin() );
            break;
        case CENTER_BOTTOM :
            _offset.set( ( _textBB.xMax() + _textBB.xMin() ) * 0.5F,
                         _textBB.yMin(),
                         _textBB.zMin() );
            break;

        case RIGHT_TOP :
            _offset.set( _textBB.xMax(), _textBB.yMax(), _textBB.zMin() );
            break;
        case RIGHT_CENTER :
            _offset.set( _textBB.xMax(),
                         ( _textBB.yMax() + _textBB.yMin() ) * 0.5F,
                         _textBB.zMin() );
            break;
        case RIGHT_BOTTOM :
            _offset.set( _textBB.xMax(), _textBB.yMin(), _textBB.zMin() );
            break;

        case LEFT_BASE_LINE :
            _offset.set( _textBB.xMin(), 0.0F, 0.0F );
            break;
        case CENTER_BASE_LINE :
            _offset.set( ( _textBB.xMax() + _textBB.xMin() ) * 0.5F, 0.0F, 0.0F );
            break;
        case RIGHT_BASE_LINE :
            _offset.set( _textBB.xMax(), 0.0F, 0.0F );
            break;

        case LEFT_BOTTOM_BASE_LINE :
            _offset.set( _textBB.xMin(),
                         static_cast<float>( -_characterHeight *
                                             ( 1.0 + _lineSpacing ) *
                                             ( _lineCount - 1 ) ),
                         0.0F );
            break;
        case CENTER_BOTTOM_BASE_LINE :
            _offset.set( ( _textBB.xMax() + _textBB.xMin() ) * 0.5F,
                         static_cast<float>( -_characterHeight *
                                             ( 1.0 + _lineSpacing ) *
                                             ( _lineCount - 1 ) ),
                         0.0F );
            break;
        case RIGHT_BOTTOM_BASE_LINE :
            _offset.set( _textBB.xMax(),
                         static_cast<float>( -_characterHeight *
                                             ( 1.0 + _lineSpacing ) *
                                             ( _lineCount - 1 ) ),
                         0.0F );
            break;
    }
}

bool
TextBase::computeMatrix( osg::dmat4& matrix,
                         osg::State* state ) const
{
    if( state && ( _characterSizeMode != OBJECT_COORDS || _autoRotateToScreen ) )
    {
        osg::dmat4 modelview  = state->getModelViewMatrix();
        osg::dmat4 projection = state->getProjectionMatrix();

        osg::dmat4 temp_matrix( modelview );
        osg::setTrans( temp_matrix, 0.0, 0.0, 0.0 );

        osg::dmat4 rotate_matrix;
        rotate_matrix = osg::inverse( temp_matrix );

        matrix        = osg::translate( -_offset );
        matrix        = matrix * osg::rotate( _rotation );

        if( _characterSizeMode != OBJECT_COORDS )
        {
            typedef osg::dmat4::value_type value_type;

            value_type                     width    = 1280.0;
            value_type                     height   = 1024.0;

            const osg::Viewport*           viewport = state->getCurrentViewport();
            if( viewport )
            {
                width  = static_cast<value_type>( viewport->width() );
                height = static_cast<value_type>( viewport->height() );
            }

            osg::dmat4 mvpw   = osg::scale( width / 2.0, height / 2.0, 1.0 ) *
                                projection *
                                modelview *
                                osg::translate( _position ) *
                                rotate_matrix;

            osg::dvec3 origin = mvpw * osg::dvec3( 0.0, 0.0, 0.0 );
            osg::dvec3 left   = mvpw * osg::dvec3( 1.0, 0.0, 0.0 ) - origin;
            osg::dvec3 up     = mvpw * osg::dvec3( 0.0, 1.0, 0.0 ) - origin;

            // compute the pixel size vector.
            value_type length_x = osg::length( left );
            value_type scale_x  = length_x > 0.0 ? 1.0 / length_x : 1.0;

            value_type length_y = osg::length( up );
            value_type scale_y  = length_y > 0.0 ? 1.0 / length_y : 1.0;

            if( _glyphNormalized )
            {
                osg::vec3 scaleVec( _characterHeight / getCharacterAspectRatio(),
                                    _characterHeight,
                                    _characterHeight );
                matrix = matrix * osg::scale( scaleVec );
            }

            if( _characterSizeMode == SCREEN_COORDS )
            {
                matrix =
                    matrix * osg::scale( osg::vec3( static_cast<float>( scale_x ),
                                                    static_cast<float>( scale_y ),
                                                    static_cast<float>( scale_x ) ) );
            }
            else
            {
                value_type pixelSizeVert = _characterHeight / scale_y;

                // avoid nasty math by preventing a divide by zero
                if( pixelSizeVert == 0.0 )
                {
                    pixelSizeVert = 1.0;
                }

                if( pixelSizeVert > getFontHeight() )
                {
                    value_type scale_font = getFontHeight() / pixelSizeVert;
                    matrix = matrix *
                             osg::scale( osg::vec3( static_cast<float>( scale_font ),
                                                    static_cast<float>( scale_font ),
                                                    static_cast<float>( scale_font ) ) );
                }
            }
        }

        if( _autoRotateToScreen )
        {
            matrix = matrix * rotate_matrix;
        }

        matrix = matrix * osg::translate( _position );
    }
    else if( !osg::zeroRotation( _rotation ) )
    {
        matrix = osg::translate( -_offset );
        if( _glyphNormalized )
        {
            osg::vec3 scaleVec( _characterHeight / getCharacterAspectRatio(),
                                _characterHeight,
                                _characterHeight );
            matrix = matrix * osg::scale( scaleVec );
        }
        matrix = matrix * osg::rotate( _rotation );
        matrix = matrix * osg::translate( _position );
        // OSG_NOTICE<<"New Need to rotate "<<matrix<<std::endl;
    }
    else
    {
        matrix = osg::translate( -_offset );
        if( _glyphNormalized )
        {
            osg::vec3 scaleVec( _characterHeight / getCharacterAspectRatio(),
                                _characterHeight,
                                _characterHeight );
            matrix = matrix * osg::scale( scaleVec );
        }
        matrix = matrix * osg::translate( _position );
    }

    if( _matrix != matrix )
    {
        _matrix = matrix;
        const_cast<TextBase*>( this )->dirtyBound();
    }

    return true;
}

void
TextBase::positionCursor( const osg::vec2& endOfLine_coords,
                          osg::vec2&       cursor,
                          unsigned int     linelength )
{
    switch( _layout )
    {
        case LEFT_TO_RIGHT :
            {
                switch( _alignment )
                {
                    // nothing to be done for these
                    // case LEFT_TOP:
                    // case LEFT_CENTER:
                    // case LEFT_BOTTOM:
                    // case LEFT_BASE_LINE:
                    // case LEFT_BOTTOM_BASE_LINE:
                    //  break;
                    case CENTER_TOP :
                    case CENTER_CENTER :
                    case CENTER_BOTTOM :
                    case CENTER_BASE_LINE :
                    case CENTER_BOTTOM_BASE_LINE :
                        cursor.x = ( cursor.x - endOfLine_coords.x ) * 0.5F;
                        break;
                    case RIGHT_TOP :
                    case RIGHT_CENTER :
                    case RIGHT_BOTTOM :
                    case RIGHT_BASE_LINE :
                    case RIGHT_BOTTOM_BASE_LINE :
                        cursor.x = cursor.x - endOfLine_coords.x;
                        break;
                    default :
                        break;
                }
                break;
            }
        case RIGHT_TO_LEFT :
            {
                switch( _alignment )
                {
                    case LEFT_TOP :
                    case LEFT_CENTER :
                    case LEFT_BOTTOM :
                    case LEFT_BASE_LINE :
                    case LEFT_BOTTOM_BASE_LINE :
                        cursor.x = 2 * cursor.x - endOfLine_coords.x;
                        break;
                    case CENTER_TOP :
                    case CENTER_CENTER :
                    case CENTER_BOTTOM :
                    case CENTER_BASE_LINE :
                    case CENTER_BOTTOM_BASE_LINE :
                        cursor.x = cursor.x + ( cursor.x - endOfLine_coords.x ) * 0.5F;
                        break;
                        // nothing to be done for these
                        // case RIGHT_TOP:
                        // case RIGHT_CENTER:
                        // case RIGHT_BOTTOM:
                        // case RIGHT_BASE_LINE:
                        // case RIGHT_BOTTOM_BASE_LINE:
                        //  break;
                    default :
                        break;
                }
                break;
            }
        case VERTICAL :
            {
                switch( _alignment )
                {
                    // TODO: current behaviour top baselines lined up in both cases -
                    // need to implement
                    //       top of characters aligment - Question is this necessary?
                    // ... otherwise, nothing to be done for these 6 cases
                    // case LEFT_TOP:
                    // case CENTER_TOP:
                    // case RIGHT_TOP:
                    //  break;
                    // case LEFT_BASE_LINE:
                    // case CENTER_BASE_LINE:
                    // case RIGHT_BASE_LINE:
                    //  break;
                    case LEFT_CENTER :
                    case CENTER_CENTER :
                    case RIGHT_CENTER :
                        cursor.y = cursor.y + ( cursor.y - endOfLine_coords.y ) * 0.5F;
                        break;
                    case LEFT_BOTTOM_BASE_LINE :
                    case CENTER_BOTTOM_BASE_LINE :
                    case RIGHT_BOTTOM_BASE_LINE :
                        cursor.y = cursor.y - ( static_cast<float>( linelength ) *
                                                _characterHeight );
                        break;
                    case LEFT_BOTTOM :
                    case CENTER_BOTTOM :
                    case RIGHT_BOTTOM :
                        cursor.y = 2 * cursor.y - endOfLine_coords.y;
                        break;
                    default :
                        break;
                }
                break;
            }
    }
}

void
TextBase::setupDecoration()
{
    unsigned int numVerticesRequired = 0;
    if( _drawMode & FILLEDBOUNDINGBOX )
    {
        numVerticesRequired += 4;
    }
    if( _drawMode & BOUNDINGBOX )
    {
        numVerticesRequired += 8;
    }
    if( _drawMode & ALIGNMENT )
    {
        numVerticesRequired += 4;
    }

    _decorationPrimitives.clear();

    if( numVerticesRequired == 0 )
    {
        return;
    }

    if( !_coords )
    {
        _coords = new osg::Vec3Array;
        _coords->setBufferObject( _vbo.get() );
        _coords->resize( numVerticesRequired );
    }

    if( !_texcoords )
    {
        _texcoords = new osg::Vec2Array;
        _texcoords->setBufferObject( _vbo.get() );
        _texcoords->resize( numVerticesRequired );
    }

    osg::vec2 default_texcoord( -1.0, -1.0 );

    if( ( _drawMode & FILLEDBOUNDINGBOX ) != 0 && _textBBWithMargin.valid() )
    {
        osg::vec3    c000( _textBBWithMargin.xMin(),
                           _textBBWithMargin.yMin(),
                           _textBBWithMargin.zMin() );
        osg::vec3    c100( _textBBWithMargin.xMax(),
                           _textBBWithMargin.yMin(),
                           _textBBWithMargin.zMin() );
        osg::vec3    c110( _textBBWithMargin.xMax(),
                           _textBBWithMargin.yMax(),
                           _textBBWithMargin.zMin() );
        osg::vec3    c010( _textBBWithMargin.xMin(),
                           _textBBWithMargin.yMax(),
                           _textBBWithMargin.zMin() );

        unsigned int base = static_cast<unsigned int>( _coords->size() );

        _coords->push_back( c000 );
        _coords->push_back( c100 );
        _coords->push_back( c110 );
        _coords->push_back( c010 );

        _texcoords->push_back( default_texcoord );
        _texcoords->push_back( default_texcoord );
        _texcoords->push_back( default_texcoord );
        _texcoords->push_back( default_texcoord );

        osg::ref_ptr<osg::DrawElementsUShort> primitives =
            new osg::DrawElementsUShort( GL_TRIANGLES );
        primitives->setBufferObject( _ebo.get() );
        _decorationPrimitives.push_back( primitives );

        primitives->push_back( static_cast<unsigned short>( base ) );
        primitives->push_back( static_cast<unsigned short>( base + 1 ) );
        primitives->push_back( static_cast<unsigned short>( base + 2 ) );
        primitives->push_back( static_cast<unsigned short>( base ) );
        primitives->push_back( static_cast<unsigned short>( base + 2 ) );
        primitives->push_back( static_cast<unsigned short>( base + 3 ) );

        _coords->dirty();
        primitives->dirty();
    }

    if( ( _drawMode & BOUNDINGBOX ) != 0 && _textBBWithMargin.valid() )
    {
        if( _textBBWithMargin.zMin() == _textBBWithMargin.zMax() )
        {
            osg::vec3    c000( _textBBWithMargin.xMin(),
                               _textBBWithMargin.yMin(),
                               _textBBWithMargin.zMin() );
            osg::vec3    c100( _textBBWithMargin.xMax(),
                               _textBBWithMargin.yMin(),
                               _textBBWithMargin.zMin() );
            osg::vec3    c110( _textBBWithMargin.xMax(),
                               _textBBWithMargin.yMax(),
                               _textBBWithMargin.zMin() );
            osg::vec3    c010( _textBBWithMargin.xMin(),
                               _textBBWithMargin.yMax(),
                               _textBBWithMargin.zMin() );

            unsigned int base = static_cast<unsigned int>( _coords->size() );

            _coords->push_back( c000 );
            _coords->push_back( c100 );
            _coords->push_back( c110 );
            _coords->push_back( c010 );

            _texcoords->push_back( default_texcoord );
            _texcoords->push_back( default_texcoord );
            _texcoords->push_back( default_texcoord );
            _texcoords->push_back( default_texcoord );

            osg::ref_ptr<osg::DrawElementsUShort> primitives =
                new osg::DrawElementsUShort( GL_LINE_LOOP );
            primitives->setBufferObject( _ebo.get() );
            _decorationPrimitives.push_back( primitives );

            primitives->push_back( static_cast<unsigned short>( base ) );
            primitives->push_back( static_cast<unsigned short>( base + 1 ) );
            primitives->push_back( static_cast<unsigned short>( base + 2 ) );
            primitives->push_back( static_cast<unsigned short>( base + 3 ) );

            _coords->dirty();
            primitives->dirty();
        }
        else
        {
            osg::vec3    c000( _textBBWithMargin.xMin(),
                               _textBBWithMargin.yMin(),
                               _textBBWithMargin.zMin() );
            osg::vec3    c100( _textBBWithMargin.xMax(),
                               _textBBWithMargin.yMin(),
                               _textBBWithMargin.zMin() );
            osg::vec3    c110( _textBBWithMargin.xMax(),
                               _textBBWithMargin.yMax(),
                               _textBBWithMargin.zMin() );
            osg::vec3    c010( _textBBWithMargin.xMin(),
                               _textBBWithMargin.yMax(),
                               _textBBWithMargin.zMin() );

            osg::vec3    c001( _textBBWithMargin.xMin(),
                               _textBBWithMargin.yMin(),
                               _textBBWithMargin.zMax() );
            osg::vec3    c101( _textBBWithMargin.xMax(),
                               _textBBWithMargin.yMin(),
                               _textBBWithMargin.zMax() );
            osg::vec3    c111( _textBBWithMargin.xMax(),
                               _textBBWithMargin.yMax(),
                               _textBBWithMargin.zMax() );
            osg::vec3    c011( _textBBWithMargin.xMin(),
                               _textBBWithMargin.yMax(),
                               _textBBWithMargin.zMax() );

            unsigned int base = static_cast<unsigned int>( _coords->size() );

            _coords->push_back( c000 );    // +0
            _coords->push_back( c100 );    // +1
            _coords->push_back( c110 );    // +2
            _coords->push_back( c010 );    // +3

            _texcoords->push_back( default_texcoord );
            _texcoords->push_back( default_texcoord );
            _texcoords->push_back( default_texcoord );
            _texcoords->push_back( default_texcoord );

            _coords->push_back( c001 );    // +4
            _coords->push_back( c101 );    // +5
            _coords->push_back( c111 );    // +6
            _coords->push_back( c011 );    // +7

            _texcoords->push_back( default_texcoord );
            _texcoords->push_back( default_texcoord );
            _texcoords->push_back( default_texcoord );
            _texcoords->push_back( default_texcoord );

            osg::ref_ptr<osg::DrawElementsUShort> primitives =
                new osg::DrawElementsUShort( GL_LINES );
            primitives->setBufferObject( _ebo.get() );
            _decorationPrimitives.push_back( primitives );

            // front loop
            primitives->push_back( static_cast<unsigned short>( base + 0 ) );
            primitives->push_back( static_cast<unsigned short>( base + 1 ) );

            primitives->push_back( static_cast<unsigned short>( base + 1 ) );
            primitives->push_back( static_cast<unsigned short>( base + 2 ) );

            primitives->push_back( static_cast<unsigned short>( base + 2 ) );
            primitives->push_back( static_cast<unsigned short>( base + 3 ) );

            primitives->push_back( static_cast<unsigned short>( base + 3 ) );
            primitives->push_back( static_cast<unsigned short>( base + 0 ) );

            // back loop
            primitives->push_back( static_cast<unsigned short>( base + 4 ) );
            primitives->push_back( static_cast<unsigned short>( base + 5 ) );

            primitives->push_back( static_cast<unsigned short>( base + 5 ) );
            primitives->push_back( static_cast<unsigned short>( base + 6 ) );

            primitives->push_back( static_cast<unsigned short>( base + 6 ) );
            primitives->push_back( static_cast<unsigned short>( base + 7 ) );

            primitives->push_back( static_cast<unsigned short>( base + 7 ) );
            primitives->push_back( static_cast<unsigned short>( base + 4 ) );

            // edges from corner 000
            primitives->push_back( static_cast<unsigned short>( base + 0 ) );
            primitives->push_back( static_cast<unsigned short>( base + 4 ) );

            primitives->push_back( static_cast<unsigned short>( base + 1 ) );
            primitives->push_back( static_cast<unsigned short>( base + 5 ) );

            primitives->push_back( static_cast<unsigned short>( base + 2 ) );
            primitives->push_back( static_cast<unsigned short>( base + 6 ) );

            primitives->push_back( static_cast<unsigned short>( base + 3 ) );
            primitives->push_back( static_cast<unsigned short>( base + 7 ) );

            _coords->dirty();
            primitives->dirty();
        }
    }

    if( _drawMode & ALIGNMENT )
    {
        float        cursorsize = _characterHeight * 0.5F;

        osg::vec3    hl( osg::vec3( _offset.x - cursorsize, _offset.y, _offset.z ) );
        osg::vec3    hr( osg::vec3( _offset.x + cursorsize, _offset.y, _offset.z ) );
        osg::vec3    vt( osg::vec3( _offset.x, _offset.y - cursorsize, _offset.z ) );
        osg::vec3    vb( osg::vec3( _offset.x, _offset.y + cursorsize, _offset.z ) );

        unsigned int base = static_cast<unsigned int>( _coords->size() );

        _coords->push_back( hl );
        _coords->push_back( hr );
        _coords->push_back( vt );
        _coords->push_back( vb );

        _texcoords->push_back( default_texcoord );
        _texcoords->push_back( default_texcoord );
        _texcoords->push_back( default_texcoord );
        _texcoords->push_back( default_texcoord );

        osg::ref_ptr<osg::DrawElementsUShort> primitives =
            new osg::DrawElementsUShort( GL_LINES );
        primitives->setBufferObject( _ebo.get() );
        _decorationPrimitives.push_back( primitives );

        // front loop
        primitives->push_back( static_cast<unsigned short>( base + 0 ) );
        primitives->push_back( static_cast<unsigned short>( base + 1 ) );

        primitives->push_back( static_cast<unsigned short>( base + 2 ) );
        primitives->push_back( static_cast<unsigned short>( base + 3 ) );

        _coords->dirty();
        primitives->dirty();
    }
}
