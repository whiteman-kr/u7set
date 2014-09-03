#pragma once

#include <QtGlobal>

#pragma pack(push, 1)


const quint32   RQSTP_BASE = 0,
                RQSTP_CONFIG = 1,
                RQSTP_FSC_AQUISION = 2,
                RQSTP_FSC_TUNING = 3,
                RQSTP_ARCHIVING = 4;


const quint16   PORT_BASE_SERVICE = 4500,
                PORT_CONFIG_SERVICE = 4510,
                PORT_FCS_AQUISION_SERVICE = 4520,
                PORT_FCS_TUNING_SERVICE = 4530,
                PORT_ARCHIVING_SERVICE = 4540;


const quint32   RQID_GET_SERVICE_INFO = 1000,

                RQID_SERVICE_MF_START = 1100,
                RQID_SERVICE_MF_STOP = 1101,
                RQID_SERVICE_MF_RESTART = 1102,

                RQID_SEND_FILE_START = 1200,
                RQID_SEND_FILE_NEXT = 1201,
                RQID_SEND_FILE_CANCEL = 1202;


const quint32   SS_MF_STOPPED = 0,
                SS_MF_STARTS = 1,
                SS_MF_WORK = 2,
                SS_MF_STOPS = 3;


struct REQUEST_HEADER
{
    quint32 id;
    quint32 clientID;
    quint32 version;
    quint32 no;
    quint32 errorCode;
    quint32 dataSize;
};


struct ACK_GET_SERVICE_INFO
{
    REQUEST_HEADER Header;

    quint32 serviceType;                // RQSTP_* constants
    quint32 majorVersion;
    quint32 minorVersion;
    quint32 buildNo;
    quint32 crc;
    quint32 serviceUptime;
    quint32 state;                      // SS_MF_* constants
    quint32 serviceFunctionUptime;

};


#pragma pack(pop)
