/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Axis-aligned scaling dragger. Provides constrained scale
 * along X, Y, or Z axes via box-shaped handles.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#pragma once

#include <osg/geometry/ShapeDrawable.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/LineWidth.hpp>
#include <osgManipulator/Scale1DDragger.hpp>

namespace osgManipulator
{

    /**
     * Dragger for performing scaling on all 3 axes.
     */
    class OSGMANIPULATOR_EXPORT ScaleAxisDragger : public CompositeDragger
    {
        public:

            ScaleAxisDragger();

            META_OSGMANIPULATOR_Object( osgManipulator,
                                        ScaleAxisDragger )

                /** Setup default geometry for dragger. */
                void setupDefaultGeometry();

            /** Sets the width of the axis lines in pixels. */
            void
            setAxisLineWidth( float linePixelWidth );

            /** Retrieves the width of the axis lines in pixels. */
            float
            getAxisLineWidth() const
            {
                return _axisLineWidth;
            }

            /** Sets the size of the boxes. */
            void
            setBoxSize( float size );

            /** Retrieves the size of the boxes. */
            float
            getBoxSize() const
            {
                return _boxSize;
            }

        protected:

            virtual ~ScaleAxisDragger();

            osg::ref_ptr<Scale1DDragger> _xDragger;
            osg::ref_ptr<Scale1DDragger> _yDragger;
            osg::ref_ptr<Scale1DDragger> _zDragger;

            float                        _boxSize;
            float                        _axisLineWidth;

            osg::ref_ptr<osg::Geode>     _lineGeode;
            osg::ref_ptr<osg::LineWidth> _lineWidth;
            osg::ref_ptr<osg::Box>       _box;
    };

}
