/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Skinned mesh geometry deformed by a Skeleton. Applies bone
 * weights to vertices via software or hardware skinning.
 */
#include <osgAnimation/skeletal/RigGeometry.hpp>

#include <osgAnimation/skeletal/RigTransformSoftware.hpp>
#include <osgAnimation/skeletal/VertexInfluence.hpp>
#include <sstream>

using namespace osgAnimation;

// The idea is to compute a bounding box with a factor x of the first step we compute the
// bounding box
osg::box
RigComputeBoundingBoxCallback::computeBound( const osg::Drawable& drawable ) const
{
    const osgAnimation::RigGeometry& rig =
        dynamic_cast<const osgAnimation::RigGeometry&>( drawable );

    // if a valid initial bounding box is set we use it without asking more
    if( rig.getInitialBound().valid() )
    {
        return rig.getInitialBound();
    }

    if( _computed )
    {
        return _boundingBox;
    }

    // if the computing of bb is invalid (like no geometry inside)
    // then don't tag the bounding box as computed
    osg::box bb = rig.computeBoundingBox();
    if( !bb.valid() )
    {
        return bb;
    }

    _boundingBox.expandBy( bb );
    osg::vec3 center = _boundingBox.center();
    osg::vec3 vec    = ( _boundingBox.max - center ) * static_cast<float>( _factor );
    _boundingBox.expandBy( center + vec );
    _boundingBox.expandBy( center - vec );
    _computed = true;
    // OSG_NOTICE << "build the bounding box for RigGeometry " << rig.getName() << " " <<
    // _boundingBox._min << " " << _boundingBox._max << std::endl;
    return _boundingBox;
}

RigGeometry::RigGeometry()
{
    setUseVertexBufferObjects( true );
    setUpdateCallback( new UpdateRigGeometry );
    setDataVariance( osg::Object::DataVariance::DYNAMIC );
    _needToComputeMatrix          = true;
    _matrixFromSkeletonToGeometry = _invMatrixFromSkeletonToGeometry = osg::dmat4();
    // disable the computation of boundingbox for the rig mesh
    setComputeBoundingBoxCallback( new RigComputeBoundingBoxCallback() );
    _rigTransformImplementation = new osgAnimation::RigTransformSoftware;
}

RigGeometry::RigGeometry( const RigGeometry& b,
                          const osg::CopyOp& copyop ) :
    Inherit( b,
             copyop ),
    _geometry( b._geometry ),
    _rigTransformImplementation( osg::clone( b._rigTransformImplementation.get(),
                                             copyop ) ),
    _vertexInfluenceMap( b._vertexInfluenceMap ),
    _needToComputeMatrix( b._needToComputeMatrix )
{
    _needToComputeMatrix          = true;
    _matrixFromSkeletonToGeometry = _invMatrixFromSkeletonToGeometry = osg::dmat4();
    // disable the computation of boundingbox for the rig mesh

    setComputeBoundingBoxCallback( new RigComputeBoundingBoxCallback() );
    // we don't copy the RigImplementation yet. because the RigImplementation need to be
    // initialized in a valid graph, with a skeleton ... don't know yet what to do with a
    // clone of a RigGeometry
}

const osg::dmat4&
RigGeometry::getMatrixFromSkeletonToGeometry() const
{
    return _matrixFromSkeletonToGeometry;
}

const osg::dmat4&
RigGeometry::getInvMatrixFromSkeletonToGeometry() const
{
    return _invMatrixFromSkeletonToGeometry;
}

void
RigGeometry::computeMatrixFromRootSkeleton()
{
    if( !_root.valid() )
    {
        OSG_WARN
            << "Warning " << className()
            << "::computeMatrixFromRootSkeleton if you have this message it means you "
               "miss to call buildTransformer(Skeleton* root), or your RigGeometry ("
            << getName() << ") is not attached to a Skeleton subgraph" << std::endl;
        return;
    }
    osg::MatrixList mtxList          = getParent( 0 )->getWorldMatrices( _root.get() );
    osg::dmat4      notRoot          = _root->getMatrix();
    _matrixFromSkeletonToGeometry    = mtxList[0] * osg::inverse( notRoot );
    _invMatrixFromSkeletonToGeometry = osg::inverse( _matrixFromSkeletonToGeometry );
    _needToComputeMatrix             = false;
}

void
RigGeometry::update()
{
    RigTransform& implementation = *_rigTransformImplementation;
    ( implementation )( *this );
}

void
RigGeometry::copyFrom( osg::Geometry& from )
{
    if( this == &from )
    {
        return;
    }

    osg::Geometry& target = *this;

    target.setStateSet( from.getStateSet() );

    // copy over primitive sets.
    target.getPrimitiveSetList() = from.getPrimitiveSetList();

    if( from.getVertexArray() )
    {
        target.setVertexArray( from.getVertexArray() );
    }

    if( from.getNormalArray() )
    {
        target.setNormalArray( from.getNormalArray() );
    }

    if( from.getColorArray() )
    {
        target.setColorArray( from.getColorArray() );
    }

    if( from.getSecondaryColorArray() )
    {
        target.setSecondaryColorArray( from.getSecondaryColorArray() );
    }

    if( from.getFogCoordArray() )
    {
        target.setFogCoordArray( from.getFogCoordArray() );
    }

    for( unsigned int ti = 0; ti < from.getNumTexCoordArrays(); ++ti )
    {
        if( from.getTexCoordArray( ti ) )
        {
            target.setTexCoordArray( ti, from.getTexCoordArray( ti ) );
        }
    }

    osg::Geometry::ArrayList& arrayList = from.getVertexAttribArrayList();
    for( unsigned int vi = 0; vi < arrayList.size(); ++vi )
    {
        osg::Array* array = arrayList[vi].get();
        if( array )
        {
            target.setVertexAttribArray( vi, array );
        }
    }
}
