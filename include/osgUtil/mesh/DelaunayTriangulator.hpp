/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Delaunay triangulation of 2D point sets. Creates optimal
 * triangulations for terrain and surface reconstruction.
 */
#pragma once

#include <list>
#include <osg/core/CopyOp.hpp>
#include <osg/core/ref_ptr.hpp>
#include <osg/core/Referenced.hpp>
#include <osg/geometry/Array.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/geometry/PrimitiveSet.hpp>
#include <osgUtil/Export.hpp>

namespace osgUtil
{

    /** DelaunayTriangulator: Utility class that triangulates an irregular network of
       sample points. Just create a DelaunayTriangulator, assign it the sample point
       array and call its triangulate() method to start the triangulation. Then you can
       obtain the generated primitive by calling the getTriangles() method.

        Add DelaunayConstraints (or derived class) to control the triangulation edges.
    */
    class OSGUTIL_EXPORT DelaunayConstraint : public osg::Geometry
    {
            // controls the edges in a Delaunay triangulation.
            // constraints can be linear (with width), areal (contains an area)
            // uses: to replace part of a terrain with an alternative textured model
            // (roads, lakes). the primitive sets in this are either LINE_LOOP or
            // LINE_STRIP

        public:

            DelaunayConstraint()
            {
            }

            /** Each primitiveset is a list of vertices which may be closed by joining up
             * to its start to make a loop.  Constraints should be simple lines, not
             * crossing themselves. Constraints which cross other constraints can cause
             * difficulties - see the example for methods of dealing with them. */

            /** collect up indices of triangle from delaunay triangles.
             *  The delaunay triangles inside the DelaunayConstraint area can be used to
             * fill the area or generate geometry that terrain follows the area in some
             * way. These triangles can form a canopy or a field. */
            void
            addtriangle( int i1,
                         int i2,
                         int i3 );

            /** Get the filling primitive. One:
             * triangulate must have bneen called and
             * two:  triangle list is filled when
             * DelaunayTriangulator::removeInternalTriangles is called.
             * These return the triangles removed from the delaunay triangulation by
             * DelaunayTriangulator::removeInternalTriangles. */
            inline const osg::DrawElementsUInt*
            getTriangles() const
            {
                return prim_tris_.get();
            }

            inline osg::DrawElementsUInt*
            getTriangles()
            {
                return prim_tris_.get();
            }

            /** Call BEFORE makeDrawable to reorder points to make optimised set
             */
            osg::Vec3Array*
            getPoints( const osg::Vec3Array* points );

            /** converts simple list of triangles into a drawarray.
             */
            osg::DrawElementsUInt*
            makeDrawable();

            /** Add vertices and constraint loops from dco
             * Can be used to generate extra vertices where dco crosses 'this' using
             * osgUtil::Tessellator to insert overlap vertices.
             */
            void
            merge( DelaunayConstraint* dco );

            /** remove from line the vertices that are inside dco
             */
            void
            removeVerticesInside( const DelaunayConstraint* dco );

            /** return winding number as a float of loop around testpoint; may use
             * multiple loops does not reject points on the edge or very very close to
             * the edge */
            float
            windingNumber( const osg::vec3& testpoint ) const;

            /** true if testpoint is internal (or external) to constraint. */
            virtual bool
            contains( const osg::vec3& testpoint ) const;
            virtual bool
            outside( const osg::vec3& testpoint ) const;

            /** Tessellate the constraint loops so that the crossing points are
             * interpolated and added to the constraints for the triangulation. */
            void
            handleOverlaps( void );

        protected:

            virtual ~DelaunayConstraint();

            typedef std::vector<int*>
                    trilist;    // array of indices in points array defining triangles

            trilist _interiorTris;    // list of triangles that fits the area.

            osg::ref_ptr<osg::DrawElementsUInt>
                prim_tris_;    // returns a PrimitiveSet to draw the interior of this DC
    };

    class OSGUTIL_EXPORT DelaunayTriangulator : public osg::Referenced
    {
        public:

            DelaunayTriangulator();
            explicit DelaunayTriangulator( osg::Vec3Array* points,
                                           osg::Vec3Array* normals = 0 );
            DelaunayTriangulator( const DelaunayTriangulator& copy,
                                  const osg::CopyOp&          copyop =
                                      osg::CopyOp::SHALLOW_COPY );

            typedef std::vector<osg::ref_ptr<DelaunayConstraint>> linelist;

            /** Set the input point array. */
            inline void
            setInputPointArray( osg::Vec3Array* points )
            {
                points_ = points;
            }

            /** Get the const input point array. */
            inline const osg::Vec3Array*
            getInputPointArray() const
            {
                return points_.get();
            }

            /** Get the input point array. */
            inline osg::Vec3Array*
            getInputPointArray()
            {
                return points_.get();
            }

            /** Set the output normal array (optional). */
            inline void
            setOutputNormalArray( osg::Vec3Array* normals )
            {
                normals_ = normals;
            }

            /** Get the const output normal array (optional). */
            inline const osg::Vec3Array*
            getOutputNormalArray() const
            {
                return normals_.get();
            }

            /** Get the output normal array (optional). */
            inline osg::Vec3Array*
            getOutputNormalArray()
            {
                return normals_.get();
            }

            /** Add an input constraint loop.
             ** the edges of the loop will constrain the triangulation.
             ** if remove!=0, the internal triangles of the constraint will be removed;
             ** the user may the replace the constraint line with an equivalent geometry.
             ** GWM July 2005 */
            void
            addInputConstraint( DelaunayConstraint* dc )
            {
                constraint_lines.push_back( dc );
            }

            /** Start triangulation. */
            bool
            triangulate();

            /** Get the generated primitive (call triangulate() first). */
            inline const osg::DrawElementsUInt*
            getTriangles() const
            {
                return prim_tris_.get();
            }

            /** Get the generated primitive (call triangulate() first). */
            inline osg::DrawElementsUInt*
            getTriangles()
            {
                return prim_tris_.get();
            }

            /** remove the triangles internal to the constraint loops.
             * (Line strips cannot remove any internal triangles). */
            void
            removeInternalTriangles( DelaunayConstraint* constraint );

        protected:

            virtual ~DelaunayTriangulator();

            DelaunayTriangulator&
            operator=( const DelaunayTriangulator& )
            {
                return *this;
            }

            int
            getindex( const osg::vec3&      pt,
                      const osg::Vec3Array* points );

        private:

            osg::ref_ptr<osg::Vec3Array>        points_;
            osg::ref_ptr<osg::Vec3Array>        normals_;
            osg::ref_ptr<osg::DrawElementsUInt> prim_tris_;

            // GWM these lines provide required edges in the triangulated shape.
            linelist                            constraint_lines;

            void
            _uniqueifyPoints();
    };

    // INLINE METHODS

}
