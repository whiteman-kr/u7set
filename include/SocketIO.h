#pragma once

#include <QtGlobal>
#include <QDebug>
#include <QHostAddress>


const int MAX_DATAGRAM_SIZE = 4096;


const quint32   STP_BASE = 0,
				STP_CONFIG = 1,
				STP_FSC_ACQUISITION = 2,
				STP_FSC_TUNING = 3,
				STP_ARCHIVING = 4;


const char* const serviceTypeStr[] =
{
	"Base Service",
	"Configuration Service",
	"FSC Data Acquisition Service",
	"FSC Tuning Service",
	"Data Archiving Service"
};


const quint32	SERVICE_TYPE_COUNT = sizeof(serviceTypeStr) / sizeof(const char*);


const quint16   PORT_BASE_SERVICE = 13300,

				PORT_CONFIG_SERVICE = 13310,

				PORT_DATA_AQUISITION_SERVICE = 13320,
				PORT_DATA_AQUISITION_SERVICE_INFO = 13321,
				PORT_DATA_AQUISITION_SERVICE_REG_DATA = 13322,			// port receiving registration data from LM's
				PORT_DATA_AQUISITION_SERVICE_DIAG_DATA = 13323,			// port receiving diagnostics data from LM's
				PORT_DATA_AQUISITION_SERVICE_CLIENT_REQUEST = 13324,

				PORT_FCS_TUNING_SERVICE = 13330,
				PORT_FCS_TUNING_SERVICE_DATA = 13332,

				PORT_ARCHIVING_SERVICE = 13340,
				PORT_ARCHIVING_SERVICE_INFO = 13341,
				PORT_ARCHIVING_SERVICE_REG_DATA = 13342,
				PORT_ARCHIVING_SERVICE_DIAG_DATA = 13343,
				PORT_ARCHIVING_SERVICE_CLIENT_REQUEST = 13344;


const quint16	PORT_DATA_AQUISITION = 13400;


const quint32   RQID_GET_SERVICE_INFO = 1000,

                RQID_SERVICE_MF_START = 1100,
                RQID_SERVICE_MF_STOP = 1101,
                RQID_SERVICE_MF_RESTART = 1102,

                RQID_SEND_FILE_START = 1200,
				RQID_SEND_FILE_NEXT = 1201,

				RQID_GET_DATA_SOURCES_IDS = 1250,
				RQID_GET_DATA_SOURCES_INFO = 1251,
				RQID_GET_DATA_SOURCES_STATISTICS = 1252;


const quint32   SS_MF_STOPPED = 0,
                SS_MF_STARTS = 1,
                SS_MF_WORK = 2,
                SS_MF_STOPS = 3;


// Request error codes
//
const quint32	RQERROR_OK = 0,
				RQERROR_UNKNOWN_REQUEST = 1,
				RQERROR_UNKNOWN_FILE_ID = 2,
				RQERROR_RECEIVE_FILE = 3,				// file receive error on receiver side
				RQERROR_TIMEOUT = 4;					// request ack timeout


struct ServiceTypeInfo
{
    quint32 serviceType;
    quint16 port;
	const char* name;
};


const ServiceTypeInfo serviceTypesInfo[] =
{
	{ STP_BASE, PORT_BASE_SERVICE, serviceTypeStr[STP_BASE] },
	{ STP_CONFIG, PORT_CONFIG_SERVICE, serviceTypeStr[STP_CONFIG]},
	{ STP_FSC_ACQUISITION, PORT_DATA_AQUISITION_SERVICE, serviceTypeStr[STP_FSC_ACQUISITION]},
	{ STP_FSC_TUNING, PORT_FCS_TUNING_SERVICE, serviceTypeStr[STP_FSC_TUNING]},
	{ STP_ARCHIVING, PORT_ARCHIVING_SERVICE, serviceTypeStr[STP_ARCHIVING]},
};


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




#pragma pack(pop)


const quint32 CRC32_INITIAL_VALUE = 0xFFFFFFFF;

quint32 CRC32(quint32 initialValue, const char* buffer, int len, bool finishCalc);

quint32 CRC32(const char* buffer, int len);

