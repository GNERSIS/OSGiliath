/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Background image loader. Pages in texture images from disk
 * using worker threads to avoid frame stalls.
 */
#pragma once

#include <atomic>
#include <mutex>
#include <osg/core/FrameStamp.hpp>
#include <osg/core/observer_ptr.hpp>
#include <osg/images/Image.hpp>
#include <osg/threading/OperationThread.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgDB/registry/Options.hpp>
#include <osgDB/registry/ReaderWriter.hpp>

namespace osgDB
{

    class OSGDB_EXPORT ImagePager : public osg::NodeVisitor::ImageRequestHandler
    {
        public:

            ImagePager();

            class OSGDB_EXPORT ImageThread : public osg::Referenced,
                                             public osg::Thread
            {
                public:

                    enum Mode
                    {
                        HANDLE_ALL_REQUESTS,
                        HANDLE_NON_HTTP,
                        HANDLE_ONLY_HTTP,
                    };

                    ImageThread( ImagePager*        pager,
                                 Mode               mode,
                                 const std::string& name );

                    ImageThread( const ImageThread& dt,
                                 ImagePager*        pager );

                    void
                    setDone( bool done )
                    {
                        _done = done;
                    }

                    bool
                    getDone() const
                    {
                        return _done;
                    }

                    virtual int
                    cancel();

                    virtual void
                    run();

                protected:

                    virtual ~ImageThread();

                    bool        _done;
                    Mode        _mode;
                    ImagePager* _pager;
                    std::string _name;
            };

            ImageThread*
            getImageThread( unsigned int i )
            {
                return _imageThreads[i].get();
            }

            const ImageThread*
            getImageThread( unsigned int i ) const
            {
                return _imageThreads[i].get();
            }

            unsigned int
            getNumImageThreads() const
            {
                return static_cast<unsigned int>( _imageThreads.size() );
            }

            void
            setPreLoadTime( double preLoadTime )
            {
                _preLoadTime = preLoadTime;
            }

            virtual double
            getPreLoadTime() const
            {
                return _preLoadTime;
            }

            virtual osg::ref_ptr<osg::Image>
            readRefImageFile( const std::string&     fileName,
                              const osg::Referenced* options = 0 );

            virtual void
            requestImageFile( const std::string&             fileName,
                              osg::Object*                   attachmentPoint,
                              int                            attachmentIndex,
                              double                         timeToMergeBy,
                              const osg::FrameStamp*         framestamp,
                              osg::ref_ptr<osg::Referenced>& imageRequest,
                              const osg::Referenced*         options );

            /** Return true if there are pending updates to the scene graph that require
             * a call to updateSceneGraph(double). */
            virtual bool
            requiresUpdateSceneGraph() const;

            /** Merge the changes to the scene graph. */
            virtual void
            updateSceneGraph( const osg::FrameStamp& frameStamp );

            /** Signal the image thread that the update, cull and draw has begun for a
             * new frame. Note, this is called by the application so that the image pager
             * can go to sleep while the CPU is busy on the main rendering threads. */
            virtual void
            signalBeginFrame( const osg::FrameStamp* framestamp );

            /** Signal the image thread that the update, cull and draw dispatch has
             * completed. Note, this is called by the application so that the image pager
             * can go to wake back up now the main rendering threads are iddle waiting
             * for the next frame.*/
            virtual void
            signalEndFrame();

            int
            cancel();

        protected:

            virtual ~ImagePager();
            // forward declare
            struct RequestQueue;

            struct SortFileRequestFunctor;
            friend struct SortFileRequestFunctor;

            struct ImageRequest : public osg::Referenced
            {
                    ImageRequest() :
                        osg::Referenced( true ),
                        _frameNumber( 0 ),
                        _timeToMergeBy( 0.0 ),
                        _attachmentIndex( -1 ),
                        _requestQueue( 0 )
                    {
                    }

                    unsigned int                   _frameNumber;
                    double                         _timeToMergeBy;
                    std::string                    _fileName;
                    osg::ref_ptr<Options>          _loadOptions;
                    osg::observer_ptr<osg::Object> _attachmentPoint;
                    int                            _attachmentIndex;
                    osg::ref_ptr<osg::Image>       _loadedImage;
                    RequestQueue*                  _requestQueue;
                    osg::ref_ptr<osgDB::Options>   _readOptions;
            };

            struct RequestQueue : public osg::Referenced
            {
                    typedef std::vector<osg::ref_ptr<ImageRequest>> RequestList;

                    void
                    sort();

                    unsigned int
                                       size() const;

                    RequestList        _requestList;
                    mutable std::mutex _requestMutex;
            };

            struct ReadQueue : public RequestQueue
            {
                    ReadQueue( ImagePager*        pager,
                               const std::string& name );

                    void
                    block()
                    {
                        _block->block();
                    }

                    void
                    release()
                    {
                        _block->release();
                    }

                    void
                    updateBlock()
                    {
                        _block->set( !_requestList.empty() &&
                                     !_pager->_databasePagerThreadPaused );
                    }

                    void
                    clear();

                    void
                    add( ImageRequest* imageRequest );

                    void
                    takeFirst( osg::ref_ptr<ImageRequest>& databaseRequest );

                    osg::ref_ptr<osg::RefBlock> _block;

                    ImagePager*                 _pager;
                    std::string                 _name;
            };

            std::mutex                                     _run_mutex;
            bool                                           _startThreadCalled;

            bool                                           _done;
            bool                                           _databasePagerThreadPaused;

            std::atomic<unsigned>                          _frameNumber;

            std::mutex                                     _ir_mutex;
            osg::ref_ptr<ReadQueue>                        _readQueue;

            typedef std::vector<osg::ref_ptr<ImageThread>> ImageThreads;
            ImageThreads                                   _imageThreads;

            osg::ref_ptr<RequestQueue>                     _completedQueue;

            double                                         _preLoadTime;
    };

}
