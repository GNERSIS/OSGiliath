/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Copy operation flags controlling deep vs. shallow cloning.
 * Used with Object::clone() for scene graph duplication.
 */
#include <osg/core/CopyOp.hpp>

#include <osg/core/Callback.hpp>
#include <osg/geometry/Array.hpp>
#include <osg/geometry/Drawable.hpp>
#include <osg/geometry/PrimitiveSet.hpp>
#include <osg/geometry/Shape.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/state/StateAttribute.hpp>
#include <osg/state/StateSet.hpp>
#include <osg/textures/Texture.hpp>

using namespace osg;

#define COPY_OP( TYPE, FLAG )                                       \
    TYPE* CopyOp::operator()( const TYPE* obj ) const               \
    {                                                               \
        if( obj && _flags & FLAG )                                  \
        {                                                           \
            if( _duplicateMap )                                     \
            {                                                       \
                auto it = _duplicateMap->find( obj );               \
                if( it != _duplicateMap->end() )                    \
                    return dynamic_cast<TYPE*>( it->second );       \
                TYPE* clone             = osg::clone( obj, *this ); \
                ( *_duplicateMap )[obj] = clone;                    \
                return clone;                                       \
            }                                                       \
            return osg::clone( obj, *this );                        \
        }                                                           \
        else                                                        \
            return const_cast<TYPE*>( obj );                        \
    }

COPY_OP( Object,
         DEEP_COPY_OBJECTS )
COPY_OP( StateSet,
         DEEP_COPY_STATESETS )
COPY_OP( Image,
         DEEP_COPY_IMAGES )
COPY_OP( UniformBase,
         DEEP_COPY_UNIFORMS )
COPY_OP( Uniform,
         DEEP_COPY_UNIFORMS )
COPY_OP( UniformCallback,
         DEEP_COPY_CALLBACKS )
COPY_OP( StateAttributeCallback,
         DEEP_COPY_CALLBACKS )
COPY_OP( Drawable,
         DEEP_COPY_DRAWABLES )
COPY_OP( Texture,
         DEEP_COPY_TEXTURES )
COPY_OP( Array,
         DEEP_COPY_ARRAYS )
COPY_OP( PrimitiveSet,
         DEEP_COPY_PRIMITIVES )
COPY_OP( Shape,
         DEEP_COPY_SHAPES )

Referenced*
CopyOp::operator()( const Referenced* ref ) const
{
    return const_cast<Referenced*>( ref );
}

Node*
CopyOp::operator()( const Node* node ) const
{
    if( !node )
    {
        return 0;
    }

    const Drawable* drawable = node->asDrawable();
    if( drawable )
    {
        return operator()( drawable );
    }
    else if( _flags & DEEP_COPY_NODES )
    {
        if( _duplicateMap )
        {
            auto it = _duplicateMap->find( node );
            if( it != _duplicateMap->end() )
            {
                return static_cast<Node*>( it->second );
            }
            Node* clone              = osg::clone( node, *this );
            ( *_duplicateMap )[node] = clone;
            return clone;
        }
        return osg::clone( node, *this );
    }
    else
    {
        return const_cast<Node*>( node );
    }
}

StateAttribute*
CopyOp::operator()( const StateAttribute* attr ) const
{
    if( attr && _flags & DEEP_COPY_STATEATTRIBUTES )
    {
        if( _duplicateMap )
        {
            auto it = _duplicateMap->find( attr );
            if( it != _duplicateMap->end() )
            {
                return static_cast<StateAttribute*>( it->second );
            }
        }
        const Texture*  textbase = dynamic_cast<const Texture*>( attr );
        StateAttribute* clone;
        if( textbase )
        {
            clone = operator()( textbase );
        }
        else
        {
            clone = osg::clone( attr, *this );
        }
        if( _duplicateMap && clone )
        {
            ( *_duplicateMap )[attr] = clone;
        }
        return clone;
    }
    else
    {
        return const_cast<StateAttribute*>( attr );
    }
}

Callback*
CopyOp::operator()( const Callback* nc ) const
{
    if( nc && _flags & DEEP_COPY_CALLBACKS )
    {
        // deep copy the full chain of callback
        Callback* first = osg::clone( nc, *this );
        if( !first )
        {
            return 0;
        }

        first->setNestedCallback( 0 );
        nc = nc->getNestedCallback();
        while( nc )
        {
            Callback* ucb = osg::clone( nc, *this );
            if( ucb )
            {
                ucb->setNestedCallback( 0 );
                first->addNestedCallback( ucb );
            }
            nc = nc->getNestedCallback();
        }
        return first;
    }
    else
    {
        return const_cast<Callback*>( nc );
    }
}
