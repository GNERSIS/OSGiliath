/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Shadow volume occluder for software occlusion culling.
 * Projects an occluder polyhedron to test scene node visibility.
 */
#pragma once

#include <osg/geometry/ConvexPlanarOccluder.hpp>
#include <osg/maths/RefMatrix.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/traversal/Polytope.hpp>

namespace osg
{

    class CullStack;

    /** ShadowVolumeOccluder is a helper class for implementing shadow occlusion culling.
     */
    class OSG_EXPORT ShadowVolumeOccluder
    {

        public:

            typedef std::vector<Polytope> HoleList;

            ShadowVolumeOccluder( const ShadowVolumeOccluder& svo ) :
                _volume( svo._volume ),
                _nodePath( svo._nodePath ),
                _projectionMatrix( svo._projectionMatrix ),
                _occluderVolume( svo._occluderVolume ),
                _holeList( svo._holeList )
            {
            }

            ShadowVolumeOccluder&
            operator=( const ShadowVolumeOccluder& ) = default;

            ShadowVolumeOccluder() :
                _volume( 0.0F )
            {
            }

            bool
            operator<( const ShadowVolumeOccluder& svo ) const
            {
                return getVolume() > svo.getVolume();
            }    // not greater volume first.

            /** compute the shadow volume occluder. */
            bool
            computeOccluder( const NodePath&             nodePath,
                             const ConvexPlanarOccluder& occluder,
                             CullStack&                  cullStack,
                             bool                        createDrawables = false );

            inline void
            disableResultMasks();

            inline void
            pushCurrentMask();
            inline void
            popCurrentMask();

            /** return true if the matrix passed in matches the projection matrix that
             * this ShadowVolumeOccluder is associated with.*/
            bool
            matchProjectionMatrix( const osg::dmat4& matrix ) const
            {
                if( _projectionMatrix.valid() )
                {
                    return matrix == *_projectionMatrix;
                }
                else
                {
                    return false;
                }
            }

            /** Set the NodePath which describes which node in the scene graph
             * that this occluder is attached to. */
            inline void
            setNodePath( NodePath& nodePath )
            {
                _nodePath = nodePath;
            }

            inline NodePath&
            getNodePath()
            {
                return _nodePath;
            }

            inline const NodePath&
            getNodePath() const
            {
                return _nodePath;
            }

            /** get the volume of the occluder minus its holes, in eye coords, the volume
             * is normalized by dividing by the volume of the view frustum in eye
             * coords.*/
            float
            getVolume() const
            {
                return _volume;
            }

            /** return the occluder polytope.*/
            Polytope&
            getOccluder()
            {
                return _occluderVolume;
            }

            /** return the const occluder polytope.*/
            const Polytope&
            getOccluder() const
            {
                return _occluderVolume;
            }

            /** return the list of holes.*/
            HoleList&
            getHoleList()
            {
                return _holeList;
            }

            /** return the const list of holes.*/
            const HoleList&
            getHoleList() const
            {
                return _holeList;
            }

            /** return true if the specified vertex list is contained entirely
             * within this shadow occluder volume.*/
            bool
            contains( const std::vector<vec3>& vertices );

            /** return true if the specified bounding sphere is contained entirely
             * within this shadow occluder volume.*/
            bool
            contains( const sphere& bound );

            /** return true if the specified bounding box is contained entirely
             * within this shadow occluder volume.*/
            bool
            contains( const box& bound );

            inline void
            transformProvidingInverse( const osg::dmat4& matrix )
            {
                _occluderVolume.transformProvidingInverse( matrix );
                for( HoleList::iterator itr = _holeList.begin(); itr != _holeList.end();
                     ++itr )
                {
                    itr->transformProvidingInverse( matrix );
                }
            }

        protected:

            float                    _volume;
            NodePath                 _nodePath;
            ref_ptr<const RefMatrix> _projectionMatrix;
            Polytope                 _occluderVolume;
            HoleList                 _holeList;
    };

    /** A list of ShadowVolumeOccluder, used by CollectOccluderVisitor and
     * CullVistor's.*/
    typedef std::vector<ShadowVolumeOccluder> ShadowVolumeOccluderList;

    inline void
    ShadowVolumeOccluder::disableResultMasks()
    {
        // std::cout<<"ShadowVolumeOccluder::disableResultMasks() -
        // _occluderVolume.getMaskStack().size()="<<_occluderVolume.getMaskStack().size()<<"
        // "<<_occluderVolume.getCurrentMask()<<std::endl;
        _occluderVolume.setResultMask( 0 );
        for( HoleList::iterator itr = _holeList.begin(); itr != _holeList.end(); ++itr )
        {
            itr->setResultMask( 0 );
        }
    }

    inline void
    ShadowVolumeOccluder::pushCurrentMask()
    {
        // std::cout<<"ShadowVolumeOccluder::pushCurrentMasks() -
        // _occluderVolume.getMaskStack().size()="<<_occluderVolume.getMaskStack().size()<<"
        // "<<_occluderVolume.getCurrentMask()<<std::endl;
        _occluderVolume.pushCurrentMask();
        if( !_holeList.empty() )
        {
            for( HoleList::iterator itr = _holeList.begin(); itr != _holeList.end();
                 ++itr )
            {
                itr->pushCurrentMask();
            }
        }
    }

    inline void
    ShadowVolumeOccluder::popCurrentMask()
    {
        _occluderVolume.popCurrentMask();
        if( !_holeList.empty() )
        {
            for( HoleList::iterator itr = _holeList.begin(); itr != _holeList.end();
                 ++itr )
            {
                itr->popCurrentMask();
            }
        }
        // std::cout<<"ShadowVolumeOccluder::popCurrentMasks() -
        // _occluderVolume.getMaskStack().size()="<<_occluderVolume.getMaskStack().size()<<"
        // "<<_occluderVolume.getCurrentMask()<<std::endl;
    }

}    // end of namespace
