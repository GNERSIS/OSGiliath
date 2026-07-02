/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * OpenFlight object record metadata. Stores attributes
 * from imported OpenFlight .flt database records.
 */
#pragma once

#include <osg/core/Inherit.hpp>
#include <osg/core/Object.hpp>
#include <ostream>

namespace osgSim
{

    /** When the OpenFlight importer encounters an Object record, it stores
        the data in one of these classes, and attaches the instance of the
        class as UserData to the corresponding osgLLGroup node.
    */

    class ObjectRecordData : public osg::Inherit<osg::Object, ObjectRecordData>
    {
        public:

            ObjectRecordData() :
                _flags( 0 ),
                _relativePriority( 0 ),
                _transparency( 0 ),
                _effectID1( 0 ),
                _effectID2( 0 ),

                _significance( 0 )
            {
            }

            ObjectRecordData( const ObjectRecordData& copy,
                              const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                Inherit( copy,
                         copyop )
            {
                _flags            = copy._flags;
                _relativePriority = copy._relativePriority;
                _transparency     = copy._transparency;
                _effectID1        = copy._effectID1;
                _effectID2        = copy._effectID2;
                _significance     = copy._significance;
            }

            OSG_REGISTER_TYPE( osgSim,
                               ObjectRecordData )

            static const unsigned int DONT_DISPLAY_IN_DAYLIGHT = 0X80'00'00'00U >> 0;
            static const unsigned int DONT_DISPLAY_AT_DUSK     = 0X80'00'00'00U >> 1;
            static const unsigned int DONT_DISPLAY_AT_NIGHT    = 0X80'00'00'00U >> 2;
            static const unsigned int DONT_ILLUMINATE          = 0X80'00'00'00U >> 3;
            static const unsigned int FLAT_SHADED              = 0X80'00'00'00U >> 4;
            static const unsigned int GROUPS_SHADOW_OBJECT     = 0X80'00'00'00U >> 5;

            unsigned int              _flags;
            short                     _relativePriority;
            unsigned short            _transparency;    // 0=opaque, 65535=totally clear
            short                     _effectID1;
            short                     _effectID2;
            short                     _significance;

    };    // end of class ObjectRecordData

}    // end of namespace osgSim
