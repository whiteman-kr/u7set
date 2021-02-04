#include "AppSignalStateFlags.h"
#include <QString>

// Sets the flag to 1 if 'on' is true, otherwise clears the flag
//
void AppSignalStateFlags::setFlag(E::AppSignalStateFlagType flagType, quint32 value)
{
	// set flagValue to corresponding flag
	//
	switch (flagType)
	{
	case E::AppSignalStateFlagType::Validity:
		valid = value;
		return;

	case E::AppSignalStateFlagType::StateAvailable:
		stateAvailable = value;
		return;

	case E::AppSignalStateFlagType::Simulated:
		simulated  = value;
		return;

	case E::AppSignalStateFlagType::Blocked:
		blocked = value;
		return;

	case E::AppSignalStateFlagType::Mismatch:
		mismatch = value;
		return;

	case E::AppSignalStateFlagType::AboveHighLimit:
		aboveHighLimit = value;
		return;

	case E::AppSignalStateFlagType::BelowLowLimit:
		belowLowLimit = value;
		return;

	case E::AppSignalStateFlagType::SwSimulated:
		swSimulated = value;
		return;
	}

	Q_ASSERT(false);
	return;
}

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
	return QString("Valid=%1 Avail=%2 Sim=%3 Blk=%4 Unbl=%5 HLim=%6 LLim=%7 SwSim=%8 "
				   "[Reasons: ValCh=%9 SBUCh=%10 Lim=%11 Auto=%12 Fine=%13 Coarse=%14]").
			arg(valid).arg(stateAvailable).arg(simulated).arg(blocked).
			arg(mismatch).arg(aboveHighLimit).arg(belowLowLimit).arg(swSimulated).
			arg(validityChange).arg(simBlockMismatchChange).arg(limitFlagsChange).
			arg(autoPoint).arg(fineAperture).arg(coarseAperture);
}
