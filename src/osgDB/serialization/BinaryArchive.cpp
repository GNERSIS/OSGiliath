/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include <osgDB/serialization/BinaryArchive.hpp>

#include <istream>
#include <ostream>

namespace osgDB::serialization
{

    BinaryArchive::BinaryArchive( std::ostream& out )
        : Archive( Direction::Write )
        , _out( &out )
        , _in( nullptr )
    {
    }

    BinaryArchive::BinaryArchive( std::istream& in )
        : Archive( Direction::Read )
        , _out( nullptr )
        , _in( &in )
    {
    }

    void BinaryArchive::beginObject( std::string_view ) {}

    void BinaryArchive::endObject() {}

    void
    BinaryArchive::beginArray( std::string_view, std::uint32_t& size )
    {
        raw( size );
    }

    void BinaryArchive::endArray() {}

    template<typename T>
    void
    BinaryArchive::raw( T& v )
    {
        if( writing() )
        {
            _out->write( reinterpret_cast<const char*>( &v ),
                         static_cast<std::streamsize>( sizeof( T ) ) );
        }
        else
        {
            _in->read( reinterpret_cast<char*>( &v ),
                       static_cast<std::streamsize>( sizeof( T ) ) );
        }
    }

    void BinaryArchive::value( std::string_view, bool& v ) { raw( v ); }
    void BinaryArchive::value( std::string_view, std::int32_t& v ) { raw( v ); }
    void BinaryArchive::value( std::string_view, std::uint32_t& v ) { raw( v ); }
    void BinaryArchive::value( std::string_view, std::int64_t& v ) { raw( v ); }
    void BinaryArchive::value( std::string_view, std::uint64_t& v ) { raw( v ); }
    void BinaryArchive::value( std::string_view, float& v ) { raw( v ); }
    void BinaryArchive::value( std::string_view, double& v ) { raw( v ); }

    void
    BinaryArchive::value( std::string_view, std::string& v )
    {
        std::uint64_t length = static_cast<std::uint64_t>( v.size() );
        raw( length );
        if( reading() )
        {
            v.resize( static_cast<std::size_t>( length ) );
        }
        if( length > 0U )
        {
            const std::streamsize count = static_cast<std::streamsize>( length );
            if( writing() )
            {
                _out->write( v.data(), count );
            }
            else
            {
                _in->read( v.data(), count );
            }
        }
    }

    void
    BinaryArchive::blob( std::string_view, std::vector<std::byte>& bytes )
    {
        std::uint64_t length = static_cast<std::uint64_t>( bytes.size() );
        raw( length );
        if( reading() )
        {
            bytes.resize( static_cast<std::size_t>( length ) );
        }
        if( length > 0U )
        {
            char* const           first = reinterpret_cast<char*>( bytes.data() );
            const std::streamsize count = static_cast<std::streamsize>( length );
            if( writing() )
            {
                _out->write( first, count );
            }
            else
            {
                _in->read( first, count );
            }
        }
    }

}
