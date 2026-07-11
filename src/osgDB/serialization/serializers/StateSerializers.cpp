/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include <cstdint>
#include <osg/maths/vec4.hpp>
#include <osg/state/BlendFunc.hpp>
#include <osg/state/Material.hpp>
#include <osg/state/Program.hpp>
#include <osg/state/Shader.hpp>
#include <osg/state/Uniform.hpp>
#include <osgDB/serialization/ObjectSerializer.hpp>
#include <stdexcept>
#include <string>
#include <string_view>

namespace osgDB::serialization
{
    namespace
    {

        void
        serializeObjectField( Archive&                   ar,
                              std::string_view           name,
                              osg::ref_ptr<osg::Object>& object )
        {
            ar.beginObject( name );
            serialize( ar, object );
            ar.endObject();
        }

        void
        serializeVec4( Archive&         ar,
                       std::string_view name,
                       osg::vec4&       value )
        {
            constexpr std::uint32_t kElementCount = 4U;
            std::uint32_t           count         = kElementCount;
            ar.beginArray( name, count );
            if( count != kElementCount )
            {
                throw std::runtime_error( "vec4 archive size mismatch" );
            }
            ar.value( "X", value.x );
            ar.value( "Y", value.y );
            ar.value( "Z", value.z );
            ar.value( "W", value.w );
            ar.endArray();
        }

        template<typename GetValue,
                 typename SetValue>
        void
        serializeMaterialColor( Archive&         ar,
                                std::string_view name,
                                bool             frontAndBack,
                                GetValue         getValue,
                                SetValue         setValue )
        {
            osg::vec4 front =
                ar.writing() ? getValue( osg::Material::FRONT ) : osg::vec4();
            osg::vec4 back =
                ar.writing() ? getValue( osg::Material::BACK ) : osg::vec4();
            ar.beginObject( name );
            ar.value( "FrontAndBack", frontAndBack );
            serializeVec4( ar, "Front", front );
            serializeVec4( ar, "Back", back );
            ar.endObject();

            if( ar.reading() )
            {
                if( frontAndBack )
                {
                    setValue( osg::Material::FRONT_AND_BACK, front );
                }
                else
                {
                    setValue( osg::Material::FRONT, front );
                    setValue( osg::Material::BACK, back );
                }
            }
        }

        void
        serializeMaterialShininess( Archive&       ar,
                                    osg::Material& material )
        {
            bool frontAndBack =
                ar.writing() ? material.getShininessFrontAndBack() : false;
            float front =
                ar.writing() ? material.getShininess( osg::Material::FRONT ) : 0.0F;
            float back =
                ar.writing() ? material.getShininess( osg::Material::BACK ) : 0.0F;
            ar.beginObject( "Shininess" );
            ar.value( "FrontAndBack", frontAndBack );
            ar.value( "Front", front );
            ar.value( "Back", back );
            ar.endObject();

            if( ar.reading() )
            {
                if( frontAndBack )
                {
                    material.setShininess( osg::Material::FRONT_AND_BACK, front );
                }
                else
                {
                    material.setShininess( osg::Material::FRONT, front );
                    material.setShininess( osg::Material::BACK, back );
                }
            }
        }

        void
        serializeProgramBindingList(
            Archive&                               ar,
            std::string_view                       name,
            const osg::Program::AttribBindingList& list,
            osg::Program&                          program,
            void ( osg::Program::*addBinding )( const std::string&,
                                                GLuint )
        )
        {
            std::uint32_t count =
                ar.writing() ? static_cast<std::uint32_t>( list.size() ) : 0U;
            ar.beginArray( name, count );
            auto it = list.begin();
            for( std::uint32_t i = 0U; i < count; ++i )
            {
                ar.beginObject( "Binding" );
                std::string   bindingName = ar.writing() ? it->first : std::string();
                std::uint32_t index =
                    ar.writing() ? static_cast<std::uint32_t>( it->second ) : 0U;
                ar.value( "Name", bindingName );
                ar.value( "Index", index );
                if( ar.reading() )
                {
                    ( program.*addBinding )( bindingName, static_cast<GLuint>( index ) );
                }
                else
                {
                    ++it;
                }
                ar.endObject();
            }
            ar.endArray();
        }

        void
        serializeUniformBlockBindingList( Archive&      ar,
                                          osg::Program& program )
        {
            const osg::Program::UniformBlockBindingList& list =
                program.getUniformBlockBindingList();
            std::uint32_t count =
                ar.writing() ? static_cast<std::uint32_t>( list.size() ) : 0U;
            ar.beginArray( "UniformBlockBindingList", count );
            auto it = list.begin();
            for( std::uint32_t i = 0U; i < count; ++i )
            {
                ar.beginObject( "Binding" );
                std::string   bindingName = ar.writing() ? it->first : std::string();
                std::uint32_t index =
                    ar.writing() ? static_cast<std::uint32_t>( it->second ) : 0U;
                ar.value( "Name", bindingName );
                ar.value( "Index", index );
                if( ar.reading() )
                {
                    program.addBindUniformBlock( bindingName,
                                                 static_cast<GLuint>( index ) );
                }
                else
                {
                    ++it;
                }
                ar.endObject();
            }
            ar.endArray();
        }

        void
        serializeProgramShaders( Archive&      ar,
                                 osg::Program& program )
        {
            std::uint32_t count = ar.writing() ? program.getNumShaders() : 0U;
            ar.beginArray( "Shaders", count );
            for( std::uint32_t i = 0U; i < count; ++i )
            {
                osg::ref_ptr<osg::Object> shader =
                    ar.writing() ? osg::ref_ptr<osg::Object>( program.getShader( i ) )
                                 : osg::ref_ptr<osg::Object>();
                serializeObjectField( ar, "Shader", shader );
                if( ar.reading() )
                {
                    osg::Shader* shaderObject =
                        shader.valid() ? dynamic_cast<osg::Shader*>( shader.get() )
                                       : nullptr;
                    if( shader.valid() && shaderObject == nullptr )
                    {
                        throw std::runtime_error(
                            "Program shader entry is not osg::Shader"
                        );
                    }
                    if( shaderObject != nullptr )
                    {
                        program.addShader( shaderObject );
                    }
                }
            }
            ar.endArray();
        }

        void
        serializeProgramParameters( Archive&      ar,
                                    osg::Program& program )
        {
            std::int32_t geometryVerticesOut =
                ar.writing() ? static_cast<std::int32_t>(
                                   program.getParameter( GL_GEOMETRY_VERTICES_OUT_EXT )
                               )
                             : 0;
            std::int32_t geometryInputType =
                ar.writing() ? static_cast<std::int32_t>(
                                   program.getParameter( GL_GEOMETRY_INPUT_TYPE_EXT )
                               )
                             : 0;
            std::int32_t geometryOutputType =
                ar.writing() ? static_cast<std::int32_t>(
                                   program.getParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT )
                               )
                             : 0;

            ar.beginObject( "GeometryParameters" );
            ar.value( "GeometryVerticesOut", geometryVerticesOut );
            ar.value( "GeometryInputType", geometryInputType );
            ar.value( "GeometryOutputType", geometryOutputType );
            ar.endObject();

            if( ar.reading() )
            {
                program.setParameter( GL_GEOMETRY_VERTICES_OUT_EXT,
                                      static_cast<GLint>( geometryVerticesOut ) );
                program.setParameter( GL_GEOMETRY_INPUT_TYPE_EXT,
                                      static_cast<GLint>( geometryInputType ) );
                program.setParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT,
                                      static_cast<GLint>( geometryOutputType ) );
            }
        }

        void
        serializeTransformFeedback( Archive&      ar,
                                    osg::Program& program )
        {
            std::uint32_t feedbackMode =
                ar.writing()
                    ? static_cast<std::uint32_t>( program.getTransformFeedBackMode() )
                    : 0U;
            ar.value( "TransformFeedbackMode", feedbackMode );
            if( ar.reading() )
            {
                program.setTransformFeedBackMode( static_cast<GLenum>( feedbackMode ) );
            }

            std::uint32_t count =
                ar.writing() ? program.getNumTransformFeedBackVaryings() : 0U;
            ar.beginArray( "TransformFeedbackVaryings", count );
            for( std::uint32_t i = 0U; i < count; ++i )
            {
                std::string varying = ar.writing()
                                        ? program.getTransformFeedBackVarying( i )
                                        : std::string();
                ar.value( "Varying", varying );
                if( ar.reading() )
                {
                    program.addTransformFeedBackVarying( varying );
                }
            }
            ar.endArray();
        }

        std::uint32_t
        uniformValueCount( const osg::Uniform& uniform )
        {
            return static_cast<std::uint32_t>( uniform.getInternalArrayNumElements() );
        }

        template<typename ArrayT,
                 typename ValueT>
        void
        serializeUniformNumericValues( Archive&         ar,
                                       std::string_view name,
                                       ArrayT*          array,
                                       std::uint32_t    expectedCount )
        {
            std::uint32_t count = ar.writing() ? expectedCount : 0U;
            ar.beginArray( name, count );
            if( count != expectedCount )
            {
                throw std::runtime_error( "Uniform value array size mismatch" );
            }
            if( count > 0U && array == nullptr )
            {
                throw std::runtime_error( "Uniform value array is not allocated" );
            }
            for( std::uint32_t i = 0U; i < count; ++i )
            {
                ValueT value =
                    ar.writing() ? static_cast<ValueT>( ( *array )[i] ) : ValueT{};
                ar.value( "Value", value );
                if( ar.reading() )
                {
                    ( *array )[i] = static_cast<typename ArrayT::value_type>( value );
                }
            }
            ar.endArray();
        }

        void
        serializeEmptyUniformValues( Archive&            ar,
                                     const osg::Uniform& uniform )
        {
            const std::uint32_t expectedCount = uniformValueCount( uniform );
            if( expectedCount != 0U )
            {
                throw std::runtime_error( "Unsupported Uniform value type" );
            }
            std::uint32_t count = 0U;
            ar.beginArray( "Values", count );
            if( count != 0U )
            {
                throw std::runtime_error( "Uniform value array size mismatch" );
            }
            ar.endArray();
        }

        void
        serializeUniformValues( Archive&      ar,
                                osg::Uniform& uniform )
        {
            const std::uint32_t expectedCount = uniformValueCount( uniform );
            switch( osg::Uniform::getInternalArrayType( uniform.getType() ) )
            {
                case GL_FLOAT :
                    serializeUniformNumericValues<osg::FloatArray, float>(
                        ar,
                        "Values",
                        uniform.getFloatArray(),
                        expectedCount
                    );
                    break;
                case GL_DOUBLE :
                    serializeUniformNumericValues<osg::DoubleArray, double>(
                        ar,
                        "Values",
                        uniform.getDoubleArray(),
                        expectedCount
                    );
                    break;
                case GL_INT :
                    serializeUniformNumericValues<osg::IntArray, std::int32_t>(
                        ar,
                        "Values",
                        uniform.getIntArray(),
                        expectedCount
                    );
                    break;
                case GL_UNSIGNED_INT :
                    serializeUniformNumericValues<osg::UIntArray, std::uint32_t>(
                        ar,
                        "Values",
                        uniform.getUIntArray(),
                        expectedCount
                    );
                    break;
                case GL_INT64_ARB :
                    serializeUniformNumericValues<osg::Int64Array, std::int64_t>(
                        ar,
                        "Values",
                        uniform.getInt64Array(),
                        expectedCount
                    );
                    break;
                case GL_UNSIGNED_INT64_ARB :
                    serializeUniformNumericValues<osg::UInt64Array, std::uint64_t>(
                        ar,
                        "Values",
                        uniform.getUInt64Array(),
                        expectedCount
                    );
                    break;
                default :
                    serializeEmptyUniformValues( ar, uniform );
                    break;
            }
            if( ar.reading() )
            {
                uniform.dirty();
            }
        }

    }

    void
    serialize( Archive&       ar,
               osg::Material& material )
    {
        std::int32_t colorMode =
            ar.writing() ? static_cast<std::int32_t>( material.getColorMode() ) : 0;
        ar.value( "ColorMode", colorMode );
        if( ar.reading() )
        {
            material.setColorMode( static_cast<osg::Material::ColorMode>( colorMode ) );
        }

        bool ambientFrontAndBack =
            ar.writing() ? material.getAmbientFrontAndBack() : false;
        serializeMaterialColor(
            ar,
            "Ambient",
            ambientFrontAndBack,
            [&]( osg::Material::Face face )
            {
                return material.getAmbient( face );
            },
            [&]( osg::Material::Face face, const osg::vec4& value )
            {
                material.setAmbient( face, value );
            }
        );

        bool diffuseFrontAndBack =
            ar.writing() ? material.getDiffuseFrontAndBack() : false;
        serializeMaterialColor(
            ar,
            "Diffuse",
            diffuseFrontAndBack,
            [&]( osg::Material::Face face )
            {
                return material.getDiffuse( face );
            },
            [&]( osg::Material::Face face, const osg::vec4& value )
            {
                material.setDiffuse( face, value );
            }
        );

        bool specularFrontAndBack =
            ar.writing() ? material.getSpecularFrontAndBack() : false;
        serializeMaterialColor(
            ar,
            "Specular",
            specularFrontAndBack,
            [&]( osg::Material::Face face )
            {
                return material.getSpecular( face );
            },
            [&]( osg::Material::Face face, const osg::vec4& value )
            {
                material.setSpecular( face, value );
            }
        );

        bool emissionFrontAndBack =
            ar.writing() ? material.getEmissionFrontAndBack() : false;
        serializeMaterialColor(
            ar,
            "Emission",
            emissionFrontAndBack,
            [&]( osg::Material::Face face )
            {
                return material.getEmission( face );
            },
            [&]( osg::Material::Face face, const osg::vec4& value )
            {
                material.setEmission( face, value );
            }
        );

        serializeMaterialShininess( ar, material );
    }

    void
    serialize( Archive&        ar,
               osg::BlendFunc& blendFunc )
    {
        std::uint32_t sourceRgb =
            ar.writing() ? static_cast<std::uint32_t>( blendFunc.getSourceRGB() ) : 0U;
        std::uint32_t destinationRgb =
            ar.writing() ? static_cast<std::uint32_t>( blendFunc.getDestinationRGB() )
                         : 0U;
        std::uint32_t sourceAlpha =
            ar.writing() ? static_cast<std::uint32_t>( blendFunc.getSourceAlpha() ) : 0U;
        std::uint32_t destinationAlpha =
            ar.writing() ? static_cast<std::uint32_t>( blendFunc.getDestinationAlpha() )
                         : 0U;

        ar.value( "SourceRGB", sourceRgb );
        ar.value( "DestinationRGB", destinationRgb );
        ar.value( "SourceAlpha", sourceAlpha );
        ar.value( "DestinationAlpha", destinationAlpha );

        if( ar.reading() )
        {
            blendFunc.setFunction( static_cast<GLenum>( sourceRgb ),
                                   static_cast<GLenum>( destinationRgb ),
                                   static_cast<GLenum>( sourceAlpha ),
                                   static_cast<GLenum>( destinationAlpha ) );
        }
    }

    void
    serialize( Archive&     ar,
               osg::Shader& shader )
    {
        std::int32_t type   = ar.writing()
                                ? static_cast<std::int32_t>( shader.getType() )
                                : static_cast<std::int32_t>( osg::Shader::UNDEFINED );
        std::string  source = ar.writing() ? shader.getShaderSource() : std::string();

        ar.value( "ShaderType", type );
        ar.value( "ShaderSource", source );

        if( ar.reading() )
        {
            if( !shader.setType( static_cast<osg::Shader::Type>( type ) ) )
            {
                throw std::runtime_error( "Failed to set Shader type" );
            }
            shader.setShaderSource( source );
        }
    }

    void
    serialize( Archive&      ar,
               osg::Program& program )
    {
        serializeProgramShaders( ar, program );
        serializeProgramBindingList( ar,
                                     "AttribBindingList",
                                     program.getAttribBindingList(),
                                     program,
                                     &osg::Program::addBindAttribLocation );
        serializeProgramBindingList( ar,
                                     "FragDataBindingList",
                                     program.getFragDataBindingList(),
                                     program,
                                     &osg::Program::addBindFragDataLocation );
        serializeUniformBlockBindingList( ar, program );
        serializeProgramParameters( ar, program );
        serializeTransformFeedback( ar, program );
    }

    void
    serialize( Archive&      ar,
               osg::Uniform& uniform )
    {
        std::string   name = ar.writing() ? uniform.getName() : std::string();
        std::int32_t  type = ar.writing()
                               ? static_cast<std::int32_t>( uniform.getType() )
                               : static_cast<std::int32_t>( osg::Uniform::UNDEFINED );
        std::uint32_t numElements = ar.writing() ? uniform.getNumElements() : 0U;

        ar.value( "Name", name );
        ar.value( "UniformType", type );
        ar.value( "NumElements", numElements );

        if( ar.reading() )
        {
            const osg::Uniform::Type uniformType =
                static_cast<osg::Uniform::Type>( type );
            uniform.setName( name );
            if( !uniform.setType( uniformType ) )
            {
                throw std::runtime_error( "Failed to set Uniform type" );
            }
            if( numElements > 0U )
            {
                uniform.setNumElements( numElements );
            }
            else if( uniformType != osg::Uniform::UNDEFINED )
            {
                throw std::runtime_error( "Uniform has a type but no elements" );
            }
        }

        serializeUniformValues( ar, uniform );
    }

}

OSG_REGISTER_SERIALIZER( osg,
                         Material );
OSG_REGISTER_SERIALIZER( osg,
                         BlendFunc );
OSG_REGISTER_SERIALIZER( osg,
                         Shader );
OSG_REGISTER_SERIALIZER( osg,
                         Program );
OSG_REGISTER_SERIALIZER( osg,
                         Uniform );
