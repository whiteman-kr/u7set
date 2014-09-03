#include "../include/SocketIO.h"

serviceTypeInfo serviceTypesInfo[RQSTP_COUNT] =
{
    {RQSTP_BASE, PORT_BASE_SERVICE, "Base Service"},
    {RQSTP_CONFIG, PORT_CONFIG_SERVICE, "Configuration Service"},
    {RQSTP_FSC_AQUISION, PORT_FCS_AQUISION_SERVICE, "FSC Data Acquisition Service"},
    {RQSTP_FSC_TUNING, PORT_FCS_TUNING_SERVICE, "FSC Tuning Service"},
    {RQSTP_ARCHIVING, PORT_ARCHIVING_SERVICE, "Data Archiving Service"},
};
