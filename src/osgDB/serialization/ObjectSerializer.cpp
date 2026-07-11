/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include <osgDB/serialization/ObjectSerializer.hpp>

#include <osg/core/Notify.hpp>

#include <stdexcept>
#include <string>
#include <unordered_set>

namespace osgDB::serialization
{
    namespace
    {

        constexpr std::uint32_t kInlineObjectReference = 0U;

        // Warn once per unregistered compound type name (cook traversal is
        // single-threaded). Keeps a full survey readable without flooding.
        void
        warnUnregisteredType( const std::string& name )
        {
            static std::unordered_set<std::string> alreadyWarned;
            if( alreadyWarned.insert( name ).second )
            {
                OSG_WARN << "Serialization: no serializer for " << name
                         << " — dropped from cook" << std::endl;
            }
        }

    }

    SerializerRegistry&
    SerializerRegistry::instance()
    {
        static SerializerRegistry registry;
        return registry;
    }

    void
    SerializerRegistry::add( std::string     compoundName,
                             ObjectFactory   factory,
                             TypeErasedWrite describe )
    {
        _byName.insert_or_assign( std::move( compoundName ),
                                  Entry{ std::move( factory ), std::move( describe ) } );
    }

    osg::ref_ptr<osg::Object>
    SerializerRegistry::createByName( std::string_view name ) const
    {
        const auto it = _byName.find( std::string( name ) );
        return it != _byName.end() ? it->second.factory() : osg::ref_ptr<osg::Object>();
    }

    const TypeErasedWrite*
    SerializerRegistry::describerForName( std::string_view name ) const
    {
        const auto it = _byName.find( std::string( name ) );
        return it != _byName.end() ? &it->second.describe : nullptr;
    }

    void
    serialize( Archive&                   ar,
               osg::ref_ptr<osg::Object>& obj )
    {
        Archive::ObjectSerializationSession session( ar );
        SerializerRegistry&                 registry = SerializerRegistry::instance();
        ar.beginObject( "object" );
        if( ar.writing() )
        {
            // An unregistered type is dropped with a logged warning (design
            // §11.2), not written and never silent. Decide serializability
            // BEFORE writing "Present" so the object is cleanly absent.
            const TypeErasedWrite* describe =
                obj.valid() ? registry.describerForName( obj->getCompoundClassName() )
                            : nullptr;
            if( obj.valid() && describe == nullptr )
            {
                warnUnregisteredType( obj->getCompoundClassName() );
            }

            bool present = obj.valid() && describe != nullptr;
            ar.value( "Present", present );
            if( !present )
            {
                ar.endObject();
                return;
            }

            std::uint32_t objectId       = 0U;
            const bool    alreadyWritten = ar.resolveWriteObjectId( *obj, objectId );
            std::uint32_t referenceId =
                alreadyWritten ? objectId : kInlineObjectReference;
            ar.value( "Ref", referenceId );
            if( alreadyWritten )
            {
                ar.endObject();
                return;
            }

            ar.value( "Id", objectId );
            std::string name = obj->getCompoundClassName();
            ar.value( "Type", name );
            ( *describe )( ar, *obj );
            ar.endObject();
        }
        else
        {
            bool present = false;
            ar.value( "Present", present );
            if( !present )
            {
                obj = nullptr;
                ar.endObject();
                return;
            }

            std::uint32_t referenceId = kInlineObjectReference;
            ar.value( "Ref", referenceId );
            if( referenceId != kInlineObjectReference )
            {
                obj = ar.readObjectById( referenceId );
                if( !obj.valid() )
                {
                    throw std::runtime_error( "Object reference id not found" );
                }
                ar.endObject();
                return;
            }

            std::uint32_t objectId = 0U;
            ar.value( "Id", objectId );
            std::string name;
            ar.value( "Type", name );
            obj                             = registry.createByName( name );
            const TypeErasedWrite* describe = registry.describerForName( name );
            if( !obj.valid() || describe == nullptr )
            {
                throw std::runtime_error(
                    std::string( "No serializer registered for " ) + name
                );
            }
            ar.rememberReadObject( objectId, obj );
            ( *describe )( ar, *obj );
            ar.endObject();
        }
    }

}
