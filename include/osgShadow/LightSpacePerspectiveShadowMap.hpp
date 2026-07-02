/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Light-space perspective shadow mapping (LiSPSM). Warps the
 * shadow projection for better near-camera depth resolution.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgShadow/MinimalCullBoundsShadowMap.hpp>
#include <osgShadow/MinimalDrawBoundsShadowMap.hpp>
#include <osgShadow/ProjectionShadowMap.hpp>

namespace osgShadow
{

    // Class implements
    // "Light Space Perspective Shadow Maps" algorithm by
    // Michael Wimmer, Daniel Scherzer, Werner Purgathofer
    // http://www.cg.tuwien.ac.at/research/vr/lispsm/

    class LispSM;

    class OSGSHADOW_EXPORT LightSpacePerspectiveShadowMapAlgorithm
    {
        public:

            LightSpacePerspectiveShadowMapAlgorithm();
            ~LightSpacePerspectiveShadowMapAlgorithm();

            void
            operator()( const osgShadow::ConvexPolyhedron* hullShadowedView,
                        const osg::Camera*                 cameraMain,
                        osg::Camera*                       cameraShadow ) const;

        protected:

            LispSM* lispsm;
    };

    // Optimized for draw traversal shadow bounds
    class OSGSHADOW_EXPORT LightSpacePerspectiveShadowMapDB
        : public ProjectionShadowMap<MinimalDrawBoundsShadowMap,
                                     LightSpacePerspectiveShadowMapAlgorithm>
    {
        public:

            /** Convenient typedef used in definition of ViewData struct and methods */
            typedef ProjectionShadowMap<MinimalDrawBoundsShadowMap,
                                        LightSpacePerspectiveShadowMapAlgorithm>
                BaseClass;

            /** Classic OSG constructor */
            LightSpacePerspectiveShadowMapDB()
            {
            }

            /** Classic OSG cloning constructor */
            LightSpacePerspectiveShadowMapDB(
                const LightSpacePerspectiveShadowMapDB& copy,
                const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY
            ) :
                BaseClass( copy,
                           copyop )
            {
            }

            /** Declaration of standard OSG object methods */
            OSG_REGISTER_TYPE( osgShadow,
                               LightSpacePerspectiveShadowMapDB )
    };

    // Optimized for cull traversal shadow bounds
    class OSGSHADOW_EXPORT LightSpacePerspectiveShadowMapCB
        : public ProjectionShadowMap<MinimalCullBoundsShadowMap,
                                     LightSpacePerspectiveShadowMapAlgorithm>
    {
        public:

            /** Convenient typedef used in definition of ViewData struct and methods */
            typedef ProjectionShadowMap<MinimalCullBoundsShadowMap,
                                        LightSpacePerspectiveShadowMapAlgorithm>
                BaseClass;

            /** Classic OSG constructor */
            LightSpacePerspectiveShadowMapCB()
            {
            }

            /** Classic OSG cloning constructor */
            LightSpacePerspectiveShadowMapCB(
                const LightSpacePerspectiveShadowMapCB& copy,
                const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY
            ) :
                BaseClass( copy,
                           copyop )
            {
            }

            /** Declaration of standard OSG object methods */
            OSG_REGISTER_TYPE( osgShadow,
                               LightSpacePerspectiveShadowMapCB )
    };

    // Optimized for view frustum bounds
    class OSGSHADOW_EXPORT LightSpacePerspectiveShadowMapVB
        : public ProjectionShadowMap<MinimalShadowMap,
                                     LightSpacePerspectiveShadowMapAlgorithm>
    {
        public:

            /** Convenient typedef used in definition of ViewData struct and methods */
            typedef ProjectionShadowMap<MinimalShadowMap,
                                        LightSpacePerspectiveShadowMapAlgorithm>
                BaseClass;

            /** Classic OSG constructor */
            LightSpacePerspectiveShadowMapVB()
            {
            }

            /** Classic OSG cloning constructor */
            LightSpacePerspectiveShadowMapVB(
                const LightSpacePerspectiveShadowMapVB& copy,
                const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY
            ) :
                BaseClass( copy,
                           copyop )
            {
            }

            /** Declaration of standard OSG object methods */
            OSG_REGISTER_TYPE( osgShadow,
                               LightSpacePerspectiveShadowMapVB )
    };

    typedef LightSpacePerspectiveShadowMapDB LightSpacePerspectiveShadowMap;

}    // namespace osgShadow
