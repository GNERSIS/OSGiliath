/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stack of ValueMaps for hierarchical metadata resolution.
 * Used internally for layered property lookups.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/ValueMap.hpp>

namespace osg
{

#define OSG_HAS_VALUESTACK

    class OSG_EXPORT ValueStack : public osg::Inherit<osg::Object, ValueStack>
    {
        public:

            ValueStack();

            ValueStack( const ValueStack&  ps,
                        const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               ValueStack )

            typedef std::vector<osg::ref_ptr<Object>>                     Values;
            typedef std::map<osg::ref_ptr<const osg::Referenced>, Values> ValueStackMap;

            void
            setValueStackMap( ValueStackMap& pm )
            {
                _valuesMap = pm;
            }

            ValueStackMap&
            getValueStackMap()
            {
                return _valuesMap;
            }

            const ValueStackMap&
            getValueStackMap() const
            {
                return _valuesMap;
            }

            inline void
            push( const Referenced* key,
                  Object*           value )
            {
                _valuesMap[key].push_back( value );
            }

            template<typename T>
            void
            push( const osg::Referenced* key,
                  const T&               value )
            {
                typedef TemplateValueObject<T> UserValueObject;
                _valuesMap[key].push_back( new UserValueObject( value ) );
            }

            inline void
            pop( const Referenced* key )
            {
                _valuesMap[key].pop_back();
            }

            inline void
            push( ValueMap* valueMap )
            {
                if( valueMap )
                {
                    ValueMap::KeyValueMap& keyValueMap = valueMap->getKeyValueMap();
                    for( ValueMap::KeyValueMap::iterator itr = keyValueMap.begin();
                         itr != keyValueMap.end();
                         ++itr )
                    {
                        push( itr->first.get(), itr->second.get() );
                    }
                }
            }

            inline void
            pop( ValueMap* valueMap )
            {
                if( valueMap )
                {
                    ValueMap::KeyValueMap& keyValueMap = valueMap->getKeyValueMap();
                    for( ValueMap::KeyValueMap::iterator itr = keyValueMap.begin();
                         itr != keyValueMap.end();
                         ++itr )
                    {
                        pop( itr->first.get() );
                    }
                }
            }

            inline osg::Object*
            getValue( const osg::Referenced* key )
            {
                ValueStackMap::iterator itr = _valuesMap.find( key );
                if( itr == _valuesMap.end() )
                {
                    return 0;
                }

                Values& values = itr->second;
                if( values.empty() )
                {
                    return 0;
                }

                return values.back().get();
            }

            inline const osg::Object*
            getValue( const osg::Referenced* key ) const
            {
                ValueStackMap::const_iterator itr = _valuesMap.find( key );
                if( itr == _valuesMap.end() )
                {
                    return 0;
                }

                const Values& values = itr->second;
                if( values.empty() )
                {
                    return 0;
                }

                return values.back().get();
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

            virtual ~ValueStack();

            ValueStackMap _valuesMap;
    };

}
