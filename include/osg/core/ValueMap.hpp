/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * String-keyed map of ValueObjects. Provides dict-like access
 * to user metadata on scene graph objects.
 */
#pragma once

#include <map>
#include <osg/core/Inherit.hpp>
#include <osg/core/Notify.hpp>
#include <osg/core/ValueObject.hpp>

namespace osg
{

#define OSG_HAS_VALUEMAP

    class OSG_EXPORT ValueMap : public osg::Inherit<osg::Object, ValueMap>
    {
        public:

            ValueMap();

            ValueMap( const ValueMap&    vm,
                      const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               ValueMap )

            typedef std::map<osg::ref_ptr<const osg::Referenced>,
                             osg::ref_ptr<osg::Object>>
                KeyValueMap;

            void
            setKeyValueMap( KeyValueMap& properties )
            {
                _keyValueMap = properties;
            }

            KeyValueMap&
            getKeyValueMap()
            {
                return _keyValueMap;
            }

            const KeyValueMap&
            getKeyValueMap() const
            {
                return _keyValueMap;
            }

            osg::Object*
            setValue( const osg::Referenced* key,
                      osg::Object*           object )
            {
                return ( _keyValueMap[key] = object ).get();
            }

            template<typename T>
            osg::Object*
            setValue( const osg::Referenced* key,
                      const T&               value )
            {
                typedef TemplateValueObject<T> UserValueObject;
                KeyValueMap::iterator          itr = _keyValueMap.find( key );
                if( itr != _keyValueMap.end() )
                {
                    osg::Object* obj = itr->second.get();
                    if( typeid( *( obj ) ) == typeid( UserValueObject ) )
                    {
                        UserValueObject* uvo =
                            static_cast<UserValueObject*>( itr->second.get() );
                        uvo->setValue( value );
                        return uvo;
                    }
                }

                return ( _keyValueMap[key] = new UserValueObject( value ) ).get();
            }

            inline osg::Object*
            getValue( const osg::Referenced* key )
            {
                KeyValueMap::iterator itr = _keyValueMap.find( key );
                return ( itr != _keyValueMap.end() ) ? itr->second.get() : 0;
            }

            inline const osg::Object*
            getValue( const osg::Referenced* key ) const
            {
                KeyValueMap::const_iterator itr = _keyValueMap.find( key );
                return ( itr != _keyValueMap.end() ) ? itr->second.get() : 0;
            }

            template<typename T>
            T*
            getValueOfType( const osg::Referenced* key )
            {
                Object* object = getValue( key );
                return ( object && typeid( *object ) == typeid( T ) )
                         ? static_cast<T*>( object )
                         : 0;
            }

            template<typename T>
            const T*
            getValueOfType( const osg::Referenced* key ) const
            {
                const Object* object = getValue( key );
                return ( object && typeid( *object ) == typeid( T ) )
                         ? static_cast<const T*>( object )
                         : 0;
            }

            template<typename T>
            bool
            getValue( const osg::Referenced* key,
                      T&                     value )
            {
                typedef TemplateValueObject<T> UserValueObject;
                UserValueObject* uvo = getValueOfType<UserValueObject>( key );
                if( uvo )
                {
                    value = uvo->getValue();
                    return true;
                }
                else
                {
                    return false;
                }
            }

            template<typename T>
            bool
            getValue( const osg::Referenced* key,
                      T&                     value ) const
            {
                typedef TemplateValueObject<T> UserValueObject;
                const UserValueObject* uvo = getValueOfType<UserValueObject>( key );
                if( uvo )
                {
                    value = uvo->getValue();
                    return true;
                }
                else
                {
                    return false;
                }
            }

        protected:

            virtual ~ValueMap();

            KeyValueMap _keyValueMap;
    };

}
