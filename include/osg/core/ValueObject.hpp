/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Type-erased user-value container attached to Objects. Stores
 * arbitrary typed values for metadata and custom properties.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/core/UserDataContainer.hpp>
#include <osg/maths/box.hpp>
#include <osg/maths/plane.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/sphere.hpp>

namespace osg
{

    class Plane;

}

#include <typeinfo>

namespace osg
{

    // math types are 'using' aliases — include the headers instead of forward-declaring

    template<typename T>
    class GetScalarValue;
    template<typename T>
    class SetScalarValue;

    class ValueObject : public osg::Inherit<Object, ValueObject>
    {
        public:

            ValueObject() :
                Inherit( true )
            {
            }

            ValueObject( const std::string& name ) :
                Inherit( true )
            {
                setName( name );
            }

            ValueObject( const ValueObject& rhs,
                         const osg::CopyOp  copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( rhs,
                         copyop )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               ValueObject )

            /** Convert 'this' into a ValueObject pointer if Object is a ValueObject,
             * otherwise return 0. Equivalent to dynamic_cast<ValueObject*>(this).*/
            virtual ValueObject*
            asValueObject()
            {
                return this;
            }

            /** Convert 'this' into a ValueObject pointer if Object is a ValueObject,
             * otherwise return 0. Equivalent to dynamic_cast<ValueObject*>(this).*/
            virtual const ValueObject*
            asValueObject() const
            {
                return this;
            }

            class GetValueVisitor
            {
                public:

                    virtual ~GetValueVisitor()
                    {
                    }

                    virtual void
                    apply( bool /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( char /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( unsigned char /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( short /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( unsigned short /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( int /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( unsigned int /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( float /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( double /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const std::string& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::bvec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::bvec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::bvec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::ubvec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::ubvec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::ubvec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::svec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::svec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::svec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::usvec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::usvec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::usvec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::ivec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::ivec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::ivec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::uivec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::uivec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::uivec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::vec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::vec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::vec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::dvec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::dvec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::dvec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::quat& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::Plane& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::mat4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::dmat4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::box& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::dbox& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::sphere& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( const osg::dsphere& /*in_value*/ )
                    {
                    }
            };

            class SetValueVisitor
            {
                public:

                    virtual ~SetValueVisitor()
                    {
                    }

                    virtual void
                    apply( bool& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( char& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( unsigned char& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( short& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( unsigned short& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( int& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( unsigned int& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( float& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( double& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( std::string& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::bvec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::bvec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::bvec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::ubvec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::ubvec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::ubvec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::svec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::svec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::svec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::usvec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::usvec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::usvec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::ivec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::ivec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::ivec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::uivec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::uivec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::uivec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::vec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::vec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::vec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::dvec2& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::dvec3& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::dvec4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::quat& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::Plane& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::mat4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::dmat4& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::box& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::dbox& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::sphere& /*in_value*/ )
                    {
                    }

                    virtual void
                    apply( osg::dsphere& /*in_value*/ )
                    {
                    }
            };

            virtual bool
            get( GetValueVisitor& /*gvv*/ ) const
            {
                return false;
            }

            virtual bool
            set( SetValueVisitor& /*gvv*/ )
            {
                return false;
            }

            template<typename T>
            bool
            getScalarValue( T& value )
            {
                GetScalarValue<T> gsv;
                if( get( gsv ) && gsv.set )
                {
                    value = gsv.value;
                    return true;
                }
                else
                {
                    return false;
                }
            }

            template<typename T>
            bool
            setScalarValue( T value )
            {
                SetScalarValue<T> ssv( value );
                return set( ssv ) && ssv.set;
            }

        protected:

            virtual ~ValueObject()
            {
            }
    };

    template<typename T>
    class GetScalarValue : public ValueObject::GetValueVisitor
    {
        public:

            GetScalarValue() :
                set( false ),
                value( 0 )
            {
            }

            bool set;
            T    value;

            virtual void
            apply( bool in_value )
            {
                value = in_value ? 1 : 0;
                set   = true;
            }

            virtual void
            apply( char in_value )
            {
                value = in_value;
                set   = true;
            }

            virtual void
            apply( unsigned char in_value )
            {
                value = in_value;
                set   = true;
            }

            virtual void
            apply( short in_value )
            {
                value = in_value;
                set   = true;
            }

            virtual void
            apply( unsigned short in_value )
            {
                value = in_value;
                set   = true;
            }

            virtual void
            apply( int in_value )
            {
                value = in_value;
                set   = true;
            }

            virtual void
            apply( unsigned int in_value )
            {
                value = in_value;
                set   = true;
            }

            virtual void
            apply( float in_value )
            {
                value = in_value;
                set   = true;
            }

            virtual void
            apply( double in_value )
            {
                value = in_value;
                set   = true;
            }
    };

    template<>
    class GetScalarValue<bool> : public ValueObject::GetValueVisitor
    {
        public:

            GetScalarValue() :
                set( false ),
                value( 0 )
            {
            }

            bool set;
            bool value;

            virtual void
            apply( bool in_value )
            {
                value = in_value;
                set   = true;
            }

            virtual void
            apply( char in_value )
            {
                value = in_value != 0;
                set   = true;
            }

            virtual void
            apply( unsigned char in_value )
            {
                value = in_value != 0;
                set   = true;
            }

            virtual void
            apply( short in_value )
            {
                value = in_value != 0;
                set   = true;
            }

            virtual void
            apply( unsigned short in_value )
            {
                value = in_value != 0;
                set   = true;
            }

            virtual void
            apply( int in_value )
            {
                value = in_value != 0;
                set   = true;
            }

            virtual void
            apply( unsigned int in_value )
            {
                value = in_value != 0;
                set   = true;
            }

            virtual void
            apply( float in_value )
            {
                value = in_value != 0.0F;
                set   = true;
            }

            virtual void
            apply( double in_value )
            {
                value = in_value != 0.0;
                set   = true;
            }
    };

    template<typename T>
    class SetScalarValue : public ValueObject::SetValueVisitor
    {
        public:

            SetScalarValue( T in_value ) :
                set( false ),
                value( in_value )
            {
            }

            bool set;
            T    value;

            virtual void
            apply( bool& in_value )
            {
                in_value = ( value != 0 );
                set      = true;
            }

            virtual void
            apply( char& in_value )
            {
                in_value = value;
                set      = true;
            }

            virtual void
            apply( unsigned char& in_value )
            {
                in_value = value;
                set      = true;
            }

            virtual void
            apply( short& in_value )
            {
                in_value = value;
                set      = true;
            }

            virtual void
            apply( unsigned short& in_value )
            {
                in_value = value;
                set      = true;
            }

            virtual void
            apply( int& in_value )
            {
                in_value = value;
                set      = true;
            }

            virtual void
            apply( unsigned int& in_value )
            {
                in_value = value;
                set      = true;
            }

            virtual void
            apply( float& in_value )
            {
                in_value = value;
                set      = true;
            }

            virtual void
            apply( double& in_value )
            {
                in_value = value;
                set      = true;
            }
    };

    template<typename T>
    struct ValueObjectClassNameTrait
    {
            static const char*
            className()
            {
                return "TemplateValueObject";
            }
    };

    template<typename T>
    class TemplateValueObject : public ValueObject
    {
        public:

            TemplateValueObject() :
                ValueObject(),
                _value()
            {
            }

            TemplateValueObject( const T& value ) :
                ValueObject(),
                _value( value )
            {
            }

            TemplateValueObject( const std::string& name,
                                 const T&           value ) :
                ValueObject( name ),
                _value( value )
            {
            }

            TemplateValueObject( const TemplateValueObject& rhs,
                                 const osg::CopyOp copyop = osg::CopyOp::SHALLOW_COPY ) :
                ValueObject( rhs,
                             copyop ),
                _value( rhs._value )
            {
            }

            virtual Object*
            cloneType() const
            {
                return new TemplateValueObject();
            }

            virtual Object*
            clone( const CopyOp& copyop ) const
            {
                return new TemplateValueObject( *this, copyop );
            }

            virtual bool
            isSameKindAs( const Object* obj ) const
            {
                return dynamic_cast<const TemplateValueObject*>( obj ) != NULL;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return ValueObjectClassNameTrait<T>::className();
            }

            void
            setValue( const T& value )
            {
                _value = value;
            }

            const T&
            getValue() const
            {
                return _value;
            }

            virtual bool
            get( GetValueVisitor& gvv ) const
            {
                gvv.apply( _value );
                return true;
            }

            virtual bool
            set( SetValueVisitor& svv )
            {
                svv.apply( _value );
                return true;
            }

        protected:

            virtual ~TemplateValueObject()
            {
            }

            static const char* s_TemplateValueObject_className;

            T                  _value;
    };

#define META_ValueObject( TYPE, NAME )      \
    template<>                              \
    struct ValueObjectClassNameTrait<TYPE>  \
    {                                       \
            static const char*              \
            className()                     \
            {                               \
                return #NAME;               \
            }                               \
    };                                      \
    typedef TemplateValueObject<TYPE> NAME;

    META_ValueObject(
        std::string,
        StringValueObject
    ) META_ValueObject( bool,
                        BoolValueObject ) META_ValueObject( char,
                                                            CharValueObject ) META_ValueObject( unsigned char,
                                                                                                UCharValueObject ) META_ValueObject( short,
                                                                                                                                     ShortValueObject ) META_ValueObject( unsigned short,
                                                                                                                                                                          UShortValueObject ) META_ValueObject( int,
                                                                                                                                                                                                                IntValueObject ) META_ValueObject( unsigned int,
                                                                                                                                                                                                                                                   UIntValueObject ) META_ValueObject( float,
                                                                                                                                                                                                                                                                                       FloatValueObject ) META_ValueObject( double,
                                                                                                                                                                                                                                                                                                                            DoubleValueObject ) META_ValueObject( vec2,
                                                                                                                                                                                                                                                                                                                                                                  Vec2fValueObject ) META_ValueObject( vec3,
                                                                                                                                                                                                                                                                                                                                                                                                       Vec3fValueObject ) META_ValueObject( vec4,
                                                                                                                                                                                                                                                                                                                                                                                                                                            Vec4fValueObject ) META_ValueObject( dvec2,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 Vec2dValueObject ) META_ValueObject( dvec3,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      Vec3dValueObject ) META_ValueObject( dvec4,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           Vec4dValueObject ) META_ValueObject( quat,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                QuatValueObject ) META_ValueObject( Plane,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    PlaneValueObject ) META_ValueObject( mat4,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         MatrixfValueObject ) META_ValueObject( dmat4,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                MatrixdValueObject ) META_ValueObject( box,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       BoundingBoxfValueObject ) META_ValueObject( dbox,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   BoundingBoxdValueObject ) META_ValueObject( sphere,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               BoundingSpherefValueObject ) META_ValueObject( dsphere,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              BoundingSpheredValueObject )

        /** provide implementation of osg::Object::getUserValue(..) template*/
        template<typename T>
        bool osg::Object::getUserValue( const std::string& name,
                                        T&                 value ) const
    {
        typedef TemplateValueObject<T> UserValueObject;

        const osg::UserDataContainer*  udc = asUserDataContainer();
        if( !udc )
        {
            udc = _userDataContainer;
        }

        if( !udc )
        {
            return false;
        }

        const Object* obj = udc->getUserObject( name );
        if( obj && typeid( *obj ) == typeid( UserValueObject ) )
        {
            const UserValueObject* uvo = static_cast<const UserValueObject*>( obj );
            value                      = uvo->getValue();
            return true;
        }
        else
        {
            return false;
        }
    }

    /** provide implementation of osg::Object::setUserValue(..) template.*/
    template<typename T>
    void
    osg::Object::setUserValue( const std::string& name,
                               const T&           value )
    {
        typedef TemplateValueObject<T> UserValueObject;

        osg::UserDataContainer*        udc = asUserDataContainer();
        if( !udc )
        {
            getOrCreateUserDataContainer();
            udc = _userDataContainer;
        }

        unsigned int i = udc->getUserObjectIndex( name );
        if( i < udc->getNumUserObjects() )
        {
            Object* obj = udc->getUserObject( i );
            if( typeid( *obj ) == typeid( UserValueObject ) )
            {
                UserValueObject* uvo = static_cast<UserValueObject*>( obj );
                uvo->setValue( value );
            }
            else
            {
                udc->setUserObject( i, new UserValueObject( name, value ) );
            }
        }
        else
        {
            udc->addUserObject( new UserValueObject( name, value ) );
        }
    }

    template<class P,
             class T>
    T*
    getOrCreateUserObjectOfType( P* parent )
    {
        T*                      object = 0;
        const char*             name   = typeid( T ).name();
        osg::UserDataContainer* udc    = parent->getOrCreateUserDataContainer();
        unsigned int            index  = udc->getUserObjectIndex( name );
        if( index < udc->getNumUserObjects() )
        {
            osg::Object* userObject = udc->getUserObject( index );
            if( typeid( *userObject ) == typeid( T ) )
            {
                object = static_cast<T*>( userObject );
                // OSG_NOTICE<<"Reusing "<<name<<std::endl;
            }
            else
            {
                // OSG_NOTICE<<"Replacing "<<name<<", original object
                // "<<userObject->className()<<std::endl;

                object = new T;
                object->setName( name );
                udc->setUserObject( index, object );
            }
        }
        else
        {
            object = new T;
            object->setName( name );
            udc->addUserObject( object );
            // OSG_NOTICE<<"Creating new "<<name<<std::endl;
        }
        return object;
    }

}
