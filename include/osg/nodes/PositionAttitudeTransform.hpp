/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Transform node using position, attitude (quaternion), scale, and pivot
 * point. Provides intuitive object placement without raw matrix math.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/nodes/Group.hpp>
#include <osg/nodes/Transform.hpp>
#include <osg/traversal/AnimationPath.hpp>

namespace osg
{

    /** PositionAttitudeTransform - is a Transform. Sets the coordinate transform
        via a vec3 position and quat attitude.
    */
    class OSG_EXPORT PositionAttitudeTransform
        : public osg::Inherit<Transform, PositionAttitudeTransform>
    {
        public:

            OSG_REGISTER_TYPE( osg,
                               PositionAttitudeTransform )

            PositionAttitudeTransform();

            PositionAttitudeTransform( const PositionAttitudeTransform& pat,
                                       const CopyOp& copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( pat,
                         copyop ),
                _position( pat._position ),
                _attitude( pat._attitude ),
                _scale( pat._scale ),
                _pivotPoint( pat._pivotPoint )
            {
            }

            virtual PositionAttitudeTransform*
            asPositionAttitudeTransform()
            {
                return this;
            }

            virtual const PositionAttitudeTransform*
            asPositionAttitudeTransform() const
            {
                return this;
            }

            inline void
            setPosition( const dvec3& pos )
            {
                _position = pos;
                dirtyBound();
            }

            inline const dvec3&
            getPosition() const
            {
                return _position;
            }

            inline void
            setAttitude( const quat& quat )
            {
                _attitude = quat;
                dirtyBound();
            }

            inline const quat&
            getAttitude() const
            {
                return _attitude;
            }

            inline void
            setScale( const dvec3& scale )
            {
                _scale = scale;
                dirtyBound();
            }

            inline const dvec3&
            getScale() const
            {
                return _scale;
            }

            inline void
            setPivotPoint( const dvec3& pivot )
            {
                _pivotPoint = pivot;
                dirtyBound();
            }

            inline const dvec3&
            getPivotPoint() const
            {
                return _pivotPoint;
            }

            virtual bool
            computeLocalToWorldMatrix( dmat4&       matrix,
                                       NodeVisitor* nv ) const;
            virtual bool
            computeWorldToLocalMatrix( dmat4&       matrix,
                                       NodeVisitor* nv ) const;

        protected:

            virtual ~PositionAttitudeTransform()
            {
            }

            dvec3 _position;
            quat  _attitude;
            dvec3 _scale;
            dvec3 _pivotPoint;
    };

}
