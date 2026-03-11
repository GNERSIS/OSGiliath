/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Indexed color write mask for MRT. Controls per-draw-buffer
 * RGBA write masks when rendering to multiple color attachments.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/ColorMask.hpp>

namespace osg
{

    /** Encapsulates glColorMaski function : the index version of glColorMask for
     * multiple render target.
     */
    class OSG_EXPORT ColorMaski : public osg::Inherit<ColorMask, ColorMaski>
    {
        public:

            ColorMaski();

            ColorMaski( unsigned int buf,
                        bool         red,
                        bool         green,
                        bool         blue,
                        bool         alpha ) :
                Inherit( red,
                         green,
                         blue,
                         alpha ),
                _index( buf )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            ColorMaski( const ColorMaski& cm,
                        const CopyOp&     copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( cm,
                         copyop ),
                _index( cm._index )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               ColorMaski )

            Type
            getType() const override
            {
                return Type::COLORMASK;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( ColorMaski, sa )

                    COMPARE_StateAttribute_Parameter( _index );

                return ColorMask::compare( sa );
            }

            /** Return the buffer index as the member identifier.*/
            unsigned int
            getMember() const override
            {
                return _index;
            }

            /** Set the renderbuffer index of the ColorMaski. */
            void
            setIndex( unsigned int buf );

            /** Get the renderbuffer index of the ColorMaski. */
            unsigned int
            getIndex() const
            {
                return _index;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~ColorMaski();

            unsigned int _index;
    };

}
