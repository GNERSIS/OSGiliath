/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract skinning strategy for RigGeometry.
 * Subclassed for software (CPU) and hardware (GPU) paths.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>

namespace osgAnimation
{

    class RigGeometry;

    class RigTransform : public osg::Inherit<osg::Object, RigTransform>
    {
        public:

            RigTransform()
            {
            }

            RigTransform( const RigTransform& org,
                          const osg::CopyOp&  copyop ) :
                Inherit( org,
                         copyop )
            {
            }

            OSG_REGISTER_TYPE( osgAnimation,
                               RigTransform )

            virtual void
            operator()( RigGeometry& )
            {
            }

            /// to call manually when a skeleton is reacheable from the rig
            /// in order to prepare technic data before rendering
            virtual bool
            prepareData( RigGeometry& )
            {
                return true;
            }

        protected:

            virtual ~RigTransform()
            {
            }
    };
    class MorphGeometry;

    class MorphTransform : public osg::Inherit<osg::Object, MorphTransform>
    {
        public:

            MorphTransform()
            {
            }

            MorphTransform( const MorphTransform& org,
                            const osg::CopyOp&    copyop ) :
                Inherit( org,
                         copyop )
            {
            }

            OSG_REGISTER_TYPE( osgAnimation,
                               MorphTransform )

            virtual void
            operator()( MorphGeometry& )
            {
            }

        protected:

            virtual ~MorphTransform()
            {
            }
    };

}
