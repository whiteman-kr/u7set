#ifndef CLIENT_LIB_DOMAIN
	#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "AdsConnectionPrivate.h"
#include <ClientLib/AdsConnection.h>

namespace ClientLib
{
	AdsConnection::AdsConnection(IAppSignalUpdater& signalUpdater, IRecentAppSignals* recentAppSignals, ILogFile* logFile) :
		m_pimpl{std::make_unique<AdsConnectionPrivate>(signalUpdater, recentAppSignals, logFile)}
	{
		return;
	}

	AdsConnection::~AdsConnection() = default;

	void AdsConnection::updateConnections(const SoftwareInfo& softwareInfo,
										  const std::vector<SoftwareEndpoint::AppDataService>& appDataServices)
	{
		return m_pimpl->updateConnections(softwareInfo, appDataServices);
	}

	std::vector<Tcp::ConnectionState> AdsConnection::tcpSignalConnStates() const
	{
		return m_pimpl->tcpSignalConnStates();
	}

	std::vector<Tcp::ConnectionState> AdsConnection::recentSignalConnStates() const
	{
		return m_pimpl->recentSignalConnStates();
	}

	bool AdsConnection::signalParamsLoaded() const
	{
		return m_pimpl->signalParamsLoaded();
	}

	bool AdsConnection::signalStatesLoaded() const
	{
		return m_pimpl->signalStatesLoaded();
	}

} // namespace ClientLib
