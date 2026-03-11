/* OSGiliath — OpenSceneGraph fork. See LICENSE.txt.
 * OscPrintReceivedElements — osgPlugins library implementation.
 */
#pragma once

#include <iosfwd>

#ifndef INCLUDED_OSCRECEIVEDELEMENTS_H
    #include "OscReceivedElements.hpp"
#endif /* INCLUDED_OSCRECEIVEDELEMENTS_H */

namespace osc
{

    std::ostream&
    operator<<( std::ostream&         os,
                const ReceivedPacket& p );
    std::ostream&
    operator<<( std::ostream&                  os,
                const ReceivedMessageArgument& arg );
    std::ostream&
    operator<<( std::ostream&          os,
                const ReceivedMessage& m );
    std::ostream&
    operator<<( std::ostream&         os,
                const ReceivedBundle& b );

}    // namespace osc
