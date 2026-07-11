/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt */
#include <osgDB/serialization/ObjectSerializer.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/MatrixTransform.hpp>

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace osgDB::serialization
{
    namespace
    {
        constexpr std::uint32_t kMatrixElementCount = 16U;
        constexpr std::size_t   kMatrixDimension    = 4U;
    }

    void
    serialize( Archive& ar, osg::Group& group )
    {
        std::uint32_t count = ar.writing() ? group.getNumChildren() : 0U;
        ar.beginArray( "children", count );
        for( std::uint32_t i = 0U; i < count; ++i )
        {
            osg::ref_ptr<osg::Object> child =
                ar.writing() ? osg::ref_ptr<osg::Object>( group.getChild( i ) )
                             : osg::ref_ptr<osg::Object>();
            serialize( ar, child );
            if( ar.reading() && child.valid() && child->asNode() != nullptr )
            {
                group.addChild( child->asNode() );
            }
        }
        ar.endArray();
    }

    void
    serialize( Archive& ar, osg::MatrixTransform& transform )
    {
        // Parent chain first: MatrixTransform -> Transform -> Group. This
        // captures children. (Transform::_referenceFrame and Node base state
        // are added with the Transform/Node serializers in M3.)
        serialize( ar, static_cast<osg::Group&>( transform ) );

        osg::dmat4 matrix = ar.writing() ? transform.getMatrix() : osg::dmat4();
        std::uint32_t elementCount = kMatrixElementCount;
        ar.beginArray( "matrix", elementCount );
        if( elementCount != kMatrixElementCount )
        {
            throw std::runtime_error( "MatrixTransform matrix archive size mismatch" );
        }
        for( std::size_t col = 0U; col < kMatrixDimension; ++col )
        {
            for( std::size_t row = 0U; row < kMatrixDimension; ++row )
            {
                ar.value( "element", matrix( row, col ) );
            }
        }
        ar.endArray();
        if( ar.reading() )
        {
            transform.setMatrix( matrix );
        }
    }

}

OSG_REGISTER_SERIALIZER( osg, Group );
OSG_REGISTER_SERIALIZER( osg, MatrixTransform );
