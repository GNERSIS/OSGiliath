/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Traversal visitor that compiles or releases GL objects.
 * Forces texture/VBO/program compilation for a given context.
 */
#pragma once

#include <mutex>
#include <osg/core/Inherit.hpp>
#include <osg/nodes/Geode.hpp>
#include <osg/state/State.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgUtil/Export>

namespace osgUtil
{

    /** Visitor for traversing scene graph and compiling osg::Drawable GL objects and
     * osg::StateAttribute's.
     */
    class OSGUTIL_EXPORT GLObjectsVisitor : public osg::DualModeVisitor
    {
        public:

            /** Operation modes of the visitor.*/
            enum ModeValues
            {
                COMPILE_VERTEX_BUFFER_OBJECTS    = 0X4,
                COMPILE_STATE_ATTRIBUTES         = 0X8,
                RELEASE_VERTEX_BUFFER_OBJECTS    = 0X10,
                RELEASE_STATE_ATTRIBUTES         = 0X20,
                SWITCH_ON_VERTEX_BUFFER_OBJECTS  = 0X40,
                SWITCH_OFF_VERTEX_BUFFER_OBJECTS = 0X80,
                CHECK_BLACK_LISTED_MODES         = 0X1'00,
            };

            typedef unsigned int Mode;

            /** Construct a GLObjectsVisitor to traverse all children, operating on
             * node according to specified mode, such as to compile or release
             * GL objects. Default mode is to compile GL objects.
             */
            GLObjectsVisitor( Mode mode = COMPILE_VERTEX_BUFFER_OBJECTS |
                                          COMPILE_STATE_ATTRIBUTES |
                                          CHECK_BLACK_LISTED_MODES );

            OSG_REGISTER_TYPE( osgUtil,
                               GLObjectsVisitor )

            virtual void
            reset()
            {
                _drawablesAppliedSet.clear();
                _stateSetAppliedSet.clear();
            }

            /** Set the operational mode of what operations to do on the scene graph.*/
            void
            setMode( Mode mode )
            {
                _mode = mode;
            }

            /** Get the operational mode.*/
            Mode
            getMode() const
            {
                return _mode;
            }

            /** Set the State to use during traversal. */
            void
            setState( osg::State* state )
            {
                _renderInfo.setState( state );
            }

            osg::State*
            getState()
            {
                return _renderInfo.getState();
            }

            void
            setRenderInfo( osg::RenderInfo& renderInfo )
            {
                _renderInfo = renderInfo;
            }

            osg::RenderInfo&
            getRenderInfo()
            {
                return _renderInfo;
            }

            /** Set whether and how often OpenGL errors should be checked for, defaults
             * to osg::State::ONCE_PER_ATTRIBUTE. */
            void
            setCheckForGLErrors( osg::State::CheckForGLErrors check )
            {
                _checkGLErrors = check;
            }

            /** Get whether and how often OpenGL errors should be checked for.*/
            osg::State::CheckForGLErrors
            getCheckForGLErrors() const
            {
                return _checkGLErrors;
            }

            using osg::DualModeVisitor::apply;

            /** Simply traverse using standard NodeVisitor traverse method.*/
            virtual void
            apply( osg::Node& node );

            void
            apply( osg::Drawable& drawable );
            void
            apply( osg::StateSet& stateset );

            /** Do a compile traversal and then reset any state,*/
            void
            compile( osg::Node& node );

        protected:

            typedef std::set<osg::Drawable*> DrawableAppliedSet;
            typedef std::set<osg::StateSet*> StatesSetAppliedSet;

            Mode                             _mode;
            osg::RenderInfo                  _renderInfo;
            osg::State::CheckForGLErrors     _checkGLErrors;

            DrawableAppliedSet               _drawablesAppliedSet;
            StatesSetAppliedSet              _stateSetAppliedSet;
            osg::ref_ptr<osg::Program>       _lastCompiledProgram;
    };

    class OSGUTIL_EXPORT GLObjectsOperation : public osg::GraphicsOperation
    {
        public:

            GLObjectsOperation( GLObjectsVisitor::Mode mode =
                                    GLObjectsVisitor::COMPILE_VERTEX_BUFFER_OBJECTS |
                                    GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES |
                                    GLObjectsVisitor::CHECK_BLACK_LISTED_MODES );

            GLObjectsOperation( osg::Node*             subgraph,
                                GLObjectsVisitor::Mode mode =
                                    GLObjectsVisitor::COMPILE_VERTEX_BUFFER_OBJECTS |
                                    GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES |
                                    GLObjectsVisitor::CHECK_BLACK_LISTED_MODES );

            virtual void
            operator()( osg::GraphicsContext* context );

        protected:

            osg::ref_ptr<osg::Node> _subgraph;
            GLObjectsVisitor::Mode  _mode;
    };

}
