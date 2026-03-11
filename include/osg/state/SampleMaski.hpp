/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Sample mask for multisample rendering. Controls which samples
 * are written during MSAA via glSampleMaski.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /**
     *  osg::SampleMaski does nothing if OpenGL 3.2 or ARB_texture_multisample are not
     * available.
     */
    class OSG_EXPORT SampleMaski : public osg::Inherit<StateAttribute, SampleMaski>
    {
        public:

            SampleMaski();

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            SampleMaski( const SampleMaski& sampleMaski,
                         const CopyOp&      copyop = CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osg,
                               SampleMaski )

            Type
            getType() const override
            {
                return Type::SAMPLEMASKI;
            } /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/

            int
            compare( const StateAttribute& sa ) const override;

            inline void
            setMask( unsigned int mask,
                     unsigned int maskNumber = 0U )
            {
                _sampleMask[maskNumber] = mask;
            }

            inline unsigned int
            getMask( unsigned int maskNumber = 0U ) const
            {
                return _sampleMask[maskNumber];
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~SampleMaski();

            // For now support only up to 64 bit mask;
            unsigned int _sampleMask[2];
    };

}
