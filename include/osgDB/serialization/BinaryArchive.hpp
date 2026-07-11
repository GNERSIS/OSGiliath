/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <osgDB/serialization/Archive.hpp>

#include <iosfwd>

namespace osgDB::serialization
{

    /** Reference backend: packed little-endian POD over a std::iostream.
     *  Property names are ignored (positional encoding). Generalizes the
     *  hand-rolled Sponza bake-cache streaming. */
    class OSGDB_EXPORT BinaryArchive final : public Archive
    {
        public:
            explicit BinaryArchive( std::ostream& out );
            explicit BinaryArchive( std::istream& in );

            void beginObject( std::string_view name ) override;
            void endObject() override;
            void beginArray( std::string_view name, std::uint32_t& size ) override;
            void endArray() override;

            void value( std::string_view name, bool& v ) override;
            void value( std::string_view name, std::int32_t& v ) override;
            void value( std::string_view name, std::uint32_t& v ) override;
            void value( std::string_view name, std::int64_t& v ) override;
            void value( std::string_view name, std::uint64_t& v ) override;
            void value( std::string_view name, float& v ) override;
            void value( std::string_view name, double& v ) override;
            void value( std::string_view name, std::string& v ) override;
            void blob( std::string_view        name,
                       std::vector<std::byte>& bytes ) override;

        private:
            template<typename T>
            void raw( T& v );

            std::ostream* _out;
            std::istream* _in;
    };

}
