/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <osg/geometry/Drawable.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/PrimitiveSet.hpp>
#include <osg/nodes/Node.hpp>
#include <osgDB/serialization/ObjectSerializer.hpp>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace osgDB::serialization
{
    namespace
    {

        std::vector<std::byte>
        copyBytes( const void* source,
                   std::size_t size )
        {
            std::vector<std::byte> bytes( size );
            if( size > 0U )
            {
                if( source == nullptr )
                {
                    throw std::runtime_error( "Cannot serialize null data pointer" );
                }
                std::memcpy( bytes.data(), source, size );
            }
            return bytes;
        }

        void
        fillBytes( void*                         destination,
                   std::size_t                   destinationSize,
                   const std::vector<std::byte>& bytes )
        {
            if( bytes.size() != destinationSize )
            {
                throw std::runtime_error( "Serialized blob size mismatch" );
            }
            if( destinationSize > 0U )
            {
                if( destination == nullptr )
                {
                    throw std::runtime_error(
                        "Cannot deserialize to null data pointer"
                    );
                }
                std::memcpy( destination, bytes.data(), destinationSize );
            }
        }

        bool
        isRegisteredArrayClass( std::string_view className )
        {
            return className ==
                   "Vec2Array" ||
                   className ==
                   "Vec3Array" ||
                   className ==
                   "Vec4Array" ||
                   className ==
                   "Vec4ubArray" ||
                   className == "FloatArray";
        }

        bool
        isRegisteredPrimitiveSetClass( std::string_view className )
        {
            return className ==
                   "DrawArrays" ||
                   className ==
                   "DrawElementsUByte" ||
                   className ==
                   "DrawElementsUShort" ||
                   className == "DrawElementsUInt";
        }

        osg::Array*
        asRegisteredArray( osg::Object* object )
        {
            if( object == nullptr )
            {
                return nullptr;
            }
            if( !isRegisteredArrayClass( object->className() ) )
            {
                throw std::runtime_error( "Serialized object is not an osg::Array" );
            }
            return static_cast<osg::Array*>( object );
        }

        osg::PrimitiveSet*
        asRegisteredPrimitiveSet( osg::Object* object )
        {
            if( object == nullptr )
            {
                return nullptr;
            }
            if( !isRegisteredPrimitiveSetClass( object->className() ) )
            {
                throw std::runtime_error(
                    "Serialized object is not an osg::PrimitiveSet"
                );
            }
            return static_cast<osg::PrimitiveSet*>( object );
        }

        void
        serializeObjectField( Archive&                   ar,
                              std::string_view           name,
                              osg::ref_ptr<osg::Object>& object )
        {
            ar.beginObject( name );
            serialize( ar, object );
            ar.endObject();
        }

        template<typename ArrayT>
        void
        serializeArrayContents( Archive& ar,
                                ArrayT&  array )
        {
            std::uint32_t type =
                ar.writing() ? static_cast<std::uint32_t>( array.getType() ) : 0U;
            std::uint32_t elementCount = ar.writing() ? array.getNumElements() : 0U;
            std::int32_t  binding =
                ar.writing() ? static_cast<std::int32_t>( array.getBinding() ) : 0;
            bool normalize        = ar.writing() ? array.getNormalize() : false;
            bool preserveDataType = ar.writing() ? array.getPreserveDataType() : false;
            std::vector<std::byte> bytes =
                ar.writing()
                    ? copyBytes( array.getDataPointer(), array.getTotalDataSize() )
                    : std::vector<std::byte>();

            ar.value( "ArrayType", type );
            ar.value( "NumElements", elementCount );
            ar.value( "Binding", binding );
            ar.value( "Normalize", normalize );
            ar.value( "PreserveDataType", preserveDataType );
            ar.blob( "Data", bytes );

            if( ar.reading() )
            {
                const auto storedType = static_cast<osg::Array::Type>( type );
                if( storedType != array.getType() )
                {
                    throw std::runtime_error( "Array serializer type mismatch" );
                }
                array.resizeArray( elementCount );
                array.setBinding( static_cast<osg::Array::Binding>( binding ) );
                array.setNormalize( normalize );
                array.setPreserveDataType( preserveDataType );
                fillBytes( const_cast<void*>( array.getDataPointer() ),
                           static_cast<std::size_t>( array.getTotalDataSize() ),
                           bytes );
            }
        }

        void
        serializeArrayField( Archive&         ar,
                             std::string_view name,
                             osg::Array*&     array )
        {
            osg::ref_ptr<osg::Object> object = ar.writing()
                                                 ? osg::ref_ptr<osg::Object>( array )
                                                 : osg::ref_ptr<osg::Object>();
            serializeObjectField( ar, name, object );
            if( ar.reading() )
            {
                array = asRegisteredArray( object.get() );
            }
        }

        template<typename DrawElementsT>
        void
        serializeDrawElementsContents( Archive&       ar,
                                       DrawElementsT& elements )
        {
            std::uint32_t primitiveType =
                ar.writing() ? static_cast<std::uint32_t>( elements.getType() ) : 0U;
            std::uint32_t mode =
                ar.writing() ? static_cast<std::uint32_t>( elements.getMode() ) : 0U;
            std::int32_t numInstances =
                ar.writing() ? static_cast<std::int32_t>( elements.getNumInstances() )
                             : 0;
            std::uint32_t dataType =
                ar.writing() ? static_cast<std::uint32_t>( elements.getDataType() ) : 0U;
            std::uint32_t indexCount = ar.writing() ? elements.getNumIndices() : 0U;
            std::vector<std::byte> bytes =
                ar.writing()
                    ? copyBytes( elements.getDataPointer(), elements.getTotalDataSize() )
                    : std::vector<std::byte>();

            ar.value( "PrimitiveType", primitiveType );
            ar.value( "Mode", mode );
            ar.value( "NumInstances", numInstances );
            ar.value( "DataType", dataType );
            ar.value( "NumIndices", indexCount );
            ar.blob( "Indices", bytes );

            if( ar.reading() )
            {
                const auto storedType =
                    static_cast<osg::PrimitiveSet::Type>( primitiveType );
                if( storedType !=
                    elements.getType() ||
                    static_cast<GLenum>( dataType ) != elements.getDataType() )
                {
                    throw std::runtime_error( "DrawElements serializer type mismatch" );
                }
                elements.setMode( static_cast<GLenum>( mode ) );
                elements.setNumInstances( numInstances );
                elements.resizeElements( indexCount );
                fillBytes( const_cast<void*>( elements.getDataPointer() ),
                           static_cast<std::size_t>( elements.getTotalDataSize() ),
                           bytes );
            }
        }

        void
        serializeDrawArraysContents( Archive&         ar,
                                     osg::DrawArrays& drawArrays )
        {
            std::uint32_t mode =
                ar.writing() ? static_cast<std::uint32_t>( drawArrays.getMode() ) : 0U;
            std::int32_t first =
                ar.writing() ? static_cast<std::int32_t>( drawArrays.getFirst() ) : 0;
            std::int32_t count =
                ar.writing() ? static_cast<std::int32_t>( drawArrays.getCount() ) : 0;
            std::int32_t numInstances =
                ar.writing() ? static_cast<std::int32_t>( drawArrays.getNumInstances() )
                             : 0;

            ar.value( "Mode", mode );
            ar.value( "First", first );
            ar.value( "Count", count );
            ar.value( "NumInstances", numInstances );

            if( ar.reading() )
            {
                drawArrays.set( static_cast<GLenum>( mode ),
                                static_cast<GLint>( first ),
                                static_cast<GLsizei>( count ) );
                drawArrays.setNumInstances( numInstances );
            }
        }

        void
        serializeNodeState( Archive&   ar,
                            osg::Node& node )
        {
            osg::ref_ptr<osg::Object> stateSet =
                ar.writing() ? osg::ref_ptr<osg::Object>( node.getStateSet() )
                             : osg::ref_ptr<osg::Object>();
            serializeObjectField( ar, "StateSet", stateSet );
            if( ar.reading() )
            {
                if( stateSet.valid() && stateSet->asStateSet() == nullptr )
                {
                    throw std::runtime_error(
                        "Node StateSet field did not contain osg::StateSet"
                    );
                }
                node.setStateSet( stateSet.valid() ? stateSet->asStateSet() : nullptr );
            }
        }

        void
        serializeTexCoordArrays( Archive&       ar,
                                 osg::Geometry& geometry )
        {
            std::uint32_t count = ar.writing() ? geometry.getNumTexCoordArrays() : 0U;
            ar.beginArray( "TexCoordArrays", count );
            for( std::uint32_t i = 0U; i < count; ++i )
            {
                ar.beginObject( "TexCoordArray" );
                std::uint32_t unit = ar.writing() ? i : 0U;
                ar.value( "Unit", unit );

                osg::Array* array =
                    ar.writing() ? geometry.getTexCoordArray( i ) : nullptr;
                serializeArrayField( ar, "Array", array );
                if( ar.reading() && array != nullptr )
                {
                    geometry.setTexCoordArray( unit, array, array->getBinding() );
                }
                ar.endObject();
            }
            ar.endArray();
        }

        void
        serializePrimitiveSets( Archive&       ar,
                                osg::Geometry& geometry )
        {
            std::uint32_t count = ar.writing() ? geometry.getNumPrimitiveSets() : 0U;
            ar.beginArray( "PrimitiveSets", count );
            for( std::uint32_t i = 0U; i < count; ++i )
            {
                osg::ref_ptr<osg::Object> primitive =
                    ar.writing()
                        ? osg::ref_ptr<osg::Object>( geometry.getPrimitiveSet( i ) )
                        : osg::ref_ptr<osg::Object>();
                serialize( ar, primitive );
                if( ar.reading() )
                {
                    osg::PrimitiveSet* primitiveSet =
                        asRegisteredPrimitiveSet( primitive.get() );
                    if( primitiveSet != nullptr )
                    {
                        geometry.addPrimitiveSet( primitiveSet );
                    }
                }
            }
            ar.endArray();
        }

    }

    void
    serialize( Archive&        ar,
               osg::Vec2Array& array )
    {
        serializeArrayContents( ar, array );
    }

    void
    serialize( Archive&        ar,
               osg::Vec3Array& array )
    {
        serializeArrayContents( ar, array );
    }

    void
    serialize( Archive&        ar,
               osg::Vec4Array& array )
    {
        serializeArrayContents( ar, array );
    }

    void
    serialize( Archive&          ar,
               osg::Vec4ubArray& array )
    {
        serializeArrayContents( ar, array );
    }

    void
    serialize( Archive&         ar,
               osg::FloatArray& array )
    {
        serializeArrayContents( ar, array );
    }

    void
    serialize( Archive&         ar,
               osg::DrawArrays& drawArrays )
    {
        serializeDrawArraysContents( ar, drawArrays );
    }

    void
    serialize( Archive&                ar,
               osg::DrawElementsUByte& elements )
    {
        serializeDrawElementsContents( ar, elements );
    }

    void
    serialize( Archive&                 ar,
               osg::DrawElementsUShort& elements )
    {
        serializeDrawElementsContents( ar, elements );
    }

    void
    serialize( Archive&               ar,
               osg::DrawElementsUInt& elements )
    {
        serializeDrawElementsContents( ar, elements );
    }

    void
    serialize( Archive&       ar,
               osg::Drawable& drawable )
    {
        serializeNodeState( ar, static_cast<osg::Node&>( drawable ) );
    }

    void
    serialize( Archive&       ar,
               osg::Geometry& geometry )
    {
        serialize( ar, static_cast<osg::Drawable&>( geometry ) );

        osg::Array* vertexArray = ar.writing() ? geometry.getVertexArray() : nullptr;
        osg::Array* normalArray = ar.writing() ? geometry.getNormalArray() : nullptr;
        osg::Array* colorArray  = ar.writing() ? geometry.getColorArray() : nullptr;

        serializeArrayField( ar, "VertexArray", vertexArray );
        serializeArrayField( ar, "NormalArray", normalArray );
        serializeArrayField( ar, "ColorArray", colorArray );

        if( ar.reading() )
        {
            geometry.setVertexArray( vertexArray );
            geometry.setNormalArray( normalArray,
                                     normalArray != nullptr
                                         ? normalArray->getBinding()
                                         : osg::Array::BIND_UNDEFINED );
            geometry.setColorArray( colorArray,
                                    colorArray != nullptr ? colorArray->getBinding()
                                                          : osg::Array::BIND_UNDEFINED );
        }

        serializeTexCoordArrays( ar, geometry );
        serializePrimitiveSets( ar, geometry );
    }

}

OSG_REGISTER_SERIALIZER_FACTORY( osg::Vec2Array,
                                 new osg::Vec2Array );
OSG_REGISTER_SERIALIZER_FACTORY( osg::Vec3Array,
                                 new osg::Vec3Array );
OSG_REGISTER_SERIALIZER_FACTORY( osg::Vec4Array,
                                 new osg::Vec4Array );
OSG_REGISTER_SERIALIZER_FACTORY( osg::Vec4ubArray,
                                 new osg::Vec4ubArray );
OSG_REGISTER_SERIALIZER_FACTORY( osg::FloatArray,
                                 new osg::FloatArray );
OSG_REGISTER_SERIALIZER_FACTORY( osg::DrawArrays,
                                 new osg::DrawArrays );
OSG_REGISTER_SERIALIZER_FACTORY( osg::DrawElementsUByte,
                                 new osg::DrawElementsUByte );
OSG_REGISTER_SERIALIZER_FACTORY( osg::DrawElementsUShort,
                                 new osg::DrawElementsUShort );
OSG_REGISTER_SERIALIZER_FACTORY( osg::DrawElementsUInt,
                                 new osg::DrawElementsUInt );
OSG_REGISTER_SERIALIZER( osg,
                         Geometry );
