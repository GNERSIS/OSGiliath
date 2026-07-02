/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Degree-of-freedom transform node for articulated models.
 * Supports translate, rotate, and scale with limits per axis.
 */
#pragma once

// base class:
#include <osg/core/Inherit.hpp>
#include <osg/nodes/Transform.hpp>
#include <osgSim/Export.hpp>

namespace osgSim
{

    /** DOFTransform - encapsulates Multigen DOF behavior*/
    class OSGSIM_EXPORT DOFTransform : public osg::Inherit<osg::Transform, DOFTransform>
    {
        public:

            /** constructor*/
            DOFTransform();

            /**copy constructor*/
            DOFTransform( const DOFTransform& dof,
                          const osg::CopyOp&  copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgSim,
                               DOFTransform )

            virtual void
            traverse( osg::NodeVisitor& nv );

            void
            setMinHPR( const osg::vec3& hpr )
            {
                _minHPR = hpr;
            }

            const osg::vec3&
            getMinHPR() const
            {
                return _minHPR;
            }

            void
            setMaxHPR( const osg::vec3& hpr )
            {
                _maxHPR = hpr;
            }

            const osg::vec3&
            getMaxHPR() const
            {
                return _maxHPR;
            }

            void
            setIncrementHPR( const osg::vec3& hpr )
            {
                _incrementHPR = hpr;
            }

            const osg::vec3&
            getIncrementHPR() const
            {
                return _incrementHPR;
            }

            void
            setCurrentHPR( const osg::vec3& hpr )
            {
                _currentHPR = hpr;
                dirtyBound();
            }

            const osg::vec3&
            getCurrentHPR() const
            {
                return _currentHPR;
            }

            void
            updateCurrentHPR( const osg::vec3& hpr );

            void
            setMinTranslate( const osg::vec3& translate )
            {
                _minTranslate = translate;
            }

            const osg::vec3&
            getMinTranslate() const
            {
                return _minTranslate;
            }

            void
            setMaxTranslate( const osg::vec3& translate )
            {
                _maxTranslate = translate;
            }

            const osg::vec3&
            getMaxTranslate() const
            {
                return _maxTranslate;
            }

            void
            setIncrementTranslate( const osg::vec3& translate )
            {
                _incrementTranslate = translate;
            }

            const osg::vec3&
            getIncrementTranslate() const
            {
                return _incrementTranslate;
            }

            void
            setCurrentTranslate( const osg::vec3& translate )
            {
                _currentTranslate = translate;
                dirtyBound();
            }

            inline const osg::vec3&
            getCurrentTranslate() const
            {
                return _currentTranslate;
            }

            void
            updateCurrentTranslate( const osg::vec3& translate );

            void
            setMinScale( const osg::vec3& scale )
            {
                _minScale = scale;
            }

            const osg::vec3&
            getMinScale() const
            {
                return _minScale;
            }

            void
            setMaxScale( const osg::vec3& scale )
            {
                _maxScale = scale;
            }

            const osg::vec3&
            getMaxScale() const
            {
                return _maxScale;
            }

            void
            setIncrementScale( const osg::vec3& scale )
            {
                _incrementScale = scale;
            }

            const osg::vec3&
            getIncrementScale() const
            {
                return _incrementScale;
            }

            void
            setCurrentScale( const osg::vec3& scale )
            {
                _currentScale = scale;
                dirtyBound();
            }

            inline const osg::vec3&
            getCurrentScale() const
            {
                return _currentScale;
            }

            void
            updateCurrentScale( const osg::vec3& scale );

            void
            setPutMatrix( const osg::dmat4& put )
            {
                _Put = put;
                dirtyBound();
            }

            inline const osg::dmat4&
            getPutMatrix() const
            {
                return _Put;
            }

            void
            setInversePutMatrix( const osg::dmat4& inversePut )
            {
                _inversePut = inversePut;
                dirtyBound();
            }

            inline const osg::dmat4&
            getInversePutMatrix() const
            {
                return _inversePut;
            }

            void
            setLimitationFlags( unsigned long flags )
            {
                _limitationFlags = flags;
            }

            inline unsigned long
            getLimitationFlags() const
            {
                return _limitationFlags;
            }

            enum MultOrder
            {
                PRH,
                PHR,
                HPR,
                HRP,
                RPH,
                RHP
            };

            void
            setHPRMultOrder( MultOrder order )
            {
                _multOrder = order;
            }

            inline MultOrder
            getHPRMultOrder() const
            {
                return _multOrder;
            }

            void
            setAnimationOn( bool do_animate );

            inline bool
            getAnimationOn() const
            {
                return _animationOn;
            }

            void
            animate( float deltaTime );

            virtual bool
            computeLocalToWorldMatrix( osg::dmat4&       matrix,
                                       osg::NodeVisitor* nv ) const;

            virtual bool
            computeWorldToLocalMatrix( osg::dmat4&       matrix,
                                       osg::NodeVisitor* nv ) const;

        protected:

            virtual ~DOFTransform()
            {
            }

            unsigned int   _previousTraversalNumber;
            double         _previousTime;

            osg::vec3      _minHPR;
            osg::vec3      _maxHPR;
            osg::vec3      _currentHPR;
            osg::vec3      _incrementHPR;

            osg::vec3      _minTranslate;
            osg::vec3      _maxTranslate;
            osg::vec3      _currentTranslate;
            osg::vec3      _incrementTranslate;

            osg::vec3      _minScale;
            osg::vec3      _maxScale;
            osg::vec3      _currentScale;
            osg::vec3      _incrementScale;

            osg::dmat4     _Put;
            osg::dmat4     _inversePut;

            unsigned long  _limitationFlags;
            /* bits from left to right
            0 = x translation limited (2^31)
            1 = y translation limited (2^30)
            2 = z translation limited (2^29)
            3 = pitch limited (2^28)
            4 = roll limited (2^27)
            5 = yaw limited (2^26)
            6 = x scale limited (2^25)
            7 = y scale limited (2^24)
            8 = z scale limited (2^23)

            else reserved
            */

            bool           _animationOn;
            /** flags indicating whether value is incerasing or decreasing in animation
            bits form right to left, 1 means increasing while 0 is decreasing
            0 = x translation
            1 = y translation
            2 = z translation
            3 = pitch
            4 = roll
            5 = yaw
            6 = x scale
            7 = y scale
            8 = z scale
            */
            unsigned short _increasingFlags;

            MultOrder      _multOrder;
    };

}
