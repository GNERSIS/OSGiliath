INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR})

SET(TARGET_SRC
    ip/IpEndpointName.cpp
    osc/OscOutboundPacketStream.cpp
    osc/OscPrintReceivedElements.cpp
    osc/OscReceivedElements.cpp
    osc/OscTypes.cpp
    OscReceivingDevice.cpp
    OscSendingDevice.cpp
    ReaderWriterOscDevice.cpp
)

SET(TARGET_H
    ip/IpEndpointName.hpp
    ip/NetworkingUtils.hpp
    ip/PacketListener.hpp
    ip/TimerListener.hpp
    ip/UdpSocket.hpp
    osc/MessageMappingOscPacketListener.hpp
    osc/OscException.hpp
    osc/OscHostEndianness.hpp
    osc/OscOutboundPacketStream.hpp
    osc/OscPacketListener.hpp
    osc/OscPrintReceivedElements.hpp
    osc/OscReceivedElements.hpp
    osc/OscTypes.hpp
    OscReceivingDevice.hpp
    OscSendingDevice.hpp
)

if(WIN32 AND NOT ANDROID)
    ADD_DEFINITIONS(-D_WINSOCK_DEPRECATED_NO_WARNINGS)

    SET(TARGET_SRC
        ${TARGET_SRC}
        ip/win32/NetworkingUtils.cpp
        ip/win32/UdpSocket.cpp
    )
    SET(TARGET_EXTERNAL_LIBRARIES "${TARGET_EXTERNAL_LIBRARIES};ws2_32.lib;winmm")
ELSE()
    SET(TARGET_SRC
        ${TARGET_SRC}
        ip/posix/NetworkingUtils.cpp
        ip/posix/UdpSocket.cpp
    )
ENDIF()

INCLUDE_DIRECTORIES(${OSGiliath_SOURCE_DIR}/src/osgPlugins/osc)

SET(TARGET_ADDED_LIBRARIES osgGA )

#### end var setup  ###
SETUP_PLUGIN(osc)
