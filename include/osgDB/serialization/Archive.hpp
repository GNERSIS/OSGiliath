/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <cstddef>
#include <cstdint>
#include <osg/core/Object.hpp>
#include <osgDB/Export.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace osgDB::serialization
{

    class Archive;

    OSGDB_EXPORT void
    serialize( Archive&                   ar,
               osg::ref_ptr<osg::Object>& obj );

    /** Bidirectional serialization sink. One serialize() body serves both read
     *  and write; the concrete archive knows its direction and either writes
     *  from, or reads into, each referenced value. Unified, cereal-style
     *  successor to osgDB::OutputStream/InputStream. */
    class OSGDB_EXPORT Archive
    {
        public:

            enum class Direction
            {
                Read,
                Write
            };

            virtual ~Archive() = default;

            Direction
            direction() const noexcept
            {
                return _direction;
            }

            bool
            writing() const noexcept
            {
                return _direction == Direction::Write;
            }

            bool
            reading() const noexcept
            {
                return _direction == Direction::Read;
            }

            virtual void
            beginObject( std::string_view name ) = 0;
            virtual void
            endObject() = 0;
            virtual void
            beginArray( std::string_view name,
                        std::uint32_t&   size ) = 0;
            virtual void
            endArray() = 0;

            virtual void
            value( std::string_view name,
                   bool&            v ) = 0;
            virtual void
            value( std::string_view name,
                   std::int32_t&    v ) = 0;
            virtual void
            value( std::string_view name,
                   std::uint32_t&   v ) = 0;
            virtual void
            value( std::string_view name,
                   std::int64_t&    v ) = 0;
            virtual void
            value( std::string_view name,
                   std::uint64_t&   v ) = 0;
            virtual void
            value( std::string_view name,
                   float&           v ) = 0;
            virtual void
            value( std::string_view name,
                   double&          v ) = 0;
            virtual void
            value( std::string_view name,
                   std::string&     v ) = 0;

            /** Bulk region: on write emits the vector, on read resizes + fills.
             *  Zero-copy mmap layout is an M2 (FlexBuffers) concern; the binary
             *  backend copies. */
            virtual void
            blob( std::string_view        name,
                  std::vector<std::byte>& bytes ) = 0;

        protected:

            explicit Archive( Direction dir ) noexcept :
                _direction( dir )
            {
            }

            void
            beginObjectSerializationSession()
            {
                if( _objectSerializationDepth == 0U )
                {
                    _writtenObjectIds.clear();
                    _readObjectsById.clear();
                    _nextObjectId = 1U;
                }
                ++_objectSerializationDepth;
            }

            void
            endObjectSerializationSession() noexcept
            {
                if( _objectSerializationDepth > 0U )
                {
                    --_objectSerializationDepth;
                }
            }

            bool
            resolveWriteObjectId( const osg::Object& object,
                                  std::uint32_t&     id )
            {
                const auto [it, inserted] =
                    _writtenObjectIds.emplace( &object, _nextObjectId );
                id = it->second;
                if( inserted )
                {
                    ++_nextObjectId;
                    return false;
                }
                return true;
            }

            osg::ref_ptr<osg::Object>
            readObjectById( std::uint32_t id ) const
            {
                const auto it = _readObjectsById.find( id );
                return it != _readObjectsById.end() ? it->second
                                                    : osg::ref_ptr<osg::Object>();
            }

            void
            rememberReadObject( std::uint32_t             id,
                                osg::ref_ptr<osg::Object> object )
            {
                _readObjectsById.insert_or_assign( id, std::move( object ) );
            }

            class ObjectSerializationSession
            {
                public:

                    explicit ObjectSerializationSession( Archive& ar ) :
                        _archive( ar )
                    {
                        _archive.beginObjectSerializationSession();
                    }

                    ObjectSerializationSession( const ObjectSerializationSession& ) =
                        delete;
                    ObjectSerializationSession&
                    operator=( const ObjectSerializationSession& ) = delete;

                    ~ObjectSerializationSession()
                    {
                        _archive.endObjectSerializationSession();
                    }

                private:

                    Archive& _archive;
            };

        private:

            friend void
                          serialize( Archive&                   ar,
                                     osg::ref_ptr<osg::Object>& obj );

            Direction     _direction;

            std::uint32_t _nextObjectId             = 1U;
            std::uint32_t _objectSerializationDepth = 0U;
            std::unordered_map<const osg::Object*, std::uint32_t> _writtenObjectIds;
            std::unordered_map<std::uint32_t, osg::ref_ptr<osg::Object>>
                _readObjectsById;
    };

}
