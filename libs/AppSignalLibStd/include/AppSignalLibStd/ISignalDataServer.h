#pragma once

#include <vector>

namespace ClientLib
{
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
		virtual std::vector<std::string> dataServiceIds(const std::string& appSignalId) const = 0;

		/// Return true if AppDataService contains signal (AppSignalID for app signals DiagSignalEquipmentID for diag signals).
		///
		virtual bool dataServiceHasSignal(const std::string& serviceEquipmentId, const std::string& signalId) const = 0;
		virtual bool dataServiceHasSignal(const std::string& serviceEquipmentId, Hash signalHash) const = 0;

		/// Get all signals for the specified DataServiceID (AppDataService or DiagDataService).
		///
		virtual std::vector<Hash> dataServiceSignals(const std::string& serviceEquipmentId) const = 0;
	};


	// Stub implementation of ISignalDataServer.
	//
	class SignalDataServerStub : public ISignalDataServer
	{
	public:
		std::vector<std::string> dataServiceIds([[maybe_unused]] const std::string& appSignalId) const override
		{
			std::vector<std::string> serviceIds;
			serviceIds.push_back("DummyServiceID1");
			serviceIds.push_back("DummyServiceID2");
			return serviceIds;
		}

		bool dataServiceHasSignal([[maybe_unused]] const std::string& serviceEquipmentId,
								  [[maybe_unused]] const std::string& signalId) const override
		{
			return true;
		}

		bool dataServiceHasSignal([[maybe_unused]] const std::string& serviceEquipmentId, [[maybe_unused]] Hash signalHash) const override
		{
			return true;
		}

		std::vector<Hash> dataServiceSignals([[maybe_unused]] const std::string& serviceEquipmentId) const override
		{
			std::vector<Hash> signalHashes;
			signalHashes.push_back(123);
			signalHashes.push_back(456);
			return signalHashes;
		}
	};
} // namespace ClientLib