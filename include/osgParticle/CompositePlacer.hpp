/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Distributes particles across multiple child Placers.
 * Combines several emission shapes into one source.
 */
// Written by Wang Rui, (C) 2010

#pragma once

#include <osg/core/Inherit.hpp>
#include <osgParticle/Particle.hpp>
#include <osgParticle/Placer.hpp>

namespace osgParticle
{

    /** A composite particle placer which allows particles to be generated from a union
     * of placers. */
    class CompositePlacer : public osg::Inherit<Placer, CompositePlacer>
    {
        public:

            CompositePlacer()
            {
            }

            CompositePlacer( const CompositePlacer& copy,
                             const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( copy,
                         copyop ),
                _placers( copy._placers )
            {
            }

            OSG_REGISTER_TYPE( osgParticle,
                               CompositePlacer )

            // Set a child placer at specific index
            void
            setPlacer( unsigned int i,
                       Placer*      p )
            {
                if( i < _placers.size() )
                {
                    _placers[i] = p;
                }
                else
                {
                    addPlacer( p );
                }
            }

            /// Add a child placer
            void
            addPlacer( Placer* p )
            {
                _placers.push_back( p );
            }

            /// Remove a child placer
            void
            removePlacer( unsigned int i )
            {
                if( i < _placers.size() )
                {
                    _placers.erase( _placers.begin() + i );
                }
            }

            /// Get a child placer
            Placer*
            getPlacer( unsigned int i )
            {
                return _placers[i].get();
            }

            const Placer*
            getPlacer( unsigned int i ) const
            {
                return _placers[i].get();
            }

            /// Get number of placers
            unsigned int
            getNumPlacers() const
            {
                return _placers.size();
            }

            /// Place a particle. Do not call it manually.
            inline void
            place( Particle* P ) const;

            /// return the volume of the box
            inline float
            volume() const;

            /// return the control position
            inline osg::vec3
            getControlPosition() const;

        protected:

            virtual ~CompositePlacer()
            {
            }

            CompositePlacer&
            operator=( const CompositePlacer& )
            {
                return *this;
            }

            typedef std::vector<osg::ref_ptr<Placer>> PlacerList;
            PlacerList                                _placers;
    };

    // INLINE METHODS

    inline void
    CompositePlacer::place( Particle* P ) const
    {
        rangef sizeRange( 0.0F, volume() );
        float  current = 0.0F, selected = sizeRange.get_random();
        for( PlacerList::const_iterator itr = _placers.begin(); itr != _placers.end();
             ++itr )
        {
            current += ( *itr )->volume();
            if( selected <= current )
            {
                ( *itr )->place( P );
            }
        }
    }

    inline float
    CompositePlacer::volume() const
    {
        float total_size = 0.0F;
        for( PlacerList::const_iterator itr = _placers.begin(); itr != _placers.end();
             ++itr )
        {
            total_size += ( *itr )->volume();
        }
        return total_size;
    }

    inline osg::vec3
    CompositePlacer::getControlPosition() const
    {
        if( !_placers.size() )
        {
            return osg::vec3();
        }
        return _placers.front()->getControlPosition();
    }

}
