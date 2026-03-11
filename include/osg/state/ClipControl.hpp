/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Clip volume control attribute. Configures clip origin
 * (lower-left or upper-left) and depth range (negative-one-to-one
 * or zero-to-one).
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Encapsulate OpenGL glClipControl functions.
     */
    class OSG_EXPORT ClipControl : public osg::Inherit<StateAttribute, ClipControl>
    {
        public:

            enum Origin
            {
                LOWER_LEFT = GL_LOWER_LEFT,
                UPPER_LEFT = GL_UPPER_LEFT,
            };

            enum DepthMode
            {
                NEGATIVE_ONE_TO_ONE = GL_NEGATIVE_ONE_TO_ONE,
                ZERO_TO_ONE         = GL_ZERO_TO_ONE,
            };

            ClipControl( Origin    origin    = LOWER_LEFT,
                         DepthMode depthMode = NEGATIVE_ONE_TO_ONE );

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            ClipControl( const ClipControl& clipControl,
                         const CopyOp&      copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               ClipControl )

            Type
            getType() const override
            {
                return Type::CLIPCONTROL;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
            int
            compare( const StateAttribute& sa ) const override
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( ClipControl, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _origin )
                        COMPARE_StateAttribute_Parameter(
                            _depthMode
                        ) return 0;    // passed all the above comparison macros, must be
                                       // equal.
            }

            void
            setOrigin( Origin origin )
            {
                _origin = origin;
            }

            Origin
            getOrigin() const
            {
                return _origin;
            }

            void
            setDepthMode( DepthMode depthMode )
            {
                _depthMode = depthMode;
            }

            DepthMode
            getDepthMode() const
            {
                return _depthMode;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~ClipControl();

            Origin    _origin;
            DepthMode _depthMode;
    };

}
