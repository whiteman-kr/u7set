#include <ClientLib/AdsSourceStateConnection.h>

#ifndef CLIENT_LIB_DOMAIN
	#error Do not include this file in the project! Link ClientLib instead.
#endif

#ifdef USE_GRPC_ADS_CONNECTION
	#include "AdsSourceStateConnectionPrivate2.h"
#else
	#include "AdsSourceStateConnectionPrivate.h"
#endif


namespace ClientLib
{
	AdsSourceStateConnection::AdsSourceStateConnection(ILogFile* logFile) :
		m_pimpl{std::make_unique<AdsConnectionType>(logFile)}
	{
		return;
	}

	AdsSourceStateConnection::~AdsSourceStateConnection() = default;

	void AdsSourceStateConnection::updateConnections(const SoftwareInfo& softwareInfo,
													 const std::vector<SoftwareEndpoint::AppDataService>& appDataService)
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
} // namespace ClientLib
