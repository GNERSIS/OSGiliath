/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Combined TabBox and Trackball dragger. Provides scale,
 * translate, and rotation in one composite manipulator.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#pragma once

#include <osgManipulator/TabBoxDragger.hpp>
#include <osgManipulator/TrackballDragger.hpp>

namespace osgManipulator
{

    /**
     * Dragger for performing rotation in all axes.
     */
    class OSGMANIPULATOR_EXPORT TabBoxTrackballDragger : public CompositeDragger
    {
        public:

            TabBoxTrackballDragger();

            META_OSGMANIPULATOR_Object( osgManipulator,
                                        TabBoxTrackballDragger )

                /** Setup default geometry for dragger. */
                void setupDefaultGeometry();

        protected:

            virtual ~TabBoxTrackballDragger();

            osg::ref_ptr<TrackballDragger> _trackballDragger;
            osg::ref_ptr<TabBoxDragger>    _tabBoxDragger;
    };

}
