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
	quint32 changedFlags = all ^ prevFlags.all;

	validityChange = (changedFlags & MASK_VALIDITY_AND_AVAILABLE_FLAGS) == 0 ? 0 : 1;
	simLockUnblChange = (changedFlags & MASK_SIM_LOCK_UNBL_FLAGS) == 0 ? 0 : 1;
	limitFlagsChange = (changedFlags & MASK_LIMITS_FLAGS) == 0 ? 0 : 1;
}


