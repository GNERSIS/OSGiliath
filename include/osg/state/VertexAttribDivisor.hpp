/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Vertex attribute instance divisor for instanced rendering.
 * Controls per-instance vs. per-vertex attribute advance rate.
 */
#pragma once

#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** VertexAttribDivisor state class which encapsulates OpenGL glVertexAttribDivisor()
     * functionality. */
    class OSG_EXPORT VertexAttribDivisor : public StateAttribute
    {
        public:

            VertexAttribDivisor();

            VertexAttribDivisor( unsigned int index,
                                 unsigned int divisor );

            /** Copy constructor using CopyOp to manage deep vs shallow copy. */
            VertexAttribDivisor( const VertexAttribDivisor& vad,
                                 const CopyOp& copyop = CopyOp::SHALLOW_COPY ) :
                StateAttribute( vad,
                                copyop ),
                _index( vad._index ),
                _divisor( vad._divisor )
            {
            }

            virtual osg::Object*
            cloneType() const
            {
                return new VertexAttribDivisor( _index, 0 );
            }

            virtual osg::Object*
            clone( const osg::CopyOp& copyop ) const
            {
                return new VertexAttribDivisor( *this, copyop );
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const VertexAttribDivisor*>( obj ) != NULL;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return "VertexAttribDivisor";
            }

            virtual Type
            getType() const
            {
                return Type::VERTEX_ATTRIB_DIVISOR;
            }

            /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
            virtual int
            compare( const StateAttribute& sa ) const
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( VertexAttribDivisor, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _index )
                        COMPARE_StateAttribute_Parameter(
                            _divisor
                        ) return 0;    // passed all the above comparison macros, must be
                                       // equal.
            }

            virtual unsigned int
            getMember() const
            {
                return _index;
            }

            /** Set the vertex attrib index - the vertex attribute slot that the divisor
             * should apply to. */
            inline void
            setIndex( unsigned int index )
            {
                _index = index;
            }

            /** Get the vertex attrib index. */
            inline unsigned int
            getIndex() const
            {
                return _index;
            }

            /** Set the vertex attrib divisor. */
            inline void
            setDivisor( unsigned int divisor )
            {
                _divisor = divisor;
            }

            /** Get the vertex attrib divisor. */
            inline unsigned int
            getDivisor() const
            {
                return _divisor;
            }

            /** Apply to the OpenGL state machine. */
            virtual void
            apply( State& state ) const;

        protected:

            virtual ~VertexAttribDivisor();

            unsigned int _index;
            unsigned int _divisor;
    };

}
