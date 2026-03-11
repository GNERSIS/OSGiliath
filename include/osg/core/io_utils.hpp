/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Stream I/O operators for osg math types (Vec2, Vec3, Vec4,
 * Matrix, Quat, Plane). Used for debugging and text serialization.
 */
#pragma once

#include <istream>
#include <osg/maths/mat4.hpp>
#include <osg/maths/Plane.hpp>
#include <osg/maths/quat.hpp>
#include <osg/maths/vec2.hpp>
#include <osg/maths/vec3.hpp>
#include <osg/maths/vec4.hpp>
#include <ostream>
#include <sstream>

namespace osg
{

    /** Convinience class for building std::string using stringstream.
     * Usage:
     *    MakeString str;
     *    std::string s = str<<"Mix strings with numbers "<<0" ;
     *    std::string s2 = str.clear()<<"and other classes such as
     * ("<<osg::vec3(0.0,1.0,3.0)<<)" ; */
    class MakeString
    {
        public:

            MakeString()
            {
            }

            std::stringstream sstream;

            template<typename T>
            MakeString&
            operator<<( const T& t )
            {
                sstream << t;
                return *this;
            }

            MakeString&
            operator<<( std::ostream& ( *fun )( std::ostream& ))
            {
                sstream << fun;
                return *this;
            }

            inline MakeString&
            clear()
            {
                sstream.str( "" );
                return *this;
            }

            inline
            operator std::string() const
            {
                return sstream.str();
            }

            inline std::string
            str() const
            {
                return sstream.str();
            }
    };

    inline std::ostream&
    operator<<( std::ostream&     output,
                const MakeString& str )
    {
        output << str.str();
        return output;
    }

    //////////////////////////////////////////////////////////////////////////
    // vec2 streaming operators
    inline std::ostream&
    operator<<( std::ostream& output,
                const vec2&   vec )
    {
        output << vec.value[0] << " " << vec.value[1];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                vec2&         vec )
    {
        input >> vec.value[0] >> std::ws >> vec.value[1];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // dvec2 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const dvec2&  vec )
    {
        output << vec.value[0] << " " << vec.value[1];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                dvec2&        vec )
    {
        input >> vec.value[0] >> std::ws >> vec.value[1];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // vec3 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const vec3&   vec )
    {
        output << vec.value[0] << " " << vec.value[1] << " " << vec.value[2];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                vec3&         vec )
    {
        input >> vec.value[0] >> std::ws >> vec.value[1] >> std::ws >> vec.value[2];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // dvec3 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const dvec3&  vec )
    {
        output << vec.value[0] << " " << vec.value[1] << " " << vec.value[2];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                dvec3&        vec )
    {
        input >> vec.value[0] >> std::ws >> vec.value[1] >> std::ws >> vec.value[2];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // vec4 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const vec4&   vec )
    {
        output << vec.value[0] << " " << vec.value[1] << " " << vec.value[2] << " "
               << vec.value[3];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                vec4&         vec )
    {
        input >>
            vec.value[0] >>
            std::ws >>
            vec.value[1] >>
            std::ws >>
            vec.value[2] >>
            std::ws >>
            vec.value[3];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // dvec4 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const dvec4&  vec )
    {
        output << vec.value[0] << " " << vec.value[1] << " " << vec.value[2] << " "
               << vec.value[3];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                dvec4&        vec )
    {
        input >>
            vec.value[0] >>
            std::ws >>
            vec.value[1] >>
            std::ws >>
            vec.value[2] >>
            std::ws >>
            vec.value[3];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // bvec2 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const bvec2&  vec )
    {
        output << ( int )vec.value[0] << " " << ( int )vec.value[1];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                bvec2&        vec )
    {
        input >> vec.value[0] >> std::ws >> vec.value[1];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // bvec3 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const bvec3&  vec )
    {
        output << ( int )vec.value[0] << " " << ( int )vec.value[1] << " "
               << ( int )vec.value[2];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                bvec3&        vec )
    {
        input >> vec.value[0] >> std::ws >> vec.value[1] >> std::ws >> vec.value[2];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // bvec4 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const bvec4&  vec )
    {
        output << ( int )vec.value[0] << " " << ( int )vec.value[1] << " "
               << ( int )vec.value[2] << " " << ( int )vec.value[3];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                bvec4&        vec )
    {
        input >>
            vec.value[0] >>
            std::ws >>
            vec.value[1] >>
            std::ws >>
            vec.value[2] >>
            std::ws >>
            vec.value[3];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // svec2 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const svec2&  vec )
    {
        output << ( int )vec.value[0] << " " << ( int )vec.value[1];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                svec2&        vec )
    {
        input >> vec.value[0] >> std::ws >> vec.value[1];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // svec3 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const svec3&  vec )
    {
        output << ( int )vec.value[0] << " " << ( int )vec.value[1] << " "
               << ( int )vec.value[2];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                svec3&        vec )
    {
        input >> vec.value[0] >> std::ws >> vec.value[1] >> std::ws >> vec.value[2];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // svec4 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const svec4&  vec )
    {
        output << ( int )vec.value[0] << " " << ( int )vec.value[1] << " "
               << ( int )vec.value[2] << " " << ( int )vec.value[3];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                svec4&        vec )
    {
        input >>
            vec.value[0] >>
            std::ws >>
            vec.value[1] >>
            std::ws >>
            vec.value[2] >>
            std::ws >>
            vec.value[3];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // ivec2 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const ivec2&  vec )
    {
        output << vec.value[0] << " " << vec.value[1];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                ivec2&        vec )
    {
        input >> vec.value[0] >> std::ws >> vec.value[1];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // ivec3 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const ivec3&  vec )
    {
        output << vec.value[0] << " " << vec.value[1] << " " << vec.value[2];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                ivec3&        vec )
    {
        input >> vec.value[0] >> std::ws >> vec.value[1] >> std::ws >> vec.value[2];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // ivec4 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const ivec4&  vec )
    {
        output << vec.value[0] << " " << vec.value[1] << " " << vec.value[2] << " "
               << vec.value[3];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                ivec4&        vec )
    {
        input >>
            vec.value[0] >>
            std::ws >>
            vec.value[1] >>
            std::ws >>
            vec.value[2] >>
            std::ws >>
            vec.value[3];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // mat4 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& os,
                const mat4&   m )
    {
        os << "{" << std::endl;
        for( std::size_t row = 0; row < 4; ++row )
        {
            os << "\t";
            for( std::size_t col = 0; col < 4; ++col )
            {
                os << m( col, row ) << " ";
            }
            os << std::endl;
        }
        os << "}" << std::endl;
        return os;
    }

    //////////////////////////////////////////////////////////////////////////
    // dmat4 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& os,
                const dmat4&  m )
    {
        os << "{" << std::endl;
        for( std::size_t row = 0; row < 4; ++row )
        {
            os << "\t";
            for( std::size_t col = 0; col < 4; ++col )
            {
                os << m( col, row ) << " ";
            }
            os << std::endl;
        }
        os << "}" << std::endl;
        return os;
    }

    //////////////////////////////////////////////////////////////////////////
    // ubvec4 steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const ubvec4& vec )
    {
        output << ( int )vec.value[0] << " " << ( int )vec.value[1] << " "
               << ( int )vec.value[2] << " " << ( int )vec.value[3];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                ubvec4&       vec )
    {
        input >>
            vec.value[0] >>
            std::ws >>
            vec.value[1] >>
            std::ws >>
            vec.value[2] >>
            std::ws >>
            vec.value[3];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // quat steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const quat&   vec )
    {
        output << vec.value[0] << " " << vec.value[1] << " " << vec.value[2] << " "
               << vec.value[3];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                quat&         vec )
    {
        input >>
            vec.value[0] >>
            std::ws >>
            vec.value[1] >>
            std::ws >>
            vec.value[2] >>
            std::ws >>
            vec.value[3];
        return input;
    }

    //////////////////////////////////////////////////////////////////////////
    // Plane steaming operators.
    inline std::ostream&
    operator<<( std::ostream& output,
                const Plane&  pl )
    {
        output << pl[0] << " " << pl[1] << " " << pl[2] << " " << pl[3];
        return output;    // to enable cascading
    }

    inline std::istream&
    operator>>( std::istream& input,
                Plane&        vec )
    {
        input >> vec[0] >> std::ws >> vec[1] >> std::ws >> vec[2] >> std::ws >> vec[3];
        return input;
    }

}    // end of namespace osg
