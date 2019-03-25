#include "AppSignalStateFlags.h"

void AppSignalStateFlags::clear()
{
	all = 0;
}

void AppSignalStateFlags::clearReasonsFlags()
{
	all &= ~MASK_ALL_ARCHIVING_REASONS;
}

bool AppSignalStateFlags::hasArchivingReason() const
{
	return (all & MASK_ALL_ARCHIVING_REASONS) != 0;
}

bool AppSignalStateFlags::hasShortTermArchivingReasonOnly() const
{
	quint32 archivingReasons = all & MASK_ALL_ARCHIVING_REASONS;

	return	(archivingReasons & ~MASK_SHORT_TERM_ARCHIVING_REASONE) == 0 &&
			(archivingReasons & MASK_SHORT_TERM_ARCHIVING_REASONE) != 0;
}

void AppSignalStateFlags::updateArchivingReasonFlags(const AppSignalStateFlags& prevFlags)
{
	validityChange = ((all ^ prevFlags.all) & MASK_VALIDITY_AND_AVAILABLE_FLAGS) == 0 ? 0 : 1;

	quint32 simLockUnblChange : 1;		//	17	any changes of simulated, locked, unbalanced flags
	quint32 limitFlagsChange : 1;		//	18	any changes of aboveHighLimit or belowLowLimit flags
	quint32 autoPoint : 1;				//	19
	quint32 fineAperture : 1;			//	20
	quint32 coarseAperture : 1;			//	21

}


