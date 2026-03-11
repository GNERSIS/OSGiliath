#include <osgSim/ScalarsToColors>

using namespace osgSim;

ScalarsToColors::ScalarsToColors( float scalarMin,
                                  float scalarMax ) :
    _min( scalarMin ),
    _max( scalarMax )
{
}

osg::vec4
ScalarsToColors::getColor( float scalar ) const
{
    if( scalar < _min )
    {
        return osg::vec4( 0.0F, 0.0F, 0.0F, 0.0F );
    }
    if( scalar > _max )
    {
        return osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F );
    }

    float c = ( _min + scalar ) / ( _max - _min );
    return osg::vec4( c, c, c, 1.0 );
}

float
ScalarsToColors::getMin() const
{
    return _min;
}

float
ScalarsToColors::getMax() const
{
    return _max;
}
