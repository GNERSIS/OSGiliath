/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Keystone correction for projection warping. Adjusts corner
 * positions to compensate for off-axis projector alignment.
 */
#include <osgViewer/handlers/Keystone.hpp>

#include <osg/core/Inherit.hpp>
#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/ValueObject.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/rendering/DisplaySettings.hpp>
#include <osg/state/PolygonMode.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/io/WriteFile.hpp>

using namespace osgViewer;

Keystone::Keystone() :
    keystoneEditingEnabled( false ),
    gridColour( 1.0F,
                1.0F,
                1.0F,
                1.0F ),
    bottom_left( -1.0,
                 -1.0 ),
    bottom_right( 1.0,
                  -1.0 ),
    top_left( -1.0,
              1.0 ),
    top_right( 1.0,
               1.0 )
{
}

Keystone::Keystone( const Keystone&    rhs,
                    const osg::CopyOp& copyop ) :
    Inherit( rhs,
             copyop ),
    keystoneEditingEnabled( rhs.keystoneEditingEnabled ),
    gridColour( rhs.gridColour ),
    bottom_left( rhs.bottom_left ),
    bottom_right( rhs.bottom_right ),
    top_left( rhs.top_left ),
    top_right( rhs.top_right )
{
}

void
Keystone::reset()
{
    bottom_left.set( -1.0, -1.0 );
    bottom_right.set( 1.0, -1.0 );
    top_left.set( -1.0, 1.0 );
    top_right.set( 1.0, 1.0 );
}

Keystone&
Keystone::operator=( const Keystone& rhs )
{
    if( &rhs == this )
    {
        return *this;
    }
    keystoneEditingEnabled = rhs.keystoneEditingEnabled;
    gridColour             = rhs.gridColour;
    bottom_left            = rhs.bottom_left;
    bottom_right           = rhs.bottom_right;
    top_left               = rhs.top_left;
    top_right              = rhs.top_right;
    return *this;
}

void
Keystone::compute3DPositions( osg::DisplaySettings* ds,
                              osg::vec3&            tl,
                              osg::vec3&            tr,
                              osg::vec3&            br,
                              osg::vec3&            bl ) const
{
    double tr_x           = ( osg::length( top_right - bottom_right ) ) /
                            ( osg::length( top_left - bottom_left ) );
    double r_left         = sqrt( tr_x );
    double r_right        = r_left / tr_x;

    double tr_y           = ( osg::length( top_right - top_left ) ) /
                            ( osg::length( bottom_right - bottom_left ) );
    double r_bottom       = sqrt( tr_y );
    double r_top          = r_bottom / tr_y;

    double screenDistance = ds->getScreenDistance();
    double screenWidth    = ds->getScreenWidth();
    double screenHeight   = ds->getScreenHeight();

    tl = osg::vec3( static_cast<float>( screenWidth * 0.5 * top_left.x ),
                    static_cast<float>( screenHeight * 0.5 * top_left.y ),
                    static_cast<float>( -screenDistance ) ) *
         static_cast<float>( r_left ) *
         static_cast<float>( r_top );
    tr = osg::vec3( static_cast<float>( screenWidth * 0.5 * top_right.x ),
                    static_cast<float>( screenHeight * 0.5 * top_right.y ),
                    static_cast<float>( -screenDistance ) ) *
         static_cast<float>( r_right ) *
         static_cast<float>( r_top );
    br = osg::vec3( static_cast<float>( screenWidth * 0.5 * bottom_right.x ),
                    static_cast<float>( screenHeight * 0.5 * bottom_right.y ),
                    static_cast<float>( -screenDistance ) ) *
         static_cast<float>( r_right ) *
         static_cast<float>( r_bottom );
    bl = osg::vec3( static_cast<float>( screenWidth * 0.5 * bottom_left.x ),
                    static_cast<float>( screenHeight * 0.5 * bottom_left.y ),
                    static_cast<float>( -screenDistance ) ) *
         static_cast<float>( r_left ) *
         static_cast<float>( r_bottom );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Keystone helper functions
//
struct KeystoneCullCallback
    : public osg::Inherit<osg::DrawableCullCallback, KeystoneCullCallback>
{
        OSG_REGISTER_TYPE( osg,
                           KeystoneCullCallback )

        KeystoneCullCallback( Keystone* keystone = 0 ) :
            _keystone( keystone )
        {
        }

        KeystoneCullCallback( const KeystoneCullCallback& rhs,
                              const osg::CopyOp&          copyop ) :
            Inherit( rhs,
                     copyop )
        {
        }

        /** do customized cull code, return true if drawable should be culled.*/
        virtual bool
        cull( osg::NodeVisitor* /*nv*/,
              osg::Drawable* /*drawable*/,
              osg::RenderInfo* /*renderInfo*/ ) const
        {
            return _keystone.valid() ? !_keystone->getKeystoneEditingEnabled() : true;
        }

        osg::ref_ptr<Keystone> _keystone;
};

struct KeystoneUpdateCallback
    : public osg::Inherit<osg::DrawableUpdateCallback, KeystoneUpdateCallback>
{
        OSG_REGISTER_TYPE( osg,
                           KeystoneUpdateCallback )

        KeystoneUpdateCallback( Keystone* keystone = 0 ) :
            _keystone( keystone )
        {
        }

        KeystoneUpdateCallback( const KeystoneUpdateCallback& rhs,
                                const osg::CopyOp&            copyop ) :
            Inherit( rhs,
                     copyop )
        {
        }

        /** do customized update code.*/
        virtual void
        update( osg::NodeVisitor*,
                osg::Drawable* drawable )
        {
            update( drawable->asGeometry() );
        }

        void
        update( osg::Geometry* geometry )
        {
            if( !geometry )
            {
                return;
            }

            osg::Vec3Array* vertices =
                dynamic_cast<osg::Vec3Array*>( geometry->getVertexArray() );
            if( !vertices )
            {
                return;
            }

            osg::Vec2Array* texcoords =
                dynamic_cast<osg::Vec2Array*>( geometry->getTexCoordArray( 0 ) );
            if( !texcoords )
            {
                return;
            }

            osg::vec3 tl, tr, br, bl;

            _keystone->compute3DPositions( osg::DisplaySettings::instance().get(),
                                           tl,
                                           tr,
                                           br,
                                           bl );

            for( unsigned int i = 0; i < vertices->size(); ++i )
            {
                osg::vec3& v = ( *vertices )[i];
                osg::vec2& t = ( *texcoords )[i];
                v            = bl *
                               ( ( 1.0F - t.x ) * ( 1.0F - t.y ) ) +
                               br *
                               ( ( t.x ) * ( 1.0F - t.y ) ) +
                               tl *
                               ( ( 1.0F - t.x ) * ( t.y ) ) +
                               tr *
                               ( ( t.x ) * ( t.y ) );
            }
            geometry->dirtyBound();
        }

        osg::ref_ptr<Keystone> _keystone;
};

osg::Geode*
Keystone::createKeystoneDistortionMesh()
{
    osg::ref_ptr<osg::Geode>    geode    = new osg::Geode;

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geode->addDrawable( geometry.get() );

    osg::ref_ptr<KeystoneUpdateCallback> kuc = new KeystoneUpdateCallback( this );
    geometry->setUpdateCallback( kuc.get() );

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    colours->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );
    geometry->setColorArray( colours.get(), osg::Array::BIND_OVERALL );

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray( vertices.get() );

    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    geometry->setTexCoordArray( 0, texcoords.get() );

    unsigned int numRows     = 7;
    unsigned int numColumns  = 7;
    unsigned int numVertices = numRows * numColumns;

    vertices->resize( numVertices );
    texcoords->resize( numVertices );

    for( unsigned j = 0; j < numRows; j++ )
    {
        for( unsigned i = 0; i < numColumns; i++ )
        {
            osg::vec2& t = ( *texcoords )[j * numColumns + i];
            t.set( static_cast<float>( i ) / static_cast<float>( numColumns - 1 ),
                   static_cast<float>( j ) / static_cast<float>( numRows - 1 ) );
        }
    }

    osg::ref_ptr<osg::DrawElementsUShort> elements =
        new osg::DrawElementsUShort( GL_TRIANGLES );
    geometry->addPrimitiveSet( elements.get() );
    for( unsigned j = 0; j < numRows - 1; j++ )
    {
        for( unsigned i = 0; i < numColumns - 1; i++ )
        {
            unsigned int vi = j * numColumns + i;

            elements->push_back( static_cast<unsigned short>( vi + numColumns ) );
            elements->push_back( static_cast<unsigned short>( vi ) );
            elements->push_back( static_cast<unsigned short>( vi + 1 ) );

            elements->push_back( static_cast<unsigned short>( vi + numColumns ) );
            elements->push_back( static_cast<unsigned short>( vi + 1 ) );
            elements->push_back( static_cast<unsigned short>( vi + 1 + numColumns ) );
        }
    }

    geometry->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    geometry->getOrCreateStateSet()->setRenderBinDetails( 0, "RenderBin" );
    geometry->getOrCreateStateSet()->setAttribute( new osg::PolygonMode(),
                                                   osg::StateAttribute::ON |
                                                       osg::StateAttribute::PROTECTED );

    kuc->update( geometry.get() );

    return geode.release();
}

osg::Node*
Keystone::createGrid()
{
    osg::ref_ptr<osg::Geode>    geode    = new osg::Geode;

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geode->addDrawable( geometry.get() );

    osg::ref_ptr<KeystoneUpdateCallback> kuc = new KeystoneUpdateCallback( this );
    geometry->setUpdateCallback( kuc.get() );

    geometry->setCullCallback( new KeystoneCullCallback( this ) );

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    colours->push_back( getGridColor() );
    geometry->setColorArray( colours.get(), osg::Array::BIND_OVERALL );

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray( vertices.get() );

    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    geometry->setTexCoordArray( 0, texcoords.get() );

    osg::vec2    origin( 0.0F, 0.0F );
    osg::vec2    widthVector( 1.0F, 0.0F );
    osg::vec2    heightVector( 0.0F, 1.0F );

    unsigned int numIntervals = 7;

    // border line
    {
        unsigned int vi = static_cast<unsigned int>( texcoords->size() );
        texcoords->push_back( origin );
        texcoords->push_back( origin + widthVector );
        texcoords->push_back( origin + widthVector + heightVector );
        texcoords->push_back( origin + heightVector );
        geometry->addPrimitiveSet(
            new osg::DrawArrays( GL_LINE_LOOP, static_cast<GLint>( vi ), 4 )
        );
    }

    // cross lines
    {
        unsigned int vi = static_cast<unsigned int>( texcoords->size() );
        osg::vec2    v  = origin;
        osg::vec2    dv =
            ( widthVector + heightVector ) / static_cast<float>( numIntervals - 1 );
        for( unsigned int i = 0; i < numIntervals; ++i )
        {
            texcoords->push_back( v );
            v += dv;
        }
        geometry->addPrimitiveSet(
            new osg::DrawArrays( GL_LINE_STRIP,
                                 static_cast<GLint>( vi ),
                                 static_cast<GLsizei>( numIntervals ) )
        );

        vi = static_cast<unsigned int>( texcoords->size() );
        v  = origin + heightVector;
        dv = ( widthVector - heightVector ) / static_cast<float>( numIntervals - 1 );
        for( unsigned int i = 0; i < numIntervals; ++i )
        {
            texcoords->push_back( v );
            v += dv;
        }
        geometry->addPrimitiveSet(
            new osg::DrawArrays( GL_LINE_STRIP,
                                 static_cast<GLint>( vi ),
                                 static_cast<GLsizei>( numIntervals ) )
        );
    }

    // vertices lines
    {
        unsigned int vi = static_cast<unsigned int>( texcoords->size() );
        osg::vec2    dv = widthVector / 6.0F;
        osg::vec2    bv = origin + dv;
        osg::vec2    tv = bv + heightVector;
        for( unsigned int i = 0; i < 5; ++i )
        {
            texcoords->push_back( bv );
            texcoords->push_back( tv );
            bv += dv;
            tv += dv;
        }
        geometry->addPrimitiveSet(
            new osg::DrawArrays( GL_LINES, static_cast<GLint>( vi ), 10 )
        );
    }

    // horizontal lines
    {
        unsigned int vi = static_cast<unsigned int>( texcoords->size() );
        osg::vec2    dv = heightVector / 6.0F;
        osg::vec2    bv = origin + dv;
        osg::vec2    tv = bv + widthVector;
        for( unsigned int i = 0; i < 5; ++i )
        {
            texcoords->push_back( bv );
            texcoords->push_back( tv );
            bv += dv;
            tv += dv;
        }
        geometry->addPrimitiveSet(
            new osg::DrawArrays( GL_LINES, static_cast<GLint>( vi ), 10 )
        );
    }

    vertices->resize( texcoords->size() );

    geometry->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    geometry->getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
    geometry->getOrCreateStateSet()->setRenderBinDetails( 1, "RenderBin" );

    kuc->update( geometry.get() );

    return geode.release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// KeystoneHandler
//
KeystoneHandler::KeystoneHandler( Keystone* keystone ) :
    _keystone( keystone ),
    _defaultIncrement( 0.0,
                       0.0 ),
    _ctrlIncrement( 1.0,
                    1.0 ),
    _shiftIncrement( 0.1,
                     0.1 ),
    _keyIncrement( 0.005,
                   0.005 ),
    _selectedRegion( NONE_SELECTED )
{
    _startControlPoints   = new Keystone;
    _currentControlPoints = keystone;    // new Keystone;
}

KeystoneHandler::Region
KeystoneHandler::computeRegion( const osgGA::GUIEventAdapter& ea ) const
{
    float x = ea.getXnormalized();
    float y = ea.getYnormalized();
    if( x < -0.33 )
    {
        // left side
        if( y < -0.33 )
        {
            return BOTTOM_LEFT;
        }
        else if( y < 0.33 )
        {
            return LEFT;
        }
        else
        {
            return TOP_LEFT;
        }
    }
    else if( x < 0.33 )
    {
        // center side
        if( y < -0.33 )
        {
            return BOTTOM;
        }
        else if( y < 0.33 )
        {
            return CENTER;
        }
        else
        {
            return TOP;
        }
    }
    else
    {
        // right side
        if( y < -0.33 )
        {
            return BOTTOM_RIGHT;
        }
        else if( y < 0.33 )
        {
            return RIGHT;
        }
        else
        {
            return TOP_RIGHT;
        }
    }
    return NONE_SELECTED;
}

void
KeystoneHandler::move( Region            region,
                       const osg::dvec2& delta )
{
    switch( region )
    {
        case( TOP_LEFT ) :
            _currentControlPoints->getTopLeft() += delta;
            break;
        case( TOP ) :
            _currentControlPoints->getTopLeft()  += delta;
            _currentControlPoints->getTopRight() += delta;
            break;
        case( TOP_RIGHT ) :
            _currentControlPoints->getTopRight() += delta;
            break;
        case( RIGHT ) :
            _currentControlPoints->getTopRight()    += delta;
            _currentControlPoints->getBottomRight() += delta;
            break;
        case( BOTTOM_RIGHT ) :
            _currentControlPoints->getBottomRight() += delta;
            break;
        case( BOTTOM ) :
            _currentControlPoints->getBottomRight() += delta;
            _currentControlPoints->getBottomLeft()  += delta;
            break;
        case( BOTTOM_LEFT ) :
            _currentControlPoints->getBottomLeft() += delta;
            break;
        case( LEFT ) :
            _currentControlPoints->getBottomLeft() += delta;
            _currentControlPoints->getTopLeft()    += delta;
            break;
        case( CENTER ) :
            _currentControlPoints->getBottomLeft()  += delta;
            _currentControlPoints->getTopLeft()     += delta;
            _currentControlPoints->getBottomRight() += delta;
            _currentControlPoints->getTopRight()    += delta;
            break;
        case( NONE_SELECTED ) :
            break;
    }
}

osg::dvec2
KeystoneHandler::incrementScale( const osgGA::GUIEventAdapter& ea ) const
{
    if( _ctrlIncrement !=
        osg::dvec2( 0.0, 0.0 ) &&
        ( ea.getModKeyMask() ==
          osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL ||
          ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL ) )
    {
        return _ctrlIncrement;
    }
    if( _shiftIncrement !=
        osg::dvec2( 0.0, 0.0 ) &&
        ( ea.getModKeyMask() ==
          osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT ||
          ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_RIGHT_SHIFT ) )
    {
        return _shiftIncrement;
    }
    return _defaultIncrement;
}

bool
KeystoneHandler::handle( const osgGA::GUIEventAdapter& ea,
                         osgGA::GUIActionAdapter& /*aa*/,
                         osg::Object* obj,
                         osg::NodeVisitor* /*nv*/ )
{
    osg::Camera*   camera   = obj ? obj->asCamera() : 0;
    osg::Viewport* viewport = camera ? camera->getViewport() : 0;

    if( !viewport )
    {
        return false;
    }

    if( ea.getEventType() ==
        osgGA::GUIEventAdapter::KEYDOWN &&
        ( ea.getModKeyMask() ==
          osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL ||
          ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL ) )
    {
        if( ea.getUnmodifiedKey() == 'g' )
        {
            setKeystoneEditingEnabled( !getKeystoneEditingEnabled() );
            return true;
        }
        if( ea.getUnmodifiedKey() == 'r' )
        {
            _selectedRegion = NONE_SELECTED;
            _startControlPoints->reset();
            _currentControlPoints->reset();
            return true;
        }
        else if( ea.getUnmodifiedKey() == 's' )
        {
            _keystone->writeToFile();
            return true;
        }
    }

    bool  haveCameraMatch = false;
    float x               = ea.getXnormalized();
    float y               = ea.getYnormalized();
    for( unsigned int i = 0; i < ea.getNumPointerData(); ++i )
    {
        const osgGA::PointerData* pd = ea.getPointerData( i );
        if( pd->object == obj )
        {
            haveCameraMatch = true;
            x               = pd->getXnormalized();
            y               = pd->getYnormalized();
            break;
        }
    }

    if( !haveCameraMatch || !getKeystoneEditingEnabled() )
    {
        return false;
    }

    switch( ea.getEventType() )
    {
        case( osgGA::GUIEventAdapter::PUSH ) :
            {
                osg::dvec2 scale = incrementScale( ea );
                if( osg::length2( scale ) != 0.0 )
                {
                    _selectedRegion          = computeRegion( ea );
                    ( *_startControlPoints ) = ( *_currentControlPoints );
                    _startPosition.set( x, y );
                }
                else
                {
                    _selectedRegion = NONE_SELECTED;
                }
                return false;
            }
        case( osgGA::GUIEventAdapter::DRAG ) :
            {
                if( _selectedRegion != NONE_SELECTED )
                {
                    ( *_currentControlPoints ) = ( *_startControlPoints );
                    osg::dvec2 currentPosition( x, y );
                    osg::dvec2 delta( currentPosition - _startPosition );
                    osg::dvec2 scale = incrementScale( ea );
                    move( _selectedRegion,
                          osg::dvec2( delta.x * scale.x, delta.y * scale.y ) );
                    return true;
                }
                return false;
            }
        case( osgGA::GUIEventAdapter::RELEASE ) :
            {
                _selectedRegion = NONE_SELECTED;
                return false;
            }
        case( osgGA::GUIEventAdapter::KEYDOWN ) :
            {
                if( ea.getKey() == osgGA::GUIEventAdapter::KEY_Up )
                {
                    move( computeRegion( ea ),
                          osg::dvec2( 0.0, _keyIncrement.y * incrementScale( ea ).y ) );
                }
                else if( ea.getKey() == osgGA::GUIEventAdapter::KEY_Down )
                {
                    move( computeRegion( ea ),
                          osg::dvec2( 0.0, -_keyIncrement.y * incrementScale( ea ).y ) );
                }
                else if( ea.getKey() == osgGA::GUIEventAdapter::KEY_Left )
                {
                    move( computeRegion( ea ),
                          osg::dvec2( -_keyIncrement.x * incrementScale( ea ).x, 0.0 ) );
                }
                else if( ea.getKey() == osgGA::GUIEventAdapter::KEY_Right )
                {
                    move( computeRegion( ea ),
                          osg::dvec2( _keyIncrement.x * incrementScale( ea ).x, 0.0 ) );
                }
                else if( ea.getKey() ==
                         osgGA::GUIEventAdapter::KEY_KP_7 ||
                         ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Home )
                {
                    _currentControlPoints->setTopLeft( osg::dvec2( x, y ) );
                }
                else if( ea.getKey() ==
                         osgGA::GUIEventAdapter::KEY_KP_9 ||
                         ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Page_Up )
                {
                    _currentControlPoints->setTopRight( osg::dvec2( x, y ) );
                }
                else if( ea.getKey() ==
                         osgGA::GUIEventAdapter::KEY_KP_3 ||
                         ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Page_Down )
                {
                    _currentControlPoints->setBottomRight( osg::dvec2( x, y ) );
                }
                else if( ea.getKey() ==
                         osgGA::GUIEventAdapter::KEY_KP_1 ||
                         ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_End )
                {
                    _currentControlPoints->setBottomLeft( osg::dvec2( x, y ) );
                }
                return false;
            }
        default :
            return false;
    }
}

bool
Keystone::writeToFile()
{
    std::string filename;
    if( getUserDataContainer() != 0 && getUserValue( "filename", filename ) )
    {
        // we don't want to write the UDC to the keystone file so take a reference to it,
        // and set the pointer to NULL.
        osg::ref_ptr<osg::UserDataContainer> temp_udc = getUserDataContainer();
        setUserDataContainer( 0 );

        OSG_NOTICE << "Writing keystone to: " << filename << std::endl;

        // write the keystone out to disk
        osgDB::writeObjectFile( *this, filename );

        // reassign the UDC
        setUserDataContainer( temp_udc.get() );

        return true;
    }
    else
    {
        return false;
    }
}

bool
Keystone::loadKeystoneFiles( osg::DisplaySettings* ds )
{
    bool keystonesLoaded = false;
    if( !ds->getKeystoneFileNames().empty() )
    {
        for( osg::DisplaySettings::FileNames::iterator itr =
                 ds->getKeystoneFileNames().begin();
             itr != ds->getKeystoneFileNames().end();
             ++itr )
        {
            const std::string&                filename = *itr;
            osg::ref_ptr<osgViewer::Keystone> keystone =
                osgDB::readRefFile<osgViewer::Keystone>( filename );
            if( keystone.valid() )
            {
                keystone->setUserValue( "filename", filename );
                ds->getKeystones().push_back( keystone.get() );
                keystonesLoaded = true;
            }
            else
            {
                OSG_NOTICE << "Creating Keystone for filename entry: " << filename
                           << std::endl;
                keystone = new Keystone;
                keystone->setUserValue( "filename", filename );
                ds->getKeystones().push_back( keystone.get() );
                keystonesLoaded = true;
            }
        }
    }
    return keystonesLoaded;
}
