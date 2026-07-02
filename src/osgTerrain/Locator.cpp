/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Geographic coordinate locator. Transforms between local tile
 * coordinates and geographic (lat/lon) coordinates.
 */
#include <osgTerrain/Locator.hpp>

#include <list>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>

using namespace osgTerrain;

//////////////////////////////////////////////////////////////////////////////
//
// Locator
//
Locator::Locator() :
    _coordinateSystemType( PROJECTED ),
    _ellipsoidModel( new osg::EllipsoidModel() ),
    _definedInFile( false ),
    _transformScaledByResolution( false )
{
}

Locator::Locator( const Locator&     locator,
                  const osg::CopyOp& copyop ) :
    Inherit( locator,
             copyop ),
    _coordinateSystemType( locator._coordinateSystemType ),
    _format( locator._format ),
    _cs( locator._cs ),
    _ellipsoidModel( locator._ellipsoidModel ),
    _transform( locator._transform ),
    _definedInFile( locator._definedInFile ),
    _transformScaledByResolution( locator._transformScaledByResolution )
{
}

Locator::~Locator()
{
}

void
Locator::setTransformAsExtents( double minX,
                                double minY,
                                double maxX,
                                double maxY )
{
    _transform.set( maxX - minX,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    maxY - minY,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    1.0,
                    0.0,
                    minX,
                    minY,
                    0.0,
                    1.0 );

    _inverse = osg::inverse( _transform );
}

bool
Locator::computeLocalBounds( Locator&    source,
                             osg::dvec3& bottomLeft,
                             osg::dvec3& topRight ) const
{
    typedef std::list<osg::dvec3> Corners;
    Corners                       corners;

    osg::dvec3                    cornerNDC;
    if( Locator::convertLocalCoordBetween( source,
                                           osg::dvec3( 0.0, 0.0, 0.0 ),
                                           *this,
                                           cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( Locator::convertLocalCoordBetween( source,
                                           osg::dvec3( 1.0, 0.0, 0.0 ),
                                           *this,
                                           cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( Locator::convertLocalCoordBetween( source,
                                           osg::dvec3( 0.0, 1.0, 0.0 ),
                                           *this,
                                           cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( Locator::convertLocalCoordBetween( source,
                                           osg::dvec3( 1.0, 1.0, 0.0 ),
                                           *this,
                                           cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( corners.empty() )
    {
        return false;
    }

    Corners::iterator itr = corners.begin();

    bottomLeft.x = topRight.x = itr->x;
    bottomLeft.y = topRight.y = itr->y;

    ++itr;

    for( ; itr != corners.end(); ++itr )
    {
        bottomLeft.x = std::min( bottomLeft.x, itr->x );
        bottomLeft.y = std::min( bottomLeft.y, itr->y );
        topRight.x   = std::max( topRight.x, itr->x );
        topRight.y   = std::max( topRight.y, itr->y );
    }

    return true;
}

bool
Locator::orientationOpenGL() const
{
    return _transform( 0, 0 ) * _transform( 1, 1 ) >= 0.0;
}

bool
Locator::convertLocalToModel( const osg::dvec3& local,
                              osg::dvec3&       world ) const
{
    switch( _coordinateSystemType )
    {
        case( GEOCENTRIC ) :
            {
                osg::dvec3 geographic = local * _transform;

                _ellipsoidModel->convertLatLongHeightToXYZ( geographic.y,
                                                            geographic.x,
                                                            geographic.z,
                                                            world.x,
                                                            world.y,
                                                            world.z );
                return true;
            }
        case( GEOGRAPHIC ) :
            {
                world = local * _transform;
                return true;
            }
        case( PROJECTED ) :
            {
                world = local * _transform;
                return true;
            }
    }

    return false;
}

bool
Locator::convertModelToLocal( const osg::dvec3& world,
                              osg::dvec3&       local ) const
{
    switch( _coordinateSystemType )
    {
        case( GEOCENTRIC ) :
            {
                double longitude, latitude, height;

                _ellipsoidModel->convertXYZToLatLongHeight( world.x,
                                                            world.y,
                                                            world.z,
                                                            latitude,
                                                            longitude,
                                                            height );

                local = osg::dvec3( longitude, latitude, height ) * _inverse;

                return true;
            }
        case( GEOGRAPHIC ) :
            {
                local = world * _inverse;

                return true;
            }
        case( PROJECTED ) :
            {
                local = world * _inverse;
                return true;
            }
    }

    return false;
}
