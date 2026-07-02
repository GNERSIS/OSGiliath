/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Viewer configuration that spans all available screens.
 * Creates synchronized windows across a multi-display setup.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgViewer/core/View.hpp>

namespace osgViewer
{

    class OSGVIEWER_EXPORT AcrossAllScreens
        : public osg::Inherit<ViewConfig, AcrossAllScreens>
    {
        public:

            AcrossAllScreens()
            {
            }

            AcrossAllScreens( const AcrossAllScreens& rhs,
                              const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( rhs,
                         copyop )
            {
            }

            OSG_REGISTER_TYPE( osgViewer,
                               AcrossAllScreens )

            virtual void
            configure( osgViewer::View& view ) const;
    };

}
