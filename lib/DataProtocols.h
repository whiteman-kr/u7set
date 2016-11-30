#pragma once

#include "SocketIO.h"

#pragma pack(push, 1)

// ----------------------------------------------------------------------------
//
// RUP – Radiy UDP-based protocol description
//
// ----------------------------------------------------------------------------

namespace Rup
{
	const int VERSION = 5;

	union Flags
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


	struct TimeStamp
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


	struct Header
	{
		quint16 frameSize;			// frame size including header, bytes
		quint16 protocolVersion;

		Rup::Flags flags;

		quint32 dataId;
		quint16 moduleType;			// module ID
		quint16 numerator;
		quint16 framesQuantity;		// >=1
		quint16 frameNumber;		// 0..(frameQuantity-1)

		Rup::TimeStamp timeStamp;

		void reverseBytes();
	};


	const int FRAME_DATA_SIZE = ENTIRE_UDP_SIZE - sizeof(Rup::Header) - sizeof(quint64/*CRC64*/);
	const int MAX_FRAME_COUNT = 10;
	const int BUFFER_SIZE = MAX_FRAME_COUNT * FRAME_DATA_SIZE;

	typedef quint8 Data[FRAME_DATA_SIZE];


	struct Frame
	{
		Rup::Header header;

		Rup::Data data;

		quint64 CRC64;			// = 1 + x + x^3 + x^4 + x^64
	};
}

// ----------------------------------------------------------------------------
//
// FOTIP - Fiber Optic Tuning Interface data protocol V2 description
//
// ----------------------------------------------------------------------------

namespace FotipV2
{
	const int VERSION = 2;

	union HeaderFlags
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
			quint16 offsetError : 1;
			quint16 succesfulApply : 1;
			quint16 setSOR : 1;
		};

		quint16 all;
	};

	union SubsystemKey
	{
		struct
		{
			quint16 lmNumber : 6;
			quint16 subsystemCode : 6;

			quint16 crc : 4;	// CRC of previous twelve bits. CRC-4-ITU = x^4 + x + 1
		};

		quint16 wordVaue;
	};

	enum class OpCode
	{
		Read = 1200,
		Write = 1400,
		Apply = 1600
	};

	enum class DataType
	{
		AnalogSignedInt = 1300,
		AnalogFloat = 1500,
		Discrete = 1700
	};

	union ComparisonResult
	{
		struct
		{
			quint16	lowBoundOk : 1;
			quint16	highBoundOk : 1;
		};

		quint16 all;
	};

	const int HEADER_RESERVE_SIZE = 94;

	struct Header
	{
		quint16 protocolVersion;					// == 2
		quint64 uniqueId;

		FotipV2::SubsystemKey subsystemKey;

		quint16 operationCode;						// enum FotipOpCode values

		FotipV2::HeaderFlags flags;

		quint32 startAddress;
		quint16 fotipFrameSize;
		quint32 romSize;
		quint16 romFrameSize;
		quint16 dataType;							// enum FotipDataType values
		quint32 offsetInFrame;

		quint8 reserve[HEADER_RESERVE_SIZE];

		void reverseBytes();
	};

	const int TX_RX_DATA_SIZE = 1016;
	const int DATA_RESERV_SIZE = 286;

	struct Frame
	{
		FotipV2::Header header;

		char data[TX_RX_DATA_SIZE];

		ComparisonResult cmpResult;

		char reserv[DATA_RESERV_SIZE];
	};
}

struct RupFotipV2
{
	Rup::Header rupHeader;

	FotipV2::Frame fotipFrame;

	quint64 CRC64;			// = 1 + x + x^3 + x^4 + x^64
};


#pragma pack(pop)
