/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for shadow rendering techniques.
 * Subclasses implement shadow map generation and projection.
 */
#include <osgShadow/ShadowTechnique.hpp>

#include <osg/core/io_utils.hpp>
#include <osg/core/Notify.hpp>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osgShadow/ShadowedScene.hpp>

using namespace osgShadow;

ShadowTechnique::CameraCullCallback::CameraCullCallback( ShadowTechnique* st ) :
    _shadowTechnique( st )
{
}

void
ShadowTechnique::CameraCullCallback::operator()( osg::Node*,
                                                 osg::NodeVisitor* nv )
{
    if( _shadowTechnique->getShadowedScene() )
    {
        _shadowTechnique->getShadowedScene()->osg::Group::traverse( *nv );
    }
}

ShadowTechnique::ShadowTechnique() :
    _shadowedScene( 0 ),
    _dirty( true )
{
}

ShadowTechnique::ShadowTechnique( const ShadowTechnique& copy,
                                  const osg::CopyOp&     copyop ) :
    osg::Object( copy,
                 copyop ),
    _shadowedScene( 0 ),
    _dirty( true )
{
}

ShadowTechnique::~ShadowTechnique()
{
}

void
ShadowTechnique::setShadowedScene( ShadowedScene* ss )
{
    _shadowedScene = ss;
}

void
ShadowTechnique::init()
{
    OSG_NOTICE << className() << "::init() not implemented yet" << std::endl;

    _dirty = false;
}

void
ShadowTechnique::update( osg::NodeVisitor& nv )
{
    OSG_NOTICE << className() << "::update(osg::NodeVisitor&) not implemented yet."
               << std::endl;
    _shadowedScene->osg::Group::traverse( nv );
}

void
ShadowTechnique::cull( osgUtil::CullVisitor& cv )
{
    OSG_NOTICE << className() << "::cull(osgUtl::CullVisitor&) not implemented yet."
               << std::endl;
    _shadowedScene->osg::Group::traverse( cv );
}

void
ShadowTechnique::cleanSceneGraph()
{
    OSG_NOTICE << className() << "::cleanSceneGraph()) not implemented yet."
               << std::endl;
}

void
ShadowTechnique::traverse( osg::NodeVisitor& nv )
{
    if( !_shadowedScene )
    {
        return;
    }

    if( nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR )
    {
        if( _dirty )
        {
            init();
        }

        update( nv );
    }
    else if( nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR )
    {
        osgUtil::CullVisitor* cv = nv.asCullVisitor();
        if( cv )
        {
            cull( *cv );
        }
        else
        {
            _shadowedScene->osg::Group::traverse( nv );
        }
    }
    else
    {
        _shadowedScene->osg::Group::traverse( nv );
    }
}

osg::vec3
ShadowTechnique::computeOrthogonalVector( const osg::vec3& direction ) const
{
    float     length           = osg::length( direction );
    osg::vec3 orthogonalVector = osg::cross( direction, osg::vec3( 0.0F, 1.0F, 0.0F ) );
    float     ortho_len        = osg::length( orthogonalVector );
    orthogonalVector           = osg::normalize( orthogonalVector );
    if( ortho_len < length * 0.5F )
    {
        orthogonalVector = osg::cross( direction, osg::vec3( 0.0F, 0.0F, 1.0F ) );
        orthogonalVector = osg::normalize( orthogonalVector );
    }
    return orthogonalVector;
}
