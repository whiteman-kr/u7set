#pragma once

#include <AdsBridge/Common.h>

#include "../../OnlineLib/SoftwareEndpoint.h"

#include <memory>
#include <set>
#include <shared_mutex>
#include <string>
#include <vector>


class ILogFile;

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
		explicit AdsBridgeFacade(ILogFile* log);
		~AdsBridgeFacade();

		AdsBridgeFacade(const AdsBridgeFacade&) = delete;
		AdsBridgeFacade(AdsBridgeFacade&&) = delete;
		AdsBridgeFacade& operator=(const AdsBridgeFacade&) = delete;
		AdsBridgeFacade& operator=(AdsBridgeFacade&&) = delete;

	public:
		void addAppDataService(const QString& adsEquipmentId, const QString& address, int port);

		void connect();
		void close();

	public:
		const QString& equipmentId() const;
		void setEquipmentId(const QString& equipmentId);

		bool signalParamsLoaded() const;
		bool signalStatesLoaded() const;

		size_t signalCount() const;
		bool signalList(MatsSignalHash* out, size_t count);

		bool signalParams(const MatsSignalHash* signalHashes, MatsAppSignalParam* out, size_t count);
		bool signalStates(const MatsSignalHash* signalHashes, MatsAppSignalState* out, size_t count);

		size_t connectionCount() const;
		bool connectionStatus(struct AdsConnectionStatus* out, size_t structSize, size_t count) const;

	private:
		ILogFile* m_log = nullptr;
		std::unique_ptr<ClientLib::AppSignalManager> m_signals;
		std::unique_ptr<ClientLib::AdsConnection> m_adsConnection;

		QString m_equipmentId;
		std::vector<SoftwareEndpoint::AppDataService> m_appDataServices;

	private:
		const char* getStringConstPointer(const QString& string) const;

		mutable std::shared_mutex m_stringTableMutex;
		mutable std::set<std::string> m_stringTable;
	};
} // namespace AdsBridge