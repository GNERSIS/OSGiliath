/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract base for view-dependent shadow techniques.
 * Provides per-camera shadow setup and clean-up hooks.
 */
#include <osgShadow/ViewDependentShadowTechnique>

#include <osgShadow/ShadowedScene>

using namespace osgShadow;

ViewDependentShadowTechnique::ViewDependentShadowTechnique()
{
    dirty();
}

ViewDependentShadowTechnique::ViewDependentShadowTechnique(
    const ViewDependentShadowTechnique& copy,
    const osg::CopyOp&                  copyop
) :
    Inherit( copy,
             copyop )
{
    dirty();
}

ViewDependentShadowTechnique::~ViewDependentShadowTechnique( void )
{
}

void
ViewDependentShadowTechnique::resizeGLObjectBuffers( unsigned int maxSize )
{
    for( ViewDataMap::iterator itr = _viewDataMap.begin(); itr != _viewDataMap.end();
         ++itr )
    {
        itr->second->resizeGLObjectBuffers( maxSize );
    }
}

void
ViewDependentShadowTechnique::releaseGLObjects( osg::State* state ) const
{
    for( ViewDataMap::const_iterator itr = _viewDataMap.begin();
         itr != _viewDataMap.end();
         ++itr )
    {
        itr->second->releaseGLObjects( state );
    }
}

void
ViewDependentShadowTechnique::traverse( osg::NodeVisitor& nv )
{
    osgShadow::ShadowTechnique::traverse( nv );
}

void
ViewDependentShadowTechnique::dirty()
{
    std::lock_guard<std::mutex> lock( _viewDataMapMutex );

    osgShadow::ShadowTechnique::_dirty = true;

    for( ViewDataMap::iterator mitr = _viewDataMap.begin(); mitr != _viewDataMap.end();
         ++mitr )
    {
        mitr->second->dirty( true );
    }
}

void
ViewDependentShadowTechnique::init()
{
    // osgShadow::ShadowTechnique::init( );
    osgShadow::ShadowTechnique::_dirty = false;
}

void
ViewDependentShadowTechnique::update( osg::NodeVisitor& nv )
{
    // osgShadow::ShadowTechnique::update( nv );
    osgShadow::ShadowTechnique::_shadowedScene->osg::Group::traverse( nv );
}

void
ViewDependentShadowTechnique::cull( osgUtil::CullVisitor& cv )
{
    // osgShadow::ShadowTechnique::cull( cv );

    ViewData* vd = getViewDependentData( &cv );

    if( !vd || vd->_dirty || vd->_cv != &cv || vd->_st != this )
    {
        vd = initViewDependentData( &cv, vd );
        setViewDependentData( &cv, vd );
    }

    if( vd )
    {
        std::lock_guard<std::mutex> lock( vd->_mutex );
        vd->cull();
    }
    else
    {
        osgShadow::ShadowTechnique::_shadowedScene->osg::Group::traverse( cv );
    }
}

void
ViewDependentShadowTechnique::cleanSceneGraph()
{
    // osgShadow::ShadowTechnique::cleanSceneGraph( );
}

ViewDependentShadowTechnique::ViewData*
ViewDependentShadowTechnique::getViewDependentData( osgUtil::CullVisitor* cv )
{
    std::lock_guard<std::mutex> lock( _viewDataMapMutex );
    return _viewDataMap[osg::Identifier::get( cv )].get();
}

void
ViewDependentShadowTechnique::setViewDependentData( osgUtil::CullVisitor* cv,
                                                    ViewData*             data )
{
    std::lock_guard<std::mutex> lock( _viewDataMapMutex );
    _viewDataMap[osg::Identifier::get( cv )] = data;
}

void
ViewDependentShadowTechnique::ViewData::dirty( bool flag )
{
    std::lock_guard<std::mutex> lock( _mutex );
    _dirty = flag;
}

void
ViewDependentShadowTechnique::ViewData::init( ViewDependentShadowTechnique* st,
                                              osgUtil::CullVisitor*         cv )
{
    _cv = cv;
    _st = st;
    dirty( false );
}

void
ViewDependentShadowTechnique::ViewData::cull( void )
{
}
