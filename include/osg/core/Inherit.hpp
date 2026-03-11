/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * CRTP helper template for implementing clone(), className(),
 * and create() factories with minimal boilerplate.
 */
#pragma once

#include <osg/core/CopyOp.hpp>
#include <osg/core/Object.hpp>
#include <osg/core/ref_ptr.hpp>
#include <type_traits>
#include <utility>

namespace osg
{

    // Forward declarations needed for InheritAccept
    class Node;
    class NodeVisitor;
    class ConstNodeVisitor;

    // Primary template: non-Node parents — no accept() generated
    template<class Parent, class Sub, bool IsNode = std::is_base_of_v<osg::Node, Parent>>
    struct InheritAccept : public Parent
    {
            using Parent::Parent;
    };

    // Specialization: Node-derived parents — generate both accept() methods
    // Method bodies are defined out-of-line below to avoid requiring
    // NodeVisitor to be complete at parse time (breaks ValueMap etc.)
    template<class Parent, class Sub>
    struct InheritAccept<Parent, Sub, true> : public Parent
    {
            using Parent::accept;
            using Parent::Parent;

            void
            accept( osg::NodeVisitor& nv ) override;
            void
            accept( osg::ConstNodeVisitor& nv ) const override;
    };

/** Lightweight macro to declare the library and class name strings inside a
 *  class that inherits from osg::Inherit<>. Replaces the name-related portion
 *  of META_Object / META_Node / META_StateAttribute.
 *
 *  Usage:
 *    class MyNode : public Inherit<Group, MyNode> {
 *    public:
 *        OSG_REGISTER_TYPE(osg, MyNode)
 *        ...
 *    };
 */
#define OSG_REGISTER_TYPE( LIB, CLASS ) \
    static const char* _s_libraryName() \
    {                                   \
        return #LIB;                    \
    }                                   \
    static const char* _s_className()   \
    {                                   \
        return #CLASS;                  \
    }

    /** CRTP (Curiously Recurring Template Pattern) base that auto-generates the
     *  boilerplate virtual methods traditionally provided by the META_Object,
     *  META_Node, META_Shape and META_StateAttribute macros.
     *
     *  Generated overrides:
     *    - cloneType()       -- returns a default-constructed Subclass
     *    - clone(CopyOp)     -- returns a copy-constructed Subclass
     *    - isSameKindAs()    -- uses is_compatible() chain (no dynamic_cast)
     *    - is_compatible()   -- type hierarchy chain via typeid
     *    - type_info()       -- returns typeid(Subclass)
     *    - sizeofObject()    -- returns sizeof(Subclass)
     *    - libraryName()     -- delegates to Subclass::_s_libraryName()
     *    - className()       -- delegates to Subclass::_s_className()
     *
     *  The Subclass must provide two static methods that return the library and
     *  class names as string literals. The easiest way is the OSG_REGISTER_TYPE
     *  macro:
     *
     *    class MatrixTransform : public Inherit<Transform, MatrixTransform> {
     *    public:
     *        OSG_REGISTER_TYPE(osg, MatrixTransform)
     *        ...
     *    };
     *
     *  Static factory methods are provided so callers can avoid naked new:
     *
     *    auto mt = MatrixTransform::create();
     *    auto mt = MatrixTransform::create_if(needTransform);
     *
     *  For Node-derived classes, InheritAccept<> (the intermediate base)
     *  auto-generates both accept(NodeVisitor&) and accept(ConstNodeVisitor&).
     *  Shape::accept and StateAttribute::getType are NOT generated here;
     *  subclasses that need those must still provide them explicitly.
     */
    template<class ParentClass, class Subclass>
    class Inherit : public InheritAccept<ParentClass, Subclass>
    {
            using Base = InheritAccept<ParentClass, Subclass>;

        public:

            // Inherit all constructors from the parent
            template<typename... Args>
            Inherit( Args&&... args ) :
                Base( std::forward<Args>( args )... )
            {
            }

            /** Factory method -- returns a ref_ptr, no naked new required. */
            template<typename... Args>
            static osg::ref_ptr<Subclass>
            create( Args&&... args )
            {
                return osg::ref_ptr<Subclass>(
                    new Subclass( std::forward<Args>( args )... )
                );
            }

            /** Conditional factory -- returns empty ref_ptr when flag is false. */
            template<typename... Args>
            static osg::ref_ptr<Subclass>
            create_if( bool flag,
                       Args&&... args )
            {
                if( flag )
                {
                    return osg::ref_ptr<Subclass>(
                        new Subclass( std::forward<Args>( args )... )
                    );
                }
                return {};
            }

            // -- META_Object equivalent overrides ----------------------------------

            osg::Object*
            cloneType() const override
            {
                return new Subclass();
            }

            osg::Object*
            clone( const osg::CopyOp& copyop ) const override
            {
                return new Subclass( *dynamic_cast<const Subclass*>( this ), copyop );
            }

            bool
            isSameKindAs( const osg::Object* obj ) const override
            {
                return obj && obj->is_compatible( typeid( Subclass ) );
            }

            // -- Type introspection ------------------------------------------------

            bool
            is_compatible( const std::type_info& type ) const noexcept override
            {
                return typeid( Subclass ) == type || ParentClass::is_compatible( type );
            }

            const std::type_info&
            type_info() const noexcept override
            {
                return typeid( Subclass );
            }

            std::size_t
            sizeofObject() const noexcept override
            {
                return sizeof( Subclass );
            }

            const char*
            libraryName() const override
            {
                return Subclass::_s_libraryName();
            }

            const char*
            className() const override
            {
                return Subclass::_s_className();
            }
    };

}    // namespace osg
