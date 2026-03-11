/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * osganimationmakepath example application
 */
#include <iostream>
#include <osg/core/Inherit.hpp>
#include <osg/core/io_utils.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/Shape.hpp>
#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/maths/compat.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/Program.hpp>
#include <osg/state/Shader.hpp>
#include <osgAnimation/core/Sampler.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

class AnimtkUpdateCallback : public osg::Inherit<osg::NodeCallback, AnimtkUpdateCallback>
{
    public:

        OSG_REGISTER_TYPE( osgAnimation,
                           AnimtkUpdateCallback )

        AnimtkUpdateCallback()
        {
            _sampler    = new osgAnimation::Vec3CubicBezierSampler;
            _playing    = false;
            _lastUpdate = 0;
        }

        AnimtkUpdateCallback( const AnimtkUpdateCallback& val,
                              const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
            Inherit( val,
                     copyop ),
            _sampler( val._sampler ),
            _startTime( val._startTime ),
            _currentTime( val._currentTime ),
            _playing( val._playing ),
            _lastUpdate( val._lastUpdate )
        {
        }

        /** Callback method called by the NodeVisitor when visiting a node.*/
        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            if( nv->getVisitorType() ==
                osg::NodeVisitor::UPDATE_VISITOR &&
                nv->getFrameStamp() &&
                nv->getFrameStamp()->getFrameNumber() != _lastUpdate )
            {

                _lastUpdate  = nv->getFrameStamp()->getFrameNumber();
                _currentTime = osg::Timer::instance()->tick();

                if( _playing && _sampler.get() && _sampler->getKeyframeContainer() )
                {
                    osg::MatrixTransform* transform =
                        dynamic_cast<osg::MatrixTransform*>( node );
                    if( transform )
                    {
                        osg::vec3 result;
                        float     t =
                            osg::Timer::instance()->delta_s( _startTime, _currentTime );
                        float duration =
                            _sampler->getEndTime() - _sampler->getStartTime();
                        t  = fmod( t, duration );
                        t += _sampler->getStartTime();
                        _sampler->getValueAt( t, result );
                        transform->setMatrix( osg::dmat4( osg::translate( result ) ) );
                    }
                }
            }
            // note, callback is responsible for scenegraph traversal so
            // they must call traverse(node,nv) to ensure that the
            // scene graph subtree (and associated callbacks) are traversed.
            traverse( node, nv );
        }

        void
        start()
        {
            _startTime   = osg::Timer::instance()->tick();
            _currentTime = _startTime;
            _playing     = true;
        }

        void
        stop()
        {
            _currentTime = _startTime;
            _playing     = false;
        }

        osg::ref_ptr<osgAnimation::Vec3CubicBezierSampler> _sampler;
        osg::Timer_t                                       _startTime;
        osg::Timer_t                                       _currentTime;
        bool                                               _playing;
        unsigned int                                       _lastUpdate;
};

class AnimtkStateSetUpdateCallback
    : public osg::Inherit<osg::StateSet::Callback, AnimtkStateSetUpdateCallback>
{
    public:

        OSG_REGISTER_TYPE( osgAnimation,
                           AnimtkStateSetUpdateCallback )

        AnimtkStateSetUpdateCallback()
        {
            _sampler    = new osgAnimation::Vec4LinearSampler;
            _playing    = false;
            _lastUpdate = 0;
        }

        AnimtkStateSetUpdateCallback( const AnimtkStateSetUpdateCallback& val,
                                      const osg::CopyOp&                  copyop =
                                          osg::CopyOp::SHALLOW_COPY ) :
            Inherit( val,
                     copyop ),
            _sampler( val._sampler ),
            _startTime( val._startTime ),
            _currentTime( val._currentTime ),
            _playing( val._playing ),
            _lastUpdate( val._lastUpdate )
        {
        }

        /** Callback method called by the NodeVisitor when visiting a node.*/
        virtual void
        operator()( osg::StateSet*    state,
                    osg::NodeVisitor* nv )
        {
            if( state &&
                nv->getVisitorType() ==
                osg::NodeVisitor::UPDATE_VISITOR &&
                nv->getFrameStamp() &&
                nv->getFrameStamp()->getFrameNumber() != _lastUpdate )
            {

                _lastUpdate  = nv->getFrameStamp()->getFrameNumber();
                _currentTime = osg::Timer::instance()->tick();

                if( _playing && _sampler.get() && _sampler->getKeyframeContainer() )
                {
                    osg::Material* material = dynamic_cast<osg::Material*>(
                        state->getAttribute( osg::StateAttribute::Type::MATERIAL )
                    );
                    if( material )
                    {
                        osg::vec4 result;
                        float     t =
                            osg::Timer::instance()->delta_s( _startTime, _currentTime );
                        float duration =
                            _sampler->getEndTime() - _sampler->getStartTime();
                        t  = fmod( t, duration );
                        t += _sampler->getStartTime();
                        _sampler->getValueAt( t, result );
                        material->setDiffuse( osg::Material::FRONT_AND_BACK, result );
                    }
                }
            }
        }

        void
        start()
        {
            _startTime   = osg::Timer::instance()->tick();
            _currentTime = _startTime;
            _playing     = true;
        }

        void
        stop()
        {
            _currentTime = _startTime;
            _playing     = false;
        }

        osg::ref_ptr<osgAnimation::Vec4LinearSampler> _sampler;
        osg::Timer_t                                  _startTime;
        osg::Timer_t                                  _currentTime;
        bool                                          _playing;
        unsigned int                                  _lastUpdate;
};

// This won't really give good results in any situation, but it does demonstrate
// on possible "fast" usage...
class MakePathTimeCallback : public AnimtkUpdateCallback
{
        osg::ref_ptr<osg::Geode> _geode;
        float                    _lastAdd;
        float                    _addSeconds;

    public:

        MakePathTimeCallback( osg::Geode* geode ) :
            _geode( geode ),
            _lastAdd( 0.0F ),
            _addSeconds( 0.08F )
        {
        }

        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            float t = osg::Timer::instance()->delta_s( _startTime, _currentTime );

            if( _lastAdd + _addSeconds <= t && t <= 8.0F )
            {
                osg::vec3 pos;

                _sampler->getValueAt( t, pos );

                _geode->addDrawable( new osg::ShapeDrawable( new osg::Sphere( pos,
                                                                              0.5F ) ) );
                _geode->dirtyBound();

                _lastAdd += _addSeconds;
            }

            AnimtkUpdateCallback::operator()( node, nv );
        }
};

// This will give great results if you DO NOT have VSYNC enabled and can generate
// decent FPS.
class MakePathDistanceCallback : public AnimtkUpdateCallback
{
        osg::ref_ptr<osg::Geode> _geode;
        osg::vec3                _lastAdd;
        float                    _threshold;
        unsigned int             _count;

    public:

        MakePathDistanceCallback( osg::Geode* geode ) :
            _geode( geode ),
            _threshold( 0.5F ),
            _count( 0 )
        {
        }

        virtual void
        operator()( osg::Node*        node,
                    osg::NodeVisitor* nv )
        {
            static bool countReported = false;

            float       t = osg::Timer::instance()->delta_s( _startTime, _currentTime );

            osg::vec3   pos;

            _sampler->getValueAt( t, pos );

            osg::vec3 distance = _lastAdd - pos;

            if( t <= 8.0F && osg::length( distance ) >= _threshold )
            {
                _geode->addDrawable(
                    new osg::ShapeDrawable( new osg::Sphere( pos, 0.25F ) )
                );
                _lastAdd = pos;
                _count++;
            }
            else if( t > 8.0F )
            {
                if( !countReported )
                {
                    std::cout << "Created " << _count << " nodes." << std::endl;
                }
                countReported = true;
            }

            AnimtkUpdateCallback::operator()( node, nv );
        }
};

osg::StateSet*
setupStateSet()
{
    osg::StateSet* st = new osg::StateSet();

    st->setAttributeAndModes( new osg::Material(), true );
    st->setMode( GL_BLEND, true );

    AnimtkStateSetUpdateCallback*        callback = new AnimtkStateSetUpdateCallback();
    osgAnimation::Vec4KeyframeContainer* keys =
        callback->_sampler->getOrCreateKeyframeContainer();
    keys->push_back( osgAnimation::Vec4Keyframe( 0, osg::vec4( 1, 0, 0, 1 ) ) );
    keys->push_back( osgAnimation::Vec4Keyframe( 2, osg::vec4( 0., 1, 0, 1 ) ) );
    keys->push_back( osgAnimation::Vec4Keyframe( 4, osg::vec4( 0, 0, 1, 1 ) ) );
    keys->push_back( osgAnimation::Vec4Keyframe( 6, osg::vec4( 0, 0, 1, 1 ) ) );
    keys->push_back( osgAnimation::Vec4Keyframe( 8, osg::vec4( 0, 1, 0, 1 ) ) );
    keys->push_back( osgAnimation::Vec4Keyframe( 10, osg::vec4( 1, 0, 0, 1 ) ) );
    callback->start();
    st->setUpdateCallback( callback );

    return st;
}

osg::MatrixTransform*
setupAnimtkNode( osg::Geode* staticGeode )
{
    osg::vec3 v[5];

    v[0]                           = osg::vec3( 0, 0, 0 );
    v[1]                           = osg::vec3( 20, 40, 60 );
    v[2]                           = osg::vec3( 40, 60, 20 );
    v[3]                           = osg::vec3( 60, 20, 40 );
    v[4]                           = osg::vec3( 0, 0, 0 );

    osg::MatrixTransform* node     = new osg::MatrixTransform();
    AnimtkUpdateCallback* callback = new MakePathDistanceCallback( staticGeode );
    osgAnimation::Vec3CubicBezierKeyframeContainer* keys =
        callback->_sampler->getOrCreateKeyframeContainer();

    keys->push_back( osgAnimation::Vec3CubicBezierKeyframe(
        0,
        osgAnimation::Vec3CubicBezier( v[0],
                                       v[0] + ( v[0] - v[3] ),
                                       v[1] - ( v[1] - v[0] ) )
    ) );

    keys->push_back( osgAnimation::Vec3CubicBezierKeyframe(
        2,
        osgAnimation::Vec3CubicBezier( v[1],
                                       v[1] + ( v[1] - v[0] ),
                                       v[2] - ( v[2] - v[1] ) )
    ) );

    keys->push_back( osgAnimation::Vec3CubicBezierKeyframe(
        4,
        osgAnimation::Vec3CubicBezier( v[2],
                                       v[2] + ( v[2] - v[1] ),
                                       v[3] - ( v[3] - v[2] ) )
    ) );

    keys->push_back( osgAnimation::Vec3CubicBezierKeyframe(
        6,
        osgAnimation::Vec3CubicBezier( v[3],
                                       v[3] + ( v[3] - v[2] ),
                                       v[4] - ( v[4] - v[3] ) )
    ) );

    keys->push_back( osgAnimation::Vec3CubicBezierKeyframe(
        8,
        osgAnimation::Vec3CubicBezier( v[4],
                                       v[4] + ( v[4] - v[3] ),
                                       v[0] - ( v[0] - v[4] ) )
    ) );

    callback->start();
    node->setUpdateCallback( callback );

    osg::Geode* geode = new osg::Geode();

    geode->setStateSet( setupStateSet() );
    geode->addDrawable(
        new osg::ShapeDrawable( new osg::Sphere( osg::vec3( 0.0F, 0.0F, 0.0F ), 2 ) )
    );

    node->addChild( geode );

    return node;
}

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

    osgViewer::Viewer            viewer( arguments );

    osgGA::TrackballManipulator* tbm = new osgGA::TrackballManipulator();

    viewer.setCameraManipulator( tbm );

    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler() );

    osg::Group* root  = new osg::Group();
    osg::Geode* geode = new osg::Geode();

    geode->setStateSet( setupStateSet() );

    root->setInitialBound( osg::sphere( osg::vec3( 10, 0, 20 ), 50 ) );
    root->addChild( setupAnimtkNode( geode ) );
    root->addChild( geode );

    // Add GL 4.6 core shaders
    {
        osg::Program* program = new osg::Program;
        program->setName( "AnimationMakePathShader" );

        program->addShader( new osg::Shader(
            osg::Shader::VERTEX,
            "#version 460 core\n"
            "layout(location = 0) in vec4 osg_Vertex;\n"
            "layout(location = 2) in vec3 osg_Normal;\n"
            "layout(location = 3) in vec4 osg_Color;\n"
            "uniform mat4 osg_ModelViewMatrix;\n"
            "uniform mat4 osg_ModelViewProjectionMatrix;\n"
            "uniform mat3 osg_NormalMatrix;\n"
            "out vec3 vNormal;\n"
            "out vec3 vFragPos;\n"
            "out vec4 vColor;\n"
            "void main() {\n"
            "    vFragPos = vec3(osg_ModelViewMatrix * osg_Vertex);\n"
            "    vNormal = normalize(osg_NormalMatrix * osg_Normal);\n"
            "    vColor = osg_Color;\n"
            "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
            "}\n"
        ) );

        program->addShader( new osg::Shader(
            osg::Shader::FRAGMENT,
            "#version 460 core\n"
            "in vec3 vNormal;\n"
            "in vec3 vFragPos;\n"
            "in vec4 vColor;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.8));\n"
            "    float diff = max(dot(normalize(vNormal), lightDir), 0.0) * 0.7 + 0.3;\n"
            "    fragColor = vec4(vColor.rgb * diff, vColor.a);\n"
            "}\n"
        ) );

        root->getOrCreateStateSet()->setAttributeAndModes( program,
                                                           osg::StateAttribute::ON );
    }

    viewer.setSceneData( root );

    // tbm->setDistance(150);
    return viewer.run();
}
