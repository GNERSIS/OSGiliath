/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * OutputIterator, derived from Referenced.
 * Provides: setStream, getStream, getStream, setOutputStream, getOutputStream,
 * getOutputStream.
 */
#include <osgDB/serialization/StreamOperator.hpp>

#include <osgDB/serialization/InputStream.hpp>

using namespace osgDB;

void
InputIterator::checkStream() const
{
    if( _in->rdstate() & _in->failbit )
    {
        OSG_NOTICE << "InputIterator::checkStream() : _in->rdstate() " << _in->rdstate()
                   << ", " << _in->failbit << std::endl;
        OSG_NOTICE << "                               _in->tellg() = " << _in->tellg()
                   << std::endl;
        _failed = true;
    }
}

void
InputIterator::readComponentArray( char*        s,
                                   unsigned int numElements,
                                   unsigned int numComponentsPerElements,
                                   unsigned int componentSizeInBytes )
{
    unsigned int size = numElements * numComponentsPerElements * componentSizeInBytes;
    if( size > 0 )
    {
        readCharArray( s, size );

        if( _byteSwap && componentSizeInBytes > 1 )
        {
            char* ptr = s;
            for( unsigned int i = 0; i < numElements; ++i )
            {
                for( unsigned int j = 0; j < numComponentsPerElements; ++j )
                {
                    osg::swapBytes( ptr, componentSizeInBytes );
                    ptr += componentSizeInBytes;
                }
            }
        }
    }
}

void
InputIterator::throwException( const std::string& msg )
{
    if( _inputStream )
    {
        _inputStream->throwException( msg );
    }
    else
    {
        OSG_WARN << msg << std::endl;
    }
}
