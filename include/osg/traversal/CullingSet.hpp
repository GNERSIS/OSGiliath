/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Set of culling volumes (frustum planes, occluders, small-feature
 * threshold) tested against bounding volumes during traversal.
 */
#pragma once

#include <math.h>
#include <osg/state/Viewport.hpp>
#include <osg/traversal/Polytope.hpp>
#include <osg/traversal/ShadowVolumeOccluder.hpp>

namespace osg
{

#define COMPILE_WITH_SHADOW_OCCLUSION_CULLING

    /** A CullingSet class which contains a frustum and a list of occluders. */
    class OSG_EXPORT CullingSet : public Referenced
    {

        public:

            typedef std::pair<osg::ref_ptr<osg::StateSet>, osg::Polytope>
                                                  StateFrustumPair;
            typedef std::vector<StateFrustumPair> StateFrustumList;

            CullingSet();

            CullingSet( const CullingSet& cs ) :
                Referenced(),
                _mask( cs._mask ),
                _frustum( cs._frustum ),
                _stateFrustumList( cs._stateFrustumList ),
                _occluderList( cs._occluderList ),
                _pixelSizeVector( cs._pixelSizeVector ),
                _smallFeatureCullingPixelSize( cs._smallFeatureCullingPixelSize )
            {
            }

            CullingSet( const CullingSet& cs,
                        const dmat4&      matrix,
                        const vec4&       pixelSizeVector ) :
                _mask( cs._mask ),
                _frustum( cs._frustum ),
                _stateFrustumList( cs._stateFrustumList ),
                _occluderList( cs._occluderList ),
                _pixelSizeVector( pixelSizeVector ),
                _smallFeatureCullingPixelSize( cs._smallFeatureCullingPixelSize )
            {
                _frustum.transformProvidingInverse( matrix );
                for( OccluderList::iterator itr = _occluderList.begin();
                     itr != _occluderList.end();
                     ++itr )
                {
                    itr->transformProvidingInverse( matrix );
                }
            }

            CullingSet&
            operator=( const CullingSet& cs )
            {
                if( this == &cs )
                {
                    return *this;
                }

                _mask                         = cs._mask;
                _frustum                      = cs._frustum;
                _stateFrustumList             = cs._stateFrustumList;
                _occluderList                 = cs._occluderList;
                _pixelSizeVector              = cs._pixelSizeVector;
                _smallFeatureCullingPixelSize = cs._smallFeatureCullingPixelSize;

                return *this;
            }

            inline void
            set( const CullingSet& cs )
            {
                _mask                         = cs._mask;
                _frustum                      = cs._frustum;
                _stateFrustumList             = cs._stateFrustumList;
                _occluderList                 = cs._occluderList;
                _pixelSizeVector              = cs._pixelSizeVector;
                _smallFeatureCullingPixelSize = cs._smallFeatureCullingPixelSize;
            }

            inline void
            set( const CullingSet& cs,
                 const dmat4&      matrix,
                 const vec4&       pixelSizeVector )
            {
                _mask                         = cs._mask;
                _stateFrustumList             = cs._stateFrustumList;
                _occluderList                 = cs._occluderList;
                _pixelSizeVector              = pixelSizeVector;
                _smallFeatureCullingPixelSize = cs._smallFeatureCullingPixelSize;

                //_frustum = cs._frustum;
                //_frustum.transformProvidingInverse(matrix);

                _frustum.setAndTransformProvidingInverse( cs._frustum, matrix );

                for( StateFrustumList::iterator sitr = _stateFrustumList.begin();
                     sitr != _stateFrustumList.end();
                     ++sitr )
                {
                    sitr->second.transformProvidingInverse( matrix );
                }

                for( OccluderList::iterator oitr = _occluderList.begin();
                     oitr != _occluderList.end();
                     ++oitr )
                {
                    oitr->transformProvidingInverse( matrix );
                }
            }

            typedef std::vector<ShadowVolumeOccluder> OccluderList;

            typedef int                               Mask;

            enum MaskValues
            {
                NO_CULLING                 = 0X0,
                VIEW_FRUSTUM_SIDES_CULLING = 0X1,
                NEAR_PLANE_CULLING         = 0X2,
                FAR_PLANE_CULLING          = 0X4,
                VIEW_FRUSTUM_CULLING       = VIEW_FRUSTUM_SIDES_CULLING |
                    NEAR_PLANE_CULLING |
                    FAR_PLANE_CULLING,
                SMALL_FEATURE_CULLING    = 0X8,
                SHADOW_OCCLUSION_CULLING = 0X10,
                DEFAULT_CULLING          = VIEW_FRUSTUM_SIDES_CULLING |
                    SMALL_FEATURE_CULLING |
                    SHADOW_OCCLUSION_CULLING,
                ENABLE_ALL_CULLING = VIEW_FRUSTUM_CULLING |
                    SMALL_FEATURE_CULLING |
                    SHADOW_OCCLUSION_CULLING,
            };

            void
            setCullingMask( Mask mask )
            {
                _mask = mask;
            }

            Mask
            getCullingMask() const
            {
                return _mask;
            }

            void
            setFrustum( Polytope& cv )
            {
                _frustum = cv;
            }

            Polytope&
            getFrustum()
            {
                return _frustum;
            }

            const Polytope&
            getFrustum() const
            {
                return _frustum;
            }

            void
            addStateFrustum( StateSet* stateset,
                             Polytope& polytope )
            {
                _stateFrustumList.push_back( StateFrustumPair( stateset, polytope ) );
            }

            void
            getStateFrustumList( StateFrustumList& sfl )
            {
                _stateFrustumList = sfl;
            }

            StateFrustumList&
            getStateFrustumList()
            {
                return _stateFrustumList;
            }

            void
            addOccluder( ShadowVolumeOccluder& cv )
            {
                _occluderList.push_back( cv );
            }

            void
            setPixelSizeVector( const vec4& v )
            {
                _pixelSizeVector = v;
            }

            vec4&
            getPixelSizeVector()
            {
                return _pixelSizeVector;
            }

            const vec4&
            getPixelSizeVector() const
            {
                return _pixelSizeVector;
            }

            /** Threshold at which small features are culled.
            \param value Bounding volume size in screen space. Default is 2.0. */
            void
            setSmallFeatureCullingPixelSize( float value )
            {
                _smallFeatureCullingPixelSize = value;
            }

            float&
            getSmallFeatureCullingPixelSize()
            {
                return _smallFeatureCullingPixelSize;
            }

            float
            getSmallFeatureCullingPixelSize() const
            {
                return _smallFeatureCullingPixelSize;
            }

            /** Compute the pixel of an object at position v, with specified radius.*/
            float
            pixelSize( const vec3& v,
                       float       radius ) const
            {
                return radius / ( v.x *
                                  _pixelSizeVector.x +
                                  v.y *
                                  _pixelSizeVector.y +
                                  v.z *
                                  _pixelSizeVector.z +
                                  _pixelSizeVector.w );
            }

            /** Compute the pixel of a bounding sphere.*/
            float
            pixelSize( const sphere& bs ) const
            {
                return bs.radius / ( bs.center.x *
                                     _pixelSizeVector.x +
                                     bs.center.y *
                                     _pixelSizeVector.y +
                                     bs.center.z *
                                     _pixelSizeVector.z +
                                     _pixelSizeVector.w );
            }

            /** Compute the pixel of an object at position v, with specified radius.
             * fabs()ed to always be positive. */
            float
            clampedPixelSize( const vec3& v,
                              float       radius ) const
            {
                return fabs( pixelSize( v, radius ) );
            }

            /** Compute the pixel of a bounding sphere. fabs()ed to always be positive.
             */
            float
            clampedPixelSize( const sphere& bs ) const
            {
                return fabs( pixelSize( bs ) );
            }

            inline bool
            isCulled( const std::vector<vec3>& vertices )
            {
                if( _mask & VIEW_FRUSTUM_CULLING )
                {
                    // is it outside the view frustum...
                    if( !_frustum.contains( vertices ) )
                    {
                        return true;
                    }
                }

                if( _mask & SMALL_FEATURE_CULLING )
                {
                }

                if( _mask & SHADOW_OCCLUSION_CULLING )
                {
                    // is it in one of the shadow occluder volumes.
                    if( !_occluderList.empty() )
                    {
                        for( OccluderList::iterator itr = _occluderList.begin();
                             itr != _occluderList.end();
                             ++itr )
                        {
                            if( itr->contains( vertices ) )
                            {
                                return true;
                            }
                        }
                    }
                }

                return false;
            }

            inline bool
            isCulled( const box& bb )
            {
                if( _mask & VIEW_FRUSTUM_CULLING )
                {
                    // is it outside the view frustum...
                    if( !_frustum.contains( bb ) )
                    {
                        return true;
                    }
                }

                if( _mask & SMALL_FEATURE_CULLING )
                {
                }

                if( _mask & SHADOW_OCCLUSION_CULLING )
                {
                    // is it in one of the shadow occluder volumes.
                    if( !_occluderList.empty() )
                    {
                        for( OccluderList::iterator itr = _occluderList.begin();
                             itr != _occluderList.end();
                             ++itr )
                        {
                            if( itr->contains( bb ) )
                            {
                                return true;
                            }
                        }
                    }
                }

                return false;
            }

            inline bool
            isCulled( const sphere& bs )
            {
                if( _mask & VIEW_FRUSTUM_CULLING )
                {
                    // is it outside the view frustum...
                    if( !_frustum.contains( bs ) )
                    {
                        return true;
                    }
                }

                if( _mask & SMALL_FEATURE_CULLING )
                {
                    if( ( ( bs.center.x *
                            _pixelSizeVector.x +
                            bs.center.y *
                            _pixelSizeVector.y +
                            bs.center.z *
                            _pixelSizeVector.z +
                            _pixelSizeVector.w ) *
                          _smallFeatureCullingPixelSize ) > bs.radius )
                    {
                        return true;
                    }
                }
#ifdef COMPILE_WITH_SHADOW_OCCLUSION_CULLING
                if( _mask & SHADOW_OCCLUSION_CULLING )
                {
                    // is it in one of the shadow occluder volumes.
                    if( !_occluderList.empty() )
                    {
                        for( OccluderList::iterator itr = _occluderList.begin();
                             itr != _occluderList.end();
                             ++itr )
                        {
                            if( itr->contains( bs ) )
                            {
                                return true;
                            }
                        }
                    }
                }
#endif
                return false;
            }

            inline void
            pushCurrentMask()
            {
                _frustum.pushCurrentMask();

                if( !_stateFrustumList.empty() )
                {
                    for( StateFrustumList::iterator itr = _stateFrustumList.begin();
                         itr != _stateFrustumList.end();
                         ++itr )
                    {
                        itr->second.pushCurrentMask();
                    }
                }

#ifdef COMPILE_WITH_SHADOW_OCCLUSION_CULLING
                if( !_occluderList.empty() )
                {
                    for( OccluderList::iterator itr = _occluderList.begin();
                         itr != _occluderList.end();
                         ++itr )
                    {
                        itr->pushCurrentMask();
                    }
                }
#endif
            }

            inline void
            popCurrentMask()
            {
                _frustum.popCurrentMask();

                if( !_stateFrustumList.empty() )
                {
                    for( StateFrustumList::iterator itr = _stateFrustumList.begin();
                         itr != _stateFrustumList.end();
                         ++itr )
                    {
                        itr->second.popCurrentMask();
                    }
                }

#ifdef COMPILE_WITH_SHADOW_OCCLUSION_CULLING
                if( !_occluderList.empty() )
                {
                    for( OccluderList::iterator itr = _occluderList.begin();
                         itr != _occluderList.end();
                         ++itr )
                    {
                        itr->popCurrentMask();
                    }
                }
#endif
            }

            inline void
            resetCullingMask()
            {
                _frustum.setResultMask( _frustum.getCurrentMask() );
            }

            void
            disableAndPushOccludersCurrentMask( NodePath& nodePath );

            void
            popOccludersCurrentMask( NodePath& nodePath );

            static osg::vec4
            computePixelSizeVector( const Viewport& W,
                                    const dmat4&    P,
                                    const dmat4&    M );

            virtual ~CullingSet();

        protected:

            Mask             _mask;
            Polytope         _frustum;
            StateFrustumList _stateFrustumList;
            OccluderList     _occluderList;
            vec4             _pixelSizeVector;
            float            _smallFeatureCullingPixelSize;
    };

}    // end of namespace
