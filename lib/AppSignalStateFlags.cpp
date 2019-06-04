#include "AppSignalStateFlags.h"
#include <QString>

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

bool AppSignalStateFlags::hasAutoPointReasonOnly() const
{
	quint32 archivingReasons = all & MASK_ALL_ARCHIVING_REASONS;

	return	(archivingReasons & ~MASK_AUTO_POINT_REASONE) == 0 &&
			(archivingReasons & MASK_AUTO_POINT_REASONE) != 0;
}

void AppSignalStateFlags::updateArchivingReasonFlags(const AppSignalStateFlags& prevFlags)
{
	quint32 changedFlags = all ^ prevFlags.all;

	validityChange = (changedFlags & MASK_VALIDITY_AND_AVAILABLE_FLAGS) == 0 ? 0 : 1;
	simBlockMismatchChange = (changedFlags & MASK_SIM_BLOCK_UNBL_FLAGS) == 0 ? 0 : 1;
	limitFlagsChange = (changedFlags & MASK_LIMITS_FLAGS) == 0 ? 0 : 1;
}

QString AppSignalStateFlags::print()
{
	return QString("Valid=%1 Avail=%2 Sim=%3 Blk=%4 Unbl=%5 HLim=%6 LLim=%7 "
				   "[Reasons: ValCh=%8 SBUCh=%9 Lim=%10 Auto=%11 Fine=%12 Coarse=%13]").
			arg(valid).arg(stateAvailable).arg(simulated).arg(blocked).
			arg(mismatch).arg(aboveHighLimit).arg(belowLowLimit).
			arg(validityChange).arg(simBlockMismatchChange).arg(limitFlagsChange).
			arg(autoPoint).arg(fineAperture).arg(coarseAperture);
}
