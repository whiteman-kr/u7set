#pragma once

#include "AdsBridgeResources.h"
#include "AdsbAppSignalManager.h"
#include <AdsBridge/Common.h>

#include <AdsConnectionLib/ServiceEndpoint.h>

#include <memory>
#include <string>
#include <vector>


class ILoggerStd;


namespace ClientLib
{
	class AdsConnection;
	class AppSignalManager;
} // namespace ClientLib


namespace AdsBridge
{
	class AdsBridgeFacade
	{
	public:
		explicit AdsBridgeFacade(Resources& res, ILoggerStd& log);
		~AdsBridgeFacade();

		AdsBridgeFacade(const AdsBridgeFacade&) = delete;
		AdsBridgeFacade(AdsBridgeFacade&&) = default;
		AdsBridgeFacade& operator=(const AdsBridgeFacade&) = delete;
		AdsBridgeFacade& operator=(AdsBridgeFacade&&) = default;

	public:
		bool setConfiguration(const std::vector<char>& data, const std::string& profile);
		void addAppDataService(const std::string& adsEquipmentId, const std::string& address, int port);
		void clearAppDataServices();

		void connect();
		void close();

	public:
		const std::string& equipmentId() const;
		void setEquipmentId(const std::string& equipmentId);

		bool signalParamsLoaded() const;
		bool signalStatesLoaded() const;

		size_t signalCount() const;
		bool signalList(MatsSignalHash* out, size_t count);

		size_t signalParams(size_t structSize, const MatsSignalHash* signalHashes, MatsAppSignalParam* out, size_t count);
		size_t signalStates(size_t structSize, const MatsSignalHash* signalHashes, MatsAppSignalState* out, size_t count);

		size_t connectionCount() const;
		bool connectionStatus(size_t structSize, struct AdsConnectionStatus* out, size_t count) const;

	private:
		Resources& m_res;
		ILoggerStd& m_log;
		std::unique_ptr<AdsBridge::AdsbAppSignalManager> m_signals;
		std::unique_ptr<ClientLib::AdsConnection> m_adsConnection;

		std::string m_equipmentId;
		std::vector<ServiceEndpoint> m_appDataServices;
	};
} // namespace AdsBridge