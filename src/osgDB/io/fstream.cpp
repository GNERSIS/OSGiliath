/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Cross-platform file stream wrappers. Handles UTF-8 paths
 * on Windows where std::fstream requires wide strings.
 */
#include <osgDB/io/fstream.hpp>

#include <osg/Config>
#include <osgDB/io/ConvertUTF.hpp>

namespace osgDB
{

#ifdef OSG_USE_UTF8_FILENAME
    #define OSGDB_CONVERT_UTF8_FILENAME( s ) convertUTF8toUTF16( s ).c_str()
#else
    #define OSGDB_CONVERT_UTF8_FILENAME( s ) s
#endif

    void
    open( std::fstream&           fs,
          const char*             filename,
          std::ios_base::openmode mode )
    {
        fs.open( OSGDB_CONVERT_UTF8_FILENAME( filename ), mode );
    }

    ifstream::ifstream()
    {
    }

    ifstream::ifstream( const char*             filename,
                        std::ios_base::openmode mode ) :
        std::ifstream( OSGDB_CONVERT_UTF8_FILENAME( filename ),
                       mode )
    {
    }

    ifstream::~ifstream()
    {
    }

    void
    ifstream::open( const char*             filename,
                    std::ios_base::openmode mode )
    {
        std::ifstream::open( OSGDB_CONVERT_UTF8_FILENAME( filename ), mode );
    }

    ofstream::ofstream()
    {
    }

    ofstream::ofstream( const char*             filename,
                        std::ios_base::openmode mode ) :
        std::ofstream( OSGDB_CONVERT_UTF8_FILENAME( filename ),
                       mode )
    {
    }

    ofstream::~ofstream()
    {
    }

    void
    ofstream::open( const char*             filename,
                    std::ios_base::openmode mode )
    {
        std::ofstream::open( OSGDB_CONVERT_UTF8_FILENAME( filename ), mode );
    }

}
