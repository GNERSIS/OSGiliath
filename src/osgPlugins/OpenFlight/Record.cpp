/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: _numberOfReplications, dispose.
 */
//
// OpenFlight loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include "Document.hpp"
#include "Record.hpp"

#include <osg/nodes/MatrixTransform.hpp>
#include <osg/textures/Texture2D.hpp>
#include <stdexcept>

using namespace flt;

Record::Record()
{
}

Record::~Record()
{
}

void
Record::read( RecordInputStream& in,
              Document&          document )
{
    _parent = document.getCurrentPrimaryRecord();

    // Read record body.
    readRecord( in, document );
}

void
Record::readRecord( RecordInputStream& /*in*/,
                    Document& /*document*/ )
{
}

PrimaryRecord::PrimaryRecord() :
    _numberOfReplications( 0 )
{
}

void
PrimaryRecord::read( RecordInputStream& in,
                     Document&          document )
{
    PrimaryRecord* parentPrimary  = document.getTopOfLevelStack();
    PrimaryRecord* currentPrimary = document.getCurrentPrimaryRecord();

    // Finally call dispose() for primary without push, pop level pair.
    if( currentPrimary && currentPrimary != parentPrimary )
    {
        currentPrimary->dispose( document );
    }

    // Update current primary record.
    document.setCurrentPrimaryRecord( this );

    _parent = parentPrimary;

    // Read record body.
    readRecord( in, document );
}

///////////////////////////////////////////////////////////////////////////////////
// Helper methods

// Insert matrix-tranform(s)
//
// node: node to apply transform
// matrix: transformation matrix
// numberOfReplications: zero for regular transform, number of copies if replication is
// used.
void
flt::insertMatrixTransform( osg::Node&        node,
                            const osg::dmat4& matrix,
                            int               numberOfReplications )
{
    osg::ref_ptr<osg::Node> ref     = &node;
    osg::Node::ParentList   parents = node.getParents();

    // Start without transformation if replication.
    osg::dmat4 accumulatedMatrix = ( numberOfReplications > 0 ) ? osg::dmat4() : matrix;

    for( int n = 0; n <= numberOfReplications; n++ )
    {
        // Accumulate transformation for each replication.
        osg::ref_ptr<osg::MatrixTransform> transform =
            new osg::MatrixTransform( accumulatedMatrix );
        transform->setDataVariance( osg::Object::DataVariance::STATIC );

        // Add transform to parents
        for( osg::Node::ParentList::iterator itr = parents.begin(); itr != parents.end();
             ++itr )
        {
            ( *itr )->replaceChild( &node, transform.get() );
        }

        // Make primary a child of matrix transform.
        transform->addChild( &node );

        // Accumulate transform if multiple replications.
        accumulatedMatrix *= matrix;
    }
}

///////////////////////////////////////////////////////////////////////////////////

osg::Vec3Array*
flt::getOrCreateVertexArray( osg::Geometry& geometry )
{
    osg::Vec3Array* vertices =
        dynamic_cast<osg::Vec3Array*>( geometry.getVertexArray() );
    if( !vertices )
    {
        vertices = new osg::Vec3Array;
        geometry.setVertexArray( vertices );
    }
    return vertices;
}

osg::Vec3Array*
flt::getOrCreateNormalArray( osg::Geometry& geometry )
{
    osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>( geometry.getNormalArray() );
    if( !normals )
    {
        normals = new osg::Vec3Array;
        geometry.setNormalArray( normals );
    }
    return normals;
}

osg::Vec4Array*
flt::getOrCreateColorArray( osg::Geometry& geometry )
{
    osg::Vec4Array* colors = dynamic_cast<osg::Vec4Array*>( geometry.getColorArray() );
    if( !colors )
    {
        colors = new osg::Vec4Array;
        geometry.setColorArray( colors );
    }
    return colors;
}

osg::Vec2Array*
flt::getOrCreateTextureArray( osg::Geometry& geometry,
                              int            unit )
{
    osg::Vec2Array* UVs =
        dynamic_cast<osg::Vec2Array*>( geometry.getTexCoordArray( unit ) );
    if( !UVs )
    {
        UVs = new osg::Vec2Array;
        geometry.setTexCoordArray( unit, UVs );
    }
    return UVs;
}
