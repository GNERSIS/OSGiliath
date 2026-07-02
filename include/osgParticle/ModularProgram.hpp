/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Particle program composed of pluggable Operator modules.
 * Applies forces and effects (gravity, wind, drag) each frame.
 */
// osgParticle - Copyright (C) 2002 Marco Jez

#pragma once

#include <osg/core/CopyOp.hpp>
#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <osg/nodes/Node.hpp>
#include <osg/traversal/NodeVisitor.hpp>
#include <osgParticle/Export.hpp>
#include <osgParticle/Operator.hpp>
#include <osgParticle/Program.hpp>

namespace osgParticle
{

    /**    A program class for performing operations on particles using a sequence of
       <I>operators</I>. To use a <CODE>ModularProgram</CODE> you have to create some
       <CODE>Operator</CODE> objects and add them to the program. All operators will be
       applied to each particle in the same order they've been added to the program.
    */
    class OSGPARTICLE_EXPORT ModularProgram
        : public osg::Inherit<Program, ModularProgram>
    {
        public:

            ModularProgram();
            ModularProgram( const ModularProgram& copy,
                            const osg::CopyOp&    copyop = osg::CopyOp::SHALLOW_COPY );

            OSG_REGISTER_TYPE( osgParticle,
                               ModularProgram )

            /// Get the number of operators.
            inline int
            numOperators() const;

            /// Add an operator to the list.
            inline void
            addOperator( Operator* o );

            /// Get a pointer to an operator in the list.
            inline Operator*
            getOperator( int i );

            /// Get a const pointer to an operator in the list.
            inline const Operator*
            getOperator( int i ) const;

            /// Remove an operator from the list.
            inline void
            removeOperator( int i );

        protected:

            virtual ~ModularProgram()
            {
            }

            ModularProgram&
            operator=( const ModularProgram& )
            {
                return *this;
            }

            void
            execute( double dt );

        private:

            typedef std::vector<osg::ref_ptr<Operator>> Operator_vector;

            Operator_vector                             _operators;
    };

    // INLINE FUNCTIONS

    inline int
    ModularProgram::numOperators() const
    {
        return static_cast<int>( _operators.size() );
    }

    inline void
    ModularProgram::addOperator( Operator* o )
    {
        _operators.push_back( o );
    }

    inline Operator*
    ModularProgram::getOperator( int i )
    {
        return _operators[static_cast<std::size_t>( i )].get();
    }

    inline const Operator*
    ModularProgram::getOperator( int i ) const
    {
        return _operators[static_cast<std::size_t>( i )].get();
    }

    inline void
    ModularProgram::removeOperator( int i )
    {
        _operators.erase( _operators.begin() + i );
    }

}
