/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Binds a buffer object to an indexed binding point (UBO or SSBO).
 * Used for per-frame data blocks shared across shaders.
 */
#include <osg/state/BufferIndexBinding.hpp>

#include <osg/maths/Math.hpp>
#include <osg/state/State.hpp>
#include <string.h>    // for memcpy

namespace osg
{

    BufferIndexBinding::BufferIndexBinding( GLenum target,
                                            GLuint index ) :
        _target( target ),
        _bufferData( 0 ),
        _index( index ),
        _offset( 0 ),
        _size( 0 )
    {
    }

    BufferIndexBinding::BufferIndexBinding( GLenum      target,
                                            GLuint      index,
                                            BufferData* bo,
                                            GLintptr    offset,
                                            GLsizeiptr  size ) :
        _target( target ),
        _index( index ),
        _offset( offset ),
        _size( size )
    {
        setBufferData( bo );
    }

    BufferIndexBinding::BufferIndexBinding( const BufferIndexBinding& rhs,
                                            const CopyOp&             copyop ) :
        StateAttribute( rhs,
                        copyop ),
        _target( rhs._target ),
        _bufferData( static_cast<BufferData*>( copyop( rhs._bufferData.get() ) ) ),
        _index( rhs._index ),
        _offset( rhs._offset ),
        _size( rhs._size )
    {
    }

    BufferIndexBinding::~BufferIndexBinding()
    {
    }

    void
    BufferIndexBinding::setIndex( unsigned int index )
    {
        if( _index == index )
        {
            return;
        }

        ReassignToParents needToReassingToParentsWhenMemberValueChanges( this );

        _index = index;
    }

    void
    BufferIndexBinding::apply( State& state ) const
    {
        if( _bufferData.valid() )
        {
            GLBufferObject* glObject =
                _bufferData->getBufferObject()->getOrCreateGLBufferObject(
                    state.getContextID()
                );
            if( glObject->isDirty() )
            {
                glObject->compileBuffer();
            }
            glObject->_extensions->glBindBufferRange(
                _target,
                _index,
                glObject->getGLObjectID(),
                glObject->getOffset( _bufferData->getBufferIndex() ) + _offset,
                _size - _offset
            );
        }
    }

    UniformBufferBinding::UniformBufferBinding() :
        Inherit( static_cast<GLenum>( GL_UNIFORM_BUFFER ),
                 GLuint( 0 ) )
    {
    }

    UniformBufferBinding::UniformBufferBinding( GLuint index ) :
        Inherit( static_cast<GLenum>( GL_UNIFORM_BUFFER ),
                 index )
    {
    }

    UniformBufferBinding::UniformBufferBinding( GLuint      index,
                                                BufferData* bo,
                                                GLintptr    offset,
                                                GLsizeiptr  size ) :
        Inherit( static_cast<GLenum>( GL_UNIFORM_BUFFER ),
                 index,
                 bo,
                 offset,
                 size )
    {
    }

    UniformBufferBinding::UniformBufferBinding( const UniformBufferBinding& rhs,
                                                const CopyOp&               copyop ) :
        Inherit( rhs,
                 copyop )
    {
    }

    TransformFeedbackBufferBinding::TransformFeedbackBufferBinding( GLuint index ) :
        Inherit( static_cast<GLenum>( GL_TRANSFORM_FEEDBACK_BUFFER ),
                 index )
    {
    }

    TransformFeedbackBufferBinding::TransformFeedbackBufferBinding( GLuint      index,
                                                                    BufferData* bo,
                                                                    GLintptr    offset,
                                                                    GLsizeiptr  size ) :
        Inherit( static_cast<GLenum>( GL_TRANSFORM_FEEDBACK_BUFFER ),
                 index,
                 bo,
                 offset,
                 size )
    {
    }

    TransformFeedbackBufferBinding::TransformFeedbackBufferBinding(
        const TransformFeedbackBufferBinding& rhs,
        const CopyOp&                         copyop
    ) :
        Inherit( rhs,
                 copyop )
    {
    }

    AtomicCounterBufferBinding::AtomicCounterBufferBinding( GLuint index ) :
        Inherit( static_cast<GLenum>( GL_ATOMIC_COUNTER_BUFFER ),
                 index )
    {
    }

    AtomicCounterBufferBinding::AtomicCounterBufferBinding( GLuint      index,
                                                            BufferData* bo,
                                                            GLintptr    offset,
                                                            GLsizeiptr  size ) :
        Inherit( static_cast<GLenum>( GL_ATOMIC_COUNTER_BUFFER ),
                 index,
                 bo,
                 offset,
                 size )
    {
    }

    AtomicCounterBufferBinding::AtomicCounterBufferBinding(
        const AtomicCounterBufferBinding& rhs,
        const CopyOp&                     copyop
    ) :
        Inherit( rhs,
                 copyop )
    {
    }

    void
    AtomicCounterBufferBinding::readData( osg::State&     state,
                                          osg::UIntArray& uintArray ) const
    {
        if( !_bufferData )
        {
            return;
        }

        GLBufferObject* bo = _bufferData->getBufferObject()->getOrCreateGLBufferObject(
            state.getContextID()
        );
        if( !bo )
        {
            return;
        }

        GLint previousID = 0;
        glGetIntegerv( GL_ATOMIC_COUNTER_BUFFER_BINDING, &previousID );

        if( static_cast<GLuint>( previousID ) != bo->getGLObjectID() )
        {
            bo->_extensions->glBindBuffer( GL_ATOMIC_COUNTER_BUFFER,
                                           bo->getGLObjectID() );
        }

        GLubyte* src =
            ( GLubyte* )bo->_extensions->glMapBuffer( GL_ATOMIC_COUNTER_BUFFER,
                                                      GL_READ_ONLY_ARB );
        if( src )
        {
            size_t size = static_cast<size_t>(
                osg::minimum( _size,
                              static_cast<GLsizeiptr>( uintArray.getTotalDataSize() ) )
            );
            memcpy( ( void* )&( uintArray.front() ), src + _offset, size );
            bo->_extensions->glUnmapBuffer( GL_ATOMIC_COUNTER_BUFFER );
        }

        if( static_cast<GLuint>( previousID ) != bo->getGLObjectID() )
        {
            bo->_extensions->glBindBuffer( GL_ATOMIC_COUNTER_BUFFER,
                                           static_cast<GLuint>( previousID ) );
        }
    }

    ShaderStorageBufferBinding::ShaderStorageBufferBinding( GLuint index ) :
        Inherit( static_cast<GLenum>( GL_SHADER_STORAGE_BUFFER ),
                 index )
    {
    }

    ShaderStorageBufferBinding::ShaderStorageBufferBinding( GLuint      index,
                                                            BufferData* bo,
                                                            GLintptr    offset,
                                                            GLsizeiptr  size ) :
        Inherit( static_cast<GLenum>( GL_SHADER_STORAGE_BUFFER ),
                 index,
                 bo,
                 offset,
                 size )
    {
    }

    ShaderStorageBufferBinding::ShaderStorageBufferBinding(
        const ShaderStorageBufferBinding& rhs,
        const CopyOp&                     copyop
    ) :
        Inherit( rhs,
                 copyop )
    {
    }

}    // namespace osg
