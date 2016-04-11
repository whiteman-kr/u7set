#pragma once

#include <QtCore>
#include "../include/SocketIO.h"
#include "../include/SimpleThread.h"

namespace Tuning
{
	struct TuningSettings
	{
		quint64 tuningID;
		quint16 subsystemKey;
		quint32 dataOffsetW;
		quint32 romSizeW;
		quint16 romFrameSizeW;
		quint16 romFrameCount;

		HostAddressPort tuningIP;
	};


	class TuningSocketWorker : public SimpleThreadWorker
	{
	private:
		static const int ROM_FRAME_SIZE_B = 1016;
		static const int ROM_FRAME_SIZE_W = ROM_FRAME_SIZE_B / 2;

		enum OperationCode
		{
			Read = 1200,
			Write = 1400,
		};

		enum DataType
		{
			AnalogSignedInt = 1300,
			AnalogFloat = 1500,
			Discrete = 1700
		};

	#pragma pack(push, 1)

		struct RequestHeader
		{
			quint16 version;				// current version 1
			quint64 tuningID;				// unique connection ID generated by RPCT
			quint16 subsystemKey;			// key of Subsystem assigned in RPCT Subsystems List Editor
			quint16 operationCode;			// OperationCode enum values
			quint16 flags;
			quint32 startAddressW;			//
			quint16 requestSizeB;			// UDP frame size = 1432 bytes
			quint32 romSizeB;				// = ROM_SIZE_B
			quint16 romFrameSizeB;			// = ROM_FRAME_SIZE_B
			quint16 dataType;				// DataType enum values
		};

		struct Request
		{
			RequestHeader header;

			char reserv[98];

			char data[ROM_FRAME_SIZE_B];

			char cmpResult[64];

			char reserv2[224];

			void initToRead(const TuningSettings& settings, quint64 tuningID, int frameNo);
		};

	#pragma pack(pop)

		TuningSettings m_settings;

		void readTunigData();

	public:
		TuningSocketWorker(const TuningSettings& settings);

	};


	class TuningSocket : public SimpleThread
	{
	public:
		TuningSocket(const TuningSettings& settings);
	};

}