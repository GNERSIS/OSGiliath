/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Template wrapper providing type-safe access to a BufferObject.
 * Maps typed arrays to their underlying GPU buffer storage.
 */
#pragma once

#include <osg/geometry/BufferObject.hpp>
#include <vector>

namespace osg
{

    /** Template buffer class to be used with a struct as template parameter.
     * This class is useful to send C++ structures on the GPU (e.g. for uniform blocks)
     * but be careful to the alignments rules on the GPU side !
     */
    template<typename T>
    class BufferTemplate : public BufferData
    {
        public:

            BufferTemplate() :
                BufferData(),
                _data( T() )
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            BufferTemplate( const BufferTemplate<T>& bt,
                            const CopyOp&            copyop = CopyOp::SHALLOW_COPY ) :
                osg::BufferData( bt,
                                 copyop ),
                _data( bt._data )
            {
            }

            virtual bool
            isSameKindAs( const Object* obj ) const
            {
                return dynamic_cast<const BufferTemplate<T>*>( obj ) != NULL;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return "BufferTemplate<T>";
            }

            virtual Object*
            cloneType() const
            {
                return new BufferTemplate<T>();
            }

            virtual Object*
            clone( const CopyOp& copyop ) const
            {
                return new BufferTemplate<T>( *this, copyop );
            }

            virtual const GLvoid*
            getDataPointer() const
            {
                return &_data;
            }

            virtual unsigned int
            getTotalDataSize() const
            {
                return sizeof( T );
            }

            const T&
            getData() const
            {
                return _data;
            }

            T&
            getData()
            {
                return _data;
            }

            void
            setData( const T& data )
            {
                _data = data;
                dirty();
            }

        protected:

            virtual ~BufferTemplate() {};

        private:

            T _data;
    };

    /** BufferTemplate specialization for std::vector
     */
    template<typename T>
    class BufferTemplate<std::vector<T>> : public BufferData
    {
        public:

            BufferTemplate() :
                BufferData(),
                _data()
            {
            }

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            BufferTemplate( const BufferTemplate<std::vector<T>>& bt,
                            const CopyOp& copyop = CopyOp::SHALLOW_COPY ) :
                osg::BufferData( bt,
                                 copyop ),
                _data( bt._data )
            {
            }

            virtual bool
            isSameKindAs( const Object* obj ) const
            {
                return dynamic_cast<const BufferTemplate<std::vector<T>>*>( obj ) !=
                       NULL;
            }

            virtual const char*
            libraryName() const
            {
                return "osg";
            }

            virtual const char*
            className() const
            {
                return "BufferTemplate<std::vector<T> >";
            }

            virtual Object*
            cloneType() const
            {
                return new BufferTemplate<std::vector<T>>();
            }

            virtual Object*
            clone( const CopyOp& copyop ) const
            {
                return new BufferTemplate<std::vector<T>>( *this, copyop );
            }

            virtual const GLvoid*
            getDataPointer() const
            {
                return &_data[0];
            }

            virtual unsigned int
            getTotalDataSize() const
            {
                return _data.size() * sizeof( T );
            }

            const std::vector<T>&
            getData() const
            {
                return _data;
            }

            std::vector<T>&
            getData()
            {
                return _data;
            }

            void
            setData( const std::vector<T>& data )
            {
                _data = data;
                dirty();
            }

        protected:

            virtual ~BufferTemplate() {};

        private:

            std::vector<T> _data;
    };

}
