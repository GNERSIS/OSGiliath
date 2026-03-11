/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * RoadSegment, derived from PrimaryRecord.
 * Provides: META_Record, META_setID, META_setComment, META_setMultitexture,
 * META_addChild, META_dispose.
 */
//
// OpenFlight� loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include "Document.hpp"
#include "RecordInputStream.hpp"
#include "Registry.hpp"

#include <osg/core/Notify.hpp>
#include <osg/nodes/Group.hpp>

namespace flt
{

    /** RoadSegment
     */
    class RoadSegment : public PrimaryRecord
    {
            osg::ref_ptr<osg::Group> _roadSegment;

        public:

            RoadSegment()
            {
            }

        META_Record( RoadSegment ) META_setID( _roadSegment ) META_setComment(
            _roadSegment
        ) META_setMultitexture( _roadSegment ) META_addChild( _roadSegment ) META_dispose( _roadSegment ) protected
            : virtual ~RoadSegment()
            {
            }

            virtual void
            readRecord( RecordInputStream& in,
                        Document& /*document*/ )
            {
                _roadSegment   = new osg::Group;
                std::string id = in.readString( 8 );

                _roadSegment->setName( id );

                // Add to parent.
                if( _parent.valid() )
                {
                    _parent->addChild( *_roadSegment );
                }
            }
    };

    REGISTER_FLTRECORD( RoadSegment,
                        ROAD_SEGMENT_OP )

    /** RoadConstruction
     */
    class RoadConstruction : public PrimaryRecord
    {
            osg::ref_ptr<osg::Group> _roadConstruction;

        public:

            RoadConstruction()
            {
            }

        META_Record( RoadConstruction ) META_setID( _roadConstruction ) META_setComment( _roadConstruction ) META_setMultitexture(
            _roadConstruction
        ) META_addChild( _roadConstruction ) META_dispose( _roadConstruction ) protected
            : virtual ~RoadConstruction()
            {
            }

            virtual void
            readRecord( RecordInputStream& in,
                        Document& /*document*/ )
            {
                _roadConstruction = new osg::Group;

                std::string id    = in.readString( 8 );

                _roadConstruction->setName( id );

                // Add to parent.
                if( _parent.valid() )
                {
                    _parent->addChild( *_roadConstruction );
                }
            }
    };

    REGISTER_FLTRECORD( RoadConstruction,
                        ROAD_CONSTRUCTION_OP )

    /** RoadPath
     */
    class RoadPath : public PrimaryRecord
    {
            osg::ref_ptr<osg::Group> _roadPath;

        public:

            RoadPath()
            {
            }

        META_Record( RoadPath ) META_setID( _roadPath ) META_setComment(
            _roadPath
        ) META_setMultitexture( _roadPath ) META_addChild( _roadPath ) META_dispose( _roadPath ) protected
            : virtual ~RoadPath()
            {
            }

            virtual void
            readRecord( RecordInputStream& /*in*/,
                        Document& /*document*/ )
            {
                _roadPath = new osg::Group;

                // Add to parent.
                if( _parent.valid() )
                {
                    _parent->addChild( *_roadPath );
                }
            }
    };

    REGISTER_FLTRECORD( RoadPath,
                        ROAD_PATH_OP )

}    // end namespace
