/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * UTF-8 to wide-string conversion utilities for cross-platform
 * file path handling (primarily Win32 Unicode support).
 */
#pragma once

#include <osg/Config>
#include <osgDB/Export.hpp>
#include <string>

#if defined( __CYGWIN__ ) || defined( __ANDROID__ )
namespace std
{

    typedef basic_string<wchar_t> wstring;

}
#endif

namespace osgDB
{

    extern OSGDB_EXPORT std::string
                        convertUTF16toUTF8( const wchar_t* source,
                                            unsigned       sourceLength );
    extern OSGDB_EXPORT std::wstring
                        convertUTF8toUTF16( const char* source,
                                            unsigned    sourceLength );

    extern OSGDB_EXPORT std::string
                        convertUTF16toUTF8( const std::wstring& s );
    extern OSGDB_EXPORT std::string
                        convertUTF16toUTF8( const wchar_t* s );

    extern OSGDB_EXPORT std::wstring
                        convertUTF8toUTF16( const std::string& s );
    extern OSGDB_EXPORT std::wstring
                        convertUTF8toUTF16( const char* s );

    extern OSGDB_EXPORT std::string
                        convertStringFromCurrentCodePageToUTF8( const char* source,
                                                                unsigned    sourceLength );
    extern OSGDB_EXPORT std::string
                        convertStringFromUTF8toCurrentCodePage( const char* source,
                                                                unsigned    sourceLength );

    extern OSGDB_EXPORT std::string
                        convertStringFromCurrentCodePageToUTF8( const std::string& s );
    extern OSGDB_EXPORT std::string
                        convertStringFromCurrentCodePageToUTF8( const char* s );

    extern OSGDB_EXPORT std::string
                        convertStringFromUTF8toCurrentCodePage( const std::string& s );
    extern OSGDB_EXPORT std::string
                        convertStringFromUTF8toCurrentCodePage( const char* s );

}
