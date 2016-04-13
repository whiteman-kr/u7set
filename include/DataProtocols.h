#pragma once

#include "SocketIO.h"
#pragma pack(push, 2)

struct RupFrameFlags
{
	quint16 appData : 1;
	quint16 diagData : 1;
	quint16 tuningData : 1;
	quint16 test : 1;
};


struct RupTimeStamp
{
	quint16 hour;			// 0..23
	quint16 minute;			// 0..59
	quint16 second;			// 0..59
	quint16 millisecond;	// 0..999

	quint16 day;	// 1..31
	quint16 month;	// 1..12
	quint16 year;	// 1970..65535
};

struct RupFrameHeader
{
	quint16 frameSize;			// frame size including header, bytes
	quint16 protocolVersion;

	RupFrameFlags flags;

	quint32 dataId;
	quint16 moduleType;			// module ID
	quint16 numerator;
	quint16 framesQuantity;		// >=1
	quint16 frameNumber;		// 0..(frameQuantity-1)

	RupTimeStamp TimeStamp;
};


const int RUP_FRAME_DATA_SIZE = ENTIRE_UDP_SIZE - sizeof(RupFrameHeader) - sizeof(quint64/*CRC64*/);
const int RUP_MAX_FRAME_COUNT = 10;
const int RUP_BUFFER_SIZE = RUP_MAX_FRAME_COUNT * RUP_FRAME_DATA_SIZE;

typedef quint8 RupFrameData[RUP_FRAME_DATA_SIZE];


struct RupFrame
{
	RupFrameHeader header;

	RupFrameData data;

	quint64 CRC64;			// = 1 + x + x^3 + x^4 + x^64
};

#pragma pack(pop)
