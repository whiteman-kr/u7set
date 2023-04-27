#pragma once

namespace Gateway
{
	#pragma pack(push, 1)

	const quint8	DEV_CODE_A_NORMAL		= 0,		// normal
					DEV_CODE_A_NRG			= 1,		// exceeding the lower regulatory limit
					DEV_CODE_A_NAG			= 2,		// exceeding the lower emergency limit
					DEV_CODE_A_NVG			= 3,		// exceeding the lower possible limit
					DEV_CODE_A_VRG			= 4,		// exceeding the higher regulatory limit
					DEV_CODE_A_VAG			= 5,		// exceeding the higher emergency limit
					DEV_CODE_A_VVG			= 6,		// exceeding the higher possible limit
					DEV_CODE_A_NO_CONTROL	= 7;		// deviation is not controlled

	// analog signal state code, 'A' format
	//
	struct	AnalogStateCode_A
	{
		union
		{
			struct
			{
				quint8	deviationCode	: 3;		// DEV_CODE_A_* const values
				quint8	notValid		: 1;		// validity flag: valid - 0, NOT valid - 1,

				quint8	reserv			: 4;
			} flag;

			quint8 allFlags = 0;
		};
	};

	// analog signal state, 'A' format
	//
	//
	struct AnalogState_A
	{
		float value;
		AnalogStateCode_A stateCode;
	};

	// discrete signal state, 'D' format
	//
	struct DiscreteState_D
	{
		quint8 value		: 1;	// value of discrete signal, 0 or 1

		quint8 reserv1		: 2;

		quint8 notValid		: 1;	// validity flag: valid - 0, NOT valid - 1,

		quint8 reserv2		: 4;
	};

	// discrete signals packed states, 'B' format
	//
	struct DiscreteState_B
	{
		quint8 values;				// [] array of bytes contained packed values of discrete signals
									// first signal in list is plased in lower bit
									//
									// arraySizeB == int((valuesCount - 1) / 8) + 1

		quint8 notValids;			// [] corresponding, same sized, array of notValid flags of signals
	};

	struct IvsImpulsePacketHeader
	{
		// packet identification
		//
		quint8	systemID = 0;
		quint8	dataType = 0;			// char 'A', 'B', 'D'
		quint8	listID = 0;
		quint8	listVersion = 0;

		// params description
		//
		quint16	firstParamIndex = 0;	// in periodic state packets must be > 0
										// in events packets must be == 0

		quint16	paramCount = 0;			// params count in DataSection

		quint32	time = 0;				// packet time UTC+0, in seconds since 1 Jan 1970
	};

	const int IVS_IMPULSE_DATA_SECTION_MAX_SIZE = 16380;

	// current states periodic packet
	//
	struct IvsImpulseStatesPacket
	{
		IvsImpulsePacketHeader header;

		// DataSection begin, size <= DATA_SECTION_MAX_SIZE_B
		//
		union
		{
			union
			{
				// arrays of signal states in header pointed format - 'A' or 'D'
				// arraySize == header.paramCount
				//
				AnalogState_A states_A;

				// or

				DiscreteState_D states_D;
			};

			// or
			// discrete packed discrete signals states
			// valuesCount == header.paramCount
			//
			DiscreteState_B states_B;
		};

		// DataSection end
	};

	// signal event record
	//
	struct IvsImpulseSignalEvent
	{
		quint16 indexInList = 0;		// signal index in exchange list
		quint16 timeOffset = 0;			// relative time of event

		union
		{
			AnalogStateCode_A prevCode_A;
			DiscreteState_D	prevState_D;
		};

		union
		{
			AnalogStateCode_A newCode_A;
			DiscreteState_D	newState_D;
		};
	};

	struct IvsImpulseEventsPacket
	{
		IvsImpulsePacketHeader header;

		// DataSection begin, size <= DATA_SECTION_MAX_SIZE_B
		//
		IvsImpulseSignalEvent signalEvents;		// [] array of signals events
												// arraySize == header.paramCount
		// DataSection end

		quint16 packetNo = 0;
	};

	const int IVS_IMPULSE_PACKET_MAX_SIZE = sizeof(IvsImpulsePacketHeader) +
											IVS_IMPULSE_DATA_SECTION_MAX_SIZE +
											sizeof(quint16);

#pragma pack(pop)
}
