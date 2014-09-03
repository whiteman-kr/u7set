#pragma once

#include <QtGlobal>


const quint32   RQSTP_BASE = 0,
                RQSTP_CONFIG = 1,
                RQSTP_FSC_AQUISION = 2,
                RQSTP_FSC_TUNING = 3,
                RQSTP_ARCHIVING = 4;


const quint16   PORT_BASE_SERVICE = 13300,
				PORT_CONFIG_SERVICE = 13301,
				PORT_FCS_AQUISION_SERVICE = 13302,
				PORT_FCS_TUNING_SERVICE = 13303,
				PORT_ARCHIVING_SERVICE = 13304;


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


struct ServiceTypeInfo
{
    quint32 serviceType;
    quint16 port;
    char* name;
};


const ServiceTypeInfo serviceTypesInfo[] =
{
    {RQSTP_BASE, PORT_BASE_SERVICE, "Base Service"},
    {RQSTP_CONFIG, PORT_CONFIG_SERVICE, "Configuration Service"},
    {RQSTP_FSC_AQUISION, PORT_FCS_AQUISION_SERVICE, "FSC Data Acquisition Service"},
    {RQSTP_FSC_TUNING, PORT_FCS_TUNING_SERVICE, "FSC Tuning Service"},
    {RQSTP_ARCHIVING, PORT_ARCHIVING_SERVICE, "Data Archiving Service"},
};

const int RQSTP_COUNT = sizeof(serviceTypesInfo) / sizeof(ServiceTypeInfo);


#pragma pack(push, 1)


struct RequestHeader
{
    quint32 id;
    quint32 clientID;
    quint32 version;
    quint32 no;
    quint32 errorCode;
    quint32 dataSize;
};


struct AckGetServiceInfo
{
    RequestHeader header;

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
