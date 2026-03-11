/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Cross-platform file stream wrappers. Handles UTF-8 paths
 * on Windows where std::fstream requires wide strings.
 */
#pragma once

#include <fstream>
#include <osg/core/Export.hpp>
#include <osgDB/Export>

namespace osgDB
{

    /**
     * Convenience function for fstream open , std::ifstream, and std::ofstream to
     * automatically handle UTF-8 to UTF-16 filename conversion. Always use one
     * of these classes in any OpenSceneGraph code instead of the STL equivalent.
     */

    void OSGDB_EXPORT
    open( std::fstream&           fs,
          const char*             filename,
          std::ios_base::openmode mode );

    class ifstream : public std::ifstream
    {
        public:

            OSGDB_EXPORT
            ifstream();
            OSGDB_EXPORT explicit ifstream( const char*             filename,
                                            std::ios_base::openmode mode =
                                                std::ios_base::in );
            OSGDB_EXPORT ~ifstream();

            void OSGDB_EXPORT
            open( const char*             filename,
                  std::ios_base::openmode mode = std::ios_base::in );
    };

    class ofstream : public std::ofstream
    {
        public:

            OSGDB_EXPORT
            ofstream();
            OSGDB_EXPORT explicit ofstream( const char*             filename,
                                            std::ios_base::openmode mode =
                                                std::ios_base::out );
            OSGDB_EXPORT ~ofstream();

            void OSGDB_EXPORT
            open( const char*             filename,
                  std::ios_base::openmode mode = std::ios_base::out );
    };

}
