/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include <osgDB/serialization/FlexBufferArchive.hpp>

#include <flatbuffers/flexbuffers.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>

namespace osgDB::serialization
{
    namespace
    {
        enum class ScopeKind
        {
            Map,
            Vector
        };

        void
        require( bool condition, const char* message )
        {
            if( !condition )
            {
                throw std::runtime_error( message );
            }
        }
    }

    class FlexBufferArchive::Impl
    {
        public:
            explicit Impl( std::vector<std::uint8_t>& out )
                : output( &out )
                , input()
                , root()
                , finished( false )
            {
                output->clear();
            }

            explicit Impl( std::span<const std::uint8_t> in )
                : output( nullptr )
                , input( in )
                , root()
                , finished( true )
            {
                require( !input.empty(), "Cannot read an empty FlexBufferArchive" );
                root = flexbuffers::GetRoot( input.data(), input.size() );
            }

            struct WriteScope
            {
                    ScopeKind   kind;
                    std::size_t token;
            };

            struct ReadScope
            {
                    ScopeKind              kind;
                    flexbuffers::Reference reference;
                    std::size_t            index;
            };

            bool writing() const noexcept { return output != nullptr; }

            void writeKey( std::string_view name )
            {
                builder.Key( name.data(), name.size() );
            }

            bool writingMapValue() const
            {
                return !writeScopes.empty() &&
                       writeScopes.back().kind == ScopeKind::Map;
            }

            void beginWriteMap( std::string_view name )
            {
                if( writingMapValue() )
                {
                    writeKey( name );
                }
                const std::size_t token = builder.StartMap();
                writeScopes.push_back( WriteScope{ ScopeKind::Map, token } );
            }

            void beginWriteVector( std::string_view name )
            {
                if( writingMapValue() )
                {
                    writeKey( name );
                }
                const std::size_t token = builder.StartVector();
                writeScopes.push_back( WriteScope{ ScopeKind::Vector, token } );
            }

            flexbuffers::Reference nextReadReference( std::string_view name )
            {
                if( readScopes.empty() )
                {
                    if( root.IsMap() && !name.empty() )
                    {
                        return root.AsMap()[std::string( name )];
                    }
                    return root;
                }

                ReadScope& scope = readScopes.back();
                if( scope.kind == ScopeKind::Map )
                {
                    return scope.reference.AsMap()[std::string( name )];
                }

                flexbuffers::Vector vector = scope.reference.AsVector();
                flexbuffers::Reference value = vector[scope.index];
                ++scope.index;
                return value;
            }

            flexbuffers::Builder              builder;
            std::vector<std::uint8_t>*        output;
            std::span<const std::uint8_t>     input;
            flexbuffers::Reference            root;
            std::vector<WriteScope>           writeScopes;
            std::vector<ReadScope>            readScopes;
            bool                              finished;
    };

    FlexBufferArchive::FlexBufferArchive( std::vector<std::uint8_t>& out )
        : Archive( Direction::Write )
        , _impl( std::make_unique<Impl>( out ) )
    {
    }

    FlexBufferArchive::FlexBufferArchive( std::span<const std::uint8_t> in )
        : Archive( Direction::Read )
        , _impl( std::make_unique<Impl>( in ) )
    {
    }

    FlexBufferArchive::~FlexBufferArchive() = default;

    void
    FlexBufferArchive::finish()
    {
        require( writing(), "Cannot finish a read-mode FlexBufferArchive" );
        if( _impl->finished )
        {
            return;
        }
        require( _impl->writeScopes.empty(),
                 "Cannot finish FlexBufferArchive with open scopes" );
        _impl->builder.Finish();
        require( !_impl->builder.HasDuplicateKeys(),
                 "FlexBufferArchive map contains duplicate keys" );
        *_impl->output = _impl->builder.GetBuffer();
        _impl->finished = true;
    }

    const std::vector<std::uint8_t>&
    FlexBufferArchive::buffer() const
    {
        require( writing(), "Read-mode FlexBufferArchive does not own a buffer" );
        require( _impl->finished, "FlexBufferArchive buffer requested before finish" );
        return *_impl->output;
    }

    void
    FlexBufferArchive::beginObject( std::string_view name )
    {
        if( writing() )
        {
            _impl->beginWriteMap( name );
            return;
        }

        flexbuffers::Reference ref =
            _impl->readScopes.empty() ? _impl->root
                                      : _impl->nextReadReference( name );
        require( ref.IsMap(), "FlexBufferArchive expected map object" );
        _impl->readScopes.push_back(
            Impl::ReadScope{ ScopeKind::Map, ref, 0U } );
    }

    void
    FlexBufferArchive::endObject()
    {
        if( writing() )
        {
            require( !_impl->writeScopes.empty() &&
                         _impl->writeScopes.back().kind == ScopeKind::Map,
                     "FlexBufferArchive object scope mismatch" );
            const std::size_t token = _impl->writeScopes.back().token;
            _impl->builder.EndMap( token );
            _impl->writeScopes.pop_back();
            return;
        }

        require( !_impl->readScopes.empty() &&
                     _impl->readScopes.back().kind == ScopeKind::Map,
                 "FlexBufferArchive object scope mismatch" );
        _impl->readScopes.pop_back();
    }

    void
    FlexBufferArchive::beginArray( std::string_view name, std::uint32_t& size )
    {
        if( writing() )
        {
            _impl->beginWriteVector( name );
            return;
        }

        flexbuffers::Reference ref =
            _impl->readScopes.empty() ? _impl->root
                                      : _impl->nextReadReference( name );
        require( ref.IsAnyVector() && !ref.IsMap(),
                 "FlexBufferArchive expected vector array" );
        const std::size_t vectorSize = ref.AsVector().size();
        require( vectorSize <=
                     static_cast<std::size_t>(
                         std::numeric_limits<std::uint32_t>::max() ),
                 "FlexBufferArchive array too large" );
        size = static_cast<std::uint32_t>( vectorSize );
        _impl->readScopes.push_back(
            Impl::ReadScope{ ScopeKind::Vector, ref, 0U } );
    }

    void
    FlexBufferArchive::endArray()
    {
        if( writing() )
        {
            require( !_impl->writeScopes.empty() &&
                         _impl->writeScopes.back().kind == ScopeKind::Vector,
                     "FlexBufferArchive array scope mismatch" );
            const std::size_t token = _impl->writeScopes.back().token;
            _impl->builder.EndVector( token, false, false );
            _impl->writeScopes.pop_back();
            return;
        }

        require( !_impl->readScopes.empty() &&
                     _impl->readScopes.back().kind == ScopeKind::Vector,
                 "FlexBufferArchive array scope mismatch" );
        _impl->readScopes.pop_back();
    }

    void
    FlexBufferArchive::value( std::string_view name, bool& v )
    {
        if( writing() )
        {
            if( _impl->writingMapValue() )
            {
                _impl->writeKey( name );
            }
            _impl->builder.Bool( v );
            return;
        }

        flexbuffers::Reference ref = _impl->nextReadReference( name );
        require( !ref.IsNull(), "FlexBufferArchive missing bool value" );
        v = ref.AsBool();
    }

    void
    FlexBufferArchive::value( std::string_view name, std::int32_t& v )
    {
        if( writing() )
        {
            if( _impl->writingMapValue() )
            {
                _impl->writeKey( name );
            }
            _impl->builder.Int( static_cast<std::int64_t>( v ) );
            return;
        }

        flexbuffers::Reference ref = _impl->nextReadReference( name );
        require( !ref.IsNull(), "FlexBufferArchive missing int32 value" );
        v = ref.AsInt32();
    }

    void
    FlexBufferArchive::value( std::string_view name, std::uint32_t& v )
    {
        if( writing() )
        {
            if( _impl->writingMapValue() )
            {
                _impl->writeKey( name );
            }
            _impl->builder.UInt( static_cast<std::uint64_t>( v ) );
            return;
        }

        flexbuffers::Reference ref = _impl->nextReadReference( name );
        require( !ref.IsNull(), "FlexBufferArchive missing uint32 value" );
        v = ref.AsUInt32();
    }

    void
    FlexBufferArchive::value( std::string_view name, std::int64_t& v )
    {
        if( writing() )
        {
            if( _impl->writingMapValue() )
            {
                _impl->writeKey( name );
            }
            _impl->builder.Int( v );
            return;
        }

        flexbuffers::Reference ref = _impl->nextReadReference( name );
        require( !ref.IsNull(), "FlexBufferArchive missing int64 value" );
        v = ref.AsInt64();
    }

    void
    FlexBufferArchive::value( std::string_view name, std::uint64_t& v )
    {
        if( writing() )
        {
            if( _impl->writingMapValue() )
            {
                _impl->writeKey( name );
            }
            _impl->builder.UInt( v );
            return;
        }

        flexbuffers::Reference ref = _impl->nextReadReference( name );
        require( !ref.IsNull(), "FlexBufferArchive missing uint64 value" );
        v = ref.AsUInt64();
    }

    void
    FlexBufferArchive::value( std::string_view name, float& v )
    {
        if( writing() )
        {
            if( _impl->writingMapValue() )
            {
                _impl->writeKey( name );
            }
            _impl->builder.Float( v );
            return;
        }

        flexbuffers::Reference ref = _impl->nextReadReference( name );
        require( !ref.IsNull(), "FlexBufferArchive missing float value" );
        v = ref.AsFloat();
    }

    void
    FlexBufferArchive::value( std::string_view name, double& v )
    {
        if( writing() )
        {
            if( _impl->writingMapValue() )
            {
                _impl->writeKey( name );
            }
            _impl->builder.Double( v );
            return;
        }

        flexbuffers::Reference ref = _impl->nextReadReference( name );
        require( !ref.IsNull(), "FlexBufferArchive missing double value" );
        v = ref.AsDouble();
    }

    void
    FlexBufferArchive::value( std::string_view name, std::string& v )
    {
        if( writing() )
        {
            if( _impl->writingMapValue() )
            {
                _impl->writeKey( name );
            }
            _impl->builder.String( v );
            return;
        }

        flexbuffers::Reference ref = _impl->nextReadReference( name );
        require( ref.IsString(), "FlexBufferArchive missing string value" );
        v = ref.AsString().str();
    }

    void
    FlexBufferArchive::blob( std::string_view        name,
                             std::vector<std::byte>& bytes )
    {
        if( writing() )
        {
            if( _impl->writingMapValue() )
            {
                _impl->writeKey( name );
            }
            _impl->builder.Blob( static_cast<const void*>( bytes.data() ),
                                 bytes.size() );
            return;
        }

        flexbuffers::Reference ref = _impl->nextReadReference( name );
        require( ref.IsBlob(), "FlexBufferArchive missing blob value" );
        flexbuffers::Blob flexBlob = ref.AsBlob();
        bytes.resize( flexBlob.size() );
        if( flexBlob.size() > 0U )
        {
            std::memcpy( bytes.data(), flexBlob.data(), flexBlob.size() );
        }
    }

}
