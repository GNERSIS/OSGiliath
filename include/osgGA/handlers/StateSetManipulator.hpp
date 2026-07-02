/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Keyboard handler for toggling rendering modes (wireframe,
 * texturing, lighting, backface culling) at runtime.
 */
#pragma once

#include <osg/state/PolygonMode.hpp>
#include <osg/state/StateSet.hpp>
#include <osgGA/events/GUIActionAdapter.hpp>
#include <osgGA/events/GUIEventAdapter.hpp>
#include <osgGA/events/GUIEventHandler.hpp>
#include <osgGA/Export.hpp>

namespace osgGA
{

    /**
    Experimental class, not been looked at for a while, but which will
    be returned to at some point :-\
    */
    class OSGGA_EXPORT StateSetManipulator : public GUIEventHandler
    {
        public:

            using GUIEventHandler::clone;

            StateSetManipulator( osg::StateSet* stateset = 0 );

            virtual const char*
            className() const
            {
                return "StateSetManipulator";
            }

            /** attach a StateSet to the manipulator to be used for specifying view.*/
            virtual void
            setStateSet( osg::StateSet* );

            /** get the attached a StateSet.*/
            virtual osg::StateSet*
            getStateSet();

            /** get the attached a StateSet.*/
            virtual const osg::StateSet*
            getStateSet() const;

            /** Handle events, return true if handled, false otherwise.*/
            virtual bool
            handle( const GUIEventAdapter& ea,
                    GUIActionAdapter&      us );

            /** Get the keyboard and mouse usage of this manipulator.*/
            virtual void
            getUsage( osg::ApplicationUsage& usage ) const;

            void
            setMaximumNumOfTextureUnits( unsigned int i )
            {
                _maxNumOfTextureUnits = i;
            }

            unsigned int
            getMaximumNumOfTextureUnits() const
            {
                return _maxNumOfTextureUnits;
            }

            void
            setBackfaceEnabled( bool newbackface );

            bool
            getBackfaceEnabled() const
            {
                return _backface;
            };

            void
            setLightingEnabled( bool newlighting );

            bool
            getLightingEnabled() const
            {
                return _lighting;
            };

            void
            setTextureEnabled( bool newtexture );

            bool
            getTextureEnabled() const
            {
                return _texture;
            };

            void
            setPolygonMode( osg::PolygonMode::Mode newpolygonmode );
            osg::PolygonMode::Mode
            getPolygonMode() const;

            void
            cyclePolygonMode();

            void
            setKeyEventToggleBackfaceCulling( int key )
            {
                _keyEventToggleBackfaceCulling = key;
            }

            int
            getKeyEventToggleBackfaceCulling() const
            {
                return _keyEventToggleBackfaceCulling;
            }

            void
            setKeyEventToggleLighting( int key )
            {
                _keyEventToggleLighting = key;
            }

            int
            getKeyEventToggleLighting() const
            {
                return _keyEventToggleLighting;
            }

            void
            setKeyEventToggleTexturing( int key )
            {
                _keyEventToggleTexturing = key;
            }

            int
            getKeyEventToggleTexturing() const
            {
                return _keyEventToggleTexturing;
            }

            void
            setKeyEventCyclePolygonMode( int key )
            {
                _keyEventCyclePolygonMode = key;
            }

            int
            getKeyEventCyclePolygonMode() const
            {
                return _keyEventCyclePolygonMode;
            }

        protected:

            virtual ~StateSetManipulator();

            void
                                        clone();

            osg::ref_ptr<osg::StateSet> _stateset;

            bool                        _initialized;
            bool                        _backface;
            bool                        _lighting;
            bool                        _texture;
            unsigned int                _maxNumOfTextureUnits;

            int                         _keyEventToggleBackfaceCulling;
            int                         _keyEventToggleLighting;
            int                         _keyEventToggleTexturing;
            int                         _keyEventCyclePolygonMode;

            osg::PolygonMode*
            getOrCreatePolygonMode();
    };

}
