/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Sets the clear color and clear mask for its subgraph.
 * Typically used as the root node for background color.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/nodes/Group.hpp>

namespace osg
{

    /** A Group node for clearing the color and depth buffers. Use setClearColor
     * to change the clear color, and setRequiresClear to disable/enable the call
     * clearing. You might want to disable clearing if you perform your clear by
     * drawing fullscreen geometry. If you do this, add child nodes to perform
     * such drawing. The default StateSet associated with this node places
     * children in render bin -1 to ensure that children are rendered prior to
     * the rest of the scene graph.
     */
    class OSG_EXPORT ClearNode : public osg::Inherit<Group, ClearNode>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               ClearNode )

            ClearNode();

            ClearNode( const ClearNode& cs,
                       const CopyOp&    copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( cs,
                         copyop ),
                _requiresClear( cs._requiresClear ),
                _clearColor( cs._clearColor ),
                _clearMask( cs._clearMask )
            {
            }

            /** Enable/disable clearing via glClear. */
            inline void
            setRequiresClear( bool requiresClear )
            {
                _requiresClear = requiresClear;
            }

            /** Gets whether clearing is enabled or disabled. */
            inline bool
            getRequiresClear() const
            {
                return _requiresClear;
            }

            /** Sets the clear color. */
            inline void
            setClearColor( const vec4& color )
            {
                _clearColor = color;
            }

            /** Returns the clear color. */
            inline const vec4&
            getClearColor() const
            {
                return _clearColor;
            }

            /** Set the clear mask used in glClear(..).
             * Defaults to GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT. */
            inline void
            setClearMask( GLbitfield mask )
            {
                _clearMask = mask;
            }

            /** Get the clear mask.*/
            inline GLbitfield
            getClearMask() const
            {
                return _clearMask;
            }

        protected:

            virtual ~ClearNode()
            {
            }

            bool       _requiresClear;
            vec4       _clearColor;
            GLbitfield _clearMask;
    };

}
