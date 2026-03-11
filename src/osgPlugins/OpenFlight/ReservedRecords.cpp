/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * Utility functions for osgPlugins: REGISTER_FLTRECORD.
 */
//
// OpenFlight� loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include "Registry.hpp"

using namespace flt;

// Prevent "unknown record" message for the following reserved records:
REGISTER_FLTRECORD( DummyRecord,
                    103 )
REGISTER_FLTRECORD( DummyRecord,
                    104 )
REGISTER_FLTRECORD( DummyRecord,
                    117 )
REGISTER_FLTRECORD( DummyRecord,
                    118 )
REGISTER_FLTRECORD( DummyRecord,
                    120 )
REGISTER_FLTRECORD( DummyRecord,
                    121 )
REGISTER_FLTRECORD( DummyRecord,
                    124 )
REGISTER_FLTRECORD( DummyRecord,
                    125 )
