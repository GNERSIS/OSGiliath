/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Ordered list of stacked transform elements. Composes
 * translate, rotate, and scale in sequence for bone transforms.
 */
#include <osgAnimation/transform/StackedTransform.hpp>

using namespace osgAnimation;

StackedTransform::StackedTransform()
{
}

StackedTransform::StackedTransform( const StackedTransform& rhs,
                                    const osg::CopyOp&      co ) :
    osg::MixinVector<osg::ref_ptr<StackedTransformElement>>()
{
    reserve( rhs.size() );
    for( StackedTransform::const_iterator it = rhs.begin(); it != rhs.end(); ++it )
    {
        const StackedTransformElement* element = it->get();
        if( element )
        {
            push_back( osg::clone( element, co ) );
        }
    }
}

void
StackedTransform::update( float t )
{
    bool dirty = false;
    for( StackedTransform::iterator it = begin(); it != end(); ++it )
    {
        StackedTransformElement* element = it->get();
        if( !element )
        {
            continue;
        }
        // update and check if there are changes
        element->update( t );
        dirty = true;
    }

    if( !dirty )
    {
        return;
    }

    // dirty update matrix
    _matrix = osg::dmat4();
    for( StackedTransform::iterator it = begin(); it != end(); ++it )
    {
        StackedTransformElement* element = it->get();
        if( !element || element->isIdentity() )
        {
            continue;
        }
        element->applyToMatrix( _matrix );
    }
}

const osg::dmat4&
StackedTransform::getMatrix() const
{
    return _matrix;
}
