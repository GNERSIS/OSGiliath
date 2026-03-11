/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Trackball manipulator extended with multi-touch gesture support.
 * Handles pinch-to-zoom and two-finger rotation.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osgGA/manipulators/TrackballManipulator.hpp>

namespace osgGA
{

    class OSGGA_EXPORT MultiTouchTrackballManipulator
        : public osg::Inherit<TrackballManipulator, MultiTouchTrackballManipulator>
    {
            typedef TrackballManipulator inherited;

        public:

            MultiTouchTrackballManipulator( int flags = DEFAULT_SETTINGS );
            MultiTouchTrackballManipulator( const MultiTouchTrackballManipulator& tm,
                                            const osg::CopyOp& copyOp =
                                                osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgGA,
                               MultiTouchTrackballManipulator )

            bool
            handle( const GUIEventAdapter& ea,
                    GUIActionAdapter&      us );

        protected:

            virtual void
            handleMultiTouchDrag( const GUIEventAdapter* now,
                                  const GUIEventAdapter* last,
                                  const double           eventTimeDelta );

            osg::ref_ptr<GUIEventAdapter> _lastEvent;
    };

}
