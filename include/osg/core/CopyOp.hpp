/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Copy operation flags controlling deep vs. shallow cloning.
 * Used with Object::clone() for scene graph duplication.
 */
#pragma once

#include <osg/core/Export.hpp>
#include <unordered_map>

namespace osg
{

    class Referenced;
    class Object;
    class Image;
    class Texture;
    class StateSet;
    class StateAttribute;
    class StateAttributeCallback;
    class Uniform;
    class UniformBase;
    class UniformCallback;
    class Node;
    class Drawable;
    class Array;
    class PrimitiveSet;
    class Shape;
    class Callback;

    /** Map of original-to-clone pointers used for graph-aware deep copy.
     *  When set on CopyOp, shared subgraphs remain shared in the clone. */
    using DuplicateMap = std::unordered_map<const Object*, Object*>;

    /** Copy Op(erator) used to control whether shallow or deep copy is used
     * during copy construction and clone operation.*/
    class OSG_EXPORT CopyOp
    {

        public:

            enum Options
            {
                SHALLOW_COPY              = 0,
                DEEP_COPY_OBJECTS         = 1 << 0,
                DEEP_COPY_NODES           = 1 << 1,
                DEEP_COPY_DRAWABLES       = 1 << 2,
                DEEP_COPY_STATESETS       = 1 << 3,
                DEEP_COPY_STATEATTRIBUTES = 1 << 4,
                DEEP_COPY_TEXTURES        = 1 << 5,
                DEEP_COPY_IMAGES          = 1 << 6,
                DEEP_COPY_ARRAYS          = 1 << 7,
                DEEP_COPY_PRIMITIVES      = 1 << 8,
                DEEP_COPY_SHAPES          = 1 << 9,
                DEEP_COPY_UNIFORMS        = 1 << 10,
                DEEP_COPY_CALLBACKS       = 1 << 11,
                DEEP_COPY_USERDATA        = 1 << 12,
                DEEP_COPY_ALL             = 0X7F'FF'FF'FF,
            };

            typedef unsigned int CopyFlags;

            inline CopyOp( CopyFlags flags = SHALLOW_COPY ) :
                _flags( flags ),
                _duplicateMap( nullptr )
            {
            }

            virtual ~CopyOp()
            {
            }

            void
            setCopyFlags( CopyFlags flags )
            {
                _flags = flags;
            }

            CopyFlags
            getCopyFlags() const
            {
                return _flags;
            }

            /** Set a DuplicateMap to enable graph-aware deep copy.
             *  When set, shared objects in the original graph stay shared in the clone.
             *  The caller owns the map and must keep it alive for the CopyOp's lifetime.
             */
            void
            setDuplicateMap( DuplicateMap* dm )
            {
                _duplicateMap = dm;
            }

            DuplicateMap*
            getDuplicateMap() const
            {
                return _duplicateMap;
            }

            virtual Referenced*
            operator()( const Referenced* ref ) const;
            virtual Object*
            operator()( const Object* obj ) const;
            virtual Node*
            operator()( const Node* node ) const;
            virtual Drawable*
            operator()( const Drawable* drawable ) const;
            virtual StateSet*
            operator()( const StateSet* stateset ) const;
            virtual StateAttribute*
            operator()( const StateAttribute* attr ) const;
            virtual Texture*
            operator()( const Texture* text ) const;
            virtual Image*
            operator()( const Image* image ) const;
            virtual Array*
            operator()( const Array* array ) const;
            virtual PrimitiveSet*
            operator()( const PrimitiveSet* primitives ) const;
            virtual Shape*
            operator()( const Shape* shape ) const;
            virtual UniformBase*
            operator()( const UniformBase* uniform ) const;
            virtual Uniform*
            operator()( const Uniform* uniform ) const;
            virtual Callback*
            operator()( const Callback* nodecallback ) const;
            virtual StateAttributeCallback*
            operator()( const StateAttributeCallback* stateattributecallback ) const;
            virtual UniformCallback*
            operator()( const UniformCallback* uniformcallback ) const;

        protected:

            CopyFlags     _flags;
            DuplicateMap* _duplicateMap;
    };

}
