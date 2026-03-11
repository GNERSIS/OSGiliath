/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Binds a buffer object to an indexed binding point (UBO or SSBO).
 * Used for per-frame data blocks shared across shaders.
 */
#pragma once

#include <osg/core/Export.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/geometry/Array.hpp>
#include <osg/geometry/BufferObject.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    class State;

    /** Encapsulate binding buffer objects to index targets. This
     * specifically supports the uniform buffer and transform feedback
     * targets.
     */

    // Common implementation superclass
    class OSG_EXPORT BufferIndexBinding : public StateAttribute
    {
        protected:

            BufferIndexBinding( GLenum target,
                                GLuint index );
            BufferIndexBinding( GLenum      target,
                                GLuint      index,
                                BufferData* bd,
                                GLintptr    offset = 0,
                                GLsizeiptr  size   = 0 );
            BufferIndexBinding( const BufferIndexBinding& rhs,
                                const CopyOp& copyop = CopyOp::SHALLOW_COPY );

        public:

            // The member value is part of the key to this state attribute in
            // the State class. Using the index target, we can separately
            // track the bindings for many different index targets.
            virtual unsigned
            getMember() const
            {
                return static_cast<unsigned int>( _index );
            }

            GLenum
            getTarget() const
            {
                return _target;
            }

            /// enable arbitrary BufferBinding (user is responsible for _target mismatch
            /// with bufferdata
            ///  what can be done is setting wrong _target and use the one of bd if not
            ///  subclassed
            void
            setTarget( GLenum t )
            {
                _target = t;
            }

            inline void
            setBufferData( BufferData* bufferdata )
            {
                if( _bufferData.valid() )
                {
                    _bufferData->removeClient( this );
                }

                _bufferData = bufferdata;

                if( _bufferData.valid() )
                {
                    if( !_bufferData->getBufferObject() )
                    {
                        _bufferData->setBufferObject( new VertexBufferObject() );
                    }
                    if( _size == 0 )
                    {
                        _size = _bufferData->getTotalDataSize();
                    }
                }
            }

            /** Get the buffer data to be bound.
             */
            inline const BufferData*
            getBufferData() const
            {
                return _bufferData.get();
            }

            inline BufferData*
            getBufferData()
            {
                return _bufferData.get();
            }

            /** Get the index target.
             */
            inline GLuint
            getIndex() const
            {
                return _index;
            }

            /** Set the index target. (and update parents StateSets)
             */
            void
            setIndex( GLuint index );

            /** Set the starting offset into the buffer data for
            the indexed target. Note: the required  alignment on the offset
            may be quite large (e.g., 256 bytes on NVidia 8600M). This
            should be checked with glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT...).
            */
            inline void
            setOffset( GLintptr offset )
            {
                _offset = offset;
            }

            inline GLintptr
            getOffset() const
            {
                return _offset;
            }

            /** Set the size override of bufferdata binded for the indexed target.
             */
            inline void
            setSize( GLsizeiptr size )
            {
                _size = size;
            }

            inline GLsizeiptr
            getSize() const
            {
                return _size;
            }

            virtual void
            apply( State& state ) const;

        protected:

            virtual ~BufferIndexBinding();
            /*const*/ GLenum    _target;
            ref_ptr<BufferData> _bufferData;
            GLuint              _index;
            GLintptr            _offset;
            GLsizeiptr          _size;
    };

    /** StateAttribute for binding a uniform buffer index target.
     */
    class OSG_EXPORT UniformBufferBinding
        : public osg::Inherit<BufferIndexBinding, UniformBufferBinding>
    {
        public:

            UniformBufferBinding();
            UniformBufferBinding( GLuint index );
            /** Create a binding for a uniform buffer index target.
             *  @param index the index target
             *  @param bd associated buffer data
             *  @param offset offset into buffer data
             *  @param size size of data in buffer data
             */
            UniformBufferBinding( GLuint      index,
                                  BufferData* bd,
                                  GLintptr    offset = 0,
                                  GLsizeiptr  size   = 0 );
            UniformBufferBinding( const UniformBufferBinding& rhs,
                                  const CopyOp& copyop = CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osg,
                               UniformBufferBinding )

            Type
            getType() const override
            {
                return Type::UNIFORMBUFFERBINDING;
            }

            int
            compare( const StateAttribute& bb ) const override
            {
                COMPARE_StateAttribute_Types( UniformBufferBinding, bb )
                    COMPARE_StateAttribute_Parameter( _target )
                        COMPARE_StateAttribute_Parameter( _index )
                            COMPARE_StateAttribute_Parameter( _bufferData )
                                COMPARE_StateAttribute_Parameter( _offset )
                                    COMPARE_StateAttribute_Parameter( _size ) return 0;
            }
    };

    /** StateAttribute for binding a transform feedback index target.
     */
    class OSG_EXPORT TransformFeedbackBufferBinding
        : public osg::Inherit<BufferIndexBinding, TransformFeedbackBufferBinding>
    {
        public:

            TransformFeedbackBufferBinding( GLuint index = 0 );
            TransformFeedbackBufferBinding( GLuint      index,
                                            BufferData* bd,
                                            GLintptr    offset = 0,
                                            GLsizeiptr  size   = 0 );
            TransformFeedbackBufferBinding( const TransformFeedbackBufferBinding& rhs,
                                            const CopyOp& copyop =
                                                CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osg,
                               TransformFeedbackBufferBinding )

            Type
            getType() const override
            {
                return Type::TRANSFORMFEEDBACKBUFFERBINDING;
            }

            int
            compare( const StateAttribute& bb ) const override
            {
                COMPARE_StateAttribute_Types( TransformFeedbackBufferBinding, bb )
                    COMPARE_StateAttribute_Parameter( _target )
                        COMPARE_StateAttribute_Parameter( _index )
                            COMPARE_StateAttribute_Parameter( _bufferData )
                                COMPARE_StateAttribute_Parameter( _offset )
                                    COMPARE_StateAttribute_Parameter( _size ) return 0;
            }
    };

    /** StateAttribute for binding a atomic counter buffer index target.
     */
    class OSG_EXPORT AtomicCounterBufferBinding
        : public osg::Inherit<BufferIndexBinding, AtomicCounterBufferBinding>
    {
        public:

            AtomicCounterBufferBinding( GLuint index = 0 );
            /** Create a binding for a atomic counter buffer index target.
             *  @param index the index target
             *  @param bd associated buffer data
             *  @param offset offset into buffer data
             *  @param size size of data in buffer data
             */
            AtomicCounterBufferBinding( GLuint      index,
                                        BufferData* bd,
                                        GLintptr    offset = 0,
                                        GLsizeiptr  size   = 0 );
            AtomicCounterBufferBinding( const AtomicCounterBufferBinding& rhs,
                                        const CopyOp& copyop = CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osg,
                               AtomicCounterBufferBinding )

            Type
            getType() const override
            {
                return Type::ATOMICCOUNTERBUFFERBINDING;
            }

            void
            readData( osg::State&     state,
                      osg::UIntArray& uintArray ) const;

            int
            compare( const StateAttribute& bb ) const override
            {
                COMPARE_StateAttribute_Types( AtomicCounterBufferBinding, bb )
                    COMPARE_StateAttribute_Parameter( _target )
                        COMPARE_StateAttribute_Parameter( _index )
                            COMPARE_StateAttribute_Parameter( _bufferData )
                                COMPARE_StateAttribute_Parameter( _offset )
                                    COMPARE_StateAttribute_Parameter( _size ) return 0;
            }
    };

    class OSG_EXPORT ShaderStorageBufferBinding
        : public osg::Inherit<BufferIndexBinding, ShaderStorageBufferBinding>
    {
        public:

            ShaderStorageBufferBinding( GLuint index = 0 );
            /** Create a binding for a shader storage buffer index target.
             *  @param index the index target
             *  @param bd associated buffer data
             *  @param offset offset into buffer data
             *  @param size size of data in buffer data
             */
            ShaderStorageBufferBinding( GLuint      index,
                                        BufferData* bd,
                                        GLintptr    offset = 0,
                                        GLsizeiptr  size   = 0 );
            ShaderStorageBufferBinding( const ShaderStorageBufferBinding& rhs,
                                        const CopyOp& copyop = CopyOp::SHALLOW_COPY );
            OSG_REGISTER_TYPE( osg,
                               ShaderStorageBufferBinding )

            Type
            getType() const override
            {
                return Type::SHADERSTORAGEBUFFERBINDING;
            }

            int
            compare( const StateAttribute& bb ) const override
            {
                COMPARE_StateAttribute_Types( ShaderStorageBufferBinding, bb )
                    COMPARE_StateAttribute_Parameter( _target )
                        COMPARE_StateAttribute_Parameter( _index )
                            COMPARE_StateAttribute_Parameter( _bufferData )
                                COMPARE_StateAttribute_Parameter( _offset )
                                    COMPARE_StateAttribute_Parameter( _size ) return 0;
            }
    };

}    // namespace osg
