#pragma once

#include <cstdint>

namespace adsgw
{
    constexpr uint16_t ADSGW_PROTOCOL_VERSION = 0x0100;

    enum GwErrorCode : uint16_t
    {
        GWC_SUCCESS = 0,
        GWC_INVALID_REQUEST = 1,
        GWC_UNSUPPORTED_VERSION = 2,
        GWC_NO_ADS_CONNECTION = 3,
        GWC_TOO_MANY_SIGNALS = 4,
        GWC_HANDSHAKE_REQUIRED = 5,
        GWC_INTERNAL_ERROR = 7,
        GWC_CRC_ERROR = 10
    };

    enum GwRequestId : uint32_t
    {
        ADSGW_HANDSHAKE = 0x0001,
        ADSGW_SIGNAL_LIST_START = 0x0100,
        ADSGW_SIGNAL_LIST_NEXT = 0x0101,
        ADSGW_SIGNAL_PARAM_START = 0x0200,
        ADSGW_SIGNAL_PARAM_NEXT = 0x0201,
        ADSGW_SIGNAL_STATE = 0x0300,
        ADSGW_SIGNAL_STATE_CHANGES = 0x0301
    };

    // Request ADSGW_HANDSHAKE
    //
    struct GwHandshakeRequest
    {
        uint16_t protocolVersion; // Protocol version client supports (e.g., 0x0100 for v1.0)
        char clientName[64];      // Null-terminated client name
    };

    static_assert(sizeof(GwHandshakeRequest) == 66);

    struct GwHandshakeResponse
    {
        uint16_t protocolVersion; // Server protocol version (must match request for success)
        uint16_t reserved;        // Reserved (must be 0)

        uint32_t maxStateRequest; // Max signal states per request (ADSGW_SIGNAL_STATE)

        // Structure size compatibility fields (bytes)
        uint32_t sizeof_GwAppSignalParam; // See Section 7.1
        uint32_t sizeof_GwAppSignalState; // See Section 7.2
    };

    static_assert(sizeof(GwHandshakeResponse) == 16);

} // namespace adsgw
