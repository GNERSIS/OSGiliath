#include <osgSim/ScalarBar.hpp>

#include <osg/geometry/Geometry.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/maths/Matrix.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/PolygonOffset.hpp>
#include <osgText/Text.hpp>
#include <sstream>

using namespace osgSim;

ScalarBar::~ScalarBar()
{
}

std::string
ScalarBar::ScalarPrinter::printScalar( float scalar )
{
    std::stringstream ostr;
    ostr << scalar;
    return ostr.str();
}

void
ScalarBar::setNumColors( int numColors )
{
    _numColors = numColors;
    createDrawables();
}

int
ScalarBar::getNumColors() const
{
    return _numColors;
}

void
ScalarBar::setNumLabels( int numLabels )
{
    _numLabels = numLabels;
    createDrawables();
}

int
ScalarBar::getNumLabels() const
{
    return _numLabels;
}

void
ScalarBar::setScalarsToColors( ScalarsToColors* stc )
{
    _stc = stc;
    createDrawables();
}

const ScalarsToColors*
ScalarBar::getScalarsToColors() const
{
    return _stc.get();
}

void
ScalarBar::setTitle( const std::string& title )
{
    _title = title;
    createDrawables();
}

const std::string&
ScalarBar::getTitle() const
{
    return _title;
}

void
ScalarBar::setPosition( const osg::vec3& pos )
{
    _position = pos;
    createDrawables();
}

void
ScalarBar::setWidth( float width )
{
    _width = width;
    createDrawables();
}

void
ScalarBar::setOrientation( ScalarBar::Orientation orientation )
{
    _orientation = orientation;
    createDrawables();
}

ScalarBar::Orientation
ScalarBar::getOrientation() const
{
    return _orientation;
}

void
ScalarBar::setAspectRatio( float aspectRatio )
{
    _aspectRatio = aspectRatio;
    createDrawables();
}

float
ScalarBar::getAspectRatio() const
{
    return _aspectRatio;
}

void
ScalarBar::setScalarPrinter( ScalarPrinter* sp )
{
    _sp = sp;
    createDrawables();
}

const ScalarBar::ScalarPrinter*
ScalarBar::getScalarPrinter() const
{
    return _sp.get();
}

void
ScalarBar::setTextProperties( const TextProperties& tp )
{
    _textProperties = tp;
    createDrawables();
}

const ScalarBar::TextProperties&
ScalarBar::getTextProperties() const
{
    return _textProperties;
}

void
ScalarBar::createDrawables()
{
    // Remove any existing Drawables
    removeDrawables( 0, getNumDrawables() );

    if( _numColors == 0 )
    {
        return;
    }

    osg::dmat4 matrix;
    if( _orientation == HORIZONTAL )
    {
        matrix = osg::translate( _position );
    }
    else
    {
        matrix = osg::rotate( osg::radians( 90.0 ), 0.0, 0.0, 1.0 ) *
                 osg::translate( _position );
    }

    // 1. First the bar
    // =================
    osg::ref_ptr<osg::Geometry>  bar = new osg::Geometry();

    // Create the bar - created in 'real' coordinate space the moment,
    // with xyz values reflecting those of the actual scalar values in play.
    // FIXME: Consider positioning at origin! Should be easy enough to do.

    // Vertices
    osg::ref_ptr<osg::Vec3Array> vs( new osg::Vec3Array );
    vs->reserve( static_cast<std::size_t>( 4 * _numColors ) );

    float incr  = ( _stc->getMax() - _stc->getMin() ) / static_cast<float>( _numColors );
    float xincr = ( _width ) / static_cast<float>( _numColors );
    float arOffset = _width * _aspectRatio;

    int   i;
    for( i = 1; i <= _numColors; ++i )
    {
        vs->push_back( osg::vec3(
            matrix * osg::dvec3( static_cast<float>( i - 1 ) * xincr, 0.0F, 0.0F )
        ) );
        vs->push_back( osg::vec3(
            matrix * osg::dvec3( static_cast<float>( i - 1 ) * xincr, arOffset, 0.0F )
        ) );
        vs->push_back( osg::vec3(
            matrix * osg::dvec3( static_cast<float>( i ) * xincr, arOffset, 0.0F )
        ) );
        vs->push_back( osg::vec3(
            matrix * osg::dvec3( static_cast<float>( i ) * xincr, 0.0F, 0.0F )
        ) );
    }
    bar->setVertexArray( vs.get() );

    // Colours
    osg::ref_ptr<osg::Vec4Array> cs( new osg::Vec4Array );
    cs->reserve( static_cast<std::size_t>( 4 * _numColors ) );
    const float halfIncr = incr * 0.5F;
    // Check whether to use interpolated colors or retain the original
    // color values if _numColors equals the number of ColorRange colors defined
    ColorRange* cr = dynamic_cast<ColorRange*>( _stc.get() );
    bool        fixedColors =
        cr && ( _numColors == static_cast<int>( cr->getColors().size() ) );
    for( i = 0; i < _numColors; ++i )
    {
        // We add half an increment to the color look-up to get the color
        // square in the middle of the 'block' unless using fixed colors.
        osg::vec4 c = fixedColors ? cr->getColors()[static_cast<std::size_t>( i )]
                                  : _stc->getColor( _stc->getMin() +
                                                    ( static_cast<float>( i ) * incr ) +
                                                    halfIncr );
        cs->push_back( c );
        cs->push_back( c );
        cs->push_back( c );
        cs->push_back( c );
    }

    bar->setColorArray( cs.get(), osg::Array::BIND_PER_VERTEX );

    // Normal
    osg::ref_ptr<osg::Vec3Array> ns( new osg::Vec3Array );
    ns->push_back( osg::transform3x3( osg::vec3( 0.0F, 0.0F, 1.0F ), matrix ) );
    bar->setNormalArray( ns.get(), osg::Array::BIND_OVERALL );

    // Triangle indices for the bar (quads decomposed to triangles)
    osg::ref_ptr<osg::DrawElementsUShort> indices =
        new osg::DrawElementsUShort( GL_TRIANGLES );
    indices->reserve( static_cast<std::size_t>( _numColors * 6 ) );
    for( int qi = 0; qi < _numColors; ++qi )
    {
        unsigned short base = static_cast<unsigned short>( qi * 4 );
        indices->push_back( base );
        indices->push_back( base + 1 );
        indices->push_back( base + 3 );
        indices->push_back( base + 1 );
        indices->push_back( base + 2 );
        indices->push_back( base + 3 );
    }
    bar->addPrimitiveSet( indices.get() );
    bar->getOrCreateStateSet()->setAttributeAndModes( new osg::PolygonOffset( 1, 1 ),
                                                      osg::StateAttribute::OVERRIDE |
                                                          osg::StateAttribute::ON );

    addDrawable( bar.get() );

#define CHARACTER_OFFSET_FACTOR ( 0.3F )

    // 2. Then the text labels
    // =======================

    // Check the character size, if it's 0, estimate a good character size
    float characterSize = _textProperties._characterSize;
    if( characterSize == 0 )
    {
        characterSize = _width * 0.03F;
    }

    osg::ref_ptr<osgText::Font> font =
        osgText::readRefFontFile( _textProperties._fontFile.c_str() );

    std::vector<osgText::Text*> texts(
        static_cast<std::size_t>( _numLabels )
    );    // We'll need to collect pointers to these for later
    float labelIncr = ( _numLabels > 0 ) ? ( _stc->getMax() - _stc->getMin() ) /
                                               static_cast<float>( _numLabels - 1 )
                                         : 0.0F;
    float labelxIncr =
        ( _numLabels > 0 ) ? ( _width ) / static_cast<float>( _numLabels - 1 ) : 0.0F;
    const float labelStickStartY = _orientation == HORIZONTAL ? arOffset : 0;
    const float labelY =
        labelStickStartY +
        ( _orientation == HORIZONTAL ? characterSize : -characterSize ) *
        CHARACTER_OFFSET_FACTOR;

    for( i = 0; i < _numLabels; ++i )
    {
        osgText::Text* text = new osgText::Text;
        text->setFont( font );
        text->setColor( _textProperties._color );
        text->setFontResolution(
            static_cast<unsigned int>( _textProperties._fontResolution.first ),
            static_cast<unsigned int>( _textProperties._fontResolution.second )
        );
        text->setCharacterSize( characterSize );
        text->setText( _sp->printScalar( _stc->getMin() +
                                         ( static_cast<float>( i ) * labelIncr ) ) );

        text->setPosition( osg::vec3(
            matrix * osg::dvec3( static_cast<float>( i ) * labelxIncr, labelY, 0.0F )
        ) );
        text->setAlignment( ( _orientation == HORIZONTAL )
                                ? osgText::Text::CENTER_BASE_LINE
                                : osgText::Text::LEFT_CENTER );

        addDrawable( text );

        texts[static_cast<std::size_t>( i )] = text;
    }

    // 3. The title
    // ============

    if( _title != "" )
    {
        osgText::Text* text = new osgText::Text;
        text->setFont( font );
        text->setColor( _textProperties._color );
        text->setFontResolution(
            static_cast<unsigned int>( _textProperties._fontResolution.first ),
            static_cast<unsigned int>( _textProperties._fontResolution.second )
        );
        text->setCharacterSize( characterSize );
        text->setText( _title );

        osg::vec3 titlePos;
        if( _orientation == HORIZONTAL )
        {
            const float titleY = ( _numLabels > 0 ) ? labelY + characterSize : labelY;
            titlePos           = osg::vec3( _width / 2.0F, titleY, 0.0F );
        }
        else
        {
            titlePos = osg::vec3( _width + characterSize * 0.5F, arOffset * 0.5F, 0 );
        }

        // Position the title at the middle of the bar above any labels.
        text->setPosition( osg::vec3( matrix * osg::dvec3( titlePos ) ) );
        text->setAlignment( _orientation == HORIZONTAL ? osgText::Text::CENTER_BASE_LINE
                                                       : osgText::Text::CENTER_BOTTOM );

        addDrawable( text );
    }

    // 4. The rectangular border and sticks
    // ====================================

    osg::ref_ptr<osg::Vec3Array> annotVertices = new osg::Vec3Array;

    // Border
    annotVertices->push_back( osg::vec3( matrix * osg::dvec3( 0.0F, 0.0F, 0.0F ) ) );
    annotVertices->push_back( osg::vec3( matrix * osg::dvec3( 0.0F, arOffset, 0.0F ) ) );

    annotVertices->push_back( osg::vec3( matrix * osg::dvec3( 0.0F, arOffset, 0.0F ) ) );
    annotVertices->push_back( osg::vec3( matrix *
                                         osg::dvec3( _width, arOffset, 0.0F ) ) );

    annotVertices->push_back( osg::vec3( matrix *
                                         osg::dvec3( _width, arOffset, 0.0F ) ) );
    annotVertices->push_back( osg::vec3( matrix * osg::dvec3( _width, 0.0F, 0.0F ) ) );

    annotVertices->push_back( osg::vec3( matrix * osg::dvec3( _width, 0.0F, 0.0F ) ) );
    annotVertices->push_back( osg::vec3( matrix * osg::dvec3( 0.0F, 0.0F, 0.0F ) ) );

    // Sticks
    for( i = 0; i < _numLabels; ++i )
    {
        const osg::vec3 p1( osg::vec3(
            matrix *
            osg::dvec3( static_cast<float>( i ) * labelxIncr, labelStickStartY, 0.0F )
        ) );
        const osg::vec3 p2( osg::vec3(
            matrix * osg::dvec3( static_cast<float>( i ) * labelxIncr, labelY, 0.0F )
        ) );
        annotVertices->push_back( p1 );
        annotVertices->push_back( p2 );
    }

    osg::ref_ptr<osg::Geometry> annotationGeom = new osg::Geometry();
    annotationGeom->addPrimitiveSet(
        new osg::DrawArrays( GL_LINES, 0, static_cast<GLsizei>( annotVertices->size() ) )
    );
    annotationGeom->setVertexArray( annotVertices.get() );
    osg::ref_ptr<osg::Material> annotMaterial = new osg::Material;
    annotMaterial->setDiffuse( osg::Material::FRONT, _textProperties._color );
    annotationGeom->getOrCreateStateSet()->setAttribute( annotMaterial.get() );

    addDrawable( annotationGeom.get() );
}
