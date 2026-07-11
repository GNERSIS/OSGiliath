/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#pragma once

#include <functional>
#include <osg/core/Object.hpp>
#include <osgDB/Export.hpp>
#include <osgDB/serialization/Archive.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace osgDB::serialization
{

    using ObjectFactory   = std::function<osg::ref_ptr<osg::Object>()>;
    using TypeErasedWrite = std::function<void( Archive&, osg::Object& )>;

    /** Runtime table mapping a compound class name ("osg::Group") to its
     *  factory and type-erased serialize(). Keyed by NAME, not type_info, so
     *  legacy non-Inherit types (StateSet/Array/Image) work too. Modern
     *  successor to OSG's ObjectWrapperManager. */
    class OSGDB_EXPORT SerializerRegistry
    {
        public:

            static SerializerRegistry&
            instance();

            void
            add( std::string     compoundName,
                 ObjectFactory   factory,
                 TypeErasedWrite describe );

            osg::ref_ptr<osg::Object>
            createByName( std::string_view name ) const;
            const TypeErasedWrite*
            describerForName( std::string_view name ) const;

        private:

            struct Entry
            {
                    ObjectFactory   factory;
                    TypeErasedWrite describe;
            };

            std::unordered_map<std::string, Entry> _byName;
    };

    /** Registrar constructs a probe instance to read the canonical compound
     *  name, guaranteeing the registered key matches what serialize() looks up
     *  at write time (obj->getCompoundClassName()). The default constructor
     *  covers concrete Inherit types; the factory overload covers legacy
     *  Object subclasses without create(). */
    template<typename T>
    struct SerializerRegistrar
    {
            SerializerRegistrar()
            {
                osg::ref_ptr<osg::Object> probe = T::create();
                SerializerRegistry::instance().add(
                    probe->getCompoundClassName(),
                    []
                    {
                        return osg::ref_ptr<osg::Object>( T::create() );
                    },
                    []( Archive& ar, osg::Object& o )
                    {
                        serialize( ar, static_cast<T&>( o ) );
                    }
                );
            }

            explicit SerializerRegistrar( ObjectFactory factory )
            {
                osg::ref_ptr<osg::Object> probe = factory();
                if( !probe.valid() )
                {
                    throw std::runtime_error(
                        "Serializer factory returned a null probe object"
                    );
                }

                SerializerRegistry::instance().add(
                    probe->getCompoundClassName(),
                    std::move( factory ),
                    []( Archive& ar, osg::Object& o )
                    {
                        serialize( ar, static_cast<T&>( o ) );
                    }
                );
            }
    };

    /** Nullable, polymorphic object (de)serialization: present flag, compound
     *  name tag, dispatch to the registered describe(). */
    OSGDB_EXPORT void
    serialize( Archive&                   ar,
               osg::ref_ptr<osg::Object>& obj );

}

#define OSG_REGISTER_SERIALIZER( LIB, CLASS )                          \
    static const osgDB::serialization::SerializerRegistrar<LIB::CLASS> \
        s_serializer_##CLASS

#define OSGDB_SERIALIZATION_CONCAT_IMPL( A, B ) A##B
#define OSGDB_SERIALIZATION_CONCAT( A, B )      OSGDB_SERIALIZATION_CONCAT_IMPL( A, B )

#define OSG_REGISTER_SERIALIZER_FACTORY( TYPE, FACTORY_EXPR )      \
    static const osgDB::serialization::SerializerRegistrar<TYPE>   \
    OSGDB_SERIALIZATION_CONCAT( s_serializer_factory_, __LINE__ )( \
        []() -> osg::ref_ptr<osg::Object>                          \
        {                                                          \
            return osg::ref_ptr<osg::Object>( FACTORY_EXPR );      \
        }                                                          \
    )
