#include "OrientationConverter.hpp"

#include <osg/maths/compat.hpp>
#include <osg/nodes/MatrixTransform.hpp>
#include <osgUtil/optimization/Optimizer.hpp>
#include <stdio.h>

using namespace osg;

OrientationConverter::OrientationConverter( void )
{
    R                = osg::dmat4();
    T                = osg::dmat4();
    _trans_set       = false;
    _use_world_frame = false;
    S                = osg::dmat4();
}

void
OrientationConverter::setRotation( const osg::vec3& from,
                                   const osg::vec3& to )
{
    R = osg::rotate( osg::dvec3( from ), osg::dvec3( to ) );
}

void
OrientationConverter::setRotation( float            degrees,
                                   const osg::vec3& axis )
{
    R = osg::rotate( osg::radians( ( double )degrees ), osg::dvec3( axis ) );
}

void
OrientationConverter::setTranslation( const osg::vec3& trans )
{
    T          = osg::translate( osg::dvec3( trans ) );
    _trans_set = true;
}

void
OrientationConverter::setScale( const osg::vec3& scale )
{
    S = osg::scale( osg::dvec3( scale ) );
}

void
OrientationConverter::useWorldFrame( bool worldFrame )
{
    _use_world_frame = worldFrame;
}

Node*
OrientationConverter::convert( Node* node )
{
    // Order of operations here is :
    // 1. If world frame option not set, translate to world origin (0,0,0)
    // 2. Rotate to new orientation
    // 3. Scale in new orientation coordinates
    // 4. If an absolute translation was specified then
    //        - translate to absolute translation in world coordinates
    //    else if world frame option not set,
    //        - translate back to model's original origin.
    osg::sphere bs = node->getBound();
    osg::dmat4  C;

    if( _use_world_frame )
    {
        C = osg::dmat4();
    }
    else
    {
        C = osg::translate( osg::dvec3( -bs.center ) );

        if( _trans_set == false )
        {
            T = osg::translate( osg::dvec3( bs.center ) );
        }
    }

    osg::Group*           root      = new osg::Group;
    osg::MatrixTransform* transform = new osg::MatrixTransform;

    transform->setDataVariance( osg::Object::DataVariance::STATIC );
    transform->setMatrix( C * R * S * T );

    root->addChild( transform );
    transform->addChild( node );

    osgUtil::Optimizer::FlattenStaticTransformsVisitor fstv;
    root->accept( fstv );
    fstv.removeTransforms( root );

    return root->getChild( 0 );
}
