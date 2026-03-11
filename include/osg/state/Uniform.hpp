/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Typed uniform variable for GLSL shaders. Supports scalars, vectors,
 * matrices, and samplers with dirty tracking for efficient uploads.
 */
/* file:   include/osg/Uniform
 * author: Mike Weiblen 2008-01-02
 */

#pragma once

#include <osg/core/Callback.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/geometry/Array.hpp>
#include <osg/maths/Plane.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/rendering/GLExtensions.hpp>
#include <osg/state/UniformBase.hpp>

namespace osg
{

    template<typename T>
    struct UniformClassNameTrait
    {
            static const char*
            className()
            {
                return "TemplateUniform";
            }
    };

    template<typename T>
    class TemplateUniform : public osg::UniformBase
    {
        public:

            typedef T value_type;

            TemplateUniform()
            {
            }

            TemplateUniform( const std::string& name ) :
                osg::UniformBase( name )
            {
            }

            TemplateUniform( const value_type& value ) :
                _value( value )
            {
            }

            TemplateUniform( const std::string& name,
                             const value_type&  value ) :
                osg::UniformBase( name ),
                _value( value )
            {
            }

            TemplateUniform( const TemplateUniform& rhs,
                             const osg::CopyOp copyop = osg::CopyOp::SHALLOW_COPY ) :
                osg::UniformBase( rhs,
                                  copyop ),
                _value( rhs._value )
            {
            }

            virtual Object*
            cloneType() const
            {
                return new TemplateUniform();
            }

            virtual Object*
            clone( const CopyOp& copyop ) const
            {
                return new TemplateUniform( *this, copyop );
            }

            virtual bool
            isSameKindAs( const Object* obj ) const
            {
                return dynamic_cast<const TemplateUniform*>( obj ) != NULL;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return UniformClassNameTrait<T>::className();
            }

            virtual void
            apply( const osg::GLExtensions* ext,
                   GLint                    location ) const
            {
                ext->glUniform( location, _value );
            }

            void
            setValue( const value_type& value )
            {
                if( _value != value )
                {
                    _value = value;
                    dirty();
                }
            }

            value_type&
            getValue() noexcept
            {
                return _value;
            }

            const value_type&
            getValue() const noexcept
            {
                return _value;
            }

        protected:

            value_type _value;
    };

#define META_Uniform( TYPE, NAME )      \
    template<>                          \
    struct UniformClassNameTrait<TYPE>  \
    {                                   \
            static const char*          \
            className()                 \
            {                           \
                return #NAME;           \
            }                           \
    };                                  \
    typedef TemplateUniform<TYPE> NAME;

    META_Uniform(
        int,
        IntUniform
    ) META_Uniform( unsigned int,
                    UIntUniform ) META_Uniform( float,
                                                FloatUniform ) META_Uniform( double,
                                                                             DoubleUniform ) META_Uniform( osg::
                                                                                                               ivec2,
                                                                                                           Vec2iUniform ) META_Uniform( osg::
                                                                                                                                            ivec3,
                                                                                                                                        Vec3iUniform ) META_Uniform( osg::
                                                                                                                                                                         ivec4,
                                                                                                                                                                     Vec4iUniform ) META_Uniform( osg::
                                                                                                                                                                                                      uivec2,
                                                                                                                                                                                                  Vec2uiUniform ) META_Uniform( osg::
                                                                                                                                                                                                                                    uivec3,
                                                                                                                                                                                                                                Vec3uiUniform ) META_Uniform( osg::
                                                                                                                                                                                                                                                                  uivec4,
                                                                                                                                                                                                                                                              Vec4uiUniform ) META_Uniform( osg::
                                                                                                                                                                                                                                                                                                vec2,
                                                                                                                                                                                                                                                                                            Vec2Uniform ) META_Uniform( osg::
                                                                                                                                                                                                                                                                                                                            vec3,
                                                                                                                                                                                                                                                                                                                        Vec3Uniform ) META_Uniform( osg::
                                                                                                                                                                                                                                                                                                                                                        vec4,
                                                                                                                                                                                                                                                                                                                                                    Vec4Uniform ) META_Uniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                    dvec2,
                                                                                                                                                                                                                                                                                                                                                                                Vec2dUniform ) META_Uniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                 dvec3,
                                                                                                                                                                                                                                                                                                                                                                                                             Vec3dUniform ) META_Uniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                                              dvec4,
                                                                                                                                                                                                                                                                                                                                                                                                                                          Vec4dUniform ) META_Uniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                                                                           Plane,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                       PlaneUniform ) META_Uniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        mat4,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    MatrixfUniform ) META_Uniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       dmat4,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   MatrixdUniform ) template<typename T>
    struct ArrayUniformClassNameTrait
    {
            static const char*
            className()
            {
                return "TemplateArrayUniform";
            }
    };

    template<typename T>
    class TemplateArrayUniform : public osg::UniformBase
    {
        public:

            typedef T                       value_type;
            typedef std::vector<value_type> array_type;

            TemplateArrayUniform()
            {
            }

            TemplateArrayUniform( const std::string& name ) :
                osg::UniformBase( name )
            {
            }

            TemplateArrayUniform( const std::string& name,
                                  unsigned int       new_size ) :
                osg::UniformBase( name ),
                _array( new_size )
            {
            }

            TemplateArrayUniform( const TemplateArrayUniform& rhs,
                                  const osg::CopyOp           copyop =
                                      osg::CopyOp::SHALLOW_COPY ) :
                osg::UniformBase( rhs,
                                  copyop ),
                _array( rhs._array )
            {
            }

            virtual Object*
            cloneType() const
            {
                return new TemplateArrayUniform();
            }

            virtual Object*
            clone( const CopyOp& copyop ) const
            {
                return new TemplateArrayUniform( *this, copyop );
            }

            virtual bool
            isSameKindAs( const Object* obj ) const
            {
                return dynamic_cast<const TemplateArrayUniform*>( obj ) != NULL;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return ArrayUniformClassNameTrait<T>::className();
            }

            virtual void
            apply( const osg::GLExtensions* ext,
                   GLint                    location ) const
            {
                ext->glUniform( location, _array );
            }

            bool
            empty() const noexcept
            {
                return _array.empty();
            }

            void
            resize( unsigned int new_size )
            {
                _array.resize( new_size );
            }

            unsigned int
            size() const noexcept
            {
                return static_cast<unsigned int>( _array.size() );
            }

            void
            setElement( unsigned int      i,
                        const value_type& value )
            {
                if( _array[i] != value )
                {
                    _array[i] = value;
                    dirty();
                }
            }

            value_type&
            getElement( unsigned int i ) noexcept
            {
                return _array[i];
            }

            const value_type&
            getElement( unsigned int i ) const noexcept
            {
                return _array[i];
            }

            void
            setArray( const array_type& array )
            {
                if( _array != array )
                {
                    _array = array;
                    dirty();
                }
            }

            array_type&
            getArray() noexcept
            {
                return _array;
            }

            const array_type&
            getArray() const noexcept
            {
                return _array;
            }

        protected:

            array_type _array;
    };

#define META_ArrayUniform( TYPE, NAME )      \
    template<>                               \
    struct ArrayUniformClassNameTrait<TYPE>  \
    {                                        \
            static const char*               \
            className()                      \
            {                                \
                return #NAME;                \
            }                                \
    };                                       \
    typedef TemplateArrayUniform<TYPE> NAME;

    META_ArrayUniform( int,
                       IntArrayUniform ) META_ArrayUniform( unsigned int,
                                                            UIntArrayUniform ) META_ArrayUniform( float,
                                                                                                  FloatArrayUniform ) META_ArrayUniform( double,
                                                                                                                                         DoubleArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                     vec2,
                                                                                                                                                                                 Vec2ArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                           vec3,
                                                                                                                                                                                                                       Vec3ArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                                                                 vec4,
                                                                                                                                                                                                                                                             Vec4ArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                                                                                                       dvec2,
                                                                                                                                                                                                                                                                                                   Vec2dArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                                                                                                                                              dvec3,
                                                                                                                                                                                                                                                                                                                                          Vec3dArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                     dvec4,
                                                                                                                                                                                                                                                                                                                                                                                 Vec4dArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                            ivec2,
                                                                                                                                                                                                                                                                                                                                                                                                                        Vec2iArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ivec3,
                                                                                                                                                                                                                                                                                                                                                                                                                                                               Vec3iArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          ivec4,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      Vec4iArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 uivec2,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             Vec2uiArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         uivec3,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     Vec3uiArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 uivec4,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             Vec4uiArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         mat4,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     MatrixfArrayUniform ) META_ArrayUniform( osg::
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  dmat4,
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              MatrixdArrayUniform )

        /** Deprecated Uniform provides backwards compatibility. */
        class OSG_EXPORT Uniform : public osg::Inherit<UniformBase,
                                                       Uniform>
    {
        public:

            enum Type
            {
                FLOAT                            = GL_FLOAT,
                FLOAT_VEC2                       = GL_FLOAT_VEC2,
                FLOAT_VEC3                       = GL_FLOAT_VEC3,
                FLOAT_VEC4                       = GL_FLOAT_VEC4,

                DOUBLE                           = GL_DOUBLE,
                DOUBLE_VEC2                      = GL_DOUBLE_VEC2,
                DOUBLE_VEC3                      = GL_DOUBLE_VEC3,
                DOUBLE_VEC4                      = GL_DOUBLE_VEC4,

                INT                              = GL_INT,
                INT_VEC2                         = GL_INT_VEC2,
                INT_VEC3                         = GL_INT_VEC3,
                INT_VEC4                         = GL_INT_VEC4,

                UNSIGNED_INT                     = GL_UNSIGNED_INT,
                UNSIGNED_INT_VEC2                = GL_UNSIGNED_INT_VEC2_EXT,
                UNSIGNED_INT_VEC3                = GL_UNSIGNED_INT_VEC3_EXT,
                UNSIGNED_INT_VEC4                = GL_UNSIGNED_INT_VEC4_EXT,

                BOOL                             = GL_BOOL,
                BOOL_VEC2                        = GL_BOOL_VEC2,
                BOOL_VEC3                        = GL_BOOL_VEC3,
                BOOL_VEC4                        = GL_BOOL_VEC4,

                INT64                            = GL_INT64_ARB,
                UNSIGNED_INT64                   = GL_UNSIGNED_INT64_ARB,

                FLOAT_MAT2                       = GL_FLOAT_MAT2,
                FLOAT_MAT3                       = GL_FLOAT_MAT3,
                FLOAT_MAT4                       = GL_FLOAT_MAT4,
                FLOAT_MAT2x3                     = GL_FLOAT_MAT2x3,
                FLOAT_MAT2x4                     = GL_FLOAT_MAT2x4,
                FLOAT_MAT3x2                     = GL_FLOAT_MAT3x2,
                FLOAT_MAT3x4                     = GL_FLOAT_MAT3x4,
                FLOAT_MAT4x2                     = GL_FLOAT_MAT4x2,
                FLOAT_MAT4x3                     = GL_FLOAT_MAT4x3,

                DOUBLE_MAT2                      = GL_DOUBLE_MAT2,
                DOUBLE_MAT3                      = GL_DOUBLE_MAT3,
                DOUBLE_MAT4                      = GL_DOUBLE_MAT4,
                DOUBLE_MAT2x3                    = GL_DOUBLE_MAT2x3,
                DOUBLE_MAT2x4                    = GL_DOUBLE_MAT2x4,
                DOUBLE_MAT3x2                    = GL_DOUBLE_MAT3x2,
                DOUBLE_MAT3x4                    = GL_DOUBLE_MAT3x4,
                DOUBLE_MAT4x2                    = GL_DOUBLE_MAT4x2,
                DOUBLE_MAT4x3                    = GL_DOUBLE_MAT4x3,

                SAMPLER_1D                       = GL_SAMPLER_1D,
                SAMPLER_2D                       = GL_SAMPLER_2D,
                SAMPLER_3D                       = GL_SAMPLER_3D,
                SAMPLER_CUBE                     = GL_SAMPLER_CUBE,
                SAMPLER_1D_SHADOW                = GL_SAMPLER_1D_SHADOW,
                SAMPLER_2D_SHADOW                = GL_SAMPLER_2D_SHADOW,
                SAMPLER_1D_ARRAY                 = GL_SAMPLER_1D_ARRAY_EXT,
                SAMPLER_2D_ARRAY                 = GL_SAMPLER_2D_ARRAY_EXT,
                SAMPLER_CUBE_MAP_ARRAY           = GL_SAMPLER_CUBE_MAP_ARRAY,
                SAMPLER_1D_ARRAY_SHADOW          = GL_SAMPLER_1D_ARRAY_SHADOW_EXT,
                SAMPLER_2D_ARRAY_SHADOW          = GL_SAMPLER_2D_ARRAY_SHADOW_EXT,
                SAMPLER_2D_MULTISAMPLE           = GL_SAMPLER_2D_MULTISAMPLE,
                SAMPLER_2D_MULTISAMPLE_ARRAY     = GL_SAMPLER_2D_MULTISAMPLE_ARRAY,
                SAMPLER_CUBE_SHADOW              = GL_SAMPLER_CUBE_SHADOW_EXT,
                SAMPLER_CUBE_MAP_ARRAY_SHADOW    = GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW,
                SAMPLER_BUFFER                   = GL_SAMPLER_BUFFER_EXT,
                SAMPLER_2D_RECT                  = GL_SAMPLER_2D_RECT,
                SAMPLER_2D_RECT_SHADOW           = GL_SAMPLER_2D_RECT_SHADOW,

                INT_SAMPLER_1D                   = GL_INT_SAMPLER_1D_EXT,
                INT_SAMPLER_2D                   = GL_INT_SAMPLER_2D_EXT,
                INT_SAMPLER_3D                   = GL_INT_SAMPLER_3D_EXT,
                INT_SAMPLER_CUBE                 = GL_INT_SAMPLER_CUBE_EXT,
                INT_SAMPLER_1D_ARRAY             = GL_INT_SAMPLER_1D_ARRAY_EXT,
                INT_SAMPLER_2D_ARRAY             = GL_INT_SAMPLER_2D_ARRAY_EXT,
                INT_SAMPLER_CUBE_MAP_ARRAY       = GL_INT_SAMPLER_CUBE_MAP_ARRAY,
                INT_SAMPLER_2D_MULTISAMPLE       = GL_INT_SAMPLER_2D_MULTISAMPLE,
                INT_SAMPLER_2D_MULTISAMPLE_ARRAY = GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
                INT_SAMPLER_BUFFER               = GL_INT_SAMPLER_BUFFER_EXT,
                INT_SAMPLER_2D_RECT              = GL_INT_SAMPLER_2D_RECT_EXT,

                UNSIGNED_INT_SAMPLER_1D          = GL_UNSIGNED_INT_SAMPLER_1D_EXT,
                UNSIGNED_INT_SAMPLER_2D          = GL_UNSIGNED_INT_SAMPLER_2D_EXT,
                UNSIGNED_INT_SAMPLER_3D          = GL_UNSIGNED_INT_SAMPLER_3D_EXT,
                UNSIGNED_INT_SAMPLER_CUBE        = GL_UNSIGNED_INT_SAMPLER_CUBE_EXT,
                UNSIGNED_INT_SAMPLER_1D_ARRAY    = GL_UNSIGNED_INT_SAMPLER_1D_ARRAY_EXT,
                UNSIGNED_INT_SAMPLER_2D_ARRAY    = GL_UNSIGNED_INT_SAMPLER_2D_ARRAY_EXT,
                UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY =
                    GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY,
                UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE =
                    GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,
                UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY =
                    GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
                UNSIGNED_INT_SAMPLER_BUFFER       = GL_UNSIGNED_INT_SAMPLER_BUFFER_EXT,
                UNSIGNED_INT_SAMPLER_2D_RECT      = GL_UNSIGNED_INT_SAMPLER_2D_RECT_EXT,

                IMAGE_1D                          = GL_IMAGE_1D,
                IMAGE_2D                          = GL_IMAGE_2D,
                IMAGE_3D                          = GL_IMAGE_3D,
                IMAGE_2D_RECT                     = GL_IMAGE_2D_RECT,
                IMAGE_CUBE                        = GL_IMAGE_CUBE,
                IMAGE_BUFFER                      = GL_IMAGE_BUFFER,
                IMAGE_1D_ARRAY                    = GL_IMAGE_1D_ARRAY,
                IMAGE_2D_ARRAY                    = GL_IMAGE_2D_ARRAY,
                IMAGE_CUBE_MAP_ARRAY              = GL_IMAGE_CUBE_MAP_ARRAY,
                IMAGE_2D_MULTISAMPLE              = GL_IMAGE_2D_MULTISAMPLE,
                IMAGE_2D_MULTISAMPLE_ARRAY        = GL_IMAGE_2D_MULTISAMPLE_ARRAY,

                INT_IMAGE_1D                      = GL_INT_IMAGE_1D,
                INT_IMAGE_2D                      = GL_INT_IMAGE_2D,
                INT_IMAGE_3D                      = GL_INT_IMAGE_3D,
                INT_IMAGE_2D_RECT                 = GL_INT_IMAGE_2D_RECT,
                INT_IMAGE_CUBE                    = GL_INT_IMAGE_CUBE,
                INT_IMAGE_BUFFER                  = GL_INT_IMAGE_BUFFER,
                INT_IMAGE_1D_ARRAY                = GL_INT_IMAGE_1D_ARRAY,
                INT_IMAGE_2D_ARRAY                = GL_INT_IMAGE_2D_ARRAY,
                INT_IMAGE_CUBE_MAP_ARRAY          = GL_INT_IMAGE_CUBE_MAP_ARRAY,
                INT_IMAGE_2D_MULTISAMPLE          = GL_INT_IMAGE_2D_MULTISAMPLE,
                INT_IMAGE_2D_MULTISAMPLE_ARRAY    = GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY,

                UNSIGNED_INT_IMAGE_1D             = GL_UNSIGNED_INT_IMAGE_1D,
                UNSIGNED_INT_IMAGE_2D             = GL_UNSIGNED_INT_IMAGE_2D,
                UNSIGNED_INT_IMAGE_3D             = GL_UNSIGNED_INT_IMAGE_3D,
                UNSIGNED_INT_IMAGE_2D_RECT        = GL_UNSIGNED_INT_IMAGE_2D_RECT,
                UNSIGNED_INT_IMAGE_CUBE           = GL_UNSIGNED_INT_IMAGE_CUBE,
                UNSIGNED_INT_IMAGE_BUFFER         = GL_UNSIGNED_INT_IMAGE_BUFFER,
                UNSIGNED_INT_IMAGE_1D_ARRAY       = GL_UNSIGNED_INT_IMAGE_1D_ARRAY,
                UNSIGNED_INT_IMAGE_2D_ARRAY       = GL_UNSIGNED_INT_IMAGE_2D_ARRAY,
                UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY = GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY,
                UNSIGNED_INT_IMAGE_2D_MULTISAMPLE = GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE,
                UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY =
                    GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY,

                UNDEFINED = 0X0,
            };

        public:

            Uniform();
            Uniform( Type               type,
                     const std::string& name,
                     int                numElements = 1 );

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            Uniform( const Uniform& rhs,
                     const CopyOp&  copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               Uniform )

            /** Convert 'this' into a Uniform pointer if Object is a Uniform, otherwise
             * return 0. Equivalent to dynamic_cast<Uniform*>(this).*/
            virtual Uniform*
            asUniform()
            {
                return this;
            }

            /** convert 'const this' into a const Uniform pointer if Object is a Uniform,
             * otherwise return 0. Equivalent to dynamic_cast<const Uniform*>(this).*/
            virtual const Uniform*
            asUniform() const
            {
                return this;
            }

            /** Set the type of glUniform, ensuring it is only set once.*/
            virtual bool
            setType( Type t );

            /** Get the type of glUniform as enum. */
            Type
            getType() const noexcept
            {
                return _type;
            }

            /** Set the length of a uniform, ensuring it is only set once (1==scalar)*/
            void
            setNumElements( unsigned int numElements );

            /** Get the number of GLSL elements of the osg::Uniform (1==scalar) */
            unsigned int
            getNumElements() const noexcept
            {
                return _numElements;
            }

            /** Get the number of elements required for the internal data array.
             * Returns 0 if the osg::Uniform is not properly configured.  */
            unsigned int
            getInternalArrayNumElements() const;

            /** Return the name of a Type enum as string. */
            static const char*
            getTypename( Type t );

            /** Return the number of components for a GLSL type. */
            static int
            getTypeNumComponents( Type t );

            /** Return the Type enum of a Uniform typename string */
            static Uniform::Type
            getTypeId( const std::string& tname );

            /** Return the GL API type corresponding to a GLSL type */
            static Type
            getGlApiType( Type t );

            /** Return the internal data array type corresponding to a GLSL type */
            static GLenum
            getInternalArrayType( Type t );

            /** Return the number that the name maps to uniquely */
            static unsigned int
            getNameID( const std::string& name );

            /** convenient scalar (non-array) constructors w/ assignment */
            explicit Uniform( const char* name,
                              float       f );
            explicit Uniform( const char* name,
                              double      d );
            explicit Uniform( const char* name,
                              int         i );
            explicit Uniform( const char*  name,
                              unsigned int ui );
            explicit Uniform( const char* name,
                              bool        b );
            explicit Uniform( const char*        name,
                              unsigned long long ull );
            explicit Uniform( const char* name,
                              long long   ll );
            Uniform( const char*      name,
                     const osg::vec2& v2 );
            Uniform( const char*      name,
                     const osg::vec3& v3 );
            Uniform( const char*      name,
                     const osg::vec4& v4 );
            Uniform( const char*       name,
                     const osg::dvec2& v2 );
            Uniform( const char*       name,
                     const osg::dvec3& v3 );
            Uniform( const char*       name,
                     const osg::dvec4& v4 );
            Uniform( const char*         name,
                     const osg::Matrix2& m2 );
            Uniform( const char*         name,
                     const osg::Matrix3& m3 );
            Uniform( const char*      name,
                     const osg::mat4& m4 );
            Uniform( const char*           name,
                     const osg::Matrix2x3& m2x3 );
            Uniform( const char*           name,
                     const osg::Matrix2x4& m2x4 );
            Uniform( const char*           name,
                     const osg::Matrix3x2& m3x2 );
            Uniform( const char*           name,
                     const osg::Matrix3x4& m3x4 );
            Uniform( const char*           name,
                     const osg::Matrix4x2& m4x2 );
            Uniform( const char*           name,
                     const osg::Matrix4x3& m4x3 );
            Uniform( const char*          name,
                     const osg::Matrix2d& m2 );
            Uniform( const char*          name,
                     const osg::Matrix3d& m3 );
            Uniform( const char*       name,
                     const osg::dmat4& m4 );
            Uniform( const char*            name,
                     const osg::Matrix2x3d& m2x3 );
            Uniform( const char*            name,
                     const osg::Matrix2x4d& m2x4 );
            Uniform( const char*            name,
                     const osg::Matrix3x2d& m3x2 );
            Uniform( const char*            name,
                     const osg::Matrix3x4d& m3x4 );
            Uniform( const char*            name,
                     const osg::Matrix4x2d& m4x2 );
            Uniform( const char*            name,
                     const osg::Matrix4x3d& m4x3 );
            Uniform( const char* name,
                     int         i0,
                     int         i1 );
            Uniform( const char* name,
                     int         i0,
                     int         i1,
                     int         i2 );
            Uniform( const char* name,
                     int         i0,
                     int         i1,
                     int         i2,
                     int         i3 );
            Uniform( const char*  name,
                     unsigned int ui0,
                     unsigned int ui1 );
            Uniform( const char*  name,
                     unsigned int ui0,
                     unsigned int ui1,
                     unsigned int ui2 );
            Uniform( const char*  name,
                     unsigned int ui0,
                     unsigned int ui1,
                     unsigned int ui2,
                     unsigned int ui3 );
            Uniform( const char* name,
                     bool        b0,
                     bool        b1 );
            Uniform( const char* name,
                     bool        b0,
                     bool        b1,
                     bool        b2 );
            Uniform( const char* name,
                     bool        b0,
                     bool        b1,
                     bool        b2,
                     bool        b3 );

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            virtual int
            compare( const UniformBase& rhs ) const;
            virtual int
            compareData( const UniformBase& rhs ) const;

            void
            copyData( const Uniform& rhs );

            /** convenient scalar (non-array) value assignment */
            bool
            set( float f );
            bool
            set( double d );
            bool
            set( int i );
            bool
            set( unsigned int ui );
            bool
            set( bool b );
            bool
            set( unsigned long long ull );
            bool
            set( long long ll );
            bool
            set( const osg::vec2& v2 );
            bool
            set( const osg::vec3& v3 );
            bool
            set( const osg::vec4& v4 );
            bool
            set( const osg::dvec2& v2 );
            bool
            set( const osg::dvec3& v3 );
            bool
            set( const osg::dvec4& v4 );
            bool
            set( const osg::Matrix2& m2 );
            bool
            set( const osg::Matrix3& m3 );
            bool
            set( const osg::mat4& m4 );
            bool
            set( const osg::Matrix2x3& m2x3 );
            bool
            set( const osg::Matrix2x4& m2x4 );
            bool
            set( const osg::Matrix3x2& m3x2 );
            bool
            set( const osg::Matrix3x4& m3x4 );
            bool
            set( const osg::Matrix4x2& m4x2 );
            bool
            set( const osg::Matrix4x3& m4x3 );
            bool
            set( const osg::Matrix2d& m2 );
            bool
            set( const osg::Matrix3d& m3 );
            bool
            set( const osg::dmat4& m4 );
            bool
            set( const osg::Matrix2x3d& m2x3 );
            bool
            set( const osg::Matrix2x4d& m2x4 );
            bool
            set( const osg::Matrix3x2d& m3x2 );
            bool
            set( const osg::Matrix3x4d& m3x4 );
            bool
            set( const osg::Matrix4x2d& m4x2 );
            bool
            set( const osg::Matrix4x3d& m4x3 );
            bool
            set( int i0,
                 int i1 );
            bool
            set( int i0,
                 int i1,
                 int i2 );
            bool
            set( int i0,
                 int i1,
                 int i2,
                 int i3 );
            bool
            set( unsigned int ui0,
                 unsigned int ui1 );
            bool
            set( unsigned int ui0,
                 unsigned int ui1,
                 unsigned int ui2 );
            bool
            set( unsigned int ui0,
                 unsigned int ui1,
                 unsigned int ui2,
                 unsigned int ui3 );
            bool
            set( bool b0,
                 bool b1 );
            bool
            set( bool b0,
                 bool b1,
                 bool b2 );
            bool
            set( bool b0,
                 bool b1,
                 bool b2,
                 bool b3 );

            /** convenient scalar (non-array) value query */
            bool
            get( float& f ) const;
            bool
            get( double& d ) const;
            bool
            get( int& i ) const;
            bool
            get( unsigned int& ui ) const;
            bool
            get( bool& b ) const;
            bool
            get( unsigned long long& ull ) const;
            bool
            get( long long& ll ) const;
            bool
            get( osg::vec2& v2 ) const;
            bool
            get( osg::vec3& v3 ) const;
            bool
            get( osg::vec4& v4 ) const;
            bool
            get( osg::dvec2& v2 ) const;
            bool
            get( osg::dvec3& v3 ) const;
            bool
            get( osg::dvec4& v4 ) const;
            bool
            get( osg::Matrix2& m2 ) const;
            bool
            get( osg::Matrix3& m3 ) const;
            bool
            get( osg::mat4& m4 ) const;
            bool
            get( osg::Matrix2x3& m2x3 ) const;
            bool
            get( osg::Matrix2x4& m2x4 ) const;
            bool
            get( osg::Matrix3x2& m3x2 ) const;
            bool
            get( osg::Matrix3x4& m3x4 ) const;
            bool
            get( osg::Matrix4x2& m4x2 ) const;
            bool
            get( osg::Matrix4x3& m4x3 ) const;
            bool
            get( osg::Matrix2d& m2 ) const;
            bool
            get( osg::Matrix3d& m3 ) const;
            bool
            get( osg::dmat4& m4 ) const;
            bool
            get( osg::Matrix2x3d& m2x3 ) const;
            bool
            get( osg::Matrix2x4d& m2x4 ) const;
            bool
            get( osg::Matrix3x2d& m3x2 ) const;
            bool
            get( osg::Matrix3x4d& m3x4 ) const;
            bool
            get( osg::Matrix4x2d& m4x2 ) const;
            bool
            get( osg::Matrix4x3d& m4x3 ) const;
            bool
            get( int& i0,
                 int& i1 ) const;
            bool
            get( int& i0,
                 int& i1,
                 int& i2 ) const;
            bool
            get( int& i0,
                 int& i1,
                 int& i2,
                 int& i3 ) const;
            bool
            get( unsigned int& ui0,
                 unsigned int& ui1 ) const;
            bool
            get( unsigned int& ui0,
                 unsigned int& ui1,
                 unsigned int& ui2 ) const;
            bool
            get( unsigned int& ui0,
                 unsigned int& ui1,
                 unsigned int& ui2,
                 unsigned int& ui3 ) const;
            bool
            get( bool& b0,
                 bool& b1 ) const;
            bool
            get( bool& b0,
                 bool& b1,
                 bool& b2 ) const;
            bool
            get( bool& b0,
                 bool& b1,
                 bool& b2,
                 bool& b3 ) const;

            /** value assignment for array uniforms */
            bool
            setElement( unsigned int index,
                        float        f );
            bool
            setElement( unsigned int index,
                        double       d );
            bool
            setElement( unsigned int index,
                        int          i );
            bool
            setElement( unsigned int index,
                        unsigned int ui );
            bool
            setElement( unsigned int index,
                        bool         b );
            bool
            setElement( unsigned int       index,
                        unsigned long long ull );
            bool
            setElement( unsigned int index,
                        long long    ll );
            bool
            setElement( unsigned int     index,
                        const osg::vec2& v2 );
            bool
            setElement( unsigned int     index,
                        const osg::vec3& v3 );
            bool
            setElement( unsigned int     index,
                        const osg::vec4& v4 );
            bool
            setElement( unsigned int      index,
                        const osg::dvec2& v2 );
            bool
            setElement( unsigned int      index,
                        const osg::dvec3& v3 );
            bool
            setElement( unsigned int      index,
                        const osg::dvec4& v4 );
            bool
            setElement( unsigned int        index,
                        const osg::Matrix2& m2 );
            bool
            setElement( unsigned int        index,
                        const osg::Matrix3& m3 );
            bool
            setElement( unsigned int     index,
                        const osg::mat4& m4 );
            bool
            setElement( unsigned int          index,
                        const osg::Matrix2x3& m2x3 );
            bool
            setElement( unsigned int          index,
                        const osg::Matrix2x4& m2x4 );
            bool
            setElement( unsigned int          index,
                        const osg::Matrix3x2& m3x2 );
            bool
            setElement( unsigned int          index,
                        const osg::Matrix3x4& m3x4 );
            bool
            setElement( unsigned int          index,
                        const osg::Matrix4x2& m4x2 );
            bool
            setElement( unsigned int          index,
                        const osg::Matrix4x3& m4x3 );
            bool
            setElement( unsigned int         index,
                        const osg::Matrix2d& m2 );
            bool
            setElement( unsigned int         index,
                        const osg::Matrix3d& m3 );
            bool
            setElement( unsigned int      index,
                        const osg::dmat4& m4 );
            bool
            setElement( unsigned int           index,
                        const osg::Matrix2x3d& m2x3 );
            bool
            setElement( unsigned int           index,
                        const osg::Matrix2x4d& m2x4 );
            bool
            setElement( unsigned int           index,
                        const osg::Matrix3x2d& m3x2 );
            bool
            setElement( unsigned int           index,
                        const osg::Matrix3x4d& m3x4 );
            bool
            setElement( unsigned int           index,
                        const osg::Matrix4x2d& m4x2 );
            bool
            setElement( unsigned int           index,
                        const osg::Matrix4x3d& m4x3 );
            bool
            setElement( unsigned int index,
                        int          i0,
                        int          i1 );
            bool
            setElement( unsigned int index,
                        int          i0,
                        int          i1,
                        int          i2 );
            bool
            setElement( unsigned int index,
                        int          i0,
                        int          i1,
                        int          i2,
                        int          i3 );
            bool
            setElement( unsigned int index,
                        unsigned int ui0,
                        unsigned int ui1 );
            bool
            setElement( unsigned int index,
                        unsigned int ui0,
                        unsigned int ui1,
                        unsigned int ui2 );
            bool
            setElement( unsigned int index,
                        unsigned int ui0,
                        unsigned int ui1,
                        unsigned int ui2,
                        unsigned int ui3 );
            bool
            setElement( unsigned int index,
                        bool         b0,
                        bool         b1 );
            bool
            setElement( unsigned int index,
                        bool         b0,
                        bool         b1,
                        bool         b2 );
            bool
            setElement( unsigned int index,
                        bool         b0,
                        bool         b1,
                        bool         b2,
                        bool         b3 );

            /** value query for array uniforms */
            bool
            getElement( unsigned int index,
                        float&       f ) const;
            bool
            getElement( unsigned int index,
                        double&      d ) const;
            bool
            getElement( unsigned int index,
                        int&         i ) const;
            bool
            getElement( unsigned int  index,
                        unsigned int& ui ) const;
            bool
            getElement( unsigned int index,
                        bool&        b ) const;
            bool
            getElement( unsigned int        index,
                        unsigned long long& ull ) const;
            bool
            getElement( unsigned int index,
                        long long&   ll ) const;
            bool
            getElement( unsigned int index,
                        osg::vec2&   v2 ) const;
            bool
            getElement( unsigned int index,
                        osg::vec3&   v3 ) const;
            bool
            getElement( unsigned int index,
                        osg::vec4&   v4 ) const;
            bool
            getElement( unsigned int index,
                        osg::dvec2&  v2 ) const;
            bool
            getElement( unsigned int index,
                        osg::dvec3&  v3 ) const;
            bool
            getElement( unsigned int index,
                        osg::dvec4&  v4 ) const;
            bool
            getElement( unsigned int  index,
                        osg::Matrix2& m2 ) const;
            bool
            getElement( unsigned int  index,
                        osg::Matrix3& m3 ) const;
            bool
            getElement( unsigned int index,
                        osg::mat4&   m4 ) const;
            bool
            getElement( unsigned int    index,
                        osg::Matrix2x3& m2x3 ) const;
            bool
            getElement( unsigned int    index,
                        osg::Matrix2x4& m2x4 ) const;
            bool
            getElement( unsigned int    index,
                        osg::Matrix3x2& m3x2 ) const;
            bool
            getElement( unsigned int    index,
                        osg::Matrix3x4& m3x4 ) const;
            bool
            getElement( unsigned int    index,
                        osg::Matrix4x2& m4x2 ) const;
            bool
            getElement( unsigned int    index,
                        osg::Matrix4x3& m4x3 ) const;
            bool
            getElement( unsigned int   index,
                        osg::Matrix2d& m2 ) const;
            bool
            getElement( unsigned int   index,
                        osg::Matrix3d& m3 ) const;
            bool
            getElement( unsigned int index,
                        osg::dmat4&  m4 ) const;
            bool
            getElement( unsigned int     index,
                        osg::Matrix2x3d& m2x3 ) const;
            bool
            getElement( unsigned int     index,
                        osg::Matrix2x4d& m2x4 ) const;
            bool
            getElement( unsigned int     index,
                        osg::Matrix3x2d& m3x2 ) const;
            bool
            getElement( unsigned int     index,
                        osg::Matrix3x4d& m3x4 ) const;
            bool
            getElement( unsigned int     index,
                        osg::Matrix4x2d& m4x2 ) const;
            bool
            getElement( unsigned int     index,
                        osg::Matrix4x3d& m4x3 ) const;
            bool
            getElement( unsigned int index,
                        int&         i0,
                        int&         i1 ) const;
            bool
            getElement( unsigned int index,
                        int&         i0,
                        int&         i1,
                        int&         i2 ) const;
            bool
            getElement( unsigned int index,
                        int&         i0,
                        int&         i1,
                        int&         i2,
                        int&         i3 ) const;
            bool
            getElement( unsigned int  index,
                        unsigned int& ui0,
                        unsigned int& ui1 ) const;
            bool
            getElement( unsigned int  index,
                        unsigned int& ui0,
                        unsigned int& ui1,
                        unsigned int& ui2 ) const;
            bool
            getElement( unsigned int  index,
                        unsigned int& ui0,
                        unsigned int& ui1,
                        unsigned int& ui2,
                        unsigned int& ui3 ) const;
            bool
            getElement( unsigned int index,
                        bool&        b0,
                        bool&        b1 ) const;
            bool
            getElement( unsigned int index,
                        bool&        b0,
                        bool&        b1,
                        bool&        b2 ) const;
            bool
            getElement( unsigned int index,
                        bool&        b0,
                        bool&        b1,
                        bool&        b2,
                        bool&        b3 ) const;

            /** Set the internal data array for a osg::Uniform */
            bool
            setArray( FloatArray* array );
            bool
            setArray( DoubleArray* array );
            bool
            setArray( IntArray* array );
            bool
            setArray( UIntArray* array );
            bool
            setArray( UInt64Array* array );
            bool
            setArray( Int64Array* array );

            /** Get the internal data array for a float osg::Uniform. */
            FloatArray*
            getFloatArray() noexcept
            {
                return _floatArray.get();
            }

            const FloatArray*
            getFloatArray() const noexcept
            {
                return _floatArray.get();
            }

            /** Get the internal data array for a double osg::Uniform. */
            DoubleArray*
            getDoubleArray() noexcept
            {
                return _doubleArray.get();
            }

            const DoubleArray*
            getDoubleArray() const noexcept
            {
                return _doubleArray.get();
            }

            /** Get the internal data array for an int osg::Uniform. */
            IntArray*
            getIntArray() noexcept
            {
                return _intArray.get();
            }

            const IntArray*
            getIntArray() const noexcept
            {
                return _intArray.get();
            }

            /** Get the internal data array for an unsigned int osg::Uniform. */
            UIntArray*
            getUIntArray() noexcept
            {
                return _uintArray.get();
            }

            const UIntArray*
            getUIntArray() const noexcept
            {
                return _uintArray.get();
            }

            /** Get the internal data array for an unsigned int osg::Uniform. */
            UInt64Array*
            getUInt64Array() noexcept
            {
                return _uint64Array.get();
            }

            const UInt64Array*
            getUInt64Array() const noexcept
            {
                return _uint64Array.get();
            }

            /** Get the internal data array for an unsigned int osg::Uniform. */
            Int64Array*
            getInt64Array() noexcept
            {
                return _int64Array.get();
            }

            const Int64Array*
            getInt64Array() const noexcept
            {
                return _int64Array.get();
            }

            virtual void
                                         apply( const GLExtensions* ext,
                                                GLint               location ) const;

            typedef osg::UniformCallback Callback;

        protected:

            virtual ~Uniform();

            Uniform&
            operator=( const Uniform& )
            {
                return *this;
            }

            bool
            isCompatibleType( Type t ) const;
            // for backward compatibility only
            // see getElement(index, osg::dmat4 &)
            // see setElement(index, osg::dmat4 &)
            bool
            isCompatibleType( Type t1,
                              Type t2 ) const;

            bool
            isScalar() const noexcept
            {
                return _numElements == 1;
            }

            void
                                     allocateDataArray();

            Type                     _type;
            unsigned int             _numElements;

            // The internal data for osg::Uniforms are stored as an array of
            // getInternalArrayType() of length getInternalArrayNumElements().
            ref_ptr<FloatArray>      _floatArray;
            ref_ptr<DoubleArray>     _doubleArray;
            ref_ptr<IntArray>        _intArray;
            ref_ptr<UIntArray>       _uintArray;
            ref_ptr<Int64Array>      _int64Array;
            ref_ptr<UInt64Array>     _uint64Array;

            ref_ptr<UniformCallback> _updateCallback;
            ref_ptr<UniformCallback> _eventCallback;

            unsigned int             _modifiedCount;
    };

}
