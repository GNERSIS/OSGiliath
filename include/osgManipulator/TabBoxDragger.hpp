/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Box with handle tabs for scaling and translation.
 * Provides face/edge/corner handles for 3D manipulation.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#pragma once

#include <osgManipulator/TabPlaneDragger.hpp>

namespace osgManipulator
{

    /**
     * TabBoxDragger consists of 6 TabPlaneDraggers to form a box dragger that
     * performs translation and scaling.
     */
    class OSGMANIPULATOR_EXPORT TabBoxDragger : public CompositeDragger
    {
        public:

            TabBoxDragger();

            META_OSGMANIPULATOR_Object( osgManipulator,
                                        TabBoxDragger )

                /** Setup default geometry for dragger. */
                void setupDefaultGeometry();

            void
            setPlaneColor( const osg::vec4& color );

        protected:

            virtual ~TabBoxDragger();

            std::vector<osg::ref_ptr<TabPlaneDragger>> _planeDraggers;
    };

}
