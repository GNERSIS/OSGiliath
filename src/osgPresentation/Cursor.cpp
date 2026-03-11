/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Custom cursor overlay for presentations. Renders a textured
 * cursor sprite following mouse position during slide shows.
 */
#include <osgPresentation/Cursor>

#include <osg/core/io_utils.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/FileUtils.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/events/EventVisitor.hpp>
#include <osgUtil/culling/CullVisitor.hpp>
#include <osgViewer/core/View.hpp>

using namespace osgPresentation;

Cursor::Cursor() :
    _size( 0.05F ),
    _cursorDirty( true )
{
    setDataVariance( osg::Object::DataVariance::DYNAMIC );
    setCullingActive( false );
    setNumChildrenRequiringEventTraversal( 1 );
    setNumChildrenRequiringUpdateTraversal( 1 );
}

Cursor::Cursor( const std::string& filename,
                float              size ) :
    _cursorDirty( true )
{
    setDataVariance( osg::Object::DataVariance::DYNAMIC );
    setCullingActive( false );
    setNumChildrenRequiringEventTraversal( 1 );
    setNumChildrenRequiringUpdateTraversal( 1 );

    setFilename( filename );
    setSize( size );
}

Cursor::Cursor( const Cursor&      rhs,
                const osg::CopyOp& copyop ) :
    Inherit( rhs,
             copyop ),
    _filename( rhs._filename ),
    _size( rhs._size ),
    _cursorDirty( true )
{
    setDataVariance( osg::Object::DataVariance::DYNAMIC );
    setCullingActive( false );
}

Cursor::~Cursor()
{
}

void
Cursor::initializeCursor()
{
    if( !_cursorDirty )
    {
        return;
    }
    if( _filename.empty() )
    {
        return;
    }

    removeChildren( 0, getNumChildren() - 1 );

    OSG_INFO << "Curosr::initializeCursor()" << std::endl;
    _cursorDirty = false;

    _transform   = new osg::AutoTransform;

    _transform->setAutoRotateMode( osg::AutoTransform::ROTATE_TO_CAMERA );
    _transform->setAutoScaleToScreen( true );

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    osg::ref_ptr<osg::Image> image =
        osgDB::readRefImageFile( osgDB::findDataFile( _filename ) );
    osg::ref_ptr<osg::Texture2D> texture =
        ( image.valid() ) ? new osg::Texture2D( image.get() ) : 0;

    // full cursor
    {
        osg::ref_ptr<osg::Geometry> geom = osg::createTexturedQuadGeometry(
            osg::vec3( -_size * 0.5F, -_size * 0.5F, 0.0F ),
            osg::vec3( _size, 0.0F, 0.0F ),
            osg::vec3( 0.0F, _size, 0.0F )
        );
        geode->addDrawable( geom.get() );

        osg::StateSet* stateset = geom->getOrCreateStateSet();
        stateset->setMode( GL_BLEND,
                           osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
        // GL_LIGHTING removed: not in core profile
        stateset->setRenderBinDetails( 1'001, "DepthSortedBin" );

        if( texture.valid() )
        {
            stateset->setTextureAttributeAndModes( 0,
                                                   texture.get(),
                                                   osg::StateAttribute::ON |
                                                       osg::StateAttribute::PROTECTED );
        }
    }

    {
        osg::ref_ptr<osg::Geometry> geom = osg::createTexturedQuadGeometry(
            osg::vec3( -_size * 0.5F, -_size * 0.5F, 0.0F ),
            osg::vec3( _size, 0.0F, 0.0F ),
            osg::vec3( 0.0F, _size, 0.0F )
        );
        geode->addDrawable( geom.get() );

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 0.25F ) );
        geom->setColorArray( colors, osg::Array::BIND_OVERALL );

        osg::StateSet* stateset = geom->getOrCreateStateSet();
        stateset->setMode( GL_BLEND,
                           osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
        // GL_LIGHTING removed: not in core profile
        stateset->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
        stateset->setRenderBinDetails( 1'000, "DepthSortedBin" );

        if( texture.valid() )
        {
            stateset->setTextureAttributeAndModes( 0,
                                                   texture.get(),
                                                   osg::StateAttribute::ON |
                                                       osg::StateAttribute::PROTECTED );
        }
    }

    _transform->addChild( geode.get() );

    addChild( _transform.get() );
}

void
Cursor::updatePosition()
{
    if( !_camera )
    {
        OSG_INFO
            << "Cursor::updatePosition() : Update position failed, no camera assigned"
            << std::endl;
        return;
    }

    double           distance = 1.0F;

    osgViewer::View* view     = dynamic_cast<osgViewer::View*>( _camera->getView() );
    if( view )
    {
        osg::DisplaySettings* ds = ( view->getDisplaySettings() != 0 )
                                     ? view->getDisplaySettings()
                                     : osg::DisplaySettings::instance().get();

        double                sd = ds->getScreenDistance();
        double                fusionDistance = sd;
        switch( view->getFusionDistanceMode() )
        {
            case( osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE ) :
                fusionDistance = view->getFusionDistanceValue();
                break;
            case( osgUtil::SceneView::PROPORTIONAL_TO_SCREEN_DISTANCE ) :
                fusionDistance *= view->getFusionDistanceValue();
                break;
        }

        distance = fusionDistance;
    }

    osg::dmat4 VP = _camera->getProjectionMatrix() * _camera->getViewMatrix();

    osg::dmat4 inverse_VP;
    inverse_VP = osg::inverse( VP );

    osg::dvec3 eye( 0.0, 0.0, 0.0 );
    osg::dvec3 farpoint( _cursorXY.x, _cursorXY.y, 1.0 );

    osg::dvec3 eye_world      = eye * osg::inverse( _camera->getViewMatrix() );
    osg::dvec3 farpoint_world = farpoint * inverse_VP;

    osg::dvec3 normal         = farpoint_world - eye_world;
    normal                    = osg::normalize( normal );

    osg::dvec3 cursorPosition = eye_world + normal * distance;
    _transform->setPosition( cursorPosition );
}

void
Cursor::traverse( osg::NodeVisitor& nv )
{
    if( nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR )
    {
        if( _cursorDirty )
        {
            initializeCursor();
        }

        // updatePosition();

        // traverse the subgraph
        Group::traverse( nv );
    }
    else if( nv.getVisitorType() == osg::NodeVisitor::EVENT_VISITOR )
    {
        osgGA::EventVisitor* ev = nv.asEventVisitor();
        if( !ev )
        {
            return;
        }

        osgGA::EventQueue::Events& events = ev->getEvents();
        for( osgGA::EventQueue::Events::iterator itr = events.begin();
             itr != events.end();
             ++itr )
        {
            osgGA::GUIEventAdapter* event = ( *itr )->asGUIEventAdapter();
            if( !event )
            {
                continue;
            }

            switch( event->getEventType() )
            {
                case( osgGA::GUIEventAdapter::PUSH ) :
                case( osgGA::GUIEventAdapter::RELEASE ) :
                case( osgGA::GUIEventAdapter::MOVE ) :
                case( osgGA::GUIEventAdapter::DRAG ) :
                    {
                        if( event->getNumPointerData() >= 1 )
                        {
                            const osgGA::PointerData* pd =
                                event->getPointerData( event->getNumPointerData() - 1 );
                            osg::Camera* camera =
                                pd->object.valid() ? pd->object->asCamera() : 0;

                            _cursorXY.set( pd->getXnormalized(), pd->getYnormalized() );
                            _camera = camera;
                        }
                        else
                        {
                            osgViewer::View* view =
                                dynamic_cast<osgViewer::View*>( ev->getActionAdapter() );
                            osg::Camera* camera = ( view != 0 ) ? view->getCamera() : 0;

                            _cursorXY.set( event->getXnormalized(),
                                           event->getYnormalized() );
                            _camera = camera;
                        }
                        break;
                    }
                case( osgGA::GUIEventAdapter::KEYDOWN ) :
                    {
                        if( event->getKey() == 'c' )
                        {
                            for( unsigned int i = 0; i < getNumChildren(); ++i )
                            {
                                osg::Node* node = getChild( i );
                                node->setNodeMask( node->getNodeMask() != 0
                                                       ? 0
                                                       : 0XFF'FF'FF );
                            }
                        }
                        break;
                    }
                default :
                    break;
            }
        }
        Group::traverse( nv );
    }
    else if( nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR )
    {
#if 0
        if (!_camera)
        {
            osgUtil::CullVisitor* cv = nv.asCullVisitor();
            if (cv)
            {
                _camera = cv->getCurrentCamera();
            }
        }
#endif
        if( _camera.valid() )
        {
            updatePosition();
            Group::traverse( nv );
        }
    }
}
