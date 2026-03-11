/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * 3D text style parameters (extrusion depth, bevel, outline).
 * Controls the shape of extruded glyph geometry in Text3D.
 */
#include <osgText/Style>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>

using namespace osgText;

/////////////////////////////////////////////////////////////////////////////////////////
//
// Bevel
//
Bevel::Bevel()
{
    _smoothConcaveJunctions = false;
    _thickness              = 0.02F;
    flatBevel();
}

Bevel::Bevel( const Bevel&       bevel,
              const osg::CopyOp& copyop ) :
    Inherit( bevel,
             copyop ),
    _smoothConcaveJunctions( bevel._smoothConcaveJunctions ),
    _thickness( bevel._thickness ),
    _vertices( bevel._vertices )
{
}

void
Bevel::flatBevel( float width )
{
    _vertices.clear();

    if( width > 0.5F )
    {
        width = 0.5F;
    }

    _vertices.push_back( osg::vec2( 0.0F, 0.0F ) );

    _vertices.push_back( osg::vec2( width, 1.0F ) );

    if( width < 0.5F )
    {
        _vertices.push_back( osg::vec2( 1 - width, 1.0F ) );
    }

    _vertices.push_back( osg::vec2( 1.0F, 0.0F ) );
}

void
Bevel::roundedBevel( float        width,
                     unsigned int numSteps )
{
    _vertices.clear();

    if( width > 0.5F )
    {
        width = 0.5F;
    }

    unsigned int i = 0;
    for( ; i <= numSteps; ++i )
    {
        float angle = float( osg::PI ) * 0.5F * ( float( i ) / float( numSteps ) );
        _vertices.push_back( osg::vec2( ( 1.0F - cosf( angle ) ) * width,
                                        sinf( angle ) ) );
    }

    // start the second half one into the curve if the width is half way across
    i = width < 0.5F ? 0 : 1;
    for( ; i <= numSteps; ++i )
    {
        float angle =
            float( osg::PI ) * 0.5F * ( float( numSteps - i ) / float( numSteps ) );
        _vertices.push_back( osg::vec2( 1.0F - ( 1.0F - cosf( angle ) ) * width,
                                        sinf( angle ) ) );
    }
}

void
Bevel::roundedBevel2( float        width,
                      unsigned int numSteps )
{
    _vertices.clear();

    if( width > 0.5F )
    {
        width = 0.5F;
    }

    float h = 0.1F;
    float r = 1.0F - h;

    _vertices.push_back( osg::vec2( 0.0, 0.0 ) );

    unsigned int i = 0;
    for( ; i <= numSteps; ++i )
    {
        float angle = float( osg::PI ) * 0.5F * ( float( i ) / float( numSteps ) );
        _vertices.push_back( osg::vec2( ( 1.0F - cosf( angle ) ) * width,
                                        h + sinf( angle ) * r ) );
    }

    // start the second half one into the curve if the width is half way across
    i = width < 0.5F ? 0 : 1;
    for( ; i <= numSteps; ++i )
    {
        float angle =
            float( osg::PI ) * 0.5F * ( float( numSteps - i ) / float( numSteps ) );
        _vertices.push_back( osg::vec2( 1.0F - ( 1.0F - cosf( angle ) ) * width,
                                        h + sinf( angle ) * r ) );
    }

    _vertices.push_back( osg::vec2( 1.0, 0.0 ) );
}

void
Bevel::print( std::ostream& /*fout*/ )
{
    OSG_NOTICE << "print bevel" << std::endl;
    for( Vertices::iterator itr = _vertices.begin(); itr != _vertices.end(); ++itr )
    {
        OSG_NOTICE << "  " << *itr << std::endl;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Style
//
Style::Style() :
    _widthRatio( 1.0F ),
    _thicknessRatio( 0.0F ),
    _outlineRatio( 0.0F ),
    _sampleDensity( 1.0F )
{
}

Style::Style( const Style&       style,
              const osg::CopyOp& copyop ) :
    Inherit( style,
             copyop ),
    _bevel( dynamic_cast<Bevel*>( copyop( style._bevel.get() ) ) ),
    _widthRatio( style._widthRatio ),
    _thicknessRatio( style._thicknessRatio ),
    _outlineRatio( style._outlineRatio ),
    _sampleDensity( style._sampleDensity )
{
}

/// default Layout implementation used if no other is specified on TextNode
osg::ref_ptr<Style>&
Style::getDefaultStyle()
{
    static std::mutex           s_DefaultStyleMutex;
    std::lock_guard<std::mutex> lock( s_DefaultStyleMutex );

    static osg::ref_ptr<Style>  s_defaultStyle = new Style;
    return s_defaultStyle;
}

bool
Style::operator==( const Style& rhs ) const
{
    if( &rhs == this )
    {
        return true;
    }

    if( _bevel.valid() )
    {
        if( !rhs._bevel )
        {
            return false;
        }

        if( !( *_bevel == *rhs._bevel ) )
        {
            return false;
        }
    }
    else
    {
        if( rhs._bevel.valid() )
        {
            return false;
        }
    }

    if( _widthRatio != rhs._widthRatio )
    {
        return false;
    }
    if( _thicknessRatio != rhs._thicknessRatio )
    {
        return false;
    }
    if( _outlineRatio != rhs._outlineRatio )
    {
        return false;
    }
    if( _sampleDensity != rhs._sampleDensity )
    {
        return false;
    }

    return true;
}
