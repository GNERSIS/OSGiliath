/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Positional node that places a Light in the scene graph.
 * Provides local/absolute reference frame for light positioning.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/lighting/Light.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/traversal/NodeVisitor.hpp>

namespace osg
{

    /** Leaf Node for defining a light in the scene. */
    class OSG_EXPORT LightSource : public osg::Inherit<Group, LightSource>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               LightSource )

            LightSource();

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            LightSource( const LightSource& ls,
                         const CopyOp&      copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( ls,
                         copyop ),
                _value( ls._value ),
                _light( dynamic_cast<osg::Light*>( copyop( ls._light.get() ) ) ),
                _referenceFrame( ls._referenceFrame )
            {
            }

            enum ReferenceFrame
            {
                RELATIVE_RF,
                ABSOLUTE_RF,
            };

            /** Set the light sources's ReferenceFrame, either to be relative to its
             * parent reference frame, or relative to an absolute coordinate
             * frame. RELATIVE_RF is the default.
             * Note: setting the ReferenceFrame to be ABSOLUTE_RF will
             * also set the CullingActive flag on the light source, and hence all
             * of its parents, to false, thereby disabling culling of it and
             * all its parents.  This is necessary to prevent inappropriate
             * culling, but may impact cull times if the absolute light source is
             * deep in the scene graph.  It is therefore recommended to only use
             * absolute light source at the top of the scene.
             */
            void
            setReferenceFrame( ReferenceFrame rf );

            ReferenceFrame
            getReferenceFrame() const
            {
                return _referenceFrame;
            }

            /** Set the attached light. */
            void
            setLight( Light* light );

            /** Get the attached light. */
            inline Light*
            getLight()
            {
                return _light.get();
            }

            /** Get the const attached light. */
            inline const Light*
            getLight() const
            {
                return _light.get();
            }

            /** Set the GLModes on StateSet associated with the LightSource. */
            void
            setStateSetModes( StateSet&,
                              StateAttribute::GLModeValue ) const;

            /** Set up the local StateSet. */
            void
            setLocalStateSetModes( StateAttribute::GLModeValue value =
                                       StateAttribute::ON );

            /** Set whether to use a mutex to ensure ref() and unref() are thread safe.*/
            virtual void
            setThreadSafeRefUnref( bool threadSafe );

            virtual sphere
            computeBound() const;

        protected:

            virtual ~LightSource();

            StateAttribute::GLModeValue _value;
            ref_ptr<Light>              _light;

            ReferenceFrame              _referenceFrame;
    };

}
