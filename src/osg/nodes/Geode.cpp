/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Leaf node containing Drawable objects for rendering.
 * Groups geometry, shapes, and text under a single scene graph node.
 */
#include <osg/nodes/Geode.hpp>

#include <osg/core/Notify.hpp>

using namespace osg;

Geode::Geode()
{
}

Geode::Geode( const Geode&  geode,
              const CopyOp& copyop ) :
    Inherit<Group,
            Geode>( geode,
                    copyop )
{
}

Geode::~Geode()
{
}

bool
Geode::addDrawable( Drawable* drawable )
{
    return addChild( drawable );
}

bool
Geode::removeDrawable( Drawable* drawable )
{
    return removeDrawables( getDrawableIndex( drawable ), 1 );
}

bool
Geode::removeDrawables( unsigned int pos,
                        unsigned int numDrawablesToRemove )
{
    return removeChildren( pos, numDrawablesToRemove );
}

bool
Geode::replaceDrawable( Drawable* origDrawable,
                        Drawable* newDrawable )
{
    return replaceChild( origDrawable, newDrawable );
}

bool
Geode::setDrawable( unsigned int i,
                    Drawable*    newDrawable )
{
    return setChild( i, newDrawable );
}

sphere
Geode::computeBound() const
{
    sphere bsphere;

    _bbox.init();

    for( NodeList::const_iterator itr = _children.begin(); itr != _children.end();
         ++itr )
    {
        if( itr->valid() )
        {
            const osg::Drawable* drawable = ( *itr )->asDrawable();
            if( drawable )
            {
                _bbox.expandBy( drawable->getBoundingBox() );
            }
            else
            {
                _bbox.expandBy( ( *itr )->getBound() );
            }
        }
    }

    if( _bbox.valid() )
    {
        bsphere.expandBy( _bbox );
    }
    return bsphere;
}

void
Geode::compileDrawables( RenderInfo& renderInfo )
{
    for( NodeList::iterator itr = _children.begin(); itr != _children.end(); ++itr )
    {
        osg::Drawable* drawable = itr->valid() ? ( *itr )->asDrawable() : 0;
        if( drawable )
        {
            drawable->compileGLObjects( renderInfo );
        }
    }
}
