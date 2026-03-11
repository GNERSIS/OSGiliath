/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Single light point definition with position, color, intensity,
 * and sector parameters for directional visibility.
 */
#include <osgSim/LightPoint>

using namespace osgSim;

LightPoint::LightPoint() :
    _on( true ),
    _position( 0.0F,
               0.0F,
               0.0F ),
    _color( 1.0F,
            1.0F,
            1.0F,
            1.0F ),
    _intensity( 1.0F ),
    _radius( 1.0F ),
    _sector( 0 ),
    _blinkSequence( 0 ),
    _blendingMode( BLENDED )
{
}

LightPoint::LightPoint( const osg::vec3& position,
                        const osg::vec4& color ) :
    _on( true ),
    _position( position ),
    _color( color ),
    _intensity( 1.0F ),
    _radius( 1.0F ),
    _sector( 0 ),
    _blinkSequence( 0 ),
    _blendingMode( BLENDED )
{
}

LightPoint::LightPoint( bool             on,
                        const osg::vec3& position,
                        const osg::vec4& color,
                        float            intensity,
                        float            radius,
                        Sector*          sector,
                        BlinkSequence*   blinkSequence,
                        BlendingMode     blendingMode ) :
    _on( on ),
    _position( position ),
    _color( color ),
    _intensity( intensity ),
    _radius( radius ),
    _sector( sector ),
    _blinkSequence( blinkSequence ),
    _blendingMode( blendingMode )
{
}

LightPoint::LightPoint( const LightPoint& lp ) :
    _on( lp._on ),
    _position( lp._position ),
    _color( lp._color ),
    _intensity( lp._intensity ),
    _radius( lp._radius ),
    _sector( lp._sector ),
    _blinkSequence( lp._blinkSequence ),
    _blendingMode( lp._blendingMode )
{
}

LightPoint&
LightPoint::operator=( const LightPoint& lp )
{
    _on            = lp._on;
    _position      = lp._position;
    _color         = lp._color;
    _intensity     = lp._intensity;
    _radius        = lp._radius;
    _sector        = lp._sector;
    _blinkSequence = lp._blinkSequence;
    _blendingMode  = lp._blendingMode;

    return *this;
}
