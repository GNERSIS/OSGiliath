/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Camera manipulator that switches between CameraView nodes.
 * Cycles through named viewpoints in the scene graph.
 */
#pragma once

#include <osg/maths/quat.hpp>
#include <osg/nodes/CameraView.hpp>
#include <osgGA/manipulators/CameraManipulator.hpp>

namespace osgGA
{

    class OSGGA_EXPORT CameraViewSwitchManipulator : public CameraManipulator
    {
        public:

            CameraViewSwitchManipulator()
            {
            }

            virtual const char*
            className() const
            {
                return "CameraViewSwitcher";
            }

            /** set the position of the matrix manipulator using a 4x4 dmat4.*/
            virtual void
            setByMatrix( const osg::dmat4& /*matrix*/ )
            {
            }

            /** set the position of the matrix manipulator using a 4x4 dmat4.*/
            virtual void
            setByInverseMatrix( const osg::dmat4& /*matrix*/ )
            {
            }

            /** get the position of the manipulator as 4x4 dmat4.*/
            virtual osg::dmat4
            getMatrix() const;

            /** get the position of the manipulator as a inverse matrix of the
             * manipulator, typically used as a model view matrix.*/
            virtual osg::dmat4
            getInverseMatrix() const;

            /** Attach a node to the manipulator.
                Automatically detaches previously attached node.
                setNode(NULL) detaches previously nodes.
                Is ignored by manipulators which do not require a reference model.*/
            virtual void
            setNode( osg::Node* );

            /** Return node if attached.*/
            virtual const osg::Node*
            getNode() const
            {
                return _node.get();
            }

            /** Return node if attached.*/
            virtual osg::Node*
            getNode()
            {
                return _node.get();
            }

            /** Start/restart the manipulator.*/
            virtual void
            init( const GUIEventAdapter& /*ea*/,
                  GUIActionAdapter& /*aa*/ )
            {
                _currentView = 0;
            }

            /** handle events, return true if handled, false otherwise.*/
            virtual bool
            handle( const GUIEventAdapter& ea,
                    GUIActionAdapter&      us );

            /** Get the keyboard and mouse usage of this manipulator.*/
            virtual void
            getUsage( osg::ApplicationUsage& usage ) const;

            typedef std::vector<osg::ref_ptr<osg::CameraView>> CameraViewList;

        protected:

            virtual ~CameraViewSwitchManipulator()
            {
            }

            osg::ref_ptr<osg::Node> _node;

            CameraViewList          _cameraViews;
            unsigned int            _currentView;
    };

}
