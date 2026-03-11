/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Typed keyframe container. Stores time-value pairs for
 * animation channels with binary-search time lookup.
 */
#pragma once

#include <osg/core/MixinVector.hpp>
#include <osg/core/Referenced.hpp>
#include <osg/maths/mat4.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osgAnimation/core/CubicBezier.hpp>
#include <osgAnimation/core/Vec3Packed.hpp>
#include <string>

namespace osgAnimation
{

    class Keyframe
    {
        public:

            double
            getTime() const
            {
                return _time;
            }

            void
            setTime( double time )
            {
                _time = time;
            }

        protected:

            double _time;
    };

    template<class T>
    class TemplateKeyframe : public Keyframe
    {
        protected:

            T _value;

        public:

            typedef T value_type;

            TemplateKeyframe()
            {
            }

            ~TemplateKeyframe()
            {
            }

            TemplateKeyframe( double   time,
                              const T& value )
            {
                _time  = time;
                _value = value;
            }

            void
            setValue( const T& value )
            {
                _value = value;
            }

            const T&
            getValue() const
            {
                return _value;
            }
    };

    class KeyframeContainer : public osg::Referenced
    {
        public:

            KeyframeContainer()
            {
            }

            virtual unsigned int
            size() const = 0;
            virtual unsigned int
            linearInterpolationDeduplicate() = 0;

        protected:

            ~KeyframeContainer()
            {
            }

            std::string _name;
    };

    template<class T>
    class TemplateKeyframeContainer : public osg::MixinVector<TemplateKeyframe<T>>,
                                      public KeyframeContainer
    {
        public:

            // const char* getKeyframeType() { return #T ;}
            TemplateKeyframeContainer()
            {
            }

            typedef TemplateKeyframe<T>                            KeyType;
            typedef typename osg::MixinVector<TemplateKeyframe<T>> VectorType;

            virtual unsigned int
            size() const
            {
                return ( unsigned int )osg::MixinVector<TemplateKeyframe<T>>::size();
            }

            virtual unsigned int
            linearInterpolationDeduplicate()
            {
                if( size() <= 1 )
                {
                    return 0;
                }

                typename VectorType::iterator keyframe = VectorType::begin(),
                                              previous = VectorType::begin();
                // 1. find number of consecutives identical keyframes
                std::vector<unsigned int>     intervalSizes;
                unsigned int                  intervalSize = 1;
                for( ++keyframe; keyframe != VectorType::end();
                     ++keyframe, ++previous, ++intervalSize )
                {
                    if( !( previous->getValue() == keyframe->getValue() ) )
                    {
                        intervalSizes.push_back( intervalSize );
                        intervalSize = 0;
                    }
                }
                intervalSizes.push_back( intervalSize );

                // 2. build deduplicated list of keyframes
                unsigned int cumul = 0;
                VectorType   deduplicated;
                for( std::vector<unsigned int>::iterator it = intervalSizes.begin();
                     it != intervalSizes.end();
                     ++it )
                {
                    deduplicated.push_back( ( *this )[cumul] );
                    if( *it > 1 )
                    {
                        deduplicated.push_back( ( *this )[cumul + ( *it ) - 1] );
                    }
                    cumul += *it;
                }

                unsigned int count = size() - deduplicated.size();
                this->swap( deduplicated );
                return count;
            }
    };

    template<>
    class TemplateKeyframeContainer<Vec3Packed>
        : public osg::MixinVector<TemplateKeyframe<Vec3Packed>>,
          public KeyframeContainer
    {
        public:

            typedef TemplateKeyframe<Vec3Packed> KeyType;

            TemplateKeyframeContainer()
            {
            }

            const char*
            getKeyframeType()
            {
                return "Vec3Packed";
            }

            void
            init( const osg::vec3& min,
                  const osg::vec3& scale )
            {
                _min   = min;
                _scale = scale;
            }

            osg::vec3 _min;
            osg::vec3 _scale;
    };

    typedef TemplateKeyframe<float>                FloatKeyframe;
    typedef TemplateKeyframeContainer<float>       FloatKeyframeContainer;

    typedef TemplateKeyframe<double>               DoubleKeyframe;
    typedef TemplateKeyframeContainer<double>      DoubleKeyframeContainer;

    typedef TemplateKeyframe<osg::vec2>            Vec2Keyframe;
    typedef TemplateKeyframeContainer<osg::vec2>   Vec2KeyframeContainer;

    typedef TemplateKeyframe<osg::vec3>            Vec3Keyframe;
    typedef TemplateKeyframeContainer<osg::vec3>   Vec3KeyframeContainer;

    typedef TemplateKeyframe<osg::usvec3>          Vec3usKeyframe;
    typedef TemplateKeyframeContainer<osg::usvec3> Vec3usKeyframeContainer;

    typedef TemplateKeyframe<osg::vec4>            Vec4Keyframe;
    typedef TemplateKeyframeContainer<osg::vec4>   Vec4KeyframeContainer;

    typedef TemplateKeyframe<osg::quat>            QuatKeyframe;
    typedef TemplateKeyframeContainer<osg::quat>   QuatKeyframeContainer;

    typedef TemplateKeyframe<osg::usvec3>          Vec3usKeyframe;
    typedef TemplateKeyframeContainer<osg::usvec3> Vec3usKeyframeContainer;

    typedef TemplateKeyframe<osg::mat4>            MatrixKeyframe;
    typedef TemplateKeyframeContainer<osg::mat4>   MatrixKeyframeContainer;

    typedef TemplateKeyframe<Vec3Packed>           Vec3PackedKeyframe;
    typedef TemplateKeyframeContainer<Vec3Packed>  Vec3PackedKeyframeContainer;

    typedef TemplateKeyframe<FloatCubicBezier>     FloatCubicBezierKeyframe;
    typedef TemplateKeyframeContainer<FloatCubicBezier>
                                                FloatCubicBezierKeyframeContainer;

    typedef TemplateKeyframe<DoubleCubicBezier> DoubleCubicBezierKeyframe;
    typedef TemplateKeyframeContainer<DoubleCubicBezier>
                                              DoubleCubicBezierKeyframeContainer;

    typedef TemplateKeyframe<Vec2CubicBezier> Vec2CubicBezierKeyframe;
    typedef TemplateKeyframeContainer<Vec2CubicBezier> Vec2CubicBezierKeyframeContainer;

    typedef TemplateKeyframe<Vec3CubicBezier>          Vec3CubicBezierKeyframe;
    typedef TemplateKeyframeContainer<Vec3CubicBezier> Vec3CubicBezierKeyframeContainer;

    typedef TemplateKeyframe<Vec4CubicBezier>          Vec4CubicBezierKeyframe;
    typedef TemplateKeyframeContainer<Vec4CubicBezier> Vec4CubicBezierKeyframeContainer;

}
