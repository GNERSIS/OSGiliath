#include <osgSim/ColorRange.hpp>

using namespace osgSim;

ColorRange::ColorRange( float min,
                        float max ) :
    ScalarsToColors( min,
                     max )
{
    // Default to something sensible
    _colors.push_back( osg::vec4( 1.0, 0.0, 0.0, 1.0 ) );    // R
    _colors.push_back( osg::vec4( 1.0, 1.0, 0.0, 1.0 ) );    // Y
    _colors.push_back( osg::vec4( 0.0, 1.0, 0.0, 1.0 ) );    // G
    _colors.push_back( osg::vec4( 0.0, 1.0, 1.0, 1.0 ) );    // C
    _colors.push_back( osg::vec4( 0.0, 0.0, 1.0, 1.0 ) );    // B
}

ColorRange::ColorRange( float                         min,
                        float                         max,
                        const std::vector<osg::vec4>& colors ) :
    ScalarsToColors( min,
                     max )
{
    setColors( colors );
}

void
ColorRange::setColors( const std::vector<osg::vec4>& colors )
{
    _colors = colors;
}

osg::vec4
ColorRange::getColor( float scalar ) const
{
    if( _colors.empty() )
    {
        return osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F );
    }
    if( _colors.size() == 1 )
    {
        return _colors.front();
    }

    if( scalar < getMin() )
    {
        return _colors.front();
    }
    if( scalar > getMax() )
    {
        return _colors.back();
    }

    float       r     = ( ( scalar - getMin() ) / ( getMax() - getMin() ) ) *
                        static_cast<float>( _colors.size() - 1 );
    std::size_t lower = static_cast<std::size_t>( floor( r ) );
    std::size_t upper = static_cast<std::size_t>( ceil( r ) );

    osg::vec4   color = _colors[lower] + ( ( _colors[upper] - _colors[lower] ) *
                                           ( r - static_cast<float>( lower ) ) );
    return color;
}
