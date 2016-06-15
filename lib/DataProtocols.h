#pragma once

#include "SocketIO.h"

#pragma pack(push, 1)


union RupFrameFlags
{
	struct
	{
		quint16 appData : 1;
		quint16 diagData : 1;
		quint16 tuningData : 1;
		quint16 test : 1;
	};

	quint16 all;
};


struct RupTimeStamp
{
	quint16 hour;			// 0..23
	quint16 minute;			// 0..59
	quint16 second;			// 0..59
	quint16 millisecond;	// 0..999

	quint16 day;			// 1..31
	quint16 month;			// 1..12
	quint16 year;			// 1970..65535

	void reverseBytes();
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

	RupTimeStamp timeStamp;

	void reverseBytes();
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


union FotipHeaderFlags
{
	struct
	{
		quint16 successfulCheck : 1;
		quint16 successfulWrite : 1;
		quint16 dataTypeError : 1;
		quint16 operationCodeError : 1;
		quint16 startAddressError : 1;
		quint16 romSizeError : 1;
		quint16 romFrameSizeError : 1;
		quint16 frameSizeError : 1;
		quint16 versionError : 1;
		quint16 subsystemKeyError : 1;
		quint16 idError : 1;
	};

	quint16 all;
};


union FotipSubsystemKey
{
	struct
	{
		quint16 lmNumber : 6;
		quint16 subsystemCode : 6;

		quint16 crc : 4;	// CRC of previous twelve bits. CRC-4-ITU = x^4 + x + 1
	};

	quint16 wordVaue;
};


const int FOTIP_OPERATION_READ = 1200,
		  FOTIP_OPERATION_WRITE = 1400;

const int FOTIP_DATA_TYPE_SIGNED_INTEGER = 1300,
		  FOTIP_DATA_TYPE_FLOAT = 1500,
		  FOTIP_DATA_TYPE_IMMITATION_INTERLOCK = 1700;


const int FOTIP_HEADER_RESERVE_SIZE = 98;

struct FotipHeader
{
	quint16 protocolVersion;
	quint64 uniqueId;
	union
	{
		FotipSubsystemKey subsystemKey;
		quint16 subsystemKeyWord;
	};
	quint16 operationCode;

	FotipHeaderFlags flags;

	quint32 startAddress;
	quint16 fotipFrameSize;
	quint32 romSize;
	quint16 romFrameSize;
	quint16 dataType;
	quint8 reserve[FOTIP_HEADER_RESERVE_SIZE];
};


const int FOTIP_TX_RX_DATA_SIZE = 1016;
const int FOTIP_COMPARISON_RESULT_SIZE = 64;
const int FOTIP_DATA_RESERV_SIZE = 224;

struct FotipFrame
{
	FotipHeader header;

	char data[FOTIP_TX_RX_DATA_SIZE];

	char comparisonResult[FOTIP_COMPARISON_RESULT_SIZE];

	char reserv[FOTIP_DATA_RESERV_SIZE];
};


struct RupFotipFrame
{
	RupFrameHeader rupHeader;

	FotipFrame fotip;

	quint64 CRC64;			// = 1 + x + x^3 + x^4 + x^64
};

#pragma pack(pop)