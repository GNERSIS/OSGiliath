/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Helper for formatted parameter output in .osg writing.
 * Handles field separators and line wrapping.
 */
#pragma once

#include <osgDB/serialization/Output.hpp>

namespace osgDB
{

    class ParameterOutput
    {
        public:

            ParameterOutput( Output& fw ) :
                _fw( fw ),
                _numItemsPerLine( fw.getNumIndicesPerLine() ),
                _column( 0 )
            {
            }

            ParameterOutput( Output& fw,
                             int     numItemsPerLine ) :
                _fw( fw ),
                _numItemsPerLine( numItemsPerLine ),
                _column( 0 )
            {
            }

            void
            begin()
            {
                _fw.indent() << "{" << std::endl;
                _fw.moveIn();
            }

            void
            newLine()
            {
                if( _column != 0 )
                {
                    _fw << std::endl;
                }
                _column = 0;
            }

            void
            end()
            {
                if( _column != 0 )
                {
                    _fw << std::endl;
                }
                _fw.moveOut();
                _fw.indent() << "}" << std::endl;
                _column = 0;
            }

            template<class T>
            void
            write( const T& t )
            {
                if( _column == 0 )
                {
                    _fw.indent();
                }

                _fw << t;

                ++_column;
                if( _column == _numItemsPerLine )
                {
                    _fw << std::endl;
                    _column = 0;
                }
                else
                {
                    _fw << " ";
                }
            }

            template<class Iterator>
            void
            write( Iterator first,
                   Iterator last )
            {
                for( Iterator itr = first; itr != last; ++itr )
                {
                    write( *itr );
                }
            }

            template<class Iterator>
            void
            writeAsInts( Iterator first,
                         Iterator last )
            {
                for( Iterator itr = first; itr != last; ++itr )
                {
                    write( ( int )*itr );
                }
            }

        protected:

            ParameterOutput&
            operator=( const ParameterOutput& )
            {
                return *this;
            }

            Output& _fw;
            int     _numItemsPerLine;
            int     _column;
    };

    template<class Iterator>
    void
    writeArray( Output&  fw,
                Iterator first,
                Iterator last,
                int      noItemsPerLine = 0 )
    {
        if( noItemsPerLine == 0 )
        {
            noItemsPerLine = fw.getNumIndicesPerLine();
        }

        fw.indent() << "{" << std::endl;
        fw.moveIn();

        int column = 0;

        for( Iterator itr = first; itr != last; ++itr )
        {
            if( column == 0 )
            {
                fw.indent();
            }

            fw << *itr;

            ++column;
            if( column == noItemsPerLine )
            {
                fw << std::endl;
                column = 0;
            }
            else
            {
                fw << " ";
            }
        }
        if( column != 0 )
        {
            fw << std::endl;
        }

        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }

    template<class Iterator>
    void
    writeArrayAsInts( Output&  fw,
                      Iterator first,
                      Iterator last,
                      int      noItemsPerLine = 0 )
    {
        if( noItemsPerLine == 0 )
        {
            noItemsPerLine = fw.getNumIndicesPerLine();
        }

        fw.indent() << "{" << std::endl;
        fw.moveIn();

        int column = 0;

        for( Iterator itr = first; itr != last; ++itr )
        {
            if( column == 0 )
            {
                fw.indent();
            }

            fw << ( int )*itr;

            ++column;
            if( column == noItemsPerLine )
            {
                fw << std::endl;
                column = 0;
            }
            else
            {
                fw << " ";
            }
        }
        if( column != 0 )
        {
            fw << std::endl;
        }

        fw.moveOut();
        fw.indent() << "}" << std::endl;
    }

}
