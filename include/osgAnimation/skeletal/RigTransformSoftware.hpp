/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * CPU-based skeletal skinning. Transforms vertices on the
 * CPU using bone matrices and vertex weights.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/observer_ptr.hpp>
#include <osg/maths/compat.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/skeletal/Bone.hpp>
#include <osgAnimation/skeletal/RigTransform.hpp>
#include <osgAnimation/skeletal/VertexInfluence.hpp>

namespace osgAnimation
{

    class RigGeometry;

    /// This class manage format for software skinning
    class OSGANIMATION_EXPORT RigTransformSoftware
        : public osg::Inherit<RigTransform, RigTransformSoftware>
    {
        public:

            RigTransformSoftware();
            RigTransformSoftware( const RigTransformSoftware& rts,
                                  const osg::CopyOp&          copyop );

            OSG_REGISTER_TYPE( osgAnimation,
                               RigTransformSoftware )
            virtual void
            operator()( RigGeometry& );
            // to call when a skeleton is reacheable from the rig to prepare technic data
            virtual bool
                                                   prepareData( RigGeometry& );

            typedef std::pair<unsigned int, float> LocalBoneIDWeight;

            class BonePtrWeight : LocalBoneIDWeight
            {
                public:

                    BonePtrWeight( unsigned int id,
                                   float        weight,
                                   Bone*        bone = 0 ) :
                        LocalBoneIDWeight( id,
                                           weight ),
                        _boneptr( bone )
                    {
                    }

                    BonePtrWeight( const BonePtrWeight& bw2 ) :
                        LocalBoneIDWeight( bw2.getBoneID(),
                                           bw2.getWeight() ),
                        _boneptr( bw2._boneptr.get() )
                    {
                    }

                    BonePtrWeight&
                    operator=( const BonePtrWeight& ) = default;

                    inline const float&
                    getWeight() const
                    {
                        return second;
                    }

                    inline void
                    setWeight( float b )
                    {
                        second = b;
                    }

                    inline const unsigned int&
                    getBoneID() const
                    {
                        return first;
                    }

                    inline void
                    setBoneID( unsigned int b )
                    {
                        first = b;
                    }

                    inline bool
                    operator<( const BonePtrWeight& b1 ) const
                    {
                        if( second > b1.second )
                        {
                            return true;
                        }
                        if( second < b1.second )
                        {
                            return false;
                        }
                        return ( first > b1.first );
                    }

                    /// set Bone pointer
                    inline const Bone*
                    getBonePtr() const
                    {
                        return _boneptr.get();
                    }

                    inline void
                    setBonePtr( Bone* b )
                    {
                        _boneptr = b;
                    }

                protected:

                    osg::observer_ptr<Bone> _boneptr;
            };

            typedef std::vector<BonePtrWeight> BonePtrWeightList;

            /// map a set of boneinfluence to a list of vertex indices sharing this set
            class VertexGroup
            {
                public:

                    inline BonePtrWeightList&
                    getBoneWeights()
                    {
                        return _boneweights;
                    }

                    inline IndexList&
                    getVertices()
                    {
                        return _vertexes;
                    }

                    inline void
                    resetMatrix()
                    {
                        _result.set( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 );
                    }

                    inline void
                    accummulateMatrix( const osg::dmat4&      invBindMatrix,
                                       const osg::dmat4&      matrix,
                                       osg::dmat4::value_type weight )
                    {
                        osg::dmat4              m          = invBindMatrix * matrix;
                        osg::dmat4::value_type* ptr        = m.data();
                        osg::dmat4::value_type* ptrresult  = _result.data();
                        ptrresult[0]                      += ptr[0] * weight;
                        ptrresult[1]                      += ptr[1] * weight;
                        ptrresult[2]                      += ptr[2] * weight;

                        ptrresult[4]                      += ptr[4] * weight;
                        ptrresult[5]                      += ptr[5] * weight;
                        ptrresult[6]                      += ptr[6] * weight;

                        ptrresult[8]                      += ptr[8] * weight;
                        ptrresult[9]                      += ptr[9] * weight;
                        ptrresult[10]                     += ptr[10] * weight;

                        ptrresult[12]                     += ptr[12] * weight;
                        ptrresult[13]                     += ptr[13] * weight;
                        ptrresult[14]                     += ptr[14] * weight;
                    }

                    inline void
                    computeMatrixForVertexSet()
                    {
                        if( _boneweights.empty() )
                        {
                            osg::notify( osg::WARN )
                                << this
                                << " RigTransformSoftware::VertexGroup no bones found"
                                << std::endl;
                            _result = osg::dmat4();
                            return;
                        }
                        resetMatrix();

                        for( BonePtrWeightList::iterator bwit = _boneweights.begin();
                             bwit != _boneweights.end();
                             ++bwit )
                        {
                            const Bone* bone = bwit->getBonePtr();
                            if( !bone )
                            {
                                osg::notify(
                                    osg::WARN
                                ) << this
                                  << " RigTransformSoftware::computeMatrixForVertexSet "
                                     "Warning a bone is null, skip it"
                                  << std::endl;
                                continue;
                            }
                            const osg::dmat4& invBindMatrix =
                                bone->getInvBindMatrixInSkeletonSpace();
                            const osg::dmat4& matrix = bone->getMatrixInSkeletonSpace();
                            osg::dmat4::value_type w = bwit->getWeight();
                            accummulateMatrix( invBindMatrix, matrix, w );
                        }
                    }

                    void
                    normalize();

                    inline const osg::dmat4&
                    getMatrix() const
                    {
                        return _result;
                    }

                protected:

                    BonePtrWeightList _boneweights;
                    IndexList         _vertexes;
                    osg::dmat4        _result;
            };

            template<class V>
            inline void
            compute( const osg::dmat4& transform,
                     const osg::dmat4& invTransform,
                     const V*          src,
                     V*                dst )
            {
                // the result of matrix mult should be cached to be used for vertices
                // transform and normal transform and maybe other computation
                for( VertexGroupList::iterator itvg = _uniqVertexGroupList.begin();
                     itvg != _uniqVertexGroupList.end();
                     ++itvg )
                {
                    VertexGroup& uniq = *itvg;
                    uniq.computeMatrixForVertexSet();
                    osg::dmat4 matrix = transform * uniq.getMatrix() * invTransform;

                    const IndexList& vertices = uniq.getVertices();
                    for( IndexList::const_iterator vertIDit = vertices.begin();
                         vertIDit != vertices.end();
                         ++vertIDit )
                    {
                        dst[*vertIDit] =
                            osg::vec3( osg::dvec3( src[*vertIDit] ) * matrix );
                    }
                }
            }

            template<class V>
            inline void
            computeNormal( const osg::dmat4& transform,
                           const osg::dmat4& invTransform,
                           const V*          src,
                           V*                dst )
            {
                for( VertexGroupList::iterator itvg = _uniqVertexGroupList.begin();
                     itvg != _uniqVertexGroupList.end();
                     ++itvg )
                {
                    VertexGroup& uniq = *itvg;
                    uniq.computeMatrixForVertexSet();
                    osg::dmat4 matrix = transform * uniq.getMatrix() * invTransform;

                    const IndexList& vertices = uniq.getVertices();
                    for( IndexList::const_iterator vertIDit = vertices.begin();
                         vertIDit != vertices.end();
                         ++vertIDit )
                    {
                        dst[*vertIDit] =
                            osg::vec3( osg::transform3x3( osg::dvec3( src[*vertIDit] ),
                                                          matrix ) );
                    }
                }
            }

        protected:

            bool _needInit;

            virtual bool
                                             init( RigGeometry& );

            std::map<std::string, bool>      _invalidInfluence;

            typedef std::vector<VertexGroup> VertexGroupList;
            VertexGroupList                  _uniqVertexGroupList;

            void
            buildMinimumUpdateSet( const RigGeometry& rig );
    };

}
