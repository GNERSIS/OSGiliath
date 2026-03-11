/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Traversal visitor that collects occluder nodes from the scene.
 * Gathers ConvexPlanarOccluders for software occlusion culling.
 */
#include <osg/traversal/CollectOccludersVisitor.hpp>

#include <algorithm>
#include <osg/maths/compat.hpp>
#include <osg/maths/Math.hpp>
#include <osg/nodes/LOD.hpp>
#include <osg/nodes/OccluderNode.hpp>
#include <osg/nodes/Projection.hpp>
#include <osg/nodes/Switch.hpp>
#include <osg/nodes/Transform.hpp>

using namespace osg;

CollectOccludersVisitor::CollectOccludersVisitor() :
    DualModeVisitor( COLLECT_OCCLUDER_VISITOR,
                     TRAVERSE_ACTIVE_CHILDREN )
{

    setCullingMode( VIEW_FRUSTUM_CULLING |
                    NEAR_PLANE_CULLING |
                    FAR_PLANE_CULLING |
                    SMALL_FEATURE_CULLING );

    _minimumShadowOccluderVolume    = 0.005F;
    _maximumNumberOfActiveOccluders = 10;
    _createDrawables                = false;
}

CollectOccludersVisitor::~CollectOccludersVisitor()
{
}

void
CollectOccludersVisitor::reset()
{
    CullStack::reset();
    _occluderSet.clear();
}

float
CollectOccludersVisitor::getDistanceToEyePoint( const vec3& pos,
                                                bool        withLODScale ) const
{
    if( withLODScale )
    {
        return osg::length( pos - getEyeLocal() ) * getLODScale();
    }
    else
    {
        return osg::length( pos - getEyeLocal() );
    }
}

float
CollectOccludersVisitor::getDistanceToViewPoint( const vec3& pos,
                                                 bool        withLODScale ) const
{
    if( withLODScale )
    {
        return osg::length( pos - getViewPointLocal() ) * getLODScale();
    }
    else
    {
        return osg::length( pos - getViewPointLocal() );
    }
}

float
CollectOccludersVisitor::getDistanceFromEyePoint( const vec3& pos,
                                                  bool        withLODScale ) const
{
    const dmat4& matrix = *_modelviewStack.back();
    float        dist   = static_cast<float>( -( pos[0] *
                                                 matrix( 2, 0 ) +
                                                 pos[1] *
                                                 matrix( 2, 1 ) +
                                                 pos[2] *
                                                 matrix( 2, 2 ) +
                                                 matrix( 2, 3 ) ) );

    return withLODScale ? dist * getLODScale() : dist;
}

void
CollectOccludersVisitor::apply( osg::Node& node )
{
    if( isCulled( node ) )
    {
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    handle_cull_callbacks_and_traverse( node );

    // pop the culling mode.
    popCurrentMask();
}

void
CollectOccludersVisitor::apply( osg::Transform& node )
{
    if( isCulled( node ) )
    {
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    ref_ptr<osg::RefMatrix> matrix = createOrReuseMatrix( *getModelViewMatrix() );
    node.computeLocalToWorldMatrix( *matrix, this );
    pushModelViewMatrix( matrix.get(), node.getReferenceFrame() );

    handle_cull_callbacks_and_traverse( node );

    popModelViewMatrix();

    // pop the culling mode.
    popCurrentMask();
}

void
CollectOccludersVisitor::apply( osg::Projection& node )
{
    if( isCulled( node ) )
    {
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    ref_ptr<osg::RefMatrix> matrix = createOrReuseMatrix( node.getMatrix() );
    pushProjectionMatrix( matrix.get() );

    handle_cull_callbacks_and_traverse( node );

    popProjectionMatrix();

    // pop the culling mode.
    popCurrentMask();
}

void
CollectOccludersVisitor::apply( osg::Switch& node )
{
    apply( ( Group& )node );
}

void
CollectOccludersVisitor::apply( osg::LOD& node )
{
    if( isCulled( node ) )
    {
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    handle_cull_callbacks_and_traverse( node );

    // pop the culling mode.
    popCurrentMask();
}

void
CollectOccludersVisitor::apply( osg::OccluderNode& node )
{
    // need to check if occlusion node is in the occluder
    // list, if so disable the appropriate ShadowOccluderVolume
    disableAndPushOccludersCurrentMask( _nodePath );

    if( isCulled( node ) )
    {
        popOccludersCurrentMask( _nodePath );
        return;
    }

    // std::cout<<"CollectOccludersVisitor:: We have found an Occlusion node in
    // frustum"<<&node<<std::endl;

    // push the culling mode.
    pushCurrentMask();

    if( node.getOccluder() )
    {
        // computeOccluder will check if the occluder is the view frustum,
        // if it isn't then the it will return false, when in it will
        // clip the occluder's polygons in clip space, then create occluder
        // planes, all with their normals facing inward towards the volume,
        // and then transform them back into projection space.
        ShadowVolumeOccluder svo;
        if( svo.computeOccluder( _nodePath,
                                 *node.getOccluder(),
                                 *this,
                                 _createDrawables ) )
        {

            if( svo.getVolume() > _minimumShadowOccluderVolume )
            {
                // need to test occluder against view frustum.
                // std::cout << "    adding in Occluder"<<std::endl;
                _occluderSet.insert( svo );
            }
            else
            {
                // std::cout << "    rejecting Occluder as its volume is too small
                // "<<svo.getVolume()<<std::endl;
            }
        }
    }

    handle_cull_callbacks_and_traverse( node );

    // pop the culling mode.
    popCurrentMask();

    // pop the current mask for the disabled occluder
    popOccludersCurrentMask( _nodePath );
}

void
CollectOccludersVisitor::removeOccludedOccluders()
{
    if( _occluderSet.empty() )
    {
        return;
    }

    ShadowVolumeOccluderSet::iterator occludeeItr = _occluderSet.begin();

    // skip the first element as this can't be occluded by anything else.
    occludeeItr++;

    // step through the rest of the occluders, remove occluders which are themselves
    // occluded.
    for( ; occludeeItr != _occluderSet.end(); ++occludeeItr )
    {

        // search for any occluders that occlude the current occluder,
        // we only need to test any occluder near the front of the set since
        // you can't be occluder by something smaller than you.
        ShadowVolumeOccluder& occludee =
            const_cast<ShadowVolumeOccluder&>( *occludeeItr );
        ShadowVolumeOccluder::HoleList& holeList = occludee.getHoleList();

        for( ShadowVolumeOccluderSet::iterator occluderItr = _occluderSet.begin();
             occluderItr != occludeeItr;
             ++occluderItr )
        {
            // cast away constness of the std::set element since
            // ShadowVolumeOccluder::contains() is non const, and the std::set is a
            // const, just for the invariance of the operator <!! Ahhhhh. oh well the
            // below should be robust since contains won't change the getVolume which is
            // used by the operator <.  Honest,  :-)
            ShadowVolumeOccluder* occluder =
                const_cast<ShadowVolumeOccluder*>( &( *occluderItr ) );
            if( occluder->contains( occludee.getOccluder().getReferenceVertexList() ) )
            {
                // erase occluder from set.
                // take a copy of the iterator then rewind it one element so to prevent
                // invalidating the occludeeItr.
                ShadowVolumeOccluderSet::iterator eraseItr = occludeeItr--;
                _occluderSet.erase( eraseItr );
                break;
            }

            // now check all the holes in the occludee against the occluder, and remove
            // the ones that won't be valid
            unsigned int previous_valid_hole_i = 0;
            for( unsigned int i = 0; i < holeList.size(); ++i )
            {
                if( !occluder->contains( holeList[i].getReferenceVertexList() ) )
                {
                    if( previous_valid_hole_i < i )
                    {
                        // copy valid holes into gaps left by invalid ones
                        holeList[previous_valid_hole_i] = holeList[i];
                    }

                    previous_valid_hole_i++;
                }
            }

            // remove the tail of the holeList if holes have been removed.
            if( previous_valid_hole_i < holeList.size() )
            {
                holeList.erase( holeList.begin() + previous_valid_hole_i,
                                holeList.end() );
            }
        }
    }

    if( _occluderSet.size() <= _maximumNumberOfActiveOccluders )
    {
        return;
    }

    // move the iterator to the _maximumNumberOfActiveOccluders th occluder.
    occludeeItr = _occluderSet.begin();
    for( unsigned int i = 0; i < _maximumNumberOfActiveOccluders; ++i )
    {
        ++occludeeItr;
    }

    // discard last occluders.
    _occluderSet.erase( occludeeItr, _occluderSet.end() );
}
