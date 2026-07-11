/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include "MainPass.hpp"
#include "ShadowPass.hpp"
#include "SponzaCameras.hpp"
#include "SponzaFrameContext.hpp"
#include "SponzaGpuRayScene.hpp"
#include "SponzaLighting.hpp"
#include "SponzaOptions.hpp"
#include "SponzaRayScene.hpp"
#include "SponzaTargets.hpp"
#include "SponzaVisibilityBake.hpp"
#include "SsaoPass.hpp"
#include "TonemapPass.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <deque>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <osg/core/ArgumentParser.hpp>
#include <osg/core/Callback.hpp>
#include <osg/core/Notify.hpp>
#include <osg/images/Image.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/transform.hpp>
#include <osg/nodes/Camera.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/rendering/HeadlessCapture.hpp>
#include <osg/rendering/RenderInfo.hpp>
#include <osg/state/StateAttribute.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/state/Uniform.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgDB/io/ReadFile.hpp>
#include <osgDB/serialization/SceneCook.hpp>
#include <osgDB/serialization/TextureCompression.hpp>
#include <osgGA/events/GUIEventHandler.hpp>
#include <osgGA/handlers/StateSetManipulator.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/handlers/ViewerEventHandlers.hpp>

namespace
{

    constexpr double       worldUpY            = 1.0;
    constexpr double       cameraMoveSpeed     = 4.0;
    constexpr double       cameraFastMoveSpeed = 14.0;
    constexpr double       mouseSensitivity    = 0.0025;
    constexpr double       maxPitchRadians     = 1.5533430342749532;    // 89 degrees.
    constexpr double       maxFrameDelta       = 0.1;
    constexpr unsigned int gpuProfileReportInterval = 30;

    template<typename T>
    void
    setOverrideUniform( osg::StateSet* stateSet,
                        const char*    name,
                        const T&       value )
    {
        if( stateSet == nullptr )
        {
            return;
        }

        osg::Uniform* uniform = stateSet->getUniform( name );
        if( uniform != nullptr )
        {
            uniform->set( value );
            return;
        }

        stateSet->addUniform( new osg::Uniform( name, value ),
                              osg::StateAttribute::OVERRIDE );
    }

    int
    normalizedKey( int key )
    {
        if( key >= 'A' && key <= 'Z' )
        {
            return key - 'A' + 'a';
        }
        return key;
    }

    osg::vec3
    toVec3( const osg::dvec3& value )
    {
        return osg::vec3( static_cast<float>( value.x ),
                          static_cast<float>( value.y ),
                          static_cast<float>( value.z ) );
    }

    void
    resizeTexture( osg::Texture2D* texture,
                   int             width,
                   int             height )
    {
        if( texture == nullptr )
        {
            return;
        }
        if( texture->getTextureWidth() ==
            width &&
            texture->getTextureHeight() == height )
        {
            return;
        }

        texture->setTextureSize( width, height );
        texture->dirtyTextureObject();
    }

    void
    setCameraViewport( osg::Camera* camera,
                       int          width,
                       int          height )
    {
        if( camera != nullptr )
        {
            camera->setViewport( 0, 0, width, height );
        }
    }

    int
    scaledResolveDimension( int   outputDimension,
                            float resolveScale )
    {
        return std::max( static_cast<int>( std::lround(
                             static_cast<double>( std::max( outputDimension, 1 ) ) *
                             static_cast<double>( resolveScale )
                         ) ),
                         1 );
    }

    osg::dvec3
    horizontalForward( const osg::dvec3& forward )
    {
        osg::dvec3 result( forward.x, 0.0, forward.z );
        if( osg::length( result ) < 1.0E-6 )
        {
            return osg::dvec3( 1.0, 0.0, 0.0 );
        }
        return osg::normalize( result );
    }

    class SponzaGpuPassTimer : public osg::Referenced
    {
        public:

            SponzaGpuPassTimer( std::string  passName,
                                unsigned int firstMeasuredFrame,
                                unsigned int lastMeasuredFrame ) :
                _passName( std::move( passName ) ),
                _firstMeasuredFrame( firstMeasuredFrame ),
                _lastMeasuredFrame( lastMeasuredFrame )
            {
            }

            void
            begin( osg::RenderInfo& renderInfo ) const
            {
                osg::State* state = renderInfo.getState();
                if( state == nullptr )
                {
                    return;
                }

                osg::GLExtensions* extensions =
                    osg::GLExtensions::Get( state->getContextID(), true );
                if( extensions ==
                    nullptr ||
                    extensions->glGenQueries ==
                    nullptr ||
                    extensions->glBeginQuery ==
                    nullptr ||
                    extensions->glGetQueryObjectiv ==
                    nullptr ||
                    extensions->glGetQueryObjectui64v == nullptr )
                {
                    return;
                }

                const unsigned int frameNumber = currentFrameNumber( state );
                drainAvailableQueries( extensions, false );
                maybeReportFinal( frameNumber );

                if( !shouldMeasureFrame( frameNumber ) || _activeQuery != 0 )
                {
                    return;
                }

                extensions->glGenQueries( 1, &_activeQuery );
                extensions->glBeginQuery( GL_TIME_ELAPSED, _activeQuery );
            }

            void
            end( osg::RenderInfo& renderInfo ) const
            {
                if( _activeQuery == 0 )
                {
                    return;
                }

                osg::State* state = renderInfo.getState();
                if( state == nullptr )
                {
                    return;
                }

                osg::GLExtensions* extensions =
                    osg::GLExtensions::Get( state->getContextID(), true );
                if( extensions == nullptr || extensions->glEndQuery == nullptr )
                {
                    _activeQuery = 0;
                    return;
                }

                extensions->glEndQuery( GL_TIME_ELAPSED );
                _pendingQueries.push_back( _activeQuery );
                _activeQuery = 0;
            }

            void
            releaseQueries( osg::State* state ) const
            {
                if( state == nullptr )
                {
                    return;
                }

                osg::GLExtensions* extensions =
                    osg::GLExtensions::Get( state->getContextID(), false );
                if( extensions == nullptr || extensions->glDeleteQueries == nullptr )
                {
                    return;
                }

                if( _activeQuery != 0 )
                {
                    extensions->glDeleteQueries( 1, &_activeQuery );
                    _activeQuery = 0;
                }

                while( !_pendingQueries.empty() )
                {
                    const GLuint query = _pendingQueries.front();
                    extensions->glDeleteQueries( 1, &query );
                    _pendingQueries.pop_front();
                }
            }

        protected:

            ~SponzaGpuPassTimer() override = default;

        private:

            static unsigned int
            currentFrameNumber( const osg::State* state )
            {
                const osg::FrameStamp* frameStamp =
                    state != nullptr ? state->getFrameStamp() : nullptr;
                return frameStamp != nullptr ? frameStamp->getFrameNumber() : 0U;
            }

            bool
            shouldMeasureFrame( unsigned int frameNumber ) const
            {
                if( frameNumber < _firstMeasuredFrame )
                {
                    return false;
                }
                return _lastMeasuredFrame == 0U || frameNumber <= _lastMeasuredFrame;
            }

            void
            drainAvailableQueries( osg::GLExtensions* extensions,
                                   bool               force ) const
            {
                while( !_pendingQueries.empty() )
                {
                    const GLuint query     = _pendingQueries.front();
                    GLint        available = 0;
                    if( !force )
                    {
                        extensions->glGetQueryObjectiv( query,
                                                        GL_QUERY_RESULT_AVAILABLE,
                                                        &available );
                        if( available == 0 )
                        {
                            return;
                        }
                    }

                    GLuint64 elapsedNanoseconds = 0;
                    extensions->glGetQueryObjectui64v( query,
                                                       GL_QUERY_RESULT,
                                                       &elapsedNanoseconds );
                    _pendingQueries.pop_front();

                    const double milliseconds =
                        static_cast<double>( elapsedNanoseconds ) * 1.0E-6;
                    _sampleCount       += 1U;
                    _totalMilliseconds += milliseconds;
                    _maxMilliseconds    = std::max( _maxMilliseconds, milliseconds );

                    if( _sampleCount % gpuProfileReportInterval == 0U )
                    {
                        report( false );
                    }
                }
            }

            void
            maybeReportFinal( unsigned int frameNumber ) const
            {
                if( _reportedFinal ||
                    _lastMeasuredFrame ==
                    0U ||
                    frameNumber <= _lastMeasuredFrame )
                {
                    return;
                }

                if( _sampleCount > 0U )
                {
                    report( true );
                    _reportedFinal = true;
                }
            }

            void
            report( bool finalReport ) const
            {
                const double average =
                    _sampleCount > 0U
                        ? _totalMilliseconds / static_cast<double>( _sampleCount )
                        : 0.0;

                std::cout << "sponza gpu pass" << ( finalReport ? " final" : "" ) << ": "
                          << _passName << " samples=" << _sampleCount
                          << " avg_ms=" << std::fixed << std::setprecision( 3 )
                          << average << " max_ms=" << _maxMilliseconds << std::endl;
            }

            std::string                _passName;
            unsigned int               _firstMeasuredFrame;
            unsigned int               _lastMeasuredFrame;
            mutable GLuint             _activeQuery = 0;
            mutable std::deque<GLuint> _pendingQueries;
            mutable unsigned int       _sampleCount       = 0;
            mutable double             _totalMilliseconds = 0.0;
            mutable double             _maxMilliseconds   = 0.0;
            mutable bool               _reportedFinal     = false;
    };

    class SponzaGpuPassBeginCallback : public osg::Camera::DrawCallback
    {
        public:

            explicit SponzaGpuPassBeginCallback( SponzaGpuPassTimer* timer ) :
                _timer( timer )
            {
            }

            void
            operator()( osg::RenderInfo& renderInfo ) const override
            {
                if( _timer.valid() )
                {
                    _timer->begin( renderInfo );
                }
            }

            void
            releaseGLObjects( osg::State* state = nullptr ) const override
            {
                if( _timer.valid() )
                {
                    _timer->releaseQueries( state );
                }
                osg::Camera::DrawCallback::releaseGLObjects( state );
            }

        private:

            osg::ref_ptr<SponzaGpuPassTimer> _timer;
    };

    class SponzaGpuPassEndCallback : public osg::Camera::DrawCallback
    {
        public:

            explicit SponzaGpuPassEndCallback( SponzaGpuPassTimer* timer ) :
                _timer( timer )
            {
            }

            void
            operator()( osg::RenderInfo& renderInfo ) const override
            {
                if( _timer.valid() )
                {
                    _timer->end( renderInfo );
                }
            }

        private:

            osg::ref_ptr<SponzaGpuPassTimer> _timer;
    };

    void
    attachGpuPassTimer( osg::Camera*                 camera,
                        const std::string&           passName,
                        const sponza::SponzaOptions& options )
    {
        if( camera == nullptr || !options.gpuProfileEnabled )
        {
            return;
        }

        const unsigned int firstFrame =
            options.benchmarkFrames > 0
                ? static_cast<unsigned int>( options.benchmarkWarmupFrames + 1 )
                : 0U;
        const unsigned int lastFrame =
            options.benchmarkFrames > 0
                ? static_cast<unsigned int>( options.benchmarkWarmupFrames +
                                             options.benchmarkFrames )
                : 0U;
        osg::ref_ptr<SponzaGpuPassTimer> timer =
            new SponzaGpuPassTimer( passName, firstFrame, lastFrame );
        camera->addPreDrawCallback( new SponzaGpuPassBeginCallback( timer.get() ) );
        camera->addPostDrawCallback( new SponzaGpuPassEndCallback( timer.get() ) );
    }

    class SponzaDiagnosticsStateManipulator : public osgGA::StateSetManipulator
    {
        public:

            explicit SponzaDiagnosticsStateManipulator( osg::StateSet* stateSet ) :
                osgGA::StateSetManipulator( stateSet )
            {
                setKeyEventCyclePolygonMode( osgGA::GUIEventAdapter::KEY_Tab );
            }

            void
            getUsage( osg::ApplicationUsage& usage ) const override
            {
                usage.addKeyboardMouseBinding(
                    "Tab",
                    "Toggle polygon fill mode between fill, wireframe, and points."
                );
                usage.addKeyboardMouseBinding( "b", "Toggle backface culling." );
                usage.addKeyboardMouseBinding( "t", "Toggle texture state." );
            }
    };

    class SponzaInteractiveCameraController : public osgGA::GUIEventHandler
    {
        public:

            explicit SponzaInteractiveCameraController(
                const sponza::CameraSettings& camera
            ) :
                _homeEye( camera.eye ),
                _homeForward( osg::normalize( camera.center - camera.eye ) ),
                _eye( camera.eye )
            {
                setForward( _homeForward );
            }

            bool
            handle( const osgGA::GUIEventAdapter& ea,
                    osgGA::GUIActionAdapter&      aa ) override
            {
                _fast =
                    ( ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT ) != 0;

                switch( ea.getEventType() )
                {
                    case osgGA::GUIEventAdapter::KEYDOWN :
                    case osgGA::GUIEventAdapter::KEYUP :
                        {
                            const bool pressed =
                                ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN;
                            if( setKeyState( ea.getKey(), pressed ) )
                            {
                                aa.requestContinuousUpdate( anyMovementKeyDown() );
                                aa.requestRedraw();
                                return true;
                            }
                            return false;
                        }
                    case osgGA::GUIEventAdapter::PUSH :
                        _rotating =
                            ( ea.getButtonMask() &
                              ( osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON |
                                osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON ) ) != 0;
                        _lastMouseX = ea.getX();
                        _lastMouseY = ea.getY();
                        return _rotating;
                    case osgGA::GUIEventAdapter::DRAG :
                        if( _rotating )
                        {
                            rotateFromMouse( ea.getX() - _lastMouseX,
                                             ea.getY() - _lastMouseY );
                            _lastMouseX = ea.getX();
                            _lastMouseY = ea.getY();
                            aa.requestRedraw();
                            return true;
                        }
                        return false;
                    case osgGA::GUIEventAdapter::RELEASE :
                        _rotating = false;
                        return false;
                    case osgGA::GUIEventAdapter::SCROLL :
                        scroll( ea );
                        aa.requestRedraw();
                        return true;
                    default :
                        return false;
                }
            }

            void
            update( double simulationTime )
            {
                if( !_haveTime )
                {
                    _lastTime = simulationTime;
                    _haveTime = true;
                    return;
                }

                const double dt =
                    std::clamp( simulationTime - _lastTime, 0.0, maxFrameDelta );
                _lastTime = simulationTime;
                if( dt <= 0.0 )
                {
                    return;
                }

                const osg::dvec3 forward = horizontalForward( forwardVector() );
                const osg::dvec3 right =
                    osg::normalize( osg::cross( forward, osg::dvec3( 0.0, 1.0, 0.0 ) ) );
                const osg::dvec3 up( 0.0, worldUpY, 0.0 );
                osg::dvec3       motion( 0.0, 0.0, 0.0 );

                if( _moveForward )
                {
                    motion += forward;
                }
                if( _moveBackward )
                {
                    motion -= forward;
                }
                if( _moveRight )
                {
                    motion += right;
                }
                if( _moveLeft )
                {
                    motion -= right;
                }
                if( _moveUp )
                {
                    motion += up;
                }
                if( _moveDown )
                {
                    motion -= up;
                }

                if( osg::length( motion ) > 1.0E-6 )
                {
                    const double speed  = _fast ? cameraFastMoveSpeed : cameraMoveSpeed;
                    _eye               += osg::normalize( motion ) * ( speed * dt );
                }
            }

            osg::dmat4
            viewMatrix() const
            {
                return osg::lookAt( _eye,
                                    _eye + forwardVector(),
                                    osg::dvec3( 0.0, 1.0, 0.0 ) );
            }

            const osg::dvec3&
            eye() const noexcept
            {
                return _eye;
            }

            void
            getUsage( osg::ApplicationUsage& usage ) const override
            {
                usage.addKeyboardMouseBinding( "W/A/S/D", "Move the Sponza camera." );
                usage.addKeyboardMouseBinding( "Q/E",
                                               "Move the Sponza camera down/up." );
                usage.addKeyboardMouseBinding( "Shift",
                                               "Use fast Sponza camera movement." );
                usage.addKeyboardMouseBinding( "Left or right mouse drag",
                                               "Rotate the Sponza camera." );
                usage.addKeyboardMouseBinding( "Mouse wheel",
                                               "Move the Sponza camera forward/back." );
                usage.addKeyboardMouseBinding( "R", "Reset the Sponza camera." );
            }

        private:

            void
            setForward( const osg::dvec3& forward )
            {
                const osg::dvec3 normalizedForward = osg::normalize( forward );
                _pitch = std::asin( std::clamp( normalizedForward.y, -1.0, 1.0 ) );
                _yaw   = std::atan2( normalizedForward.z, normalizedForward.x );
                clampPitch();
            }

            osg::dvec3
            forwardVector() const
            {
                const double cosPitch = std::cos( _pitch );
                return osg::normalize( osg::dvec3( cosPitch * std::cos( _yaw ),
                                                   std::sin( _pitch ),
                                                   cosPitch * std::sin( _yaw ) ) );
            }

            void
            rotateFromMouse( float dx,
                             float dy )
            {
                _yaw   += static_cast<double>( dx ) * mouseSensitivity;
                _pitch += static_cast<double>( dy ) * mouseSensitivity;
                clampPitch();
            }

            void
            clampPitch()
            {
                _pitch = std::clamp( _pitch, -maxPitchRadians, maxPitchRadians );
            }

            void
            resetHome()
            {
                _eye = _homeEye;
                setForward( _homeForward );
            }

            void
            scroll( const osgGA::GUIEventAdapter& ea )
            {
                const double step = _fast ? 3.0 : 1.0;
                switch( ea.getScrollingMotion() )
                {
                    case osgGA::GUIEventAdapter::SCROLL_UP :
                        _eye += horizontalForward( forwardVector() ) * step;
                        break;
                    case osgGA::GUIEventAdapter::SCROLL_DOWN :
                        _eye -= horizontalForward( forwardVector() ) * step;
                        break;
                    default :
                        break;
                }
            }

            bool
            setKeyState( int  key,
                         bool pressed )
            {
                switch( normalizedKey( key ) )
                {
                    case osgGA::GUIEventAdapter::KEY_W :
                    case osgGA::GUIEventAdapter::KEY_Up :
                        _moveForward = pressed;
                        return true;
                    case osgGA::GUIEventAdapter::KEY_S :
                    case osgGA::GUIEventAdapter::KEY_Down :
                        _moveBackward = pressed;
                        return true;
                    case osgGA::GUIEventAdapter::KEY_A :
                    case osgGA::GUIEventAdapter::KEY_Left :
                        _moveLeft = pressed;
                        return true;
                    case osgGA::GUIEventAdapter::KEY_D :
                    case osgGA::GUIEventAdapter::KEY_Right :
                        _moveRight = pressed;
                        return true;
                    case osgGA::GUIEventAdapter::KEY_E :
                        _moveUp = pressed;
                        return true;
                    case osgGA::GUIEventAdapter::KEY_Q :
                        _moveDown = pressed;
                        return true;
                    case osgGA::GUIEventAdapter::KEY_R :
                        if( pressed )
                        {
                            resetHome();
                        }
                        return true;
                    default :
                        return false;
                }
            }

            bool
            anyMovementKeyDown() const
            {
                return _moveForward ||
                       _moveBackward ||
                       _moveLeft ||
                       _moveRight ||
                       _moveUp ||
                       _moveDown;
            }

            osg::dvec3 _homeEye;
            osg::dvec3 _homeForward;
            osg::dvec3 _eye;
            double     _yaw          = 0.0;
            double     _pitch        = 0.0;
            double     _lastTime     = 0.0;
            float      _lastMouseX   = 0.0F;
            float      _lastMouseY   = 0.0F;
            bool       _haveTime     = false;
            bool       _rotating     = false;
            bool       _fast         = false;
            bool       _moveForward  = false;
            bool       _moveBackward = false;
            bool       _moveLeft     = false;
            bool       _moveRight    = false;
            bool       _moveUp       = false;
            bool       _moveDown     = false;
    };

    class InteractiveSponzaPassCallback : public osg::NodeCallback
    {
        public:

            InteractiveSponzaPassCallback( SponzaInteractiveCameraController* controller,
                                           osg::Camera*                     viewerCamera,
                                           osg::Camera*                     rttCamera,
                                           osg::Camera*                     ssaoCamera,
                                           const sponza::TonemapPassResult& tonemapPass,
                                           const sponza::SponzaTargets&     targets,
                                           const sponza::SponzaOptions&     options,
                                           const sponza::SponzaFrameContext& frame,
                                           const osg::mat4& initialShadowMatrix ) :
                _controller( controller ),
                _viewerCamera( viewerCamera ),
                _rttCamera( rttCamera ),
                _ssaoCamera( ssaoCamera ),
                _tonemapResolveCamera( tonemapPass.resolveCamera ),
                _tonemapOutputCamera( tonemapPass.outputCamera ),
                _receiverStateSet( rttCamera ? rttCamera->getOrCreateStateSet()
                                             : nullptr ),
                _ssaoStateSet( ssaoCamera ? ssaoCamera->getOrCreateStateSet()
                                          : nullptr ),
                _tonemapStateSet(
                    tonemapPass.resolveCamera.valid()
                        ? tonemapPass.resolveCamera->getOrCreateStateSet()
                        : ( tonemapPass.outputCamera.valid()
                                ? tonemapPass.outputCamera->getOrCreateStateSet()
                                : nullptr )
                ),
                _targets( targets ),
                _resolvedColor( tonemapPass.resolvedColor ),
                _fallbackWidth( sponza::outputWidth( options ) ),
                _fallbackHeight( sponza::outputHeight( options ) ),
                _renderScale( std::max( options.renderScale,
                                        1 ) ),
                _ssaoScale( options.ssaoScale ),
                _resolveScale( options.resolveScale ),
                _fovDeg( options.camera.fovDeg ),
                _lightClipFromWorld( osg::dmat4( initialShadowMatrix ) * frame.view ),
                _sunDirectionWorld( sponza::computeSunDirectionWorld( options ) )
            {
                sync( frame.view,
                      options.camera.eye,
                      sponza::outputWidth( options ),
                      sponza::outputHeight( options ) );
            }

            void
            operator()( osg::Node*        node,
                        osg::NodeVisitor* nv ) override
            {
                const osg::FrameStamp* frameStamp = nv ? nv->getFrameStamp() : nullptr;
                if( frameStamp != nullptr && _controller.valid() )
                {
                    _controller->update( frameStamp->getSimulationTime() );
                }

                const WindowSize windowSize = currentWindowSize();
                const osg::dmat4 view =
                    _controller.valid() ? _controller->viewMatrix() : osg::dmat4();
                const osg::dvec3 eyeWorld =
                    _controller.valid() ? _controller->eye() : osg::dvec3();
                sync( view, eyeWorld, windowSize.width, windowSize.height );
                traverse( node, nv );
            }

        private:

            struct WindowSize
            {
                    int width  = 1;
                    int height = 1;
            };

            WindowSize
            currentWindowSize() const
            {
                int width  = _fallbackWidth;
                int height = _fallbackHeight;

                if( _viewerCamera.valid() && _viewerCamera->getViewport() != nullptr )
                {
                    width = static_cast<int>(
                        std::lround( _viewerCamera->getViewport()->width() )
                    );
                    height = static_cast<int>(
                        std::lround( _viewerCamera->getViewport()->height() )
                    );
                }
                else if( _viewerCamera.valid() &&
                         _viewerCamera->getGraphicsContext() !=
                         nullptr &&
                         _viewerCamera->getGraphicsContext()->getTraits() != nullptr )
                {
                    width  = _viewerCamera->getGraphicsContext()->getTraits()->width;
                    height = _viewerCamera->getGraphicsContext()->getTraits()->height;
                }

                return WindowSize{ std::max( width, 1 ), std::max( height, 1 ) };
            }

            void
            resizeTargetsIfNeeded( int windowWidth,
                                   int windowHeight )
            {
                const int targetWidth  = std::max( windowWidth * _renderScale, 1 );
                const int targetHeight = std::max( windowHeight * _renderScale, 1 );
                const int ssaoWidth =
                    sponza::ssaoTargetDimension( targetWidth, _ssaoScale );
                const int ssaoHeight =
                    sponza::ssaoTargetDimension( targetHeight, _ssaoScale );
                if( targetWidth ==
                    _targetWidth &&
                    targetHeight ==
                    _targetHeight &&
                    ssaoWidth ==
                    _ssaoWidth &&
                    ssaoHeight ==
                    _ssaoHeight &&
                    windowWidth ==
                    _windowWidth &&
                    windowHeight == _windowHeight )
                {
                    return;
                }

                _windowWidth  = windowWidth;
                _windowHeight = windowHeight;
                _targetWidth  = targetWidth;
                _targetHeight = targetHeight;
                _ssaoWidth    = ssaoWidth;
                _ssaoHeight   = ssaoHeight;

                resizeTexture( _targets.hdrColor.get(), targetWidth, targetHeight );
                resizeTexture( _targets.indirectColor.get(), targetWidth, targetHeight );
                resizeTexture( _targets.sceneDepth.get(), targetWidth, targetHeight );
                resizeTexture( _targets.aoTexture.get(), ssaoWidth, ssaoHeight );
                const int resolveWidth =
                    scaledResolveDimension( windowWidth, _resolveScale );
                const int resolveHeight =
                    scaledResolveDimension( windowHeight, _resolveScale );
                resizeTexture( _resolvedColor.get(), resolveWidth, resolveHeight );

                setCameraViewport( _rttCamera.get(), targetWidth, targetHeight );
                setCameraViewport( _ssaoCamera.get(), ssaoWidth, ssaoHeight );
                setCameraViewport( _tonemapResolveCamera.get(),
                                   resolveWidth,
                                   resolveHeight );
                setCameraViewport( _tonemapOutputCamera.get(),
                                   windowWidth,
                                   windowHeight );
            }

            osg::dmat4
            projectionMatrix() const
            {
                const double aspect =
                    static_cast<double>( std::max( _targetWidth, 1 ) ) /
                    static_cast<double>( std::max( _targetHeight, 1 ) );
                return osg::perspective( _fovDeg, aspect, sponza::nearZ, sponza::farZ );
            }

            void
            sync( const osg::dmat4& view,
                  const osg::dvec3& eyeWorldD,
                  int               windowWidth,
                  int               windowHeight )
            {
                resizeTargetsIfNeeded( windowWidth, windowHeight );

                const osg::dmat4 projection    = projectionMatrix();
                const osg::dmat4 invProjection = osg::inverse( projection );
                if( _rttCamera.valid() )
                {
                    _rttCamera->setViewMatrix( view );
                    _rttCamera->setProjectionMatrix( projection );
                }

                const osg::Matrix3 viewToWorldRot =
                    sponza::makeViewToWorldRotation( view );
                const osg::vec3  eyeWorld    = toVec3( eyeWorldD );
                const osg::dmat4 viewToWorld = osg::inverse( view );
                const osg::mat4  shadowMatrix( _lightClipFromWorld * viewToWorld );

                setOverrideUniform( _receiverStateSet.get(),
                                    "uViewToWorldRot",
                                    viewToWorldRot );
                setOverrideUniform( _receiverStateSet.get(),
                                    "uRtViewOriginWorld",
                                    eyeWorld );
                setOverrideUniform( _receiverStateSet.get(),
                                    "uShadowMatrix",
                                    shadowMatrix );
                setOverrideUniform( _ssaoStateSet.get(),
                                    "uProj",
                                    osg::mat4( projection ) );
                setOverrideUniform( _ssaoStateSet.get(),
                                    "uInvProj",
                                    osg::mat4( invProjection ) );
                setOverrideUniform( _ssaoStateSet.get(),
                                    "uResolution",
                                    osg::vec2( static_cast<float>( _ssaoWidth ),
                                               static_cast<float>( _ssaoHeight ) ) );
                setOverrideUniform( _tonemapStateSet.get(),
                                    "uInvProj",
                                    osg::mat4( invProjection ) );
                setOverrideUniform( _tonemapStateSet.get(),
                                    "uViewToWorldRot",
                                    viewToWorldRot );
            }

            osg::ref_ptr<SponzaInteractiveCameraController> _controller;
            osg::ref_ptr<osg::Camera>                       _viewerCamera;
            osg::ref_ptr<osg::Camera>                       _rttCamera;
            osg::ref_ptr<osg::Camera>                       _ssaoCamera;
            osg::ref_ptr<osg::Camera>                       _tonemapResolveCamera;
            osg::ref_ptr<osg::Camera>                       _tonemapOutputCamera;
            osg::ref_ptr<osg::StateSet>                     _receiverStateSet;
            osg::ref_ptr<osg::StateSet>                     _ssaoStateSet;
            osg::ref_ptr<osg::StateSet>                     _tonemapStateSet;
            sponza::SponzaTargets                           _targets;
            osg::ref_ptr<osg::Texture2D>                    _resolvedColor;
            int                                             _fallbackWidth;
            int                                             _fallbackHeight;
            int                                             _renderScale;
            float                                           _ssaoScale;
            float                                           _resolveScale;
            int                                             _windowWidth  = 0;
            int                                             _windowHeight = 0;
            int                                             _targetWidth  = 0;
            int                                             _targetHeight = 0;
            int                                             _ssaoWidth    = 0;
            int                                             _ssaoHeight   = 0;
            double                                          _fovDeg;
            osg::dmat4                                      _lightClipFromWorld;
            osg::dvec3                                      _sunDirectionWorld;
    };

}

namespace
{

    bool
    sceneCookIsFresh( const std::filesystem::path& cookPath,
                      const std::filesystem::path& modelPath )
    {
        std::error_code error;
        if( !std::filesystem::exists( cookPath, error ) || error )
        {
            return false;
        }

        const std::filesystem::file_time_type cookTime =
            std::filesystem::last_write_time( cookPath, error );
        if( error )
        {
            return false;
        }

        const std::filesystem::file_time_type modelTime =
            std::filesystem::last_write_time( modelPath, error );
        if( error )
        {
            return false;
        }

        return cookTime > modelTime;
    }

}

int
main( int    argc,
      char** argv )
{
    osg::ArgumentParser   arguments( &argc, argv );

    sponza::SponzaOptions options;
    if( !sponza::parseSponzaOptions( arguments, options ) )
    {
        return 1;
    }

    osg::ref_ptr<osg::Node> model;
    const std::string       cookPath = options.modelPath + ".scenecook";
    const auto              sceneLoadStart = std::chrono::steady_clock::now();
    const bool              tryCook = sceneCookIsFresh( cookPath, options.modelPath );
    const char*             sceneLoadPath = tryCook ? "cook" : "gltf";
    if( tryCook )
    {
        osg::ref_ptr<osg::Object> cookedObject =
            osgDB::serialization::readSceneCook( cookPath );
        if( cookedObject.valid() && cookedObject->asNode() != nullptr )
        {
            model = cookedObject->asNode();
            OSG_NOTICE << "loaded scene cook " << cookPath << std::endl;
        }
        else
        {
            sceneLoadPath = "gltf";
        }
    }

    if( !model )
    {
        model = osgDB::readRefNodeFile( options.modelPath );
        sceneLoadPath = "gltf";
        if( model )
        {
            osgDB::serialization::compressSceneTextures( *model );
            if( osgDB::serialization::writeSceneCook( *model, cookPath ) )
            {
                OSG_NOTICE << "wrote scene cook " << cookPath << std::endl;
            }
            else
            {
                OSG_WARN << "failed to write scene cook " << cookPath << std::endl;
            }
        }
    }
    const auto sceneLoadEnd = std::chrono::steady_clock::now();
    const auto sceneLoadMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            sceneLoadEnd - sceneLoadStart
        )
            .count();
    OSG_NOTICE << "SCENE LOAD: " << sceneLoadMs << " ms via " << sceneLoadPath
               << std::endl;
    if( !model )
    {
        std::cerr << "Failed to load " << options.modelPath << std::endl;
        return 1;
    }

    const osg::dmat4 rttView            = sponza::makeViewMatrix( options.camera );
    const osg::dmat4 projectionMatrix   = sponza::makeProjectionMatrix( options.camera );
    osg::ref_ptr<osg::Image>   envImage = sponza::loadEnvironmentImage();
    sponza::IrradianceShResult irradianceSh;
    if( envImage )
    {
        irradianceSh = sponza::computeSunExcludedIrradianceSh( *envImage );
    }

    sponza::applyVisibilityBake( model.get(), options, envImage.get(), &irradianceSh );

    sponza::GpuRayScene gpuRayScene;
    if( options.rtShadowsEnabled )
    {
        gpuRayScene = sponza::loadOrCreateGpuRayScene( *model, options );
    }

    osg::ref_ptr<osg::Texture2D> envTexture = sponza::applySunAndIbl( model.get(),
                                                                      options,
                                                                      rttView,
                                                                      envImage.get(),
                                                                      &irradianceSh );

    sponza::SponzaTargets        targets    = sponza::createSponzaTargets( options );
    const sponza::SponzaFrameContext frame{
        rttView,
        projectionMatrix,
        osg::inverse( projectionMatrix ),
        sponza::makeViewToWorldRotation( rttView ),
        envTexture,
        options.envRotation
    };

    sponza::ShadowPassResult shadowPass =
        sponza::createShadowPass( model.get(), options, frame );
    osg::ref_ptr<osg::Camera> rtt =
        sponza::createRttCamera( model.get(), options, targets, frame );
    sponza::applyShadowReceiverState( rtt->getOrCreateStateSet(),
                                      options,
                                      shadowPass.shadowTexture.get(),
                                      shadowPass.shadowMatrix,
                                      shadowPass.lightSpaceExtent,
                                      shadowPass.hasShadow );
    sponza::applyRayShadowReceiverState( rtt->getOrCreateStateSet(),
                                         options,
                                         &gpuRayScene );
    osg::ref_ptr<osg::Camera> ssao = sponza::createSsaoCamera( options, targets, frame );
    sponza::TonemapPassResult tonemapPass =
        sponza::createTonemapPass( options, targets, frame );

    osg::ref_ptr<osg::Group> root = new osg::Group;
    if( shadowPass.camera )
    {
        shadowPass.camera->setName( "Sponza shadow map" );
        attachGpuPassTimer( shadowPass.camera.get(), "shadow", options );
        root->addChild( shadowPass.camera.get() );
    }
    rtt->setName( "Sponza main scene" );
    ssao->setName( "Sponza SSAO" );
    if( tonemapPass.resolveCamera.valid() )
    {
        tonemapPass.resolveCamera->setName( "Sponza tonemap resolve" );
        attachGpuPassTimer( tonemapPass.resolveCamera.get(),
                            "tonemap-resolve",
                            options );
        root->addChild( tonemapPass.resolveCamera.get() );
    }
    if( tonemapPass.outputCamera.valid() )
    {
        tonemapPass.outputCamera->setName( tonemapPass.resolvedFxaa ? "Sponza FXAA"
                                                                    : "Sponza tonemap" );
    }
    attachGpuPassTimer( rtt.get(), "main", options );
    attachGpuPassTimer( ssao.get(), "ssao", options );
    attachGpuPassTimer( tonemapPass.outputCamera.get(),
                        tonemapPass.resolvedFxaa ? "fxaa" : "tonemap",
                        options );
    root->addChild( rtt.get() );
    root->addChild( ssao.get() );
    root->addChild( tonemapPass.outputCamera.get() );

    if( options.headless )
    {
        return osg::headlessCapture( root.get(),
                                     options.headlessOutput,
                                     sponza::outputWidth( options ),
                                     sponza::outputHeight( options ),
                                     options.camera.eye,
                                     options.camera.center,
                                     options.camera.up,
                                     options.benchmarkFrames,
                                     options.benchmarkWarmupFrames )
                 ? 0
                 : 1;
    }

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.getCamera()->setAllowEventFocus( false );
    viewer.getCamera()->setName( "Sponza output" );

    osg::ref_ptr<osgGA::StateSetManipulator> stateManipulator =
        new SponzaDiagnosticsStateManipulator( model->getOrCreateStateSet() );
    viewer.addEventHandler( stateManipulator.get() );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    osg::ref_ptr<osgViewer::StatsHandler> statsHandler =
        new osgViewer::StatsHandler;
    // Toggle the stats overlay with 'z' so 's' stays free for WASD movement.
    statsHandler->setKeyEventTogglesOnScreenStats( 'z' );
    viewer.addEventHandler( statsHandler.get() );
    viewer.addEventHandler(
        new osgViewer::HelpHandler( arguments.getApplicationUsage() )
    );
    viewer.addEventHandler( new osgViewer::ScreenCaptureHandler );

    osg::ref_ptr<SponzaInteractiveCameraController> controller =
        new SponzaInteractiveCameraController( options.camera );
    viewer.addEventHandler( controller.get() );
    root->setUpdateCallback(
        new InteractiveSponzaPassCallback( controller.get(),
                                           viewer.getCamera(),
                                           rtt.get(),
                                           ssao.get(),
                                           tonemapPass,
                                           targets,
                                           options,
                                           frame,
                                           shadowPass.shadowMatrix )
    );
    viewer.setRunMaxFrameRate( options.runMaxFrameRate );

    return viewer.run();
}
