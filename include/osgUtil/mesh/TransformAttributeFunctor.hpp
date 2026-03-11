/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Functor applying a matrix transform to vertex arrays.
 * Used for baking transforms into geometry data.
 */
#pragma once

#include <osg/core/Notify.hpp>
#include <osg/geometry/Drawable.hpp>
#include <osgUtil/Export>

namespace osgUtil
{

    /** Functor for transforming a drawable's vertex and normal attributes by specified
     * matrix. typically used for flattening transform down onto drawable leaves. */
    class OSGUTIL_EXPORT TransformAttributeFunctor
        : public osg::Drawable::AttributeFunctor
    {
        public:

            /** Construct a functor to transform a drawable's vertex and normal
             * attributes by specified matrix.*/
            TransformAttributeFunctor( const osg::dmat4& m );

            virtual ~TransformAttributeFunctor();

            /** Do the work of transforming vertex and normal attributes. */
            virtual void
            apply( osg::Drawable::AttributeType type,
                   unsigned int                 count,
                   osg::vec3*                   begin );
            virtual void
                       apply( osg::Drawable::AttributeType type,
                              unsigned int                 count,
                              osg::dvec3*                  begin );

            osg::dmat4 _m;
            osg::dmat4 _im;
    };

}
