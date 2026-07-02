/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Update callback that applies an AnimationPath transform.
 * Simpler alternative to AnimationPathCallback for nodes.
 */
// C++ header

#pragma once

#include <osg/core/Callback.hpp>
#include <osg/maths/compat.hpp>
#include <osgUtil/Export.hpp>

namespace osgUtil
{

    /** TransformCallback is now deprecated, use osg::AnimationPathCallback instead.*/
    class OSGUTIL_EXPORT TransformCallback : public osg::NodeCallback
    {

        public:

            TransformCallback( const osg::vec3& pivot,
                               const osg::vec3& axis,
                               float            angularVelocity );

            void
            setPause( bool pause )
            {
                _pause = pause;
            }

            /** implements the callback*/
            virtual void
            operator()( osg::Node*        node,
                        osg::NodeVisitor* nv );

        protected:

            float        _angular_velocity;
            osg::vec3    _pivot;
            osg::vec3    _axis;

            unsigned int _previousTraversalNumber;
            double       _previousTime;
            bool         _pause;
    };

}
