/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Planar dragger with corner/edge tabs for 2D scale and
 * translate in a single plane.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#pragma once

#include <osgManipulator/Scale1DDragger.hpp>
#include <osgManipulator/Scale2DDragger.hpp>
#include <osgManipulator/TranslatePlaneDragger.hpp>

namespace osgManipulator
{

    /**
     * Tab plane dragger consists of a plane with tabs on it's corners and edges
     * for scaling. And the plane is used as a 2D translate dragger.
     */
    class OSGMANIPULATOR_EXPORT TabPlaneDragger : public CompositeDragger
    {
        public:

            TabPlaneDragger( float handleScaleFactor = 20.0F );

            META_OSGMANIPULATOR_Object(
                osgManipulator,
                TabPlaneDragger
            ) virtual bool handle( const PointerInfo&            pi,
                                   const osgGA::GUIEventAdapter& ea,
                                   osgGA::GUIActionAdapter&      us );

            using CompositeDragger::setupDefaultGeometry;

            /** Setup default geometry for dragger. */
            void
            setupDefaultGeometry( bool twoSidedHandle = true );

            void
            setPlaneColor( const osg::vec4& color )
            {
                _translateDragger->setColor( color );
            }

        protected:

            virtual ~TabPlaneDragger();

            osg::ref_ptr<TranslatePlaneDragger> _translateDragger;
            osg::ref_ptr<Scale2DDragger>        _cornerScaleDragger;
            osg::ref_ptr<Scale1DDragger>        _horzEdgeScaleDragger;
            osg::ref_ptr<Scale1DDragger>        _vertEdgeScaleDragger;

            float                               _handleScaleFactor;
    };

}
