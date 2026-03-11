/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Out-of-line definitions for InheritAccept<Parent, Sub, true>.
 * Must be included AFTER NodeVisitor is fully defined.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/traversal/NodeVisitor.hpp>

namespace osg
{

    template<class Parent,
             class Sub>
    void
    InheritAccept<Parent,
                  Sub,
                  true>::accept( osg::NodeVisitor& nv )
    {
        if( nv.validNodeMask( *this ) )
        {
            nv.pushOntoNodePath( this );
            nv.apply( static_cast<Sub&>( *this ) );
            nv.popFromNodePath();
        }
    }

    template<class Parent,
             class Sub>
    void
    InheritAccept<Parent,
                  Sub,
                  true>::accept( osg::ConstNodeVisitor& nv ) const
    {
        if( nv.validNodeMask( *this ) )
        {
            nv.pushOntoNodePath( this );
            nv.apply( static_cast<const Sub&>( *this ) );
            nv.popFromNodePath();
        }
    }

}    // namespace osg
