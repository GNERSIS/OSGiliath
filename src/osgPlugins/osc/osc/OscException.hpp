/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Exception, derived from exception.
 * Provides: what_, what_, what.
 */
#pragma once

#include <exception>

namespace osc
{

    class Exception : public std::exception
    {
            const char* what_;

        public:

            Exception() throw()
            {
            }

            Exception( const Exception& src ) throw() :
                what_( src.what_ )
            {
            }

            Exception( const char* w ) throw() :
                what_( w )
            {
            }

            Exception&
            operator=( const Exception& src ) throw()
            {
                what_ = src.what_;
                return *this;
            }

            virtual ~Exception() throw()
            {
            }

            virtual const char*
            what() const throw()
            {
                return what_;
            }
    };

}    // namespace osc
