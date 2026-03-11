/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Volume coordinate locator. Transforms between voxel indices
 * and world coordinates for volume positioning.
 */
#include <osgVolume/Locator>

#include <list>
#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/state/FrontFace.hpp>

using namespace osgVolume;

void
Locator::setTransformAsExtents( double minX,
                                double minY,
                                double maxX,
                                double maxY,
                                double minZ,
                                double maxZ )
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
                    maxZ - minZ,
                    0.0,
                    minX,
                    minY,
                    minZ,
                    1.0 );

    _inverse = osg::inverse( _transform );

    locatorModified();
}

bool
Locator::convertLocalToModel( const osg::dvec3& local,
                              osg::dvec3&       world ) const
{
    world = local * _transform;
    return true;
}

bool
Locator::convertModelToLocal( const osg::dvec3& world,
                              osg::dvec3&       local ) const
{
    local = world * _inverse;
    return true;
}

bool
Locator::computeLocalBounds( Locator& /*source*/,
                             osg::dvec3& bottomLeft,
                             osg::dvec3& topRight ) const
{
    typedef std::list<osg::dvec3> Corners;
    Corners                       corners;

    osg::dvec3                    cornerNDC;
    if( convertLocalToModel( osg::dvec3( 0.0, 0.0, 0.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 1.0, 0.0, 0.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 0.0, 1.0, 0.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 1.0, 1.0, 0.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 0.0, 0.0, 1.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 1.0, 0.0, 1.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 0.0, 1.0, 1.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 1.0, 1.0, 1.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( corners.empty() )
    {
        return false;
    }

    for( Corners::iterator itr = corners.begin(); itr != corners.end(); ++itr )
    {
        bottomLeft.x = std::min( bottomLeft.x, itr->x );
        bottomLeft.y = std::min( bottomLeft.y, itr->y );
        bottomLeft.z = std::min( bottomLeft.z, itr->z );
        topRight.x   = std::max( topRight.x, itr->x );
        topRight.y   = std::max( topRight.y, itr->y );
        topRight.z   = std::max( topRight.z, itr->z );
    }

    return true;
}

bool
Locator::computeLocalBounds( osg::dvec3& bottomLeft,
                             osg::dvec3& topRight ) const
{
    OSG_INFO << "Locator::computeLocalBounds" << std::endl;

    typedef std::list<osg::dvec3> Corners;
    Corners                       corners;

    osg::dvec3                    cornerNDC;
    if( convertLocalToModel( osg::dvec3( 0.0, 0.0, 0.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 1.0, 0.0, 0.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 0.0, 1.0, 0.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 1.0, 1.0, 0.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 0.0, 0.0, 1.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 1.0, 0.0, 1.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 0.0, 1.0, 1.0 ), cornerNDC ) )
    {
        corners.push_back( cornerNDC );
    }

    if( convertLocalToModel( osg::dvec3( 1.0, 1.0, 1.0 ), cornerNDC ) )
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
    bottomLeft.z = topRight.z = itr->z;

    ++itr;

    for( ; itr != corners.end(); ++itr )
    {
        bottomLeft.x = std::min( bottomLeft.x, itr->x );
        bottomLeft.y = std::min( bottomLeft.y, itr->y );
        bottomLeft.z = std::min( bottomLeft.z, itr->z );
        topRight.x   = std::max( topRight.x, itr->x );
        topRight.y   = std::max( topRight.y, itr->y );
        topRight.z   = std::max( topRight.z, itr->z );
    }

    return true;
}

void
Locator::addCallback( LocatorCallback* callback )
{
    // check if callback is already attached, if so just return early
    for( LocatorCallbacks::iterator itr = _locatorCallbacks.begin();
         itr != _locatorCallbacks.end();
         ++itr )
    {
        if( *itr == callback )
        {
            return;
        }
    }

    // callback is not attached so now attach it.
    _locatorCallbacks.push_back( callback );
}

void
Locator::removeCallback( LocatorCallback* callback )
{
    // checl if callback is attached, if so erase it.
    for( LocatorCallbacks::iterator itr = _locatorCallbacks.begin();
         itr != _locatorCallbacks.end();
         ++itr )
    {
        if( *itr == callback )
        {
            _locatorCallbacks.erase( itr );
            return;
        }
    }
}

void
Locator::locatorModified()
{
    for( LocatorCallbacks::iterator itr = _locatorCallbacks.begin();
         itr != _locatorCallbacks.end();
         ++itr )
    {
        ( *itr )->locatorModified( this );
    }
}

bool
Locator::inverted() const
{
    osg::dvec3 xAxis( _transform( 0, 0 ), _transform( 1, 0 ), _transform( 2, 0 ) );
    osg::dvec3 yAxis( _transform( 0, 1 ), _transform( 1, 1 ), _transform( 2, 1 ) );
    osg::dvec3 zAxis( _transform( 0, 2 ), _transform( 1, 2 ), _transform( 2, 2 ) );
    double     volume = osg::dot( osg::cross( xAxis, yAxis ), zAxis );
    return volume < 0.0;
}

void
Locator::applyAppropriateFrontFace( osg::StateSet* ss ) const
{
    osg::StateAttribute* sa = ss->getAttribute( osg::StateAttribute::Type::FRONTFACE );
    osg::FrontFace*      ff = dynamic_cast<osg::FrontFace*>( sa );
    if( !ff )
    {
        ff = new osg::FrontFace;
        ss->setAttribute( ff );
    }
    ff->setMode( inverted() ? osg::FrontFace::Mode::CLOCKWISE
                            : osg::FrontFace::Mode::COUNTER_CLOCKWISE );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TransformLocatorCallback
//
TransformLocatorCallback::TransformLocatorCallback( osg::MatrixTransform* transform ) :
    _transform( transform )
{
}

void
TransformLocatorCallback::locatorModified( Locator* locator )
{
    if( _transform.valid() )
    {
        locator->applyAppropriateFrontFace( _transform->getOrCreateStateSet() );

        _transform->setMatrix( locator->getTransform() );
    }
}
