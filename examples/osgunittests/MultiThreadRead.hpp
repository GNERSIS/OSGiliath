/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * MultiThreadRead example application
 */
#ifndef MULTITHREADEDREAD_H
    #define MULTITHREADEDREAD_H 1

    #include <osg/core/ArgumentParser.hpp>
    #include <osg/maths/compat.hpp>

extern void
runMultiThreadReadTests( int                  numThreads,
                         osg::ArgumentParser& arguments );

#endif
