/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Bone-weight mapping for skeletal animation. Maps bone names
 * to per-vertex weight arrays for mesh skinning.
 */
#pragma once

#include <map>
#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osgAnimation/core/Export.hpp>
#include <string>
#include <vector>

namespace osgAnimation
{

    class Skeleton;

    // first is bonename, and second the weight
    typedef std::pair<std::string, float>  BoneWeight;
    // first is vertex index, and second the weight
    typedef std::pair<unsigned int, float> VertexIndexWeight;
    // list of IndexWeight
    typedef std::vector<VertexIndexWeight> IndexWeightList;
    // list of IndexWeight
    typedef std::vector<BoneWeight>        BoneWeightList;
    // list of Index
    typedef std::vector<unsigned int>      IndexList;

    // Bone influence list
    class OSGANIMATION_EXPORT VertexInfluence : public IndexWeightList
    {
        public:

            const std::string&
            getName() const
            {
                return _name;
            }

            void
            setName( const std::string& name )
            {
                _name = name;
            }

        protected:

            // the name is the bone to link to
            std::string _name;
    };

    class VertexInfluenceMap : public osg::Inherit<osg::Object, VertexInfluenceMap>,
                               public std::map<std::string, VertexInfluence>
    {
        public:

            OSG_REGISTER_TYPE( osgAnimation,
                               VertexInfluenceMap )

            VertexInfluenceMap()
            {
            }

            VertexInfluenceMap( const osgAnimation::VertexInfluenceMap& org,
                                const osg::CopyOp&                      copyop ) :
                Inherit( org,
                         copyop ),
                std::map<std::string,
                         VertexInfluence>( org )
            {
            }

            /// normalize per vertex weights given numvert of the attached mesh
            void
            normalize( unsigned int numvert );

            /// remove weakest influences in order to fit targeted numbonepervertex
            void
            cullInfluenceCountPerVertex( unsigned int maxnumbonepervertex,
                                         float        minweight   = 0,
                                         bool         renormalize = true );

            // compute PerVertexInfluenceList
            void
            computePerVertexInfluenceList(
                std::vector<BoneWeightList>& perVertexInfluenceList,
                unsigned int                 numvert
            ) const;

            /// map a set of boneinfluence to a list of vertex indices sharing this set
            class VertexGroup : public std::pair<BoneWeightList, IndexList>
            {
                public:

                    inline const BoneWeightList&
                    getBoneWeights() const
                    {
                        return first;
                    }

                    inline void
                    setBoneWeights( BoneWeightList& o )
                    {
                        first = o;
                    }

                    inline IndexList&
                    vertIDs()
                    {
                        return second;
                    }
            };

            /// compute the minimal VertexGroup Set in which vertices shares the same
            /// influence set
            void
            computeMinimalVertexGroupList( std::vector<VertexGroup>& uniqVertexGroupList,
                                           unsigned int              numvert ) const;

            // Experimental removal of unexpressed bone from the skeleton
            void
            removeUnexpressedBones( Skeleton& skel ) const;
    };

}
