#pragma once

#include <QtGlobal>

#pragma pack(push, 1)

const int RQSTP_CONFIG = 1,
    RQSTP_FSC_AQUISION = 2,
    RQSTP_FSC_TUNING = 3,
    RQSTP_ARCHIVING = 4;

const int RQID_GET_SERVICE_INFO = 1000,
    RQID_GET_SERVICE_STATE = 1001,
    RQID_SEND_FILE_START = 1100,
    RQID_SEND_FILE_NEXT = 1101,
    RQID_SEND_FILE_CANCEL = 1102,
    RQID_SERVICE_START = 1200,
    RQID_SERVICE_STOP = 1201,
    RQID_SERVICE_RESTART = 1202;

struct REQUEST_HEADER
{
    quint32 ID;
    quint32 ClientID;
    quint32 Version;
    quint32 No;
    quint32 ErrorCode;
    quint32 DataLen;
};


#pragma pack(pop)
