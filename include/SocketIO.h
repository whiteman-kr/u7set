#pragma once

#include <QtGlobal>


const int MAX_DATAGRAM_SIZE = 4096;


const quint32   STP_BASE = 0,
				STP_CONFIG = 1,
				STP_FSC_AQUISION = 2,
				STP_FSC_TUNING = 3,
				STP_ARCHIVING = 4;


const quint16   PORT_BASE_SERVICE = 13300,
				PORT_CONFIG_SERVICE = 13310,
				PORT_FCS_AQUISION_SERVICE = 13320,
				PORT_FCS_TUNING_SERVICE = 13330,
				PORT_ARCHIVING_SERVICE = 13340;


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


// Request error codes
const quint32	RQERROR_OK = 0,
				RQERROR_UNKNOWN = 1;


struct ServiceTypeInfo
{
    quint32 serviceType;
    quint16 port;
    char* name;
};


const ServiceTypeInfo serviceTypesInfo[] =
{
	{STP_BASE, PORT_BASE_SERVICE, "Base Service"},
	{STP_CONFIG, PORT_CONFIG_SERVICE, "Configuration Service"},
	{STP_FSC_AQUISION, PORT_FCS_AQUISION_SERVICE, "FSC Data Acquisition Service"},
	{STP_FSC_TUNING, PORT_FCS_TUNING_SERVICE, "FSC Tuning Service"},
	{STP_ARCHIVING, PORT_ARCHIVING_SERVICE, "Data Archiving Service"},
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


struct ServiceInformation
{
	quint32 type;						// RQSTP_* constants
	quint32 majorVersion;
	quint32 minorVersion;
	quint32 buildNo;
	quint32 crc;
	quint32 uptime;
	quint32 mainFunctionState;           // SS_MF_* constants
	quint32 mainFunctionUptime;
};


struct AckGetServiceInfo
{
    RequestHeader header;

	ServiceInformation serviceInfo;
};


// RQID_SEND_FILE_START request data format
//
struct SendFileStart
{
	ushort fileName[64];				// unicode 0-terminated string
	quint32 fileSize;
};


// RQID_SEND_FILE_NEXT request data format
//
const int SEND_FILE_DATA_SIZE = MAX_DATAGRAM_SIZE - sizeof(RequestHeader) - 5 * sizeof(quint32);

const int SEND_FILE_MAX_SIZE = 1024 * 1024 * 10;	// max file - 10 MBytes

struct SendFileNext
{
	quint32 fileID;
	quint32 partNo;
	quint32 partCount;
	quint32 dataSize;
	quint32 CRC32;

	char data[SEND_FILE_DATA_SIZE];
};


#pragma pack(pop)

const quint32 CRC32_INITIAL_VALUE = 0xFFFFFFFF;

quint32 CRC32(quint32 initialValue, const char* buffer, int len, bool finishCalc);

quint32 CRC32(const char* buffer, int len);
