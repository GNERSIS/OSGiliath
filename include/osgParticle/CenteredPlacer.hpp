/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Abstract placer with a configurable center point.
 * Base for SectorPlacer and PointPlacer.
 */
// osgParticle - Copyright (C) 2002 Marco Jez

#pragma once

#include <osg/core/CopyOp.hpp>
#include <osg/core/Object.hpp>
#include <osg/maths/vec3.hpp>
#include <osgParticle/Placer.hpp>

namespace osgParticle
{

    /**    An abstract placer base class for placers which need a <I>center point</I>.
     */
    class CenteredPlacer : public Placer
    {
        public:

            inline CenteredPlacer();
            inline CenteredPlacer( const CenteredPlacer& copy,
                                   const osg::CopyOp&    copyop =
                                       osg::CopyOp::SHALLOW_COPY );

            virtual const char*
            libraryName() const
            {
                return "osgParticle";
            }

            virtual const char*
            className() const
            {
                return "CenteredPlacer";
            }

            virtual bool
            isSameKindAs( const osg::Object* obj ) const
            {
                return dynamic_cast<const Placer*>( obj ) != 0;
            }

            /// Get the center point.
            inline const osg::vec3&
            getCenter() const;

            /// Set the center point.
            inline void
            setCenter( const osg::vec3& v );

            /// Set the center point.
            inline void
            setCenter( float x,
                       float y,
                       float z );

        protected:

            virtual ~CenteredPlacer()
            {
            }

        private:

            osg::vec3 center_;
    };

    // INLINE FUNCTIONS

    inline CenteredPlacer::CenteredPlacer() :
        Placer(),
        center_( 0,
                 0,
                 0 )
    {
    }

    inline CenteredPlacer::CenteredPlacer( const CenteredPlacer& copy,
                                           const osg::CopyOp&    copyop ) :
        Placer( copy,
                copyop ),
        center_( copy.center_ )
    {
    }

    inline const osg::vec3&
    CenteredPlacer::getCenter() const
    {
        return center_;
    }

    inline void
    CenteredPlacer::setCenter( const osg::vec3& v )
    {
        center_ = v;
    }

    inline void
    CenteredPlacer::setCenter( float x,
                               float y,
                               float z )
    {
        center_.set( x, y, z );
    }

}
