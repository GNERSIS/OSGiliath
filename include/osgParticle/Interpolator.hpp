/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract interpolator base for particle property transitions.
 * Maps normalized particle age [0,1] to a typed value.
 */
// osgParticle - Copyright (C) 2002 Marco Jez

#pragma once

#include <osg/core/CopyOp.hpp>
#include <osg/core/Object.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <osgParticle/range.hpp>

namespace osgParticle
{

    /// An abstract base class for implementing interpolators.
    class Interpolator : public osg::Object
    {
        public:

            Interpolator() :
                osg::Object()
            {
            }

            Interpolator( const Interpolator& copy,
                          const osg::CopyOp&  copyop = osg::CopyOp::SHALLOW_COPY ) :
                osg::Object( copy,
                             copyop )
            {
            }

            virtual const char*
            libraryName() const
            {
                return "osgParticle";
            }

            virtual const char*
            className() const
            {
                return "Interpolator";
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const Interpolator*>( obj ) != 0;
            }

            /// Interpolate between floats. Must be overridden in descendant classes.
            virtual float
            interpolate( float t,
                         float y1,
                         float y2 ) const = 0;

            /// Interpolate between 2-dimensional vectors. Default behavior is to
            /// interpolate each component separately.
            virtual osg::vec2
            interpolate( float            t,
                         const osg::vec2& y1,
                         const osg::vec2& y2 ) const
            {
                return osg::vec2( interpolate( t, y1.x, y2.x ),
                                  interpolate( t, y1.y, y2.y ) );
            }

            /// Interpolate between 3-dimensional vectors. Default behavior is to
            /// interpolate each component separately.
            virtual osg::vec3
            interpolate( float            t,
                         const osg::vec3& y1,
                         const osg::vec3& y2 ) const
            {
                return osg::vec3( interpolate( t, y1.x, y2.x ),
                                  interpolate( t, y1.y, y2.y ),
                                  interpolate( t, y1.z, y2.z ) );
            }

            /// Interpolate between 4-dimensional vectors. Default behavior is to
            /// interpolate each component separately.
            virtual osg::vec4
            interpolate( float            t,
                         const osg::vec4& y1,
                         const osg::vec4& y2 ) const
            {
                return osg::vec4( interpolate( t, y1.x, y2.x ),
                                  interpolate( t, y1.y, y2.y ),
                                  interpolate( t, y1.z, y2.z ),
                                  interpolate( t, y1.w, y2.w ) );
            }

            template<class ValueType>
            ValueType
            interpolate( float                   t,
                         const range<ValueType>& r ) const
            {
                return interpolate( t, r.minimum, r.maximum );
            }

        protected:

            virtual ~Interpolator()
            {
            }
    };

}
