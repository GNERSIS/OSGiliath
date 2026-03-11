/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Functor applying a matrix transform to vertex arrays.
 * Used for baking transforms into geometry data.
 */
#include <osgUtil/mesh/TransformAttributeFunctor.hpp>

#include <osg/maths/compat.hpp>

using namespace osgUtil;

TransformAttributeFunctor::TransformAttributeFunctor( const osg::dmat4& m )
{
    _m  = m;
    _im = osg::inverse( _m );
}

TransformAttributeFunctor::~TransformAttributeFunctor()
{
}

void
TransformAttributeFunctor::apply( osg::Drawable::AttributeType type,
                                  unsigned int                 count,
                                  osg::vec3*                   begin )
{
    if( type == osg::Drawable::VERTICES )
    {
        osg::vec3* end = begin + count;
        for( osg::vec3* itr = begin; itr < end; ++itr )
        {
            ( *itr ) = ( *itr ) * _m;
        }
    }
    else if( type == osg::Drawable::NORMALS )
    {
        osg::vec3* end = begin + count;
        for( osg::vec3* itr = begin; itr < end; ++itr )
        {
            // note post mult by inverse for normals.
            ( *itr ) = osg::transform3x3( _im, *itr );
            ( *itr ) = osg::normalize( *itr );
        }
    }
}

void
TransformAttributeFunctor::apply( osg::Drawable::AttributeType type,
                                  unsigned int                 count,
                                  osg::dvec3*                  begin )
{
    if( type == osg::Drawable::VERTICES )
    {
        osg::dvec3* end = begin + count;
        for( osg::dvec3* itr = begin; itr < end; ++itr )
        {
            ( *itr ) = ( *itr ) * _m;
        }
    }
    else if( type == osg::Drawable::NORMALS )
    {
        osg::dvec3* end = begin + count;
        for( osg::dvec3* itr = begin; itr < end; ++itr )
        {
            // note post mult by inverse for normals.
            ( *itr ) = osg::transform3x3( _im, *itr );
            ( *itr ) = osg::normalize( *itr );
        }
    }
}
