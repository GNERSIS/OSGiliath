/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osganimationeasemotion example application
 */
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osgAnimation/core/EaseMotion.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>
#include <osgWidget/Box>
#include <osgWidget/Label>
#include <osgWidget/Table>
#include <osgWidget/WindowManager>

class EaseMotionSampler;

const unsigned int WINDOW_WIDTH        = 800;
const unsigned int WINDOW_HEIGHT       = 600;
const unsigned int MASK_2D             = 0XF0'00'00'00;
const unsigned int MASK_3D             = 0X0F'00'00'00;
const float        M_START             = 0.0F;
const float        M_DURATION          = 2.0F;
const float        M_CHANGE            = 1.0F;

EaseMotionSampler* EASE_MOTION_SAMPLER = 0;
osg::Geode*        EASE_MOTION_GEODE   = 0;

osg::Geometry*
createEaseMotionGeometry( osgAnimation::Motion* motion )
{
    osg::Geometry*  geom = new osg::Geometry();
    osg::Vec4Array* cols = new osg::Vec4Array();
    osg::Vec3Array* v    = new osg::Vec3Array();

    for( float i = 0.0F; i < M_DURATION; i += M_DURATION / 256.0F )
    {
        v->push_back( osg::vec3( i * 30.0F, motion->getValueAt( i ) * 30.0F, 0.0F ) );
    }

    cols->push_back( osg::vec4( 1.0F, 1.0F, 1.0F, 1.0F ) );

    geom->setVertexArray( v );
    geom->setColorArray( cols, osg::Array::BIND_OVERALL );
    geom->addPrimitiveSet(
        new osg::DrawArrays( osg::PrimitiveSet::LINE_STRIP, 0, v->size() )
    );

    return geom;
}

class EaseMotionSampler : public osg::NodeCallback
{
    public:

        float                              _previous;
        osg::vec3                          _pos;

        osg::ref_ptr<osgAnimation::Motion> _motion;

        EaseMotionSampler( const osg::vec3& pos ) :
            _previous( 0.0F ),
            _pos( pos )
        {
        }

        void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            if( !_motion.valid() )
            {
                return;
            }

            osg::MatrixTransform* mt = dynamic_cast<osg::MatrixTransform*>( node );

            if( !mt )
            {
                return;
            }

            double t = nv->getFrameStamp()->getSimulationTime();

            // This avoids a little glitch when the animation doesn't start right away
            // when the application is launched.
            if( _previous == 0.0F )
            {
                _previous = t;
            }

            _motion->update( t - _previous );

            _previous = t;

            mt->setMatrix( osg::dmat4( osg::translate( _pos * _motion->getValue() ) ) );
        }

        template<typename T>
        void
        setMotion()
        {
            _motion = new T( M_START, M_DURATION, M_CHANGE, osgAnimation::Motion::LOOP );

            EASE_MOTION_GEODE->removeDrawables( 0,
                                                EASE_MOTION_GEODE->getNumDrawables() );
            EASE_MOTION_GEODE->addDrawable( createEaseMotionGeometry( _motion.get() ) );
        }
};

struct ColorLabel : public osgWidget::Label
{
        ColorLabel( const char* label ) :
            osgWidget::Label( label,
                              "" )
        {
            setFont( "fonts/VeraMono.ttf" );
            setFontSize( 14 );
            setFontColor( 1.0F, 1.0F, 1.0F, 1.0F );

            setColor( 0.3F, 0.3F, 0.3F, 1.0F );
            setPadding( 2.0F );
            setCanFill( true );

            addSize( 150.0F, 25.0F );

            setLabel( label );
            setEventMask( static_cast<int>( osgWidget::EVENT_MOUSE_PUSH ) |
                          osgWidget::EVENT_MASK_MOUSE_MOVE );
        }

        bool
        mousePush( double,
                   double,
                   const osgWidget::WindowManager* )
        {
            osgWidget::Table* p = dynamic_cast<osgWidget::Table*>( _parent );

            if( !p )
            {
                return false;
            }

            p->hide();

            const std::string& name = getName();

            if( !name.compare( "OutQuadMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::OutQuadMotion>();
            }

            else if( !name.compare( "InQuadMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InQuadMotion>();
            }

            else if( !name.compare( "InOutQuadMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InOutQuadMotion>();
            }

            else if( !name.compare( "OutCubicMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::OutCubicMotion>();
            }

            else if( !name.compare( "InCubicMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InCubicMotion>();
            }

            else if( !name.compare( "InOutCubicMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InOutCubicMotion>();
            }

            else if( !name.compare( "OutQuartMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::OutQuartMotion>();
            }

            else if( !name.compare( "InQuartMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InQuartMotion>();
            }

            else if( !name.compare( "InOutQuartMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InOutQuartMotion>();
            }

            else if( !name.compare( "OutBounceMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::OutBounceMotion>();
            }

            else if( !name.compare( "InBounceMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InBounceMotion>();
            }

            else if( !name.compare( "InOutBounceMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InOutBounceMotion>();
            }

            else if( !name.compare( "OutElasticMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::OutElasticMotion>();
            }

            else if( !name.compare( "InElasticMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InElasticMotion>();
            }

            else if( !name.compare( "InOutElasticMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InOutElasticMotion>();
            }

            else if( !name.compare( "OutSineMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::OutSineMotion>();
            }

            else if( !name.compare( "InSineMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InSineMotion>();
            }

            else if( !name.compare( "InOutSineMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InOutSineMotion>();
            }

            else if( !name.compare( "OutBackMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::OutBackMotion>();
            }

            else if( !name.compare( "InBackMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InBackMotion>();
            }

            else if( !name.compare( "InOutBackMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InOutBackMotion>();
            }

            else if( !name.compare( "OutCircMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::OutCircMotion>();
            }

            else if( !name.compare( "InCircMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InCircMotion>();
            }

            else if( !name.compare( "InOutCircMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InOutCircMotion>();
            }

            else if( !name.compare( "OutExpoMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::OutExpoMotion>();
            }

            else if( !name.compare( "InExpoMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InExpoMotion>();
            }

            else if( !name.compare( "InOutExpoMotion" ) )
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::InOutExpoMotion>();
            }

            else
            {
                EASE_MOTION_SAMPLER->setMotion<osgAnimation::LinearMotion>();
            }

            return true;
        }

        bool
        mouseEnter( double,
                    double,
                    const osgWidget::WindowManager* )
        {
            setColor( 0.9F, 0.6F, 0.1F, 1.0F );

            return true;
        }

        bool
        mouseLeave( double,
                    double,
                    const osgWidget::WindowManager* )
        {
            setColor( 0.3F, 0.3F, 0.3F, 1.0F );

            return true;
        }
};

class ColorLabelMenu : public ColorLabel
{
        osg::ref_ptr<osgWidget::Table> _window;

    public:

        ColorLabelMenu( const char* label ) :
            ColorLabel( label )
        {
            _window = new osgWidget::Table( std::string( "Menu_" ) + label, 6, 5 );

            _window->addWidget( new ColorLabel( "OutQuadMotion" ), 0, 0 );
            _window->addWidget( new ColorLabel( "InQuadMotion" ), 1, 0 );
            _window->addWidget( new ColorLabel( "InOutQuadMotion" ), 2, 0 );
            _window->addWidget( new ColorLabel( "OutCubicMotion" ), 3, 0 );
            _window->addWidget( new ColorLabel( "InCubicMotion" ), 4, 0 );
            _window->addWidget( new ColorLabel( "InOutCubicMotion" ), 5, 0 );

            _window->addWidget( new ColorLabel( "OutQuartMotion" ), 0, 1 );
            _window->addWidget( new ColorLabel( "InQuartMotion" ), 1, 1 );
            _window->addWidget( new ColorLabel( "InOutQuartMotion" ), 2, 1 );
            _window->addWidget( new ColorLabel( "OutBounceMotion" ), 3, 1 );
            _window->addWidget( new ColorLabel( "InBounceMotion" ), 4, 1 );
            _window->addWidget( new ColorLabel( "InOutBounceMotion" ), 5, 1 );

            _window->addWidget( new ColorLabel( "OutElasticMotion" ), 0, 2 );
            _window->addWidget( new ColorLabel( "InElasticMotion" ), 1, 2 );
            _window->addWidget( new ColorLabel( "InOutElasticMotion" ), 2, 2 );
            _window->addWidget( new ColorLabel( "OutSineMotion" ), 3, 2 );
            _window->addWidget( new ColorLabel( "InSineMotion" ), 4, 2 );
            _window->addWidget( new ColorLabel( "InOutSineMotion" ), 5, 2 );

            _window->addWidget( new ColorLabel( "OutBackMotion" ), 0, 3 );
            _window->addWidget( new ColorLabel( "InBackMotion" ), 1, 3 );
            _window->addWidget( new ColorLabel( "InOutBackMotion" ), 2, 3 );
            _window->addWidget( new ColorLabel( "OutCircMotion" ), 3, 3 );
            _window->addWidget( new ColorLabel( "InCircMotion" ), 4, 3 );
            _window->addWidget( new ColorLabel( "InOutCircMotion" ), 5, 3 );

            _window->addWidget( new ColorLabel( "OutExpoMotion" ), 0, 4 );
            _window->addWidget( new ColorLabel( "InExpoMotion" ), 1, 4 );
            _window->addWidget( new ColorLabel( "InOutExpoMotion" ), 2, 4 );
            _window->addWidget( new ColorLabel( "Linear" ), 3, 4 );

            _window->resize();
        }

        void
        managed( osgWidget::WindowManager* wm )
        {
            osgWidget::Label::managed( wm );

            wm->addChild( _window.get() );

            _window->hide();
        }

        void
        positioned()
        {
            osgWidget::Label::positioned();

            _window->setOrigin( _parent->getX(),
                                _parent->getY() + _parent->getHeight() );
        }

        bool
        mousePush( double,
                   double,
                   const osgWidget::WindowManager* )
        {
            if( !_window->isVisible() )
            {
                _window->show();
            }

            else
            {
                _window->hide();
            }

            return true;
        }
};

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    // Headless capture: early check before viewer construction
    {
        std::string headlessOutput;
        if( arguments.read( "--headless", headlessOutput ) )
        {
            auto node = osgDB::readRefNodeFiles( arguments );
            if( !node )
            {
                node = osgDB::readRefNodeFile( "duck.glb" );
            }
            return osg::headlessCapture( node.get(), headlessOutput, 640, 480 ) ? 0 : 1;
        }
    }

    osgViewer::Viewer         viewer;

    osgWidget::WindowManager* wm =
        new osgWidget::WindowManager( &viewer, WINDOW_WIDTH, WINDOW_HEIGHT, MASK_2D );

    osgWidget::Window* menu = new osgWidget::Box( "menu", osgWidget::Box::HORIZONTAL );

    menu->addWidget( new ColorLabelMenu( "Choose EaseMotion" ) );
    menu->getBackground()->setColor( 1.0F, 1.0F, 1.0F, 1.0F );
    menu->setPosition( 15.0F, 15.0F, 0.0F );

    wm->addChild( menu );

    osg::Group*           group = new osg::Group();
    osg::Geode*           geode = new osg::Geode();
    osg::MatrixTransform* mt    = new osg::MatrixTransform();

    geode->addDrawable( new osg::ShapeDrawable( new osg::Sphere( osg::vec3(), 4.0F ) ) );

    EASE_MOTION_SAMPLER = new EaseMotionSampler( osg::vec3( 50.0F, 0.0F, 0.0F ) );
    EASE_MOTION_GEODE   = new osg::Geode();

    mt->addChild( geode );
    mt->setUpdateCallback( EASE_MOTION_SAMPLER );
    mt->setNodeMask( MASK_3D );

    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.getCameraManipulator()->setHomePosition( osg::dvec3( 0.0F, 0.0F, 200.0F ),
                                                    osg::dvec3( 20.0F, 0.0F, 0.0F ),
                                                    osg::dvec3( 0.0F, 1.0F, 0.0F ) );
    viewer.home();

    group->addChild( mt );
    group->addChild( EASE_MOTION_GEODE );

    return osgWidget::createExample( viewer, wm, group );
}
