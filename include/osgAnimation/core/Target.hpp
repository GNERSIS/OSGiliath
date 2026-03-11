/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract animation target receiving interpolated values.
 * Accumulates weighted contributions from multiple channels.
 */
#pragma once

#include <osg/core/Referenced.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osgAnimation/core/Export.hpp>
#include <vector>

namespace osgAnimation
{

    class Channel;

    class OSGANIMATION_EXPORT Target : public osg::Referenced
    {
        public:

            Target();

            virtual ~Target()
            {
            }

            void
            reset()
            {
                _weight         = 0;
                _priorityWeight = 0;
            }

            int
            getCount() const
            {
                return referenceCount();
            }

            float
            getWeight() const
            {
                return _weight;
            }

        protected:

            float _weight;
            float _priorityWeight;
            int   _lastPriority;
    };

    template<class T>
    class TemplateTarget : public Target
    {
        public:

            TemplateTarget() :
                _target()
            {
            }

            TemplateTarget( const T& v )
            {
                setValue( v );
            }

            TemplateTarget( const TemplateTarget& v )
            {
                setValue( v.getValue() );
            }

            inline void
            lerp( float    t,
                  const T& a,
                  const T& b );

            /**
             *  The priority is used to detect a change of priority
             *  It's important to update animation target in priority
             *  order. eg:
             *  all animation with priority 1
             *  all animation with priority 0
             *  all animation with priority -1
             *  ...
             */
            void
            update( float    weight,
                    const T& val,
                    int      priority )
            {
                if( _weight || _priorityWeight )
                {
                    if( _lastPriority != priority )
                    {
                        // change in priority
                        // add to weight with the same previous priority cumulated weight
                        _weight         += _priorityWeight * ( 1.0 - _weight );
                        _priorityWeight  = 0;
                        _lastPriority    = priority;
                    }

                    _priorityWeight += weight;
                    float t          = ( 1.0 - _weight ) * weight / _priorityWeight;
                    lerp( t, _target, val );
                }
                else
                {
                    _priorityWeight = weight;
                    _lastPriority   = priority;
                    _target         = val;
                }
            }

            const T&
            getValue() const
            {
                return _target;
            }

            void
            setValue( const T& value )
            {
                _target = value;
            }

        protected:

            T _target;
    };

    template<class T>
    inline void
    TemplateTarget<T>::lerp( float    t,
                             const T& a,
                             const T& b )
    {
        _target = a * ( 1.0F - t ) + b * t;
    }

    template<>
    inline void
    TemplateTarget<osg::quat>::lerp( float            t,
                                     const osg::quat& a,
                                     const osg::quat& b )
    {
        if( osg::dot( a, b ) < 0.0F )
        {
            _target = a * ( 1.0F - t ) + b * -t;
        }
        else
        {
            _target = a * ( 1.0F - t ) + b * t;
        }

        osg::quat::value_type len2 = osg::dot( _target, _target );
        if( len2 != 1.0F && len2 != 0.0F )
        {
            _target = _target * ( 1.0F / std::sqrt( len2 ) );
        }
    }

    typedef TemplateTarget<osg::mat4> MatrixTarget;
    typedef TemplateTarget<osg::quat> QuatTarget;
    typedef TemplateTarget<osg::vec3> Vec3Target;
    typedef TemplateTarget<osg::vec4> Vec4Target;
    typedef TemplateTarget<osg::vec2> Vec2Target;
    typedef TemplateTarget<float>     FloatTarget;
    typedef TemplateTarget<double>    DoubleTarget;

}
