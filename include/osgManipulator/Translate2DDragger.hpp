/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Planar translation dragger. Constrains movement to a
 * 2D plane for planar positioning.
 */
// osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#pragma once

#include <osg/state/PolygonOffset.hpp>
#include <osgManipulator/Dragger.hpp>
#include <osgManipulator/Projector.hpp>

namespace osgManipulator
{

    /**
     * Dragger for performing 2D translation.
     */
    class OSGMANIPULATOR_EXPORT Translate2DDragger : public Dragger
    {
        public:

            Translate2DDragger();

            Translate2DDragger( const osg::Plane& plane );

            META_OSGMANIPULATOR_Object( osgManipulator,
                                        Translate2DDragger )

                /** Handle pick events on dragger and generate TranslateInLine commands.
                 */
                virtual bool handle( const PointerInfo&            pi,
                                     const osgGA::GUIEventAdapter& ea,
                                     osgGA::GUIActionAdapter&      us );

            /** Setup default geometry for dragger. */
            void
            setupDefaultGeometry();

            /** Set/Get color for dragger. */
            inline void
            setColor( const osg::vec4& color )
            {
                _color = color;
                setMaterialColor( _color, *this );
            }

            inline const osg::vec4&
            getColor() const
            {
                return _color;
            }

            /** Set/Get pick color for dragger. Pick color is color of the dragger when
               picked. It gives a visual feedback to show that the dragger has been
               picked. */
            inline void
            setPickColor( const osg::vec4& color )
            {
                _pickColor = color;
            }

            inline const osg::vec4&
            getPickColor() const
            {
                return _pickColor;
            }

        protected:

            virtual ~Translate2DDragger();

            osg::ref_ptr<PlaneProjector>     _projector;
            osg::dvec3                       _startProjectedPoint;

            osg::vec4                        _color;
            osg::vec4                        _pickColor;
            osg::ref_ptr<osg::PolygonOffset> _polygonOffset;
    };

}
