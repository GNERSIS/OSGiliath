/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for releasing and flushing GL objects
 * (textures, buffers, programs) across contexts.
 */
#pragma once

#include <list>
#include <osg/core/Referenced.hpp>
#include <osg/GL>
#include <string>

namespace osg
{

    // forward declare
    class FrameStamp;

    /** Flush all deleted OpenGL objects within the specified availableTime.
     * Note, must be called from a thread which has current the graphics context
     * associated with contextID. */
    extern OSG_EXPORT void
    flushDeletedGLObjects( unsigned int contextID,
                           double       currentTime,
                           double&      availableTime );

    /** Flush all deleted OpenGL objects.
     * Note, must be called from a thread which has current the graphics context
     * associated with contextID. */
    extern OSG_EXPORT void
    flushAllDeletedGLObjects( unsigned int contextID );

    /** Do a GL delete all OpenGL objects.
     * Note, must be called from a thread which has current the graphics context
     * associated with contextID. */
    extern OSG_EXPORT void
    deleteAllGLObjects( unsigned int contextID );

    /** Discard all OpenGL objects.
     * Note, unlike deleteAllGLjects discard does not
     * do any OpenGL calls so can be called from any thread, but as a consequence it
     * also doesn't remove the associated OpenGL resource so discard should only be
     * called when the associated graphics context is being/has been closed. */
    extern OSG_EXPORT void
    discardAllGLObjects( unsigned int contextID );

    class OSG_EXPORT GraphicsObject : public osg::Referenced
    {
        public:

            GraphicsObject();

        protected:

            virtual ~GraphicsObject();
    };

    class OSG_EXPORT GraphicsObjectManager : public osg::Referenced
    {
        public:

            GraphicsObjectManager( const std::string& name,
                                   unsigned int       contextID );

            unsigned int
            getContextID() const
            {
                return _contextID;
            }

            /** Signal that a new frame has started.*/
            virtual void
            newFrame( osg::FrameStamp* /*fs*/ )
            {
            }

            virtual void
            resetStats()
            {
            }

            virtual void
            reportStats( std::ostream& /*out*/ )
            {
            }

            virtual void
            recomputeStats( std::ostream& /*out*/ ) const
            {
            }

            /** Flush all deleted OpenGL objects within the specified availableTime.
             * Note, must be called from a thread which has current the graphics context
             * associated with contextID. */
            virtual void
            flushDeletedGLObjects( double  currentTime,
                                   double& availableTime ) = 0;

            /** Flush all deleted OpenGL objects.
             * Note, must be called from a thread which has current the graphics context
             * associated with contextID. */
            virtual void
            flushAllDeletedGLObjects() = 0;

            /** Do a GL delete all OpenGL objects.
             * Note, must be called from a thread which has current the graphics context
             * associated with contextID. */
            virtual void
            deleteAllGLObjects() = 0;

            /** Discard all OpenGL objects.
             * Note, unlike deleteAllGLjects discard does not
             * do any OpenGL calls so can be called from any thread, but as a consequence
             * it also doesn't remove the associated OpenGL resource so discard should
             * only be called when the associated graphics context is being/has been
             * closed. */
            virtual void
            discardAllGLObjects() = 0;

        protected:

            virtual ~GraphicsObjectManager();

            std::string  _name;
            unsigned int _contextID;
    };

    class OSG_EXPORT GLObjectManager : public GraphicsObjectManager
    {
        public:

            GLObjectManager( const std::string& name,
                             unsigned int       contextID );

            virtual void
            flushDeletedGLObjects( double  currentTime,
                                   double& availableTime );

            virtual void
            flushAllDeletedGLObjects();

            virtual void
            deleteAllGLObjects();

            virtual void
            discardAllGLObjects();

            /** schedule a GL object for deletion by the graphics thread.*/
            virtual void
            scheduleGLObjectForDeletion( GLuint globj );

            /** implementation of the actual creation of an GL object - subclasses from
             * GLObjectManager must implement the appropriate GL calls.*/
            virtual GLuint
            createGLObject();

            /** implementation of the actual deletion of an GL object - subclasses from
             * GLObjectManager must implement the appropriate GL calls.*/
            virtual void
            deleteGLObject( GLuint globj ) = 0;

        protected:

            virtual ~GLObjectManager();

            typedef std::list<GLuint> GLObjectHandleList;
            std::mutex                _mutex;
            GLObjectHandleList        _deleteGLObjectHandles;
    };

}
