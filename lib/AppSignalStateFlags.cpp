#include "AppSignalStateFlags.h"

void AppSignalStateFlags::clear()
{
	all = 0;
}

void AppSignalStateFlags::clearReasonsFlags()
{
	validityChange = 0;
	autoPoint = 0;
	coarseAperture = 0;
	fineAperture = 0;
}

bool AppSignalStateFlags::hasArchivingReason() const
{
	return	validityChange == 1 ||
			autoPoint == 1 ||
			coarseAperture == 1 ||
			fineAperture == 1;
}

bool AppSignalStateFlags::hasShortTermArchivingReasonOnly() const
{
	return	validityChange == 0 &&
			autoPoint == 0 &&
			coarseAperture == 0 &&
			fineAperture == 1;
}

