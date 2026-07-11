/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <osgDB/serialization/Archive.hpp>

#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace osgDB::serialization
{

    /** FlexBuffers backend for structured, key-addressable archive data.
     *  The public header hides FlatBuffers implementation details so osgDB's
     *  vendored include can stay private to the library target. */
    class OSGDB_EXPORT FlexBufferArchive final : public Archive
    {
        public:
            explicit FlexBufferArchive( std::vector<std::uint8_t>& out );
            explicit FlexBufferArchive( std::span<const std::uint8_t> in );
            ~FlexBufferArchive() override;

            FlexBufferArchive( const FlexBufferArchive& )            = delete;
            FlexBufferArchive& operator=( const FlexBufferArchive& ) = delete;

            void finish();
            const std::vector<std::uint8_t>& buffer() const;

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
            class Impl;
            std::unique_ptr<Impl> _impl;
    };

}
