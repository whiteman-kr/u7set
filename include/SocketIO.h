#pragma once

#include <QtGlobal>
#include <QDebug>
#include <QHostAddress>
#include <QAbstractSocket>

#include "../include/BuildInfo.h"
#include "../include/JsonSerializable.h"


const int MAX_DATAGRAM_SIZE = 4096;


const quint16   PORT_BASE_SERVICE = 13300,

				PORT_CONFIGURATION_SERVICE = 13310,
				PORT_CONFIGURATION_SERVICE_INFO = 13311,
				PORT_CONFIGURATION_SERVICE_REQUEST = 13321,

				PORT_APP_DATA_SERVICE = 13320,
				PORT_APP_DATA_SERVICE_INFO = 13321,
				PORT_APP_DATA_SERVICE_DATA = 13322,				// port receiving application data from LM's
				PORT_APP_DATA_SERVICE_CLIENT_REQUEST = 13323,

				PORT_TUNING_SERVICE = 13330,
				PORT_TUNING_SERVICE_DATA = 13332,
				PORT_TUNING_SERVICE_CLIENT_REQUEST = 13333,

				PORT_ARCHIVING_SERVICE = 13340,
				PORT_ARCHIVING_SERVICE_INFO = 13341,
				PORT_ARCHIVING_SERVICE_REG_DATA = 13342,
				PORT_ARCHIVING_SERVICE_DIAG_DATA = 13343,
				PORT_ARCHIVING_SERVICE_CLIENT_REQUEST = 13344,

				PORT_DIAG_DATA_SERVICE = 13350,
				PORT_DIAG_DATA_SERVICE_INFO = 13351,
				PORT_DIAG_DATA_SERVICE_DATA = 13352,				// port receiving diagnostics data from LM's
				PORT_DIAG_DATA_SERVICE_CLIENT_REQUEST = 13353;


const quint16	PORT_DATA_AQUISITION = 13400;


// All services request IDs
//
const quint32   RQID_SERVICE_GET_INFO = 1000,

				RQID_SERVICE_START = 1100,
				RQID_SERVICE_STOP = 1101,
				RQID_SERVICE_RESTART = 1102,

				RQID_SERVICE_GET_SETTINGS = 1103,
				RQID_SERVICE_SET_SETTINGS = 1104,

				RQID_SEND_FILE_START = 1200,
				RQID_SEND_FILE_NEXT = 1201;


// DataAcquisitionService specific request IDs
//
const quint32	RQID_GET_DATA_SOURCES_IDS = 1250,
				RQID_GET_DATA_SOURCES_INFO = 1251,
				RQID_GET_DATA_SOURCES_STATISTICS = 1252;


// ConfigurationService specific request IDs
//
const quint32	RQID_GET_CONFIGURATION_SERVICE_INFO = 1300,
				RQID_GET_CONFIGURATION_SERVICE_SETTINGS = 1301,
				RQID_SET_CONFIGURATION_SERVICE_SETTINGS = 1302;




// Request error codes
//
const quint32	RQERROR_OK = 0,
				RQERROR_UNKNOWN_REQUEST = 1,
				RQERROR_UNKNOWN_FILE_ID = 2,
				RQERROR_RECEIVE_FILE = 3,				// file receive error on receiver side
				RQERROR_TIMEOUT = 4;					// request ack timeout




class HostAddressPort
{
private:
	QHostAddress m_hostAddress;
	quint16 m_port = 0;

public:
	HostAddressPort() {}

	explicit HostAddressPort(quint32 ip4Addr, quint16 port)
	{
		m_hostAddress.setAddress(ip4Addr);
		m_port = port;
	}

	explicit HostAddressPort(quint8 *ip6Addr, quint16 port)
	{
		m_hostAddress.setAddress(ip6Addr);
		m_port = port;
	}

	explicit HostAddressPort(const Q_IPV6ADDR &ip6Addr, quint16 port)
	{
		m_hostAddress.setAddress(ip6Addr);
		m_port = port;
	}

	explicit HostAddressPort(const sockaddr *sockaddr, quint16 port)
	{
		m_hostAddress.setAddress(sockaddr);
		m_port = port;
	}

	explicit HostAddressPort(const QString &address, quint16 port)
	{
		m_hostAddress.setAddress(address);
		m_port = port;
	}

	HostAddressPort(const HostAddressPort &copy)
	{
		m_hostAddress = copy.m_hostAddress;
		m_port = copy.m_port;
	}

	HostAddressPort &operator=(const HostAddressPort &other)
	{
		m_hostAddress = other.m_hostAddress;
		m_port = other.m_port;

		return *this;
	}

	void setAddress(quint32 ip4Addr)
	{
		m_hostAddress.setAddress(ip4Addr);
	}

	void setAddress(quint8 *ip6Addr)
	{
		m_hostAddress.setAddress(ip6Addr);
	}

	void setAddress(const Q_IPV6ADDR &ip6Addr)
	{
		m_hostAddress.setAddress(ip6Addr);
	}

	void setAddress(const sockaddr *sockaddr)
	{
		m_hostAddress.setAddress(sockaddr);
	}

	bool setAddress(const QString &address)
	{
		return m_hostAddress.setAddress(address);
	}

	void setPort(quint16 port)
	{
		m_port = port;
	}

	quint32 address32() const { return m_hostAddress.toIPv4Address(); }
	QHostAddress address() const { return m_hostAddress; }

	quint16 port() const { return m_port; }

	QString addressPortStr() const { return QString("%1:%2").arg(address().toString()).arg(port()); }
	QString addressStr() const { return QString("%1").arg(address().toString()); }
};


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



struct ServiceSettings
{
	quint32 cfgServiceIPAddress;
	quint32 cfgServicePort;
	char serviceStrID[256];
};


enum ServiceType
{
	BaseService,
	ConfigurationService,
	AppDataService,
	TuningService,
	ArchivingService,
	DiagDataService
};


enum ServiceState
{
	Stopped,
	Starts,
	Work,
	Stops,

	Undefined,			// this states used by 'Service Control Manager' only
	Unavailable,
};


struct ServiceInformation
{
	ServiceType type;
	quint32 majorVersion;
	quint32 minorVersion;
	quint32 buildNo;
	quint32 crc;
	quint64 uptime;
	ServiceState serviceState;
	quint32 serviceUptime;

	ServiceInformation()
	{
		memset(this, 0, sizeof(*this));
		serviceState = Undefined;
	}
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


// RQID_GET_CONFIGURATION_SERVICE_INFO reply format
//
class ConfigurationServiceInfo : public JsonSerializable
{
private:
	Builder::BuildInfo m_buildInfo;

	virtual void toJson(QJsonObject& jsonObject) const final;
	virtual bool fromJson(const QJsonObject& jsonObject, int version) final;

public:
	Builder::BuildInfo buildInfo() { return m_buildInfo; }
};


// RQID_SET_CONFIGURATION_SERVICE_SETTINGS request format &
// RQID_GET_CONFIGURATION_SERVICE_SETTINGS reply format
//
class ConfigurationServiceSettings : public JsonSerializable
{
private:
	quint32 m_cfgRequestAddress = 0;
	int m_cfgRequestPort = 0;

	QString m_buildFolder;

	virtual void toJson(QJsonObject& jsonObject) const final;
	virtual bool fromJson(const QJsonObject& jsonObject, int version) final;

public:
	quint32 cfgRequestAddress() { return m_cfgRequestAddress; }
	QString cfgRequestAddressStr() { return QHostAddress(m_cfgRequestAddress).toString(); }
	void setCfgRequestAddress(quint32 cfgRequestAddress) { m_cfgRequestAddress = cfgRequestAddress; }

	int cfgRequestPort() { return m_cfgRequestPort; }
	void setCfgRequestPort(int cfgRequestPort) { m_cfgRequestPort = cfgRequestPort; }

	QString buildFolder() { return m_buildFolder; }
	void setBuildFolder(const QString& buildFolder) { m_buildFolder = buildFolder; }
};






// Serialization framework
//
#define BEGIN_SERIALIZATION() char* _ptr = buffer;

#define SERIALIZE_VAR(variable_type, variable) { if (write) { *((variable_type*)_ptr) = variable; } else { variable = *((variable_type*)_ptr);} _ptr += sizeof(variable_type); }

#define SERIALIZE_ARRAY(variable_type, variable, len) { if (write) { memcpy(_ptr, variable, sizeof(variable_type)*len); } else { memcpy(variable, _ptr, sizeof(variable_type)*len);} _ptr += sizeof(variable_type)*len; }

#define END_SERIALIZATION() if (write) { *(buffer - sizeof(quint16)) = addSize(_ptr - buffer); } else { Q_ASSERT(_ptr - buffer == userDataSize()); _ptr = buffer + userDataSize(); } return _ptr;


struct Serializable
{
	Serializable(quint16 version) :
		m_structureVersion(version),
		m_structureSize(0) {}

	char* serializeTo(char* buffer)
	{
		return serialize(serializeVersion(buffer, true), true);
	}

	char* serializeFrom(char* buffer)
	{
		return serialize(serializeVersion(buffer, false), false);
	}

	quint16 version() { return m_structureVersion; }
	quint16 size() { return m_structureSize; }
	quint16 userDataSize() { return m_structureSize - sizeof(quint16) * 2; }

	static void copyStringToBuffer(const QString& str, quint16* buffer, quint32 bufferSize);
	static void copyBufferToString(const quint16* buffer, QString& str);

private:
	quint16 m_structureVersion;
	quint16 m_structureSize;			// full size of structure derived from Serializable,
										// = sizeof(m_structureVersion & m_structureSize fields) + sizeof(user data)

	char* serializeVersion(char* buffer, bool write);

protected:
	virtual char* serialize(char* buffer, bool write) = 0;
	void setSize(quint16 size) { m_structureSize = size; }
	quint16 addSize(quint16 size) { return (m_structureSize += size); }
};



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



#pragma pack(pop)


const quint32 CRC32_INITIAL_VALUE = 0xFFFFFFFF;

quint32 CRC32(quint32 initialValue, const char* buffer, int len, bool finishCalc);

quint32 CRC32(const char* buffer, int len);

