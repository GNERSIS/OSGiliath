/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Root node for shadow-enabled scenes. Attaches a ShadowTechnique
 * that adds shadow mapping passes to the rendering pipeline.
 */
#include <osgShadow/ShadowedScene>

#include <osg/core/io_utils.hpp>
#include <osg/nodes/CoordinateSystemNode.hpp>
#include <osg/textures/Texture2D.hpp>
#include <osgUtil/culling/CullVisitor.hpp>

using namespace osgShadow;

ShadowedScene::ShadowedScene( ShadowTechnique* st )
{
    setNumChildrenRequiringUpdateTraversal( 1 );

    setShadowSettings( new ShadowSettings );

    if( st )
    {
        setShadowTechnique( st );
    }
}

ShadowedScene::ShadowedScene( const ShadowedScene& ss,
                              const osg::CopyOp&   copyop ) :
    Inherit( ss,
             copyop )
{
    setNumChildrenRequiringUpdateTraversal( getNumChildrenRequiringUpdateTraversal() +
                                            1 );

    if( ss._shadowTechnique.valid() )
    {
        setShadowTechnique( dynamic_cast<osgShadow::ShadowTechnique*>(
            ss._shadowTechnique->clone( copyop )
        ) );
    }

    if( ss._shadowSettings )
    {
        setShadowSettings( ss._shadowSettings.get() );
    }
    else
    {
        setShadowSettings( new ShadowSettings );
    }
}

ShadowedScene::~ShadowedScene()
{
    setShadowTechnique( 0 );
}

void
ShadowedScene::traverse( osg::NodeVisitor& nv )
{
    if( _shadowTechnique.valid() )
    {
        _shadowTechnique->traverse( nv );
    }
    else
    {
        osg::Group::traverse( nv );
    }
}

void
ShadowedScene::setShadowSettings( ShadowSettings* ss )
{
    _shadowSettings = ss;
}

void
ShadowedScene::setShadowTechnique( ShadowTechnique* technique )
{
    if( _shadowTechnique == technique )
    {
        return;
    }

    if( _shadowTechnique.valid() )
    {
        _shadowTechnique->cleanSceneGraph();
        _shadowTechnique->setShadowedScene( 0 );
    }

    _shadowTechnique = technique;

    if( _shadowTechnique.valid() )
    {
        _shadowTechnique->setShadowedScene( this );
        _shadowTechnique->dirty();
    }
}

void
ShadowedScene::cleanSceneGraph()
{
    if( _shadowTechnique.valid() )
    {
        _shadowTechnique->cleanSceneGraph();
    }
}

void
ShadowedScene::dirty()
{
    if( _shadowTechnique.valid() )
    {
        _shadowTechnique->dirty();
    }
}

void
ShadowedScene::resizeGLObjectBuffers( unsigned int maxSize )
{
    if( _shadowTechnique.valid() )
    {
        _shadowTechnique->resizeGLObjectBuffers( maxSize );
    }
    Group::resizeGLObjectBuffers( maxSize );
}

void
ShadowedScene::releaseGLObjects( osg::State* state ) const
{
    if( _shadowTechnique.valid() )
    {
        _shadowTechnique->releaseGLObjects( state );
    }
    Group::releaseGLObjects( state );
}
