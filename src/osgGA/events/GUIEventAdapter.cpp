/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Event data container carrying input event details (key, button,
 * coordinates, scroll, touch). Immutable once dispatched.
 */
#include <osgGA/events/GUIEventAdapter.hpp>

#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>

using namespace osgGA;

osg::ref_ptr<GUIEventAdapter>&
GUIEventAdapter::getAccumulatedEventState()
{
    static osg::ref_ptr<GUIEventAdapter> s_eventState = new GUIEventAdapter;
    return s_eventState;
}

GUIEventAdapter::GUIEventAdapter() :
    _eventType( NONE ),
    _windowX( 0 ),
    _windowY( 0 ),
    _windowWidth( 1'280 ),
    _windowHeight( 1'024 ),
    _key( 0 ),
    _unmodifiedKey( 0 ),
    _button( 0 ),
    _Xmin( -1.0 ),
    _Xmax( 1.0 ),
    _Ymin( -1.0 ),
    _Ymax( 1.0 ),
    _mx( 0.0 ),
    _my( 0.0 ),
    _buttonMask( 0 ),
    _modKeyMask( 0 ),
    _mouseYOrientation( Y_INCREASING_DOWNWARDS ),
    _scrolling(),
    _tabletPen(),
    _touchData( NULL )
{
}

GUIEventAdapter::GUIEventAdapter( const GUIEventAdapter& rhs,
                                  const osg::CopyOp&     copyop ) :
    Inherit( rhs,
             copyop ),
    _eventType( rhs._eventType ),
    _context( rhs._context ),
    _windowX( rhs._windowX ),
    _windowY( rhs._windowY ),
    _windowWidth( rhs._windowWidth ),
    _windowHeight( rhs._windowHeight ),
    _key( rhs._key ),
    _unmodifiedKey( rhs._unmodifiedKey ),
    _button( rhs._button ),
    _Xmin( rhs._Xmin ),
    _Xmax( rhs._Xmax ),
    _Ymin( rhs._Ymin ),
    _Ymax( rhs._Ymax ),
    _mx( rhs._mx ),
    _my( rhs._my ),
    _buttonMask( rhs._buttonMask ),
    _modKeyMask( rhs._modKeyMask ),
    _mouseYOrientation( rhs._mouseYOrientation ),
    _scrolling( rhs._scrolling ),
    _tabletPen( rhs._tabletPen ),
    _touchData( NULL )
{
    if( TouchData* td = rhs.getTouchData() )
    {
        setTouchData( osg::clone( td, copyop ) );
    }
}

GUIEventAdapter::~GUIEventAdapter()
{
}

void
GUIEventAdapter::setWindowRectangle( int  x,
                                     int  y,
                                     int  width,
                                     int  height,
                                     bool updateMouseRange )
{
    _windowX      = x;
    _windowY      = y;
    _windowWidth  = width;
    _windowHeight = height;

    if( updateMouseRange )
    {
        setInputRange( 0.0F,
                       0.0F,
                       static_cast<float>( width - 1 ),
                       static_cast<float>( height - 1 ) );
    }
}

void
GUIEventAdapter::setInputRange( float Xmin,
                                float Ymin,
                                float Xmax,
                                float Ymax )
{
    _Xmin = Xmin;
    _Ymin = Ymin;
    _Xmax = Xmax;
    _Ymax = Ymax;
}

const osg::dmat4
GUIEventAdapter::getPenOrientation() const
{
    float      xRad = osg::DegreesToRadians( getPenTiltY() );
    float      yRad = osg::DegreesToRadians( getPenTiltX() );
    float      zRad = osg::DegreesToRadians( getPenRotation() );
    osg::dmat4 xrot = osg::rotate( ( double )xRad, osg::dvec3( 1.0, 0.0, 0.0 ) );
    osg::dmat4 yrot = osg::rotate( ( double )yRad, osg::dvec3( 0.0, 0.0, 1.0 ) );
    osg::dmat4 zrot = osg::rotate( ( double )zRad, osg::dvec3( 0.0, 1.0, 0.0 ) );

    return ( zrot * yrot * xrot );
}

void
GUIEventAdapter::addTouchPoint( unsigned int id,
                                TouchPhase   phase,
                                float        x,
                                float        y,
                                unsigned int tapCount )
{
    if( !_touchData.valid() )
    {
        _touchData = new TouchData();
        setX( x );
        setY( y );
    }

    _touchData->addTouchPoint( id, phase, x, y, tapCount );
}

void
GUIEventAdapter::copyPointerDataFrom( const osgGA::GUIEventAdapter& sourceEvent )
{
    setGraphicsContext(
        const_cast<osg::GraphicsContext*>( sourceEvent.getGraphicsContext() )
    );
    setX( sourceEvent.getX() );
    setY( sourceEvent.getY() );
    setInputRange( sourceEvent.getXmin(),
                   sourceEvent.getYmin(),
                   sourceEvent.getXmax(),
                   sourceEvent.getYmax() );
    setButtonMask( sourceEvent.getButtonMask() );
    setMouseYOrientation( sourceEvent.getMouseYOrientation() );
    setPointerDataList( sourceEvent.getPointerDataList() );
}

void
GUIEventAdapter::setMouseYOrientationAndUpdateCoords(
    osgGA::GUIEventAdapter::MouseYOrientation myo
)
{
    if( myo == _mouseYOrientation )
    {
        return;
    }

    setMouseYOrientation( myo );

    _my = _Ymax - _my + _Ymin;
    if( isMultiTouchEvent() )
    {
        for( TouchData::iterator itr = getTouchData()->begin();
             itr != getTouchData()->end();
             itr++ )
        {
            itr->y = _Ymax - itr->y + _Ymin;
        }
    }
}
