/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Directional visibility sector for light points. Defines
 * azimuth and elevation angle ranges for light visibility.
 */
#include <osgSim/Sector.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/Matrix.hpp>
#include <osg/maths/vec2.hpp>

using namespace osgSim;

//
// Elevation Range
//

void
AzimRange::setAzimuthRange( float minAzimuth,
                            float maxAzimuth,
                            float fadeAngle )
{
    // clamp the azimuth range.
    const float twoPI = 2.0F * ( float )osg::PI;
    while( minAzimuth > maxAzimuth )
    {
        minAzimuth -= twoPI;
    }

    // compute the centerline
    float centerAzim = ( minAzimuth + maxAzimuth ) * 0.5F;
    _cosAzim         = cosf( centerAzim );
    _sinAzim         = sinf( centerAzim );

    // compute the half angle range of the sector.
    float angle = ( maxAzimuth - minAzimuth ) * 0.5F;
    _cosAngle   = cosf( angle );

    // clamp the fade angle to valid values.
    fadeAngle = osg::clampAbove( fadeAngle, 0.0F );
    if( angle + fadeAngle > static_cast<float>( osg::PI ) )
    {
        _cosFadeAngle = -1.0F;
    }
    else
    {
        _cosFadeAngle = cosf( angle + fadeAngle );
    }
}

void
AzimRange::getAzimuthRange( float& minAzimuth,
                            float& maxAzimuth,
                            float& fadeAngle ) const
{
    float centerAzim = atan2f( _sinAzim, _cosAzim );
    float angle      = acosf( _cosAngle );
    minAzimuth       = centerAzim - angle;
    maxAzimuth       = centerAzim + angle;
    if( _cosFadeAngle == -1.0F )
    {
        fadeAngle = 2.0F * static_cast<float>( osg::PI );
    }
    else
    {
        fadeAngle = acosf( _cosFadeAngle ) - angle;
    }
}

//
// Elevation Range
//
void
ElevationRange::setElevationRange( float minElevation,
                                   float maxElevation,
                                   float fadeAngle )
{
    if( minElevation > maxElevation )
    {
        // need to swap angle pair.
        float tmp    = minElevation;
        minElevation = maxElevation;
        maxElevation = tmp;
    }

    minElevation = osg::clampTo( minElevation, ( float )-osg::PI_2, ( float )osg::PI_2 );
    maxElevation = osg::clampTo( maxElevation, ( float )-osg::PI_2, ( float )osg::PI_2 );
    fadeAngle    = osg::clampTo( fadeAngle, 0.0F, ( float )osg::PI_2 );

    _cosMinElevation   = cosf( static_cast<float>( osg::PI_2 ) - minElevation );
    _cosMaxElevation   = cosf( static_cast<float>( osg::PI_2 ) - maxElevation );

    float minFadeAngle = static_cast<float>( osg::PI_2 ) - minElevation + fadeAngle;
    if( minFadeAngle >= static_cast<float>( osg::PI ) )
    {
        _cosMinFadeElevation = -1.0F;
    }
    else
    {
        _cosMinFadeElevation = cosf( minFadeAngle );
    }

    float maxFadeAngle = static_cast<float>( osg::PI_2 ) - maxElevation - fadeAngle;
    if( maxFadeAngle <= 0.0F )
    {
        _cosMaxFadeElevation = 1.0F;
    }
    else
    {
        _cosMaxFadeElevation = cosf( maxFadeAngle );
    }
}

float
ElevationRange::getMinElevation() const
{
    return static_cast<float>( osg::PI_2 ) - acosf( _cosMinElevation );
}

float
ElevationRange::getMaxElevation() const
{
    return static_cast<float>( osg::PI_2 ) - acosf( _cosMaxElevation );
}

float
ElevationRange::getFadeAngle() const
{
    float fadeAngle = 0.0F;

    // Take the appropriate (unclipped) elevation angle to calculate the fade angle
    if( _cosMinFadeElevation != -1.0F )
    {
        float minFadeAngle = acosf( _cosMinFadeElevation );
        float minElevation = static_cast<float>( osg::PI_2 ) - acosf( _cosMinElevation );
        fadeAngle = minFadeAngle + minElevation - static_cast<float>( osg::PI_2 );
    }
    else if( _cosMaxFadeElevation != 1.0F )
    {
        float maxFadeAngle = acosf( _cosMaxFadeElevation );
        float maxElevation = static_cast<float>( osg::PI_2 ) - acosf( _cosMaxElevation );
        fadeAngle = static_cast<float>( osg::PI_2 ) - maxFadeAngle - maxElevation;
    }

    return fadeAngle;
}

//
// ElevationSector
//
AzimSector::AzimSector( float minAzimuth,
                        float maxAzimuth,
                        float fadeAngle ) :

    AzimRange()
{
    setAzimuthRange( minAzimuth, maxAzimuth, fadeAngle );
}

float
AzimSector::operator()( const osg::vec3& eyeLocal ) const
{
    return azimSector( eyeLocal );
}

//
// ElevationSector
//
ElevationSector::ElevationSector( float minElevation,
                                  float maxElevation,
                                  float fadeAngle ) :

    ElevationRange()
{
    setElevationRange( minElevation, maxElevation, fadeAngle );
}

float
ElevationSector::operator()( const osg::vec3& eyeLocal ) const
{
    return elevationSector( eyeLocal );
}

//
// AzimElevationSector
//
AzimElevationSector::AzimElevationSector( float minAzimuth,
                                          float maxAzimuth,
                                          float minElevation,
                                          float maxElevation,
                                          float fadeAngle ) :

    AzimRange(),
    ElevationRange()
{
    setAzimuthRange( minAzimuth, maxAzimuth, fadeAngle );
    setElevationRange( minElevation, maxElevation, fadeAngle );
}

float
AzimElevationSector::operator()( const osg::vec3& eyeLocal ) const
{
    float azimIntensity = azimSector( eyeLocal );
    if( azimIntensity == 0.0 )
    {
        return 0.0;    // out of sector.
    }
    float elevIntensity = elevationSector( eyeLocal );
    if( elevIntensity == 0.0 )
    {
        return 0.0;    // out of sector.
    }
    if( azimIntensity <= elevIntensity )
    {
        return azimIntensity;
    }
    return elevIntensity;
}

//
// ConeSector
//
ConeSector::ConeSector( const osg::vec3& axis,
                        float            angle,
                        float            fadeangle )
{
    setAxis( axis );
    setAngle( angle, fadeangle );
}

void
ConeSector::setAxis( const osg::vec3& axis )
{
    _axis = axis;
    _axis = osg::normalize( _axis );
}

const osg::vec3&
ConeSector::getAxis() const
{
    return _axis;
}

void
ConeSector::setAngle( float angle,
                      float fadeangle )
{
    _cosAngle     = cosf( angle );
    _cosAngleFade = cosf( angle + fadeangle );
}

float
ConeSector::getAngle() const
{
    return acosf( _cosAngle );
}

float
ConeSector::getFadeAngle() const
{
    return acosf( _cosAngleFade ) - acosf( _cosAngle );
}

float
ConeSector::operator()( const osg::vec3& eyeLocal ) const
{
    float dotproduct = osg::dot( eyeLocal, _axis );
    float length     = osg::length( eyeLocal );
    if( dotproduct > _cosAngle * length )
    {
        return 1.0F;    // fully in sector
    }
    if( dotproduct < _cosAngleFade * length )
    {
        return 0.0F;    // out of sector
    }
    return ( dotproduct - _cosAngleFade * length ) /
           ( ( _cosAngle - _cosAngleFade ) * length );
}

//
// DirectionalSector
//
DirectionalSector::DirectionalSector( const osg::vec3& direction,
                                      float            horizLobeAngle,
                                      float            vertLobeAngle,
                                      float            lobeRollAngle,
                                      float            fadeAngle )
{
    _direction     = direction;
    _cosHorizAngle = cosf( horizLobeAngle * 0.5F );
    _cosVertAngle  = cosf( vertLobeAngle * 0.5F );
    _rollAngle     = lobeRollAngle;

    setFadeAngle( fadeAngle );

    computeMatrix();
}

void
DirectionalSector::computeMatrix()
{
    double heading = atan2f( _direction[0], _direction[1] );
    double pitch =
        atan2( _direction[2],
               sqrt( _direction[0] * _direction[0] + _direction[1] * _direction[1] ) );
    double roll = _rollAngle;

    osg::setRotate( _local_to_LP, osg::dquat( heading, osg::dvec3( 0.0, 0.0, -1.0 ) ) );
    osg::preMultRotate( _local_to_LP, osg::dquat( pitch, osg::dvec3( 1.0, 0.0, 0.0 ) ) );
    osg::preMultRotate( _local_to_LP, osg::dquat( roll, osg::dvec3( 0.0, 1.0, 0.0 ) ) );
}

void
DirectionalSector::setDirection( const osg::vec3& direction )
{
    _direction = direction;
    computeMatrix();
}

const osg::vec3&
DirectionalSector::getDirection() const
{
    return _direction;
}

void
DirectionalSector::setHorizLobeAngle( float angle )
{
    _cosHorizAngle = cosf( angle * 0.5F );
}

float
DirectionalSector::getHorizLobeAngle() const
{
    return acosf( _cosHorizAngle ) * 2.0F;
}

void
DirectionalSector::setVertLobeAngle( float angle )
{
    _cosVertAngle = cosf( angle * 0.5F );
}

float
DirectionalSector::getVertLobeAngle() const
{
    return acosf( _cosVertAngle ) * 2.0F;
}

void
DirectionalSector::setLobeRollAngle( float angle )
{
    _rollAngle = angle;
    computeMatrix();
}

float
DirectionalSector::getLobeRollAngle() const
{
    return _rollAngle;
}

void
DirectionalSector::setFadeAngle( float angle )
{
    float ang = acosf( _cosHorizAngle ) + angle;
    if( ang > static_cast<float>( osg::PI ) )
    {
        _cosHorizFadeAngle = -1.0;
    }
    else
    {
        _cosHorizFadeAngle = cosf( ang );
    }

    ang = acosf( _cosVertAngle ) + angle;
    if( ang > static_cast<float>( osg::PI ) )
    {
        _cosVertFadeAngle = -1.0;
    }
    else
    {
        _cosVertFadeAngle = cosf( ang );
    }
}

float
DirectionalSector::getFadeAngle() const
{
    return acosf( _cosHorizFadeAngle ) - acosf( _cosHorizAngle );
}

float
DirectionalSector::operator()( const osg::vec3& eyeLocal ) const
{
    float     elev_intensity, azim_intensity;

    // Transform eyeLocal into the LightPoint frame
    osg::vec3 EPlp =
        osg::vec3( _local_to_LP * osg::dvec3( eyeLocal.x, eyeLocal.y, eyeLocal.z ) );

    /*fprintf(stderr, "    eyeLocal = %f, %f, %f\n", eyeLocal[0], eyeLocal[1],
    eyeLocal[2]) ; fprintf(stderr, "    EPlp     = %f, %f, %f\n", EPlp[0], EPlp[1],
    EPlp[2]) ;*/

    // Elevation check
    // Project EPlp into LP YZ plane and dot with LPy
    osg::vec2 EPyz( EPlp[1], EPlp[2] );
    EPyz = osg::normalize( EPyz );
    /*fprintf(stderr, "    EPyz = osg::normalize(EPyz) = %f, %f\n", EPyz[0], EPyz[1]) ;
    fprintf(stderr, "        _cosVertFadeAngle = %f\n", _cosVertFadeAngle) ;
    fprintf(stderr, "        _cosVertAngle     = %f\n", _cosVertAngle) ;*/
    // cosElev = EPyz* LPy = EPyz[0]
    if( EPyz[0] < _cosVertFadeAngle )
    {
        // Completely outside elevation range
        // fprintf(stderr, "   >> outside el range\n") ;
        return ( 0.0F );
    }
    if( EPyz[0] < _cosVertAngle )
    {
        // In the fade range
        // fprintf(stderr, "   >> inside el fade range\n") ;
        elev_intensity =
            ( EPyz[0] - _cosVertFadeAngle ) / ( _cosVertAngle - _cosVertFadeAngle );
    }
    else
    {
        // Fully in elevation range
        elev_intensity = 1.0;
        // fprintf(stderr, "   >> fully inside el range\n") ;
    }
    // Elevation check passed

    // Azimuth check
    // Project EPlp into LP XY plane and dot with LPy
    osg::vec2 EPxy( EPlp[0], EPlp[1] );
    EPxy = osg::normalize( EPxy );
    /*fprintf(stderr, "    EPxy = osg::normalize(EPxy) = %f, %f\n", EPxy[0], EPxy[1]) ;
    fprintf(stderr, "        _cosHorizFadeAngle = %f\n", _cosHorizFadeAngle) ;
    fprintf(stderr, "        _cosHorizAngle     = %f\n", _cosHorizAngle) ;*/
    // cosAzim = EPxy * LPy = EPxy[1]
    // if cosElev < 0.0, then need to negate EP for azimuth check
    if( EPyz[0] < 0.0 )
    {
        EPxy.set( -EPxy[0], -EPxy[1] );
    }
    if( EPxy[1] < _cosHorizFadeAngle )
    {
        // Completely outside azimuth range
        // fprintf(stderr, "   >> outside az range\n") ;
        return ( 0.0F );
    }
    if( EPxy[1] < _cosHorizAngle )
    {
        // In fade range
        // fprintf(stderr, "   >> inside az fade range\n") ;
        azim_intensity =
            ( EPxy[1] - _cosHorizFadeAngle ) / ( _cosHorizAngle - _cosHorizFadeAngle );
    }
    else
    {
        // Fully in azimuth range
        // fprintf(stderr, "   >> fully inside az range\n") ;
        azim_intensity = 1.0;
    }
    // Azimuth check passed

    // We're good! Return full intensity
    // fprintf(stderr, "   %%%% Returning intensity = %f\n", elev_intensity *
    // azim_intensity) ;
    return elev_intensity * azim_intensity;
}
