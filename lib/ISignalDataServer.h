#pragma once
#include "../CommonLib/Hash.h"

/// Interface for getting signal's data server, like AppDataServiceID or DiagDataServiceID.
///
class ISignalDataServer
{
public:
	virtual ~ISignalDataServer() = default;

public:
	/// Get AppDataServiceIDs or DiagDataServiceIDs list by AppSignalID for AppSignal or DiagSignalEquipmentID
	/// correspond.
	///
	virtual QStringList dataServiceIds(const QString& appSignalId) const = 0;

	/// Return true if AppDataService contains signal (AppSignalID for app signals DiagSignalEquipmentID for diag signals).
	///
	virtual bool dataServiceHasSignal(const QString& serviceEquipmentId, const QString& signalId) const = 0;
	virtual bool dataServiceHasSignal(const QString& serviceEquipmentId, Hash signalHash) const = 0;

	/// Get all signals for the specified DataServiceID (AppDataService or DiagDataService).
	///
	virtual std::vector<Hash> dataServiceSignals(const QString& serviceEquipmentId) const = 0;
};
