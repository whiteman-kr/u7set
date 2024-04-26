#ifndef CLIENT_LIB_DOMAIN
#error Do not include this file in the project! Link ClientLib instead.
#endif

#include <ClientLib/AdsSourceStateConnection.h>
#include "AdsSourceStateConnectionPrivate.h"

namespace ClientLib
{
	AdsSourceStateConnection::AdsSourceStateConnection(ILogFile* logFile) :
		m_pimpl{std::make_unique<AdsSourceStateConnectionPrivate>(logFile)}
	{
		return;
	}

	AdsSourceStateConnection::~AdsSourceStateConnection() = default;

	void AdsSourceStateConnection::updateConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataService)
	{
		return m_pimpl->updateConnections(softwareInfo, appDataService);
	}

	std::vector<Tcp::ConnectionState> AdsSourceStateConnection::adsConnectionStates() const
	{
		return m_pimpl->adsConnectionStates();
	}

	std::vector<ClientLib::AppDataSourceState> AdsSourceStateConnection::appDataSourceStates() const
	{
		return m_pimpl->appDataSourceStates();
	}
}
