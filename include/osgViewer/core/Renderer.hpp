/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Camera rendering backend. Manages SceneView instances and
 * orchestrates cull/draw operations for a single camera.
 */
#pragma once

#include <condition_variable>
#include <mutex>
#include <osg/core/Timer.hpp>
#include <osgDB/cache/DatabasePager.hpp>
#include <osgUtil/culling/SceneView.hpp>
#include <osgViewer/core/Export.hpp>

namespace osgViewer
{

    class OSGVIEWER_EXPORT OpenGLQuerySupport : public osg::Referenced
    {
        public:

            OpenGLQuerySupport();

            virtual void
            checkQuery( osg::Stats*  stats,
                        osg::State*  state,
                        osg::Timer_t startTick ) = 0;

            virtual void
            beginQuery( unsigned int frameNumber,
                        osg::State*  state ) = 0;
            virtual void
            endQuery( osg::State* state ) = 0;
            virtual void
            initialize( osg::State*  state,
                        osg::Timer_t startTick );

        protected:

            const osg::GLExtensions* _extensions;
    };

    class OSGVIEWER_EXPORT Renderer : public osg::GraphicsOperation
    {
        public:

            Renderer( osg::Camera* camera );

            osgUtil::SceneView*
            getSceneView( unsigned int i )
            {
                return _sceneView[i].get();
            }

            const osgUtil::SceneView*
            getSceneView( unsigned int i ) const
            {
                return _sceneView[i].get();
            }

            void
            setDone( bool done )
            {
                _done = done;
            }

            bool
            getDone()
            {
                return _done;
            }

            void
            setGraphicsThreadDoesCull( bool flag );

            bool
            getGraphicsThreadDoesCull() const
            {
                return _graphicsThreadDoesCull;
            }

            virtual void
            cull();
            virtual void
            draw();
            virtual void
            cull_draw();

            virtual void
            compile();

            virtual void
            resizeGLObjectBuffers( unsigned int maxSize );
            virtual void
            releaseGLObjects( osg::State* = 0 ) const;

            void
            setCompileOnNextDraw( bool flag )
            {
                _compileOnNextDraw = flag;
            }

            bool
            getCompileOnNextDraw() const
            {
                return _compileOnNextDraw;
            }

            virtual void
            operator()( osg::Object* object );

            virtual void
            operator()( osg::GraphicsContext* context );

            virtual void
            release();

            virtual void
            reset();

            /** Force update of state associated with cameras. */
            void
            setCameraRequiresSetUp( bool flag );
            bool
            getCameraRequiresSetUp() const;

        protected:

            void
            initialize( osg::State* state );
            virtual ~Renderer();

            virtual void
            updateSceneView( osgUtil::SceneView* sceneView );

            osg::observer_ptr<osg::Camera>   _camera;

            bool                             _done;
            bool                             _graphicsThreadDoesCull;
            bool                             _compileOnNextDraw;
            bool                             _serializeDraw;

            osg::ref_ptr<osgUtil::SceneView> _sceneView[2];

            struct OSGVIEWER_EXPORT ThreadSafeQueue
            {
                    std::mutex                             _mutex;
                    std::condition_variable_any            _cond;
                    typedef std::list<osgUtil::SceneView*> SceneViewList;
                    SceneViewList                          _queue;
                    bool                                   _isReleased;

                    ThreadSafeQueue();
                    ~ThreadSafeQueue();

                    /** Release any thread waiting on the queue, even if the queue is
                     * empty. */
                    void
                    release();

                    /** Reset to fefault state (_isReleased = false)*/
                    void
                    reset();

                    /** Take a SceneView from the queue. Can return 0 if release() is
                     * called when the queue is empty. */
                    osgUtil::SceneView*
                    takeFront();

                    /**  Add a SceneView object to the back of the queue. */
                    void
                    add( osgUtil::SceneView* sv );
            };

            ThreadSafeQueue                  _availableQueue;
            ThreadSafeQueue                  _drawQueue;

            bool                             _initialized;
            osg::ref_ptr<OpenGLQuerySupport> _querySupport;
            osg::Timer_t                     _startTick;
    };

}
