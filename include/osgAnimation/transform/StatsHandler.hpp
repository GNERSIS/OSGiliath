/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * On-screen animation statistics display. Shows active
 * animations, blend weights, and timeline state.
 */
#pragma once

#include <osgAnimation/core/Timeline.hpp>
#include <osgGA/events/GUIEventHandler.hpp>
#include <osgText/Text.hpp>
#include <osgViewer/core/Viewer.hpp>
#include <osgViewer/core/ViewerBase.hpp>

namespace osgAnimation
{

#if 0
    struct StatAction
    {

        std::string _name;
        osg::ref_ptr<osg::Group> _group;
        osg::ref_ptr<osg::Geode> _label;
        osg::ref_ptr<osg::MatrixTransform> _graph;
        osg::ref_ptr<osgText::Text> _textLabel;

        void init(osg::Stats* stats, const std::string& name, const osg::vec3& pos, float width, float heigh, const osg::vec4& color);
        void setPosition(const osg::vec3& pos);
        void setAlpha(float v);
    };

#endif

    /** Event handler for adding on screen stats reporting to Viewers.*/
    class OSGANIMATION_EXPORT StatsHandler : public osgGA::GUIEventHandler
    {
        public:

            StatsHandler();

            enum StatsType
            {
                NO_STATS   = 0,
                FRAME_RATE = 1,
                LAST       = 2,
            };

            void
            setKeyEventTogglesOnScreenStats( int key )
            {
                _keyEventTogglesOnScreenStats = key;
            }

            int
            getKeyEventTogglesOnScreenStats() const
            {
                return _keyEventTogglesOnScreenStats;
            }

            void
            setKeyEventPrintsOutStats( int key )
            {
                _keyEventPrintsOutStats = key;
            }

            int
            getKeyEventPrintsOutStats() const
            {
                return _keyEventPrintsOutStats;
            }

            double
            getBlockMultiplier() const
            {
                return _blockMultiplier;
            }

            void
            reset();

            osg::Camera*
            getCamera()
            {
                return _camera.get();
            }

            const osg::Camera*
            getCamera() const
            {
                return _camera.get();
            }

            virtual bool
            handle( const osgGA::GUIEventAdapter& ea,
                    osgGA::GUIActionAdapter&      aa );

            /** Get the keyboard and mouse usage of this manipulator.*/
            virtual void
            getUsage( osg::ApplicationUsage& usage ) const;

        protected:

            void
            setUpHUDCamera( osgViewer::ViewerBase* viewer );

            osg::Geometry*
            createBackgroundRectangle( const osg::vec3& pos,
                                       const float      width,
                                       const float      height,
                                       osg::vec4&       color );

            osg::Geometry*
            createGeometry( const osg::vec3& pos,
                            float            height,
                            const osg::vec4& colour,
                            unsigned int     numBlocks );

            osg::Geometry*
            createFrameMarkers( const osg::vec3& pos,
                                float            height,
                                const osg::vec4& colour,
                                unsigned int     numBlocks );

            osg::Geometry*
            createTick( const osg::vec3& pos,
                        float            height,
                        const osg::vec4& colour,
                        unsigned int     numTicks );

            osg::Node*
            createCameraTimeStats( const std::string& font,
                                   osg::vec3&         pos,
                                   float              startBlocks,
                                   bool               acquireGPUStats,
                                   float              characterSize,
                                   osg::Stats*        viewerStats,
                                   osg::Camera*       camera );

            void
                                      setUpScene( osgViewer::Viewer* viewer );

            int                       _keyEventTogglesOnScreenStats;
            int                       _keyEventPrintsOutStats;

            int                       _statsType;

            bool                      _initialized;
            osg::ref_ptr<osg::Camera> _camera;

            osg::ref_ptr<osg::Switch> _switch;
            osg::ref_ptr<osg::Group>  _group;

            unsigned int              _frameRateChildNum;
            unsigned int              _numBlocks;
            double                    _blockMultiplier;

            float                     _statsWidth;
            float                     _statsHeight;

            // std::map<std::string, StatAction >      _actions;
    };

}
