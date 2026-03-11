/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * CameraGroup, derived from Referenced.
 * Provides: getDefaultThreadModel, addRenderSurface, CameraConfig, beginVisual,
 * beginVisual, setVisualSimpleConfiguration.
 */
#pragma once

#include "Camera.hpp"
#include "RenderSurface.hpp"

#include <iostream>
#include <map>
#include <osg/core/Notify.hpp>
#include <osg/core/Referenced.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/vec2.hpp>
#include <osgViewer/core/View.hpp>
#include <stdio.h>
#include <string>
#include <vector>

// #undef SUPPORT_CPP

namespace osgProducer
{

#define notImplemented                                         \
    {                                                          \
        std::cout << __FILE__ << " " << __LINE__ << std::endl; \
    }

    class CameraGroup : public osg::Referenced
    {
        public:

            enum ThreadModel
            {
                SingleThreaded,
                ThreadPerRenderSurface,
                ThreadPerCamera,
            };

            static ThreadModel
            getDefaultThreadModel()
            {
                return SingleThreaded;
            }
    };

    class InputArea : public osg::Referenced
    {
        public:

            void
            addRenderSurface( RenderSurface* s )
            {
                _rs.push_back( s );
            }

            std::vector<osg::ref_ptr<RenderSurface>> _rs;
    };

#undef notImplemented

    class CameraConfig : public osg::Referenced
    {
        public:

            CameraConfig();

            void
            beginVisual( void );

            void
            beginVisual( const char* name );

            void
            setVisualSimpleConfiguration( void );

            void
            setVisualByID( unsigned int id );

            void
            addVisualAttribute( VisualChooser::AttributeName token,
                                int                          param );

            void
            addVisualAttribute( VisualChooser::AttributeName token );

            void
            addVisualExtendedAttribute( unsigned int token );

            void
            addVisualExtendedAttribute( unsigned int token,
                                        int          param );

            void
            endVisual( void );

            VisualChooser*
            findVisual( const char* name );

            bool
            parseFile( const std::string& file );

            void
            beginRenderSurface( const char* name );

            void
            setRenderSurfaceVisualChooser( const char* name );

            void
            setRenderSurfaceVisualChooser( void );

            void
            setRenderSurfaceWindowRectangle( int          x,
                                             int          y,
                                             unsigned int width,
                                             unsigned int height );

            void
            setRenderSurfaceCustomFullScreenRectangle( int          x,
                                                       int          y,
                                                       unsigned int width,
                                                       unsigned int height );

            void
            setRenderSurfaceOverrideRedirect( bool flag );

            void
            setRenderSurfaceHostName( const std::string& name );

            void
            setRenderSurfaceDisplayNum( int n );

            void
            setRenderSurfaceScreen( int n );

            void
            setRenderSurfaceBorder( bool flag );

            void
            setRenderSurfaceDrawableType( RenderSurface::DrawableType drawableType );

            void
            setRenderSurfaceRenderToTextureMode(
                RenderSurface::RenderToTextureMode rttMode
            );

            void
            setRenderSurfaceReadDrawable( const char* name );

            void
            setRenderSurfaceInputRectangle( float x0,
                                            float x1,
                                            float y0,
                                            float y1 );

            void
            endRenderSurface( void );

            RenderSurface*
            findRenderSurface( const char* name );

            unsigned int
            getNumberOfRenderSurfaces();

            RenderSurface*
            getRenderSurface( unsigned int index );

            void
            addCamera( std::string name,
                       Camera*     camera );

            void
            beginCamera( std::string name );

            void
            setCameraRenderSurface( const char* name );

            void
            setCameraRenderSurface( void );

            void
            setCameraProjectionRectangle( float x0,
                                          float x1,
                                          float y0,
                                          float y1 );

            void
            setCameraProjectionRectangle( int x0,
                                          int x1,
                                          int y0,
                                          int y1 );

            void
            setCameraOrtho( float left,
                            float right,
                            float bottom,
                            float top,
                            float nearClip,
                            float farClip,
                            float xshear = 0.0,
                            float yshear = 0.0 );

            void
            setCameraPerspective( float hfov,
                                  float vfov,
                                  float nearClip,
                                  float farClip,
                                  float xshear = 0.0,
                                  float yshear = 0.0 );

            void
            setCameraFrustum( float left,
                              float right,
                              float bottom,
                              float top,
                              float nearClip,
                              float farClip,
                              float xshear = 0.0,
                              float yshear = 0.0 );

            void
            setCameraLensShear( osg::dmat4::value_type xshear,
                                osg::dmat4::value_type yshear );

            void
            setCameraShareLens( bool shared );

            void
            setCameraShareView( bool shared );

            void
            setCameraClearColor( float r,
                                 float g,
                                 float b,
                                 float a );

            void
            beginCameraOffset();

            void
            rotateCameraOffset( osg::dmat4::value_type deg,
                                osg::dmat4::value_type x,
                                osg::dmat4::value_type y,
                                osg::dmat4::value_type z );

            void
            translateCameraOffset( osg::dmat4::value_type x,
                                   osg::dmat4::value_type y,
                                   osg::dmat4::value_type z );

            void
            scaleCameraOffset( osg::dmat4::value_type x,
                               osg::dmat4::value_type y,
                               osg::dmat4::value_type z );

            void
            shearCameraOffset( osg::dmat4::value_type shearx,
                               osg::dmat4::value_type sheary );

            void
            setCameraOffsetMultiplyMethod( Camera::Offset::MultiplyMethod method );

            void
            endCameraOffset();

            void
            endCamera( void );

            Camera*
            findCamera( const char* name );

            unsigned int
            getNumberOfCameras() const;

            const Camera*
            getCamera( unsigned int n ) const;

            Camera*
            getCamera( unsigned int n );

            void
            beginInputArea();

            void
            addInputAreaEntry( char* renderSurfaceName );

            void
            endInputArea();

            void
            setInputArea( InputArea* ia );

            InputArea*
            getInputArea();

            const InputArea*
            getInputArea() const;

            void
            realize( void );

            bool
            defaultConfig();

            struct StereoSystemCommand
            {
                    int         _screen;
                    std::string _setStereoCommand;
                    std::string _restoreMonoCommand;

                    StereoSystemCommand( int         screen,
                                         std::string setStereoCommand,
                                         std::string restoreMonoCommand ) :
                        _screen( screen ),
                        _setStereoCommand( setStereoCommand ),
                        _restoreMonoCommand( restoreMonoCommand )
                    {
                    }
            };

            static std::string findFile( std::string );

            void
            addStereoSystemCommand( int         screen,
                                    std::string stereoCmd,
                                    std::string monoCmd );

            const std::vector<StereoSystemCommand>&
            getStereoSystemCommands();

            void
            setThreadModelDirective( CameraGroup::ThreadModel directive )
            {
                _threadModelDirective = directive;
            }

            CameraGroup::ThreadModel
            getThreadModelDirective()
            {
                return _threadModelDirective;
            }

        protected:

            virtual ~CameraConfig();

        private:

            std::map<std::string, VisualChooser*> _visual_map;
            osg::ref_ptr<VisualChooser>           _current_visual_chooser;
            bool                                  _can_add_visual_attributes;

            std::map<std::string, osg::ref_ptr<RenderSurface>> _render_surface_map;
            osg::ref_ptr<RenderSurface>                        _current_render_surface;
            bool _can_add_render_surface_attributes;

            std::map<std::string, osg::ref_ptr<Camera>> _camera_map;
            osg::ref_ptr<Camera>                        _current_camera;
            bool                                        _can_add_camera_attributes;

            osg::ref_ptr<InputArea>                     _input_area;
            bool                                        _can_add_input_area_entries;

            unsigned int
                                             getNumberOfScreens();

            osg::dmat4::value_type           _offset_matrix[16];
            osg::dmat4::value_type           _offset_shearx, _offset_sheary;

            std::vector<StereoSystemCommand> _stereoSystemCommands;

            bool                             _postmultiply;

            CameraGroup::ThreadModel         _threadModelDirective;
    };

}
