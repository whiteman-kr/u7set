#pragma once

#include <QtCore>

// --------------------------- FCS Data stream formats ------------------------
//

#pragma pack(1)

const int FSC_PACKET_SIZE = 1472;

struct FSC_PACKET_FLAGS
{
	quint16 registration : 1;
	quint16 diagnostics : 1;
	quint16 control : 1;
	quint16 test : 1;
};


struct FSC_DATE_TIME_STAMP
{
	quint16 hour;						// hours 0..23
	quint16 minute;						// minutes 0..59
	quint16 second;						// seconds 0..59
	quint16 millisecond;				// milliseconds 0..999

	quint16 day;						// day 1..31
	quint16 month;						// month 1..12
	quint16 year;						// year 1970..65535
};


struct FSC_DATE_STAMP
{
	quint16 Day;
	quint16 Month;
	quint16 Year;
};


// FSC Packet header format
//
struct FSC_PACKET_HEADER
{
	quint16 packetSize;					// entire packet size with header, bytes
	quint16 protocolVersion;

	FSC_PACKET_FLAGS flags;

	quint32 moduleFactoryNo;
	quint16 moduleType;					// module type ID
	quint16 subblockID;					// subblock ID, lowest 16 bit from subblock IP
	quint32 packetNo;					// packet's serial number
	quint16 partCount;					// number of parts of the composite packet
	quint16 partNo;						// part number

	FSC_DATE_TIME_STAMP timeStamp;		// packet time stamp
};


// FSC Data packet format
//
const int FSC_PACKET_DATA_SIZE = FSC_PACKET_SIZE - sizeof(FSC_PACKET_HEADER) - sizeof(quint64);	//1428;


typedef quint8 FSC_PACKET_DATA[FSC_PACKET_DATA_SIZE];


struct FSC_PACKET
{
	FSC_PACKET_HEADER header;			// packet header

	FSC_PACKET_DATA data;				// FSC data

	quint64 CRC64;						// checksum, CRC64 = 1 + x + x^3 + x^4 + x^64
};


struct FSC_DIAG_MSG_HEADER
{
	quint16 msgLen;						// entire message lenght with header
	quint16 modulePlace;				// module place in subblock
	quint16 moduleType;
	quint16 version;
	quint32 factoryNo;
	FSC_DATE_STAMP firmwareDate;
	FSC_DATE_STAMP productionDate;
	quint16 firmwareVersion;
	quint16 plVersion;
	quint16 comVersion;
	quint32 plCRC;
	quint32 comCRC;
	quint16 Reserv[6];
};


const quint16 NO_ACK_MODULE_TYPE = 0;		// для блоков которые не ответили, в поле BlockType возвращается 0


#pragma pack()
