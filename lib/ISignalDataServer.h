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


// Stub implementation of ISignalDataServer.
//
class SignalDataServerStub : public ISignalDataServer
{
public:
    QStringList dataServiceIds(const QString& appSignalId) const override
    {
        QStringList serviceIds;
        serviceIds << "DummyServiceID1" << "DummyServiceID2";
        return serviceIds;
    }

    bool dataServiceHasSignal(const QString& serviceEquipmentId, const QString& signalId) const override
    {
        Q_UNUSED(serviceEquipmentId);
        Q_UNUSED(signalId);
        return true;
    }

    bool dataServiceHasSignal(const QString& serviceEquipmentId, Hash signalHash) const override
    {
        Q_UNUSED(serviceEquipmentId);
        Q_UNUSED(signalHash);
        return true;
    }

    std::vector<Hash> dataServiceSignals(const QString& serviceEquipmentId) const override
    {
        std::vector<Hash> signalHashes;
        signalHashes.push_back(123);
        signalHashes.push_back(456);
        return signalHashes;
    }
};