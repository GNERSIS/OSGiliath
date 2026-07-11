/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include <osgDB/serialization/SceneCook.hpp>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <ios>
#include <limits>
#include <osg/core/Notify.hpp>

#include <chrono>
#include <osgDB/io/fstream.hpp>
#include <osgDB/serialization/FlexBufferArchive.hpp>
#include <osgDB/serialization/ObjectSerializer.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace osgDB::serialization
{
    namespace
    {

        constexpr std::array<std::uint8_t, 8U> kSceneCookMagic{
            'O',
            'S',
            'G',
            'C',
            'O',
            'O',
            'K',
            '\0'
        };
        constexpr std::size_t kUint32Size       = sizeof( std::uint32_t );
        constexpr std::size_t kUint64Size       = sizeof( std::uint64_t );
        constexpr std::size_t kPayloadSizeOffset =
            kSceneCookMagic.size() + kUint32Size;
        constexpr std::size_t kChecksumOffset = kPayloadSizeOffset + kUint64Size;
        constexpr std::size_t kHeaderSize     = kChecksumOffset + kUint64Size;
        constexpr std::uint64_t kFnvOffsetBasis = 14695981039346656037ULL;
        constexpr std::uint64_t kFnvPrime       = 1099511628211ULL;
        constexpr unsigned int  kBitsPerByte    = 8U;
        constexpr std::uint64_t kByteMask       = 0XFFULL;

        std::uint64_t
        fnv1a64( std::span<const std::uint8_t> bytes )
        {
            std::uint64_t hash = kFnvOffsetBasis;
            for( const std::uint8_t byte : bytes )
            {
                hash ^= static_cast<std::uint64_t>( byte );
                hash *= kFnvPrime;
            }
            return hash;
        }

        template<typename T>
        void
        appendLittleEndian( std::vector<std::uint8_t>& bytes,
                            T                          value )
        {
            static_assert( std::is_unsigned_v<T> );
            for( std::size_t byteIndex = 0U; byteIndex < sizeof( T ); ++byteIndex )
            {
                const unsigned int shift =
                    static_cast<unsigned int>( byteIndex ) * kBitsPerByte;
                bytes.push_back(
                    static_cast<std::uint8_t>( ( value >> shift ) & kByteMask )
                );
            }
        }

        template<typename T>
        T
        readLittleEndian( std::span<const std::uint8_t> bytes,
                          std::size_t                   offset )
        {
            static_assert( std::is_unsigned_v<T> );
            T value = 0U;
            for( std::size_t byteIndex = 0U; byteIndex < sizeof( T ); ++byteIndex )
            {
                const unsigned int shift =
                    static_cast<unsigned int>( byteIndex ) * kBitsPerByte;
                value |= static_cast<T>( bytes[offset + byteIndex] ) << shift;
            }
            return value;
        }

        bool
        writeBytes( osgDB::ofstream&                 output,
                    std::span<const std::uint8_t>    bytes )
        {
            if( bytes.size() >
                static_cast<std::size_t>(
                    std::numeric_limits<std::streamsize>::max()
                ) )
            {
                return false;
            }
            output.write( reinterpret_cast<const char*>( bytes.data() ),
                          static_cast<std::streamsize>( bytes.size() ) );
            return static_cast<bool>( output );
        }

        // Read-only memory map of a cook file. Avoids reading multi-GB payloads
        // into a heap buffer; FlexBuffers reads directly from the mapping and
        // the pages fault in lazily as the deserializer touches them.
        class MappedFile
        {
            public:
                explicit MappedFile( const std::string& path )
                {
                    const int fd = ::open( path.c_str(), O_RDONLY );
                    if( fd < 0 )
                    {
                        return;
                    }
                    struct stat status
                    {
                    };
                    if( ::fstat( fd, &status ) == 0 && status.st_size > 0 )
                    {
                        const std::size_t size =
                            static_cast<std::size_t>( status.st_size );
                        void* mapping =
                            ::mmap( nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0 );
                        if( mapping != MAP_FAILED )
                        {
                            _data = static_cast<const std::uint8_t*>( mapping );
                            _size = size;
                        }
                    }
                    ::close( fd );
                }

                ~MappedFile()
                {
                    if( _data != nullptr )
                    {
                        ::munmap( const_cast<std::uint8_t*>( _data ), _size );
                    }
                }

                MappedFile( const MappedFile& )            = delete;
                MappedFile& operator=( const MappedFile& ) = delete;
                MappedFile( MappedFile&& )                 = delete;
                MappedFile& operator=( MappedFile&& )      = delete;

                bool valid() const { return _data != nullptr; }
                std::span<const std::uint8_t> bytes() const
                {
                    return { _data, _size };
                }

            private:
                const std::uint8_t* _data = nullptr;
                std::size_t         _size = 0U;
        };

        bool
        validateSceneCook( const std::string&                path,
                           std::span<const std::uint8_t>    file,
                           std::span<const std::uint8_t>&   payload,
                           bool                             verifyChecksum )
        {
            if( file.size() < kHeaderSize )
            {
                OSG_WARN << "SceneCook: truncated header in " << path << std::endl;
                return false;
            }

            const std::span<const std::uint8_t> magic = file.first(
                kSceneCookMagic.size()
            );
            if( !std::equal( magic.begin(), magic.end(), kSceneCookMagic.begin() ) )
            {
                OSG_WARN << "SceneCook: bad magic in " << path << std::endl;
                return false;
            }

            const std::uint32_t formatVersion =
                readLittleEndian<std::uint32_t>( file, kSceneCookMagic.size() );
            if( formatVersion != sceneCookFormatVersion )
            {
                OSG_WARN << "SceneCook: unsupported format version " << formatVersion
                         << " in " << path << std::endl;
                return false;
            }

            const std::uint64_t payloadSize =
                readLittleEndian<std::uint64_t>( file, kPayloadSizeOffset );
            if( payloadSize >
                static_cast<std::uint64_t>(
                    std::numeric_limits<std::size_t>::max()
                ) )
            {
                OSG_WARN << "SceneCook: payload too large in " << path << std::endl;
                return false;
            }

            const std::size_t payloadSizeAsSizeT =
                static_cast<std::size_t>( payloadSize );
            if( payloadSizeAsSizeT > file.size() - kHeaderSize )
            {
                OSG_WARN << "SceneCook: truncated payload in " << path << std::endl;
                return false;
            }
            if( file.size() != kHeaderSize + payloadSizeAsSizeT )
            {
                OSG_WARN << "SceneCook: payload size mismatch in " << path
                         << std::endl;
                return false;
            }

            payload = file.subspan( kHeaderSize, payloadSizeAsSizeT );
            // The full-payload hash is O(payload) — several seconds on a multi-GB
            // cook. Header + size validation always runs; the content checksum is
            // opt-in (OSG_COOK_VERIFY) for locally produced caches.
            if( verifyChecksum )
            {
                const std::uint64_t expectedChecksum =
                    readLittleEndian<std::uint64_t>( file, kChecksumOffset );
                const std::uint64_t actualChecksum = fnv1a64( payload );
                if( actualChecksum != expectedChecksum )
                {
                    OSG_WARN << "SceneCook: checksum mismatch in " << path << std::endl;
                    return false;
                }
            }

            return true;
        }

    }

    bool
    writeSceneCook( const osg::Object& root,
                    const std::string& path )
    {
        std::vector<std::uint8_t> payload;
        try
        {
            FlexBufferArchive out( payload );
            osg::ref_ptr<osg::Object> object =
                const_cast<osg::Object*>( &root );
            try
            {
                serialize( out, object );
                out.finish();
            }
            catch( ... )
            {
                object.release();
                throw;
            }
            object.release();
        }
        catch( const std::exception& e )
        {
            OSG_WARN << "SceneCook: could not serialize " << path << ": " << e.what()
                     << std::endl;
            return false;
        }

        const std::uint64_t payloadSize =
            static_cast<std::uint64_t>( payload.size() );
        std::vector<std::uint8_t> header;
        header.reserve( kHeaderSize );
        header.insert( header.end(), kSceneCookMagic.begin(), kSceneCookMagic.end() );
        appendLittleEndian( header, sceneCookFormatVersion );
        appendLittleEndian( header, payloadSize );
        appendLittleEndian( header, fnv1a64( payload ) );

        osgDB::ofstream output(
            path.c_str(),
            std::ios::binary | std::ios::out | std::ios::trunc
        );
        if( !output )
        {
            OSG_WARN << "SceneCook: could not open " << path << " for write"
                     << std::endl;
            return false;
        }

        if( !writeBytes( output, header ) || !writeBytes( output, payload ) )
        {
            OSG_WARN << "SceneCook: failed while writing " << path << std::endl;
            return false;
        }
        return true;
    }

    osg::ref_ptr<osg::Object>
    readSceneCook( const std::string& path )
    {
        const auto t0 = std::chrono::steady_clock::now();
        MappedFile mapped( path );
        if( !mapped.valid() )
        {
            OSG_WARN << "SceneCook: could not map " << path << std::endl;
            return osg::ref_ptr<osg::Object>();
        }

        std::span<const std::uint8_t> payload;
        const bool verifyChecksum = std::getenv( "OSG_COOK_NOVERIFY" ) == nullptr;
        if( !validateSceneCook( path, mapped.bytes(), payload, verifyChecksum ) )
        {
            return osg::ref_ptr<osg::Object>();
        }
        const auto t1 = std::chrono::steady_clock::now();

        try
        {
            FlexBufferArchive           in( payload );
            osg::ref_ptr<osg::Object>   object;
            serialize( in, object );
            const auto t2 = std::chrono::steady_clock::now();
            OSG_NOTICE
                << "SceneCook profile: read+validate "
                << std::chrono::duration_cast<std::chrono::milliseconds>( t1 - t0 ).count()
                << " ms, deserialize "
                << std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count()
                << " ms" << std::endl;
            return object;
        }
        catch( const std::exception& e )
        {
            OSG_WARN << "SceneCook: could not deserialize " << path << ": " << e.what()
                     << std::endl;
            return osg::ref_ptr<osg::Object>();
        }
    }

}
