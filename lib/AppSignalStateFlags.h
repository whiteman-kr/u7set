#pragma once

#include <QtGlobal>

union AppSignalStateFlags
{
	struct
	{
		quint32	valid : 1;

		// reasons to archiving
		//
		quint32 fineAperture : 1;
		quint32 coarseAperture : 1;
		quint32 autoPoint : 1;
		quint32 validityChange : 1;

		// The last flag (the 32nd bit) is RESERVED for use in trends
		//
	};

	quint32 all = 0;

	void clear();

	void clearReasonsFlags();

	bool hasArchivingReason() const;
	bool hasShortTermArchivingReasonOnly() const;
};


