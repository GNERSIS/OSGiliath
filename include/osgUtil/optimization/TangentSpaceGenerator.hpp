/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Computes tangent and binormal vectors for normal mapping.
 * Generates per-vertex tangent-space basis from UV coordinates.
 */
#pragma once

#include <osg/core/ref_ptr.hpp>
#include <osg/core/Referenced.hpp>
#include <osg/geometry/Array.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osgUtil/Export>

namespace osgUtil
{

    /**
     The TangentSpaceGenerator class generates three arrays containing tangent-space
     basis vectors. It takes a texture-mapped Geometry object as input, traverses its
     primitive sets and computes Tangent, Normal and Binormal vectors for each vertex,
     storing them into arrays. The resulting arrays can be used as vertex program varying
     (per-vertex) parameters, enabling advanced effects like bump-mapping. To use this
     class, simply call the generate() method specifying the Geometry object you want to
     process and the texture unit that contains UV mapping for the normal map; then you
     can retrieve the TBN arrays by calling getTangentArray(), getNormalArray() and
     getBinormalArray() methods.
     */
    class OSGUTIL_EXPORT TangentSpaceGenerator : public osg::Referenced
    {
        public:

            TangentSpaceGenerator();
            TangentSpaceGenerator( const TangentSpaceGenerator& copy,
                                   const osg::CopyOp&           copyop =
                                       osg::CopyOp::SHALLOW_COPY );

            void
            generate( osg::Geometry* geo,
                      int            normal_map_tex_unit = 0 );

            inline osg::Vec4Array*
            getTangentArray()
            {
                return T_.get();
            }

            inline const osg::Vec4Array*
            getTangentArray() const
            {
                return T_.get();
            }

            inline void
            setTangentArray( osg::Vec4Array* array )
            {
                T_ = array;
            }

            inline osg::Vec4Array*
            getNormalArray()
            {
                return N_.get();
            }

            inline const osg::Vec4Array*
            getNormalArray() const
            {
                return N_.get();
            }

            inline void
            setNormalArray( osg::Vec4Array* array )
            {
                N_ = array;
            }

            inline osg::Vec4Array*
            getBinormalArray()
            {
                return B_.get();
            }

            inline const osg::Vec4Array*
            getBinormalArray() const
            {
                return B_.get();
            }

            inline void
            setBinormalArray( osg::Vec4Array* array )
            {
                B_ = array;
            }

            inline osg::IndexArray*
            getIndices()
            {
                return indices_.get();
            }

        protected:

            virtual ~TangentSpaceGenerator()
            {
            }

            TangentSpaceGenerator&
            operator=( const TangentSpaceGenerator& )
            {
                return *this;
            }

            void
                                         compute( osg::PrimitiveSet* pset,
                                                  const osg::Array*  vx,
                                                  const osg::Array*  nx,
                                                  const osg::Array*  tx,
                                                  int                iA,
                                                  int                iB,
                                                  int                iC );

            osg::ref_ptr<osg::Vec4Array> T_;
            osg::ref_ptr<osg::Vec4Array> B_;
            osg::ref_ptr<osg::Vec4Array> N_;
            osg::ref_ptr<osg::UIntArray> indices_;
    };

}
