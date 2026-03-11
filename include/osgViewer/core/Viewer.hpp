/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Single-window scene viewer. Manages the main rendering loop, event
 * processing, camera manipulators, and threading model selection.
 */
#pragma once

#include <osg/core/ArgumentParser.hpp>
#include <osg/core/Inherit.hpp>
#include <osgGA/events/EventVisitor.hpp>
#include <osgUtil/culling/UpdateVisitor.hpp>
#include <osgViewer/core/View.hpp>
#include <osgViewer/platform/GraphicsWindow.hpp>

namespace osgViewer
{

    /** Viewer holds a single view on to a single scene.*/
    class OSGVIEWER_EXPORT Viewer : public ViewerBase,
                                    public osgViewer::View
    {
        public:

            Viewer();

            Viewer( osg::ArgumentParser& arguments );

            Viewer( const osgViewer::Viewer& viewer,
                    const osg::CopyOp&       copyop = osg::CopyOp::SHALLOW_COPY );

            virtual ~Viewer();

            OSG_REGISTER_TYPE( osgViewer,
                               Viewer )

            // Type-info methods (normally from Inherit<>)
            static osg::ref_ptr<Viewer>
            create()
            {
                return new Viewer();
            }

            osg::Object*
            cloneType() const override
            {
                return new Viewer();
            }

            osg::Object*
            clone( const osg::CopyOp& copyop ) const override
            {
                return new Viewer( *this, copyop );
            }

            bool
            isSameKindAs( const osg::Object* obj ) const override
            {
                return dynamic_cast<const Viewer*>( obj ) != nullptr;
            }

            bool
            is_compatible( const std::type_info& type ) const noexcept override
            {
                return typeid( Viewer ) == type || ViewerBase::is_compatible( type );
            }

            const std::type_info&
            type_info() const noexcept override
            {
                return typeid( Viewer );
            }

            std::size_t
            sizeofObject() const noexcept override
            {
                return sizeof( Viewer );
            }

            const char*
            libraryName() const override
            {
                return _s_libraryName();
            }

            const char*
            className() const override
            {
                return _s_className();
            }

            /** Take all the settings, Camera and Slaves from the passed in view(er),
             * leaving it empty. */
            void
            take( osg::View& rhs ) override;

            /** Set the Stats object used to collect various frame related timing and
             * scene graph stats.*/
            void
            setViewerStats( osg::Stats* stats ) override
            {
                setStats( stats );
            }

            /** Get the Viewers Stats object.*/
            osg::Stats*
            getViewerStats() override
            {
                return getStats();
            }

            /** Get the Viewers Stats object.*/
            const osg::Stats*
            getViewerStats() const override
            {
                return getStats();
            }

            /** read the viewer configuration from a configuration file.*/
            bool
            readConfiguration( const std::string& filename ) override;

            /** Get whether at least of one of this viewers windows are realized.*/
            bool
            isRealized() const override;

            /** set up windows and associated threads.*/
            void
            realize() override;

            void
            setStartTick( osg::Timer_t tick ) override;
            void
            setReferenceTime( double time = 0.0 );

            using osgViewer::View::setSceneData;

            /** Set the sene graph data that viewer with view.*/
            void
            setSceneData( osg::Node* node ) override;

            /** Convenience method for setting up the viewer so it can be used embedded
             * in an external managed window. Returns the GraphicsWindowEmbedded that can
             * be used by applications to pass in events to the viewer. */
            virtual GraphicsWindowEmbedded*
            setUpViewerAsEmbeddedInWindow( int x,
                                           int y,
                                           int width,
                                           int height );

            double
            elapsedTime() override;

            osg::FrameStamp*
            getViewerFrameStamp() override
            {
                return getFrameStamp();
            }

            /** Execute a main frame loop.
             * Equivalent to while (!viewer.done()) viewer.frame();
             * Also calls realize() if the viewer is not already realized,
             * and installs trackball manipulator if one is not already assigned.
             */
            int
            run() override;

            /** check to see if the new frame is required, called by run(..) when
             * FrameScheme is set to ON_DEMAND.*/
            bool
            checkNeedToDoFrame() override;

            /** check to see if events have been received, return true if events are now
             * available.*/
            bool
            checkEvents() override;

            void
            advance( double simulationTime = USE_REFERENCE_TIME ) override;

            void
            eventTraversal() override;

            void
            updateTraversal() override;

            void
            getCameras( Cameras& cameras,
                        bool     onlyActive = true ) override;

            void
            getContexts( Contexts& contexts,
                         bool      onlyValid = true ) override;

            void
            getAllThreads( Threads& threads,
                           bool     onlyActive = true ) override;

            void
            getOperationThreads( OperationThreads& threads,
                                 bool              onlyActive = true ) override;

            void
            getScenes( Scenes& scenes,
                       bool    onlyValid = true ) override;

            void
            getViews( Views& views,
                      bool   onlyValid = true ) override;

            /** Get the keyboard and mouse usage of this viewer.*/
            void
            getUsage( osg::ApplicationUsage& usage ) const override;

            // ensure that osg::View provides the reiszerGLObjects and releaseGLObjects
            // methods
            void
            resizeGLObjectBuffers( unsigned int maxSize ) override
            {
                osg::View::resizeGLObjectBuffers( maxSize );
            }

            void
            releaseGLObjects( osg::State* state = 0 ) const override
            {
                osg::View::releaseGLObjects( state );
            }

        protected:

            void
            constructorInit();

            void
            viewerInit() override
            {
                init();
            }

            void
            generateSlavePointerData( osg::Camera*            camera,
                                      osgGA::GUIEventAdapter& event );
            void
            generatePointerData( osgGA::GUIEventAdapter& event );
            void
            reprojectPointerData( osgGA::GUIEventAdapter& source_event,
                                  osgGA::GUIEventAdapter& dest_event );
    };

}
