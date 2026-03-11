/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Keystone correction for projection warping. Adjusts corner
 * positions to compensate for off-axis projector alignment.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/rendering/DisplaySettings.hpp>
#include <osgGA/events/GUIEventHandler.hpp>
#include <osgViewer/core/Export.hpp>

namespace osgViewer
{

    class OSGVIEWER_EXPORT Keystone : public osg::Inherit<osg::Object, Keystone>
    {
        public:

            Keystone();

            Keystone( const Keystone&    rhs,
                      const osg::CopyOp& copop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgViewer,
                               Keystone )
            void
            reset();

            Keystone&
            operator=( const Keystone& rhs );

            void
            setKeystoneEditingEnabled( bool flag )
            {
                keystoneEditingEnabled = flag;
            }

            bool
            getKeystoneEditingEnabled() const
            {
                return keystoneEditingEnabled;
            }

            void
            setGridColor( const osg::vec4& color )
            {
                gridColour = color;
            }

            osg::vec4&
            getGridColor()
            {
                return gridColour;
            }

            const osg::vec4&
            getGridColor() const
            {
                return gridColour;
            }

            void
            setBottomLeft( const osg::dvec2& v )
            {
                bottom_left = v;
            }

            osg::dvec2&
            getBottomLeft()
            {
                return bottom_left;
            }

            const osg::dvec2&
            getBottomLeft() const
            {
                return bottom_left;
            }

            void
            setBottomRight( const osg::dvec2& v )
            {
                bottom_right = v;
            }

            osg::dvec2&
            getBottomRight()
            {
                return bottom_right;
            }

            const osg::dvec2&
            getBottomRight() const
            {
                return bottom_right;
            }

            void
            setTopLeft( const osg::dvec2& v )
            {
                top_left = v;
            }

            osg::dvec2&
            getTopLeft()
            {
                return top_left;
            }

            const osg::dvec2&
            getTopLeft() const
            {
                return top_left;
            }

            void
            setTopRight( const osg::dvec2& v )
            {
                top_right = v;
            }

            osg::dvec2&
            getTopRight()
            {
                return top_right;
            }

            const osg::dvec2&
            getTopRight() const
            {
                return top_right;
            }

            void
            compute3DPositions( osg::DisplaySettings* ds,
                                osg::vec3&            tl,
                                osg::vec3&            tr,
                                osg::vec3&            br,
                                osg::vec3&            bl ) const;

            osg::Geode*
            createKeystoneDistortionMesh();

            osg::Node*
            createGrid();

            /** Write the file specified by the "filename" user value field. Return true
             * if file successfully written. */
            bool
            writeToFile();

            /** Convenience function that loads and assigns any keystone files specified
             * in the DisplaySettings::KeystoneFileNames list, return true if Keystone's
             * assigned to DisplaySettings.*/
            static bool
            loadKeystoneFiles( osg::DisplaySettings* ds );

        protected:

            bool       keystoneEditingEnabled;

            osg::vec4  gridColour;

            osg::dvec2 bottom_left;
            osg::dvec2 bottom_right;
            osg::dvec2 top_left;
            osg::dvec2 top_right;

        protected:

            virtual ~Keystone()
            {
            }
    };

    class OSGVIEWER_EXPORT KeystoneHandler : public osgGA::GUIEventHandler
    {
        public:

            KeystoneHandler( Keystone* keystone );

            ~KeystoneHandler()
            {
            }

            bool
            handle( const osgGA::GUIEventAdapter& ea,
                    osgGA::GUIActionAdapter&      aa,
                    osg::Object*                  obj,
                    osg::NodeVisitor*             nv );

            void
            setKeystoneEditingEnabled( bool enabled )
            {
                if( _currentControlPoints.valid() )
                {
                    _currentControlPoints->setKeystoneEditingEnabled( enabled );
                }
            }

            bool
            getKeystoneEditingEnabled() const
            {
                return _currentControlPoints.valid()
                         ? _currentControlPoints->getKeystoneEditingEnabled()
                         : false;
            }

            enum Region
            {
                NONE_SELECTED,
                TOP_LEFT,
                TOP,
                TOP_RIGHT,
                RIGHT,
                BOTTOM_RIGHT,
                BOTTOM,
                BOTTOM_LEFT,
                LEFT,
                CENTER,
            };

            osg::dvec2
            incrementScale( const osgGA::GUIEventAdapter& ea ) const;
            Region
            computeRegion( const osgGA::GUIEventAdapter& ea ) const;
            void
            move( Region            region,
                  const osg::dvec2& delta );

        protected:

            osg::ref_ptr<Keystone> _keystone;

            osg::dvec2             _defaultIncrement;
            osg::dvec2             _ctrlIncrement;
            osg::dvec2             _shiftIncrement;
            osg::dvec2             _keyIncrement;

            osg::dvec2             _startPosition;
            osg::ref_ptr<Keystone> _startControlPoints;

            Region                 _selectedRegion;
            osg::ref_ptr<Keystone> _currentControlPoints;
    };

}
