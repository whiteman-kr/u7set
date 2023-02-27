#pragma once

/// Interface for getting signal's data server, like AppDtataServiceID or DiagDataServiceID.
///
class ISignalDataServer
{
public:
	virtual ~ISignalDataServer() = default;

public:
	/// Get AppDataServiceIDs or DiagDataServiceIDs list by AppSignalID for AppSignal or DiagSignalEquipmentID
	/// correspondly.
	///
	virtual QStringList dataServiceIds(const QString& appSignalId) const = 0;

	/// Return true if AppDataService contains signal (AppSignalID for app signals DiagSignalEquipmentID for diag signals).
	///
	virtual bool dataServiceHasSignal(const QString& serviceEquipmentId, const QString& signalId) const = 0;
};
