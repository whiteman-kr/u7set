#pragma once
#include <QString>

/// Interface for detecting if signal has a specified tag.
///
class ISignalHasTag
{
public:
	virtual ~ISignalHasTag() = default;

public:
	/// Return true if signal has tag (AppSignalID for app signals DiagSignalEquipmentID for diag signals).
	///
	virtual bool signalHasTag(const QString& signalId, const QString& tag) const = 0;
};
