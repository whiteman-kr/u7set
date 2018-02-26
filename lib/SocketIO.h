#pragma once

#include <QtGlobal>
#include <QDebug>
#include <QHostAddress>
#include <QAbstractSocket>

#include "../lib/BuildInfo.h"
#include "../lib/JsonSerializable.h"
#include "../lib/HostAddressPort.h"


const int MAX_DATAGRAM_SIZE = 4096;

const quint16   PORT_BASE_SERVICE = 13300,

				PORT_CONFIGURATION_SERVICE = 13310,
				PORT_CONFIGURATION_SERVICE_INFO = 13311,
				PORT_CONFIGURATION_SERVICE_REQUEST = 13312,

				PORT_APP_DATA_SERVICE = 13320,
				PORT_APP_DATA_SERVICE_INFO = 13321,
				PORT_APP_DATA_SERVICE_DATA = 13322,				// port receiving application data from LM's
				PORT_APP_DATA_SERVICE_CLIENT_REQUEST = 13323,

				PORT_TUNING_SERVICE = 13330,
				PORT_TUNING_SERVICE_DATA = 13332,
				PORT_TUNING_SERVICE_CLIENT_REQUEST = 13333,

				PORT_ARCHIVING_SERVICE = 13340,
				PORT_ARCHIVING_SERVICE_INFO = 13341,
				PORT_ARCHIVING_SERVICE_APP_DATA = 13342,
				PORT_ARCHIVING_SERVICE_DIAG_DATA = 13343,
				PORT_ARCHIVING_SERVICE_CLIENT_REQUEST = 13344,

				PORT_DIAG_DATA_SERVICE = 13350,
				PORT_DIAG_DATA_SERVICE_INFO = 13351,
				PORT_DIAG_DATA_SERVICE_DATA = 13352,				// port receiving diagnostics data from LM's
				PORT_DIAG_DATA_SERVICE_CLIENT_REQUEST = 13353;

// All services request IDs
//
const quint32   RQID_SERVICE_GET_INFO = 1000,

				RQID_INTRODUCE_MYSELF = 1050,

				RQID_SERVICE_START = 1100,
				RQID_SERVICE_STOP = 1101,
				RQID_SERVICE_RESTART = 1102,

				RQID_GET_CLIENT_LIST = 1105;

// Request error codes
//
const quint32	RQERROR_OK = 0,
				RQERROR_UNKNOWN_REQUEST = 1;


// --------------------------- Data structs used for parsing "Radiy" platform packets ------------------------
//

#pragma pack(push, 2)

const int ENTIRE_UDP_SIZE = 1472;

struct RpPacketFlags
{
	quint16 registration : 1;
	quint16 diagnostics : 1;
	quint16 control : 1;
	quint16 test : 1;
};


struct RpTimeStamp
{
	quint16 hour;			// 0..23
	quint16 Minute;			// 0..59
	quint16 Second;			// 0..59
	quint16 Millisecond;	// 0..999

	quint16 day;	// 1..31
	quint16 month;	// 1..12
	quint16 year;	// 1970..65535
};


struct RpDateStamp
{
	quint16 day;
	quint16 month;
	quint16 year;
};


struct RpPacketHeader
{
	quint16 packetSize;			// packet size including header, bytes
	quint16 protocolVersion;

	RpPacketFlags flags;

	quint32 moduleFactoryNo;
	quint16 moduleType;			// module type ID
	quint16 subblockID;			// = 16 less significant bits of subblock IP
	quint32 packetNo;
	quint16 partCount;			// >=1
	quint16 partNo;				// 0..(partCount-1)

	RpTimeStamp TimeStamp;
};


const int RP_PACKET_DATA_SIZE = 1428;
const int RP_MAX_FRAME_COUNT = 10;
const int RP_BUFFER_SIZE = RP_MAX_FRAME_COUNT * RP_PACKET_DATA_SIZE;


typedef quint8 RpPacketData[RP_PACKET_DATA_SIZE];

struct RpPacket
{
	RpPacketHeader Header;

	RpPacketData Data;

	quint64 CRC64;			// = 1 + x + x^3 + x^4 + x^64
};


struct RpDiagMsgHeader
{
	quint16 msgLen;					// message size including header
	quint16 blockPlace;
	quint16 blockType;
	quint16 version;
	quint32 factoryNo;
	RpDateStamp firmwareDate;
	RpDateStamp productionDate;
	quint16 firmwareVersion;
	quint16 plVersion;				// PL FPGA version
	quint16 comVersion;				// COM FPGA version
	quint32 plCRC;					// CRC PL FPGA
	quint32 comCRC;					// CRC COM FPGA
	quint16 reserv[6];
};


const quint16 NO_ACK_BLOCK_TYPE = 0;		// block type value if block didn't ack

#pragma pack(pop)


const quint32 CRC32_INITIAL_VALUE = 0xFFFFFFFF;

quint32 CRC32(quint32 initialValue, const char* buffer, int len, bool finishCalc);

quint32 CRC32(const char* buffer, int len);

quint32 CRC32(quint32 initialValue, const QString& str, bool finishCalc);

quint32 CRC32(quint32 initialValue, int value, bool finishCalc);

// ConfigurationService specific request IDs
//
const quint32	CFGS_GET_SERVICE_STATE = 0x1100,
				CFGS_GET_CLIENT_LIST = 0x1101,
				CFGS_GET_LOADED_BUILD_INFO = 0x1102,
				CFGS_GET_SETTINGS = 0x1103,
				CFGS_GET_LOG = 0x1104;						// Could be couple diferent queries


// AppSignal Param/State Communication, Port PORT_APP_DATA_SERVICE_CLIENT_REQUEST
//
const quint32 ADS_GET_APP_SIGNAL_LIST_START = 0x1200;
const quint32 ADS_GET_APP_SIGNAL_LIST_NEXT = 0x1201;

const quint32 ADS_GET_APP_SIGNAL_PARAM = 0x1301;
const quint32 ADS_GET_APP_SIGNAL = 0x1302;
const quint32 ADS_GET_APP_SIGNAL_STATE = 0x1303;

// Limiters and other constants
//
const int ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART = 1000;
const int ADS_GET_APP_SIGNAL_PARAM_MAX = 500;
const int ADS_GET_APP_SIGNAL_STATE_MAX = 2000;


// Data Sources info/state communication, Port PORT_APP_DATA_SERVICE_CLIENT_REQUEST
//
const quint32 ADS_GET_DATA_SOURCES_INFO = 0x1401;
const quint32 ADS_GET_DATA_SOURCES_STATES = 0x1402;

const int ADS_GET_DATA_SOURCES_STATES_MAX = 1000;


// Tuning Sources info/state communication, Port PORT_TUNING_SERVICE_CLIENT_REQUEST
//
const quint32 TDS_GET_TUNING_SOURCES_INFO = 0x1501;
const quint32 TDS_GET_TUNING_SOURCES_STATES = 0x1502;
const quint32 TDS_TUNING_SIGNALS_READ = 0x1503;
const quint32 TDS_TUNING_SIGNALS_WRITE = 0x1504;
const quint32 TDS_TUNING_SIGNALS_APPLY = 0x1505;
const quint32 TDS_CHANGE_CONTROLLED_TUNING_SOURCE = 0x1506;
const quint32 TDS_GET_TUNING_SERVICE_SETTINGS = 0x1507;

const int TDS_TUNING_MAX_READ_STATES = 1000;
const int TDS_TUNING_MAX_WRITE_RECORDS = 1000;

// ArchivingService and AppData Service communications, Port PORT_ARCHIVING_SERVICE_APP_DATA
//
const quint32 ARCHS_CONNECTION_ALIVE = 0x1601;
const quint32 ARCHS_SAVE_APP_SIGNALS_STATES = 0x1602;


// Monitor (and other clients) and ArchivingService communications, Port PORT_ARCHIVING_SERVICE_CLIENT_REQUEST
//
const quint32 ARCHS_GET_APP_SIGNALS_STATES_START = 0x1701;
const quint32 ARCHS_GET_APP_SIGNALS_STATES_NEXT = 0x1702;
const quint32 ARCHS_GET_APP_SIGNALS_STATES_CANCEL = 0x1703;

const int ARCH_REQUEST_MAX_SIGNALS = 32;
const int ARCH_REQUEST_MAX_STATES = 20000; //65000;

// Getting application signals Units
//
const quint32 ADS_GET_UNITS = 0x1500;

const int ADS_GET_DATA_UNITS_MAX = 1000;

const quint32 ADS_GET_STATE = 0x1600;
const quint32 ADS_GET_SETTINGS = 0x1800;

enum class NetworkError
{
	Success,
	WrongPartNo,
	RequestParamExceed,
	RequestStateExceed,
	ParseRequestError,
	RequestDataSourcesStatesExceed,
	UnitsExceed,
	UnknownTuningClientID,
	UnknownSignalHash,
	InternalError,
	ArchiveError,								// for detail information check archError field
	WrongTuningValueType,
	TuningValueOutOfRange,
	SingleLmControlDisabled,
	LmControlIsNotActive,
	ClientIsNotActive
};

enum class ArchiveError
{
	Success = 100,

	ArchRequestSignalsExceed,
	UnknownArchRequestID,
	PreviousArchRequestIsNotFinished,			// use ARCHS_GET_APP_SIGNALS_STATES_CANCEL to finish requests
	DbConnectionError,
	ExecQueryError,
	NoSignals,
	BuildQueryError,
};


extern QString getNetworkErrorStr(NetworkError err);
