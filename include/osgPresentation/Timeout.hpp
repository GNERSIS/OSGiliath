/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Idle timeout handler for presentations. Returns to a home
 * slide after a configurable period of inactivity.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/nodes/Transform.hpp>
#include <osgPresentation/SlideEventHandler.hpp>

namespace osgPresentation
{

    class OSGPRESENTATION_EXPORT HUDSettings : public osg::Referenced
    {
        public:

            HUDSettings( double       slideDistance,
                         float        eyeOffset,
                         unsigned int leftMask,
                         unsigned int rightMask );

            virtual bool
            getModelViewMatrix( osg::dmat4&       matrix,
                                osg::NodeVisitor* nv ) const;

            virtual bool
                         getInverseModelViewMatrix( osg::dmat4&       matrix,
                                                    osg::NodeVisitor* nv ) const;

            double       _slideDistance;
            double       _eyeOffset;
            unsigned int _leftMask;
            unsigned int _rightMask;

        protected:

            virtual ~HUDSettings();
    };

    class OSGPRESENTATION_EXPORT Timeout : public osg::Inherit<osg::Transform, Timeout>
    {
        public:

            Timeout( HUDSettings* hudSettings = 0 );

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            Timeout( const Timeout&     timeout,
                     const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgPresentation,
                               Timeout )

            void
            setIdleDurationBeforeTimeoutDisplay( double t )
            {
                _idleDurationBeforeTimeoutDisplay = t;
            }

            double
            getIdleDurationBeforeTimeoutDisplay() const
            {
                return _idleDurationBeforeTimeoutDisplay;
            }

            void
            setIdleDurationBeforeTimeoutAction( double t )
            {
                _idleDurationBeforeTimeoutAction = t;
            }

            double
            getIdleDurationBeforeTimeoutAction() const
            {
                return _idleDurationBeforeTimeoutAction;
            }

            void
            setKeyStartsTimoutDisplay( int key )
            {
                _keyStartsTimoutDisplay = key;
            }

            int
            getKeyStartsTimoutDisplay() const
            {
                return _keyStartsTimoutDisplay;
            }

            void
            setKeyDismissTimoutDisplay( int key )
            {
                _keyDismissTimoutDisplay = key;
            }

            int
            getKeyDismissTimoutDisplay() const
            {
                return _keyDismissTimoutDisplay;
            }

            void
            setKeyRunTimoutAction( int key )
            {
                _keyRunTimeoutAction = key;
            }

            int
            getKeyRunTimoutAction() const
            {
                return _keyRunTimeoutAction;
            }

            void
            setDisplayBroadcastKeyPosition( const osgPresentation::KeyPosition& keyPos )
            {
                _displayBroadcastKeyPos = keyPos;
            }

            const osgPresentation::KeyPosition&
            getDisplayBroadcastKeyPosition() const
            {
                return _displayBroadcastKeyPos;
            }

            void
            setDismissBroadcastKeyPosition( const osgPresentation::KeyPosition& keyPos )
            {
                _dismissBroadcastKeyPos = keyPos;
            }

            const osgPresentation::KeyPosition&
            getDismissBroadcastKeyPosition() const
            {
                return _dismissBroadcastKeyPos;
            }

            void
            setActionKeyPosition( const osgPresentation::KeyPosition& keyPos )
            {
                _actionKeyPos = keyPos;
            }

            const osgPresentation::KeyPosition&
            getActionKeyPosition() const
            {
                return _actionKeyPos;
            }

            void
            setActionBroadcastKeyPosition( const osgPresentation::KeyPosition& keyPos )
            {
                _actionBroadcastKeyPos = keyPos;
            }

            const osgPresentation::KeyPosition&
            getActionBroadcastKeyPosition() const
            {
                return _actionBroadcastKeyPos;
            }

            void
            setActionJumpData( const JumpData& jumpData )
            {
                _actionJumpData = jumpData;
            }

            const JumpData&
            getActionJumpData() const
            {
                return _actionJumpData;
            }

            virtual bool
            computeLocalToWorldMatrix( osg::dmat4&       matrix,
                                       osg::NodeVisitor* nv ) const;

            virtual bool
            computeWorldToLocalMatrix( osg::dmat4& matrix,
                                       osg::NodeVisitor* ) const;

            virtual void
            traverse( osg::NodeVisitor& nv );

        protected:

            virtual ~Timeout();

            void
            broadcastEvent( osgViewer::Viewer*                  viewer,
                            const osgPresentation::KeyPosition& keyPos );

            osg::ref_ptr<HUDSettings>    _hudSettings;

            int                          _previousFrameNumber;
            double                       _timeOfLastEvent;
            bool                         _displayTimeout;

            double                       _idleDurationBeforeTimeoutDisplay;
            double                       _idleDurationBeforeTimeoutAction;

            int                          _keyStartsTimoutDisplay;
            int                          _keyDismissTimoutDisplay;
            int                          _keyRunTimeoutAction;

            osgPresentation::KeyPosition _displayBroadcastKeyPos;
            osgPresentation::KeyPosition _dismissBroadcastKeyPos;

            osgPresentation::KeyPosition _actionKeyPos;
            osgPresentation::KeyPosition _actionBroadcastKeyPos;
            JumpData                     _actionJumpData;
    };

}
