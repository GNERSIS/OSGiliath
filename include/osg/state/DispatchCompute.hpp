/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Compute shader dispatch drawable. Wraps glDispatchCompute
 * to launch compute work groups from the draw traversal.
 */
#pragma once

#include <osg/core/Export.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/geometry/Geometry.hpp>

namespace osg
{

    /** Wrapper around glDispatchCompute.*/
    class OSG_EXPORT DispatchCompute
        : public osg::Inherit<osg::Drawable, DispatchCompute>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               DispatchCompute )

            DispatchCompute( GLint numGroupsX = 0,
                             GLint numGroupsY = 0,
                             GLint numGroupsZ = 0 ) :
                _numGroupsX( numGroupsX ),
                _numGroupsY( numGroupsY ),
                _numGroupsZ( numGroupsZ )
            {
            }

            DispatchCompute( const DispatchCompute&,
                             const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            virtual void
            compileGLObjects( RenderInfo& ) const
            {
            }

            virtual VertexArrayState*
            createVertexArrayStateImplememtation( RenderInfo& ) const
            {
                return 0;
            }

            virtual void
            drawImplementation( RenderInfo& renderInfo ) const;

            /** Set compute shader work groups */
            void
            setComputeGroups( GLint numGroupsX,
                              GLint numGroupsY,
                              GLint numGroupsZ )
            {
                _numGroupsX = numGroupsX;
                _numGroupsY = numGroupsY;
                _numGroupsZ = numGroupsZ;
            }

            /** Get compute shader work groups */
            void
            getComputeGroups( GLint& numGroupsX,
                              GLint& numGroupsY,
                              GLint& numGroupsZ ) const
            {
                numGroupsX = _numGroupsX;
                numGroupsY = _numGroupsY;
                numGroupsZ = _numGroupsZ;
            }

        protected:

            GLint _numGroupsX, _numGroupsY, _numGroupsZ;
    };

}
