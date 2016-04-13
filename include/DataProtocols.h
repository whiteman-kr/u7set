#ifndef DATAPROTOCOLS_H
#define DATAPROTOCOLS_H

#include "SocketIO.h"
#pragma pack(push, 2)

struct RupFrameFlags
{
	quint16 registration : 1;
	quint16 diagnostics : 1;
	quint16 control : 1;
	quint16 test : 1;
};


struct RupTimeStamp
{
	quint16 hour;			// 0..23
	quint16 Minute;			// 0..59
	quint16 Second;			// 0..59
	quint16 Millisecond;	// 0..999

	quint16 day;	// 1..31
	quint16 month;	// 1..12
	quint16 year;	// 1970..65535
};

struct RupFrameHeader
{
	quint16 packetSize;			// packet size including header, bytes
	quint16 protocolVersion;

	RupFrameFlags flags;

	quint32 moduleFactoryNo;
	quint16 moduleType;			// module type ID
	quint16 packetNo;
	quint16 partCount;			// >=1
	quint16 partNo;				// 0..(partCount-1)

	RupTimeStamp TimeStamp;
};


const int RUP_FRAME_DATA_SIZE = ENTIRE_UDP_SIZE - sizeof(RupFrameHeader) - sizeof(quint64/*CRC*/);
const int RUP_MAX_FRAME_COUNT = 10;
const int RUP_BUFFER_SIZE = RUP_MAX_FRAME_COUNT * RUP_FRAME_DATA_SIZE;

typedef quint8 RupFrameData[RUP_FRAME_DATA_SIZE];


struct RupFrame
{
	RupFrameHeader Header;

	RupFrameData Data;

	quint64 CRC64;			// = 1 + x + x^3 + x^4 + x^64
};

#pragma pack(pop)

#endif // DATAPROTOCOLS_H
