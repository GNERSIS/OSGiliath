/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Traverses the scene to detect display requirements.
 * Checks for stereo, stencil, and multisampling needs.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/rendering/DisplaySettings.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgUtil/Export.hpp>

namespace osgUtil
{

    /** A visitor for traversing a scene graph establishing which OpenGL visuals are
     * required to support rendering of that scene graph.  The results can then be used
     * by applications to set up their windows with the correct visuals.  Have a look at
     * src/osgGLUT/Viewer.cpp's Viewer::open() method for an example of how to use it.
     */
    class OSGUTIL_EXPORT DisplayRequirementsVisitor : public osg::DualModeVisitor
    {
        public:

            /** Default to traversing all children, and requiresDoubleBuffer,
             * requiresRGB and requiresDepthBuffer to true and with
             * alpha and stencil off.*/
            DisplayRequirementsVisitor();

            OSG_REGISTER_TYPE( osgUtil,
                               DisplayRequirementsVisitor )

            /** Set the DisplaySettings. */
            inline void
            setDisplaySettings( osg::DisplaySettings* ds )
            {
                _ds = ds;
            }

            /** Get the DisplaySettings */
            inline const osg::DisplaySettings*
            getDisplaySettings() const
            {
                return _ds.get();
            }

            virtual void
            applyStateSet( osg::StateSet& stateset );

            using osg::DualModeVisitor::apply;

            virtual void
            apply( osg::Node& node );

        protected:

            osg::ref_ptr<osg::DisplaySettings> _ds;
    };

}
