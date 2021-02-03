#pragma once

#include <QtGlobal>
#include <type_traits>
#include <QHash>

#include "Types.h"

#pragma pack(push, 1)

union AppSignalStateFlags
{
	struct
	{
		// signal state flags
		//
		quint32	valid : 1;					//	0	this flag is set according to validity signal of signal
											//		if validity signal isn't exists - validity flag is equal to stateAvailable flag

		quint32 stateAvailable : 1;			//	1	sets to 1 if application data received from LM
											//		if no data available from LM this flag sets to 0

		quint32 simulated : 1;				//	2	sets according to simulation signal of signal (see AFB sim_lock)
		quint32 blocked : 1;				//	3	sets according to blocking signal of signal (see AFB sim_lock)
		quint32 mismatch : 1;				//	4	sets according to mismatch signal of signal (see AFB mismatch)

		quint32 aboveHighLimit: 1;			//	5	sets to 1 if value of signal is greate than HighEngineeringUnits limit
		quint32 belowLowLimit: 1;			//	6	sets to 1 if value of signal is less than LowEngineeringUnits limit

		quint32 swSimulated : 1;			//  7	state of signal is acquire from software simulated packet

		// bits reserved for state flags
		//
		quint32 _bit8: 1;					//	8
		quint32 _bit9: 1;					//	9
		quint32 _bit10: 1;					//	10
		quint32 _bit11: 1;					//	11
		quint32 _bit12: 1;					//	12
		quint32 _bit13: 1;					//	13
		quint32 _bit14: 1;					//	14
		quint32 _bit15: 1;					//	15

		// archiving reasons flags
		//
		quint32 validityChange : 1;			//	16	any changes of valid or stateAvailable flags
		quint32 simBlockMismatchChange : 1;	//	17	any changes of simulated, blocked, mismatch flags
		quint32 limitFlagsChange : 1;		//	18	any changes of aboveHighLimit or belowLowLimit flags
		quint32 autoPoint : 1;				//	19
		quint32 fineAperture : 1;			//	20
		quint32 coarseAperture : 1;			//	21

		// bits reserved for archiving reasons flags
		//
		quint32 _bit22 : 1;					//	22
		quint32 _bit23 : 1;					//	23
		quint32 _bit24 : 1;					//	24
		quint32 _bit25 : 1;					//	25
		quint32 _bit26 : 1;					//	26
		quint32 _bit27 : 1;					//	27
		quint32 _bit28 : 1;					//	28
		quint32 _bit29 : 1;					//	29
		quint32 _bit30 : 1;					//	30

		quint32 realtimePoint: 1;			//	31	special flag for real time trends displaying
	};

	quint32 all = 0;

	void setFlag(E::AppSignalStateFlagType flagType, quint32 value);

	void clear();
	void clearReasonsFlags();

	bool hasArchivingReason() const;
	bool hasShortTermArchivingReasonOnly() const;
	bool hasAutoPointReasonOnly() const;

	void updateArchivingReasonFlags(const AppSignalStateFlags& prevFlags);

	QString print();

	static const quint32 MASK_VALIDITY_AND_AVAILABLE_FLAGS = 0x00000003;
	static const quint32 MASK_SIM_BLOCK_UNBL_FLAGS = 0x0000001C;
	static const quint32 MASK_LIMITS_FLAGS = 0x00000060;
	static const quint32 MASK_ALL_ARCHIVING_REASONS = 0x003F0000;
	static const quint32 MASK_SHORT_TERM_ARCHIVING_REASONE = 0x00100000;		// for now this is fineAperture flag only
	static const quint32 MASK_AUTO_POINT_REASONE = 0x00080000;
};

#pragma pack(pop)

typedef QHash<E::AppSignalStateFlagType, QString> AppSignalStateFlagsMap;
