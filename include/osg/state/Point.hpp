/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Point size state attribute and per-vertex point size enable.
 * Configures gl_PointSize for GL_POINTS rendering.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/state/StateAttribute.hpp>

namespace osg
{

    /** Point - encapsulates the OpenGL point smoothing and size state.*/
    class OSG_EXPORT Point : public osg::Inherit<StateAttribute, Point>
    {
        public:

            Point();

            Point( float size );

            /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
            Point( const Point&  point,
                   const CopyOp& copyop = CopyOp::SHALLOW_COPY ) :
                Inherit( point,
                         copyop ),
                _size( point._size ),
                _fadeThresholdSize( point._fadeThresholdSize ),
                _distanceAttenuation( point._distanceAttenuation ),
                _minSize( point._minSize ),
                _maxSize( point._maxSize )
            {
            }

            OSG_REGISTER_TYPE( osg,
                               Point )

            Type
            getType() const override
            {
                return Type::POINT;
            }

            /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
            int
            compare( const StateAttribute& sa ) const override
            {
                // check the types are equal and then create the rhs variable
                // used by the COMPARE_StateAttribute_Parameter macros below.
                COMPARE_StateAttribute_Types( Point, sa )

                    // compare each parameter in turn against the rhs.
                    COMPARE_StateAttribute_Parameter( _size )
                        COMPARE_StateAttribute_Parameter( _fadeThresholdSize )
                            COMPARE_StateAttribute_Parameter( _distanceAttenuation )
                                COMPARE_StateAttribute_Parameter( _minSize )
                                    COMPARE_StateAttribute_Parameter(
                                        _maxSize
                                    ) return 0;    // passed all the above comparison
                                                   // macros, must be equal.
            }

            void
            setSize( float size );

            inline float
            getSize() const
            {
                return _size;
            }

            void
            setFadeThresholdSize( float fadeThresholdSize );

            inline float
            getFadeThresholdSize() const
            {
                return _fadeThresholdSize;
            }

            void
            setDistanceAttenuation( const vec3& distanceAttenuation );

            inline const vec3&
            getDistanceAttenuation() const
            {
                return _distanceAttenuation;
            }

            void
            setMinSize( float minSize );

            inline float
            getMinSize() const
            {
                return _minSize;
            }

            void
            setMaxSize( float maxSize );

            inline float
            getMaxSize() const
            {
                return _maxSize;
            }

            void
            apply( State& state ) const override;

        protected:

            virtual ~Point();

            float _size;
            float _fadeThresholdSize;
            vec3  _distanceAttenuation;
            float _minSize;
            float _maxSize;
    };

}
