/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Skinned mesh geometry deformed by a Skeleton. Applies bone
 * weights to vertices via software or hardware skinning.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/geometry/Geometry.hpp>
#include <osgAnimation/core/Export.hpp>
#include <osgAnimation/skeletal/RigTransform.hpp>
#include <osgAnimation/skeletal/Skeleton.hpp>
#include <osgAnimation/skeletal/VertexInfluence.hpp>

namespace osgAnimation
{

    // The idea is to compute a bounding box with a factor x of the first step we compute
    // the bounding box
    class OSGANIMATION_EXPORT RigComputeBoundingBoxCallback
        : public osg::Inherit<osg::Drawable::ComputeBoundingBoxCallback,
                              RigComputeBoundingBoxCallback>
    {
        public:

            RigComputeBoundingBoxCallback( double factor = 2.0 ) :
                _computed( false ),
                _factor( factor )
            {
            }

            RigComputeBoundingBoxCallback( const RigComputeBoundingBoxCallback& rhs,
                                           const osg::CopyOp& copyop ) :
                Inherit( rhs,
                         copyop ),
                _computed( false ),
                _factor( rhs._factor )
            {
            }

            OSG_REGISTER_TYPE( osgAnimation,
                               RigComputeBoundingBoxCallback )

            void
            reset()
            {
                _computed = false;
            }

            virtual osg::box
            computeBound( const osg::Drawable& drawable ) const;

        protected:

            mutable bool     _computed;
            double           _factor;
            mutable osg::box _boundingBox;
    };

    class OSGANIMATION_EXPORT RigGeometry
        : public osg::Inherit<osg::Geometry, RigGeometry>
    {
        public:

            RigGeometry();

            RigGeometry( const RigGeometry& b,
                         const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgAnimation,
                               RigGeometry )

            inline void
            setInfluenceMap( VertexInfluenceMap* vertexInfluenceMap )
            {
                _vertexInfluenceMap = vertexInfluenceMap;
            }

            inline const VertexInfluenceMap*
            getInfluenceMap() const
            {
                return _vertexInfluenceMap.get();
            }

            inline VertexInfluenceMap*
            getInfluenceMap()
            {
                return _vertexInfluenceMap.get();
            }

            inline const Skeleton*
            getSkeleton() const
            {
                return _root.get();
            }

            inline Skeleton*
            getSkeleton()
            {
                return _root.get();
            }

            // will be used by the update callback to init correctly the rig mesh
            inline void
            setSkeleton( Skeleton* root )
            {
                _root = root;
            }

            void
            setNeedToComputeMatrix( bool state )
            {
                _needToComputeMatrix = state;
            }

            bool
            getNeedToComputeMatrix() const
            {
                return _needToComputeMatrix;
            }

            void
            computeMatrixFromRootSkeleton();

            // set implementation of rig method
            inline RigTransform*
            getRigTransformImplementation()
            {
                return _rigTransformImplementation.get();
            }

            inline void
            setRigTransformImplementation( RigTransform* rig )
            {
                _rigTransformImplementation = rig;
            }

            inline const RigTransform*
            getRigTransformImplementation() const
            {
                return _rigTransformImplementation.get();
            }

            void
            update();

            void
            buildVertexInfluenceSet()
            {
                _rigTransformImplementation->prepareData( *this );
            }

            const osg::dmat4&
            getMatrixFromSkeletonToGeometry() const;

            const osg::dmat4&
            getInvMatrixFromSkeletonToGeometry() const;

            inline osg::Geometry*
            getSourceGeometry()
            {
                return _geometry.get();
            }

            inline const osg::Geometry*
            getSourceGeometry() const
            {
                return _geometry.get();
            }

            inline void
            setSourceGeometry( osg::Geometry* geometry )
            {
                _geometry = geometry;
            }

            void
            copyFrom( osg::Geometry& from );

            struct FindNearestParentSkeleton : public osg::DualModeVisitor
            {
                    using osg::DualModeVisitor::apply;

                    osg::ref_ptr<Skeleton> _root;

                    FindNearestParentSkeleton() :
                        osg::DualModeVisitor( osg::DualModeVisitor::TRAVERSE_PARENTS )
                    {
                    }

                    void
                    apply( osg::Transform& node )
                    {
                        if( _root.valid() )
                        {
                            return;
                        }
                        _root = dynamic_cast<osgAnimation::Skeleton*>( &node );
                        traverse( node );
                    }
            };

        protected:

            osg::ref_ptr<osg::Geometry>      _geometry;
            osg::ref_ptr<RigTransform>       _rigTransformImplementation;
            osg::ref_ptr<VertexInfluenceMap> _vertexInfluenceMap;

            osg::dmat4                       _matrixFromSkeletonToGeometry;
            osg::dmat4                       _invMatrixFromSkeletonToGeometry;
            osg::observer_ptr<Skeleton>      _root;
            bool                             _needToComputeMatrix;
    };

    struct UpdateRigGeometry : public osg::Drawable::UpdateCallback
    {
            UpdateRigGeometry()
            {
            }

            UpdateRigGeometry( const UpdateRigGeometry& org,
                               const osg::CopyOp&       copyop ) :
                osg::Object( org,
                             copyop ),
                osg::Callback( org,
                               copyop ),
                osg::DrawableUpdateCallback( org,
                                             copyop )
            {
            }

            OSG_REGISTER_TYPE( osgAnimation,
                               UpdateRigGeometry )

            virtual void
            update( osg::NodeVisitor* nv,
                    osg::Drawable*    drw )
            {
                RigGeometry* geom = dynamic_cast<RigGeometry*>( drw );
                if( !geom )
                {
                    return;
                }
                if( !geom->getSkeleton() && !geom->getParents().empty() )
                {
                    RigGeometry::FindNearestParentSkeleton finder;
                    if( geom->getParents().size() > 1 )
                    {
                        osg::notify( osg::WARN )
                            << "A RigGeometry should not have multi parent ( "
                            << geom->getName() << " )" << std::endl;
                    }
                    geom->getParents()[0]->accept( finder );

                    if( !finder._root.valid() )
                    {
                        osg::notify( osg::WARN ) << "A RigGeometry did not find a "
                                                    "parent skeleton for RigGeometry ( "
                                                 << geom->getName() << " )" << std::endl;
                        return;
                    }
                    geom->getRigTransformImplementation()->prepareData( *geom );
                    geom->setSkeleton( finder._root.get() );
                }

                if( !geom->getSkeleton() )
                {
                    return;
                }

                if( geom->getNeedToComputeMatrix() )
                {
                    geom->computeMatrixFromRootSkeleton();
                }

                if( geom->getSourceGeometry() )
                {
                    osg::Drawable::UpdateCallback* up =
                        dynamic_cast<osg::Drawable::UpdateCallback*>(
                            geom->getSourceGeometry()->getUpdateCallback()
                        );
                    if( up )
                    {
                        up->update( nv, geom->getSourceGeometry() );
                    }
                }

                geom->update();
            }
    };

}
