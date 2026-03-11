/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Named identifier with an optional integer ID. Used for
 * efficient string-to-ID lookups in state and buffer management.
 */
#pragma once

#include <cctype>
#include <osg/core/Observer.hpp>
#include <osg/core/Referenced.hpp>
#include <string>

#define OSG_HAS_IDENTIFIER

namespace osg
{

    /** helper function for doing a case insenstive compare of two strings.*/
    inline bool
    iequals( const std::string& lhs,
             const std::string& rhs )
    {
        if( lhs.size() != rhs.size() )
        {
            return false;
        }

        for( std::string::size_type i = 0; i < lhs.size(); ++i )
        {
            if( std::tolower( lhs[i] ) != std::tolower( rhs[i] ) )
            {
                return false;
            }
        }

        return true;
    }

    /** Unique Identifier class to help with efficiently comparing
     * road classification or region via pointers.*/
    class OSG_EXPORT Identifier : public osg::Referenced,
                                  public osg::Observer
    {
        public:

            static Identifier*
            get( const std::string& name,
                 int                number = 0,
                 osg::Referenced*   first  = 0,
                 osg::Referenced*   second = 0 );
            static Identifier*
            get( int              number,
                 osg::Referenced* first  = 0,
                 osg::Referenced* second = 0 );
            static Identifier*
            get( osg::Referenced* first,
                 osg::Referenced* second = 0 );

            const std::string&
            name() const
            {
                return _name;
            }

            const int&
            number() const
            {
                return _number;
            }

        protected:

            Identifier( const std::string& name,
                        int                number,
                        osg::Referenced*   f,
                        osg::Referenced*   s );
            virtual ~Identifier();

            virtual void
                             objectDeleted( void* ptr );

            std::string      _name;
            int              _number;
            osg::Referenced* _first;
            osg::Referenced* _second;
    };

}
