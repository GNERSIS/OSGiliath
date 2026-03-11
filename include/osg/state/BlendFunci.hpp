/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Indexed alpha blending for MRT. Configures per-draw-buffer
 * blend factors when rendering to multiple color attachments.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/state/BlendFunc.hpp>

namespace osg
{

    /** Encapsulates glBlendFunci function : the index version of glBlendEquation for
     * multiple render target.
     */
    class OSG_EXPORT BlendFunci : public osg::Inherit<BlendFunc, BlendFunci>
    {
        public:

            BlendFunci();

            BlendFunci( unsigned int buf,
                        GLenum       source,
                        GLenum       destination ) :
                Inherit( source,
                         destination ),
                _index( buf )
            {
            }

            BlendFunci( unsigned int buf,
                        GLenum       source,
                        GLenum       destination,
                        GLenum       source_alpha,
                        GLenum       destination_alpha ) :
                Inherit( source,
                         destination,
                         source_alpha,
                         destination_alpha ),
                _index( buf )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            BlendFunci( const BlendFunci& cm,
                        const CopyOp&     copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( cm,
                         copyop ),
                _index( cm._index )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               BlendFunci )

            Type
            getType() const override
            {
                return Type::BLENDFUNC;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            int
            compare( const StateAttribute& sa ) const override
            {
                // Check for equal types, then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( BlendFunci, sa )

                    COMPARE_StateAttribute_Parameter( _index );

                return BlendFunc::compare( sa );
            }

            /** Return the buffer index as the member identifier.*/
            unsigned int
            getMember() const override
            {
                return _index;
            }

            /** Set the renderbuffer index of the BlendFunci. */
            void
            setIndex( unsigned int buf );

            /** Get the renderbuffer index of the BlendFunci. */
            unsigned int
            getIndex() const
            {
                return _index;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~BlendFunci();

            unsigned int _index;
    };

}
