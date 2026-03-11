/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Polygon rasterization mode (fill, line, point).
 * Used for wireframe rendering and debug visualization.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/GL>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** State Class for setting OpenGL's polygon culling mode.
     */
    class OSG_EXPORT PolygonMode : public osg::Inherit<StateAttribute, PolygonMode>
    {
        public:

            enum class Mode
            {
                POINT = GL_POINT,
                LINE  = GL_LINE,
                FILL  = GL_FILL,
            };

            enum class Face
            {
                FRONT_AND_BACK,
                FRONT,
                BACK,
            };

            PolygonMode();

            PolygonMode( Face face,
                         Mode mode );

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            PolygonMode( const PolygonMode& pm,
                         const CopyOp&      copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( pm,
                         copyop ),
                _modeFront( pm._modeFront ),
                _modeBack( pm._modeBack )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               PolygonMode )

            Type
            getType() const override
            {
                return Type::POLYGONMODE;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
            int
            compare( const StateAttribute& sa ) const override
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( PolygonMode, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _modeFront )
                        COMPARE_StateAttribute_Parameter(
                            _modeBack
                        ) return 0;    // passed all the above comparison macros, must be
                                       // equal.
            }

            void
            setMode( Face face,
                     Mode mode );
            Mode
            getMode( Face face ) const;

            inline bool
            getFrontAndBack() const
            {
                return _modeFront == _modeBack;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~PolygonMode();

            Mode _modeFront;
            Mode _modeBack;
    };

}
