#ifdef USE_GRPC_ADS_CONNECTION
	#include "AdsConnectionPrivate2.h"
#else
	#include "AdsConnectionPrivate.h"
#endif


#include <AdsConnectionLib/AdsConnection.h>


namespace ClientLib
{
	AdsConnection::AdsConnection(IAppSignalUpdater& signalUpdater,
								 IRecentAppSignals* recentAppSignals,
								 ISignalLogUpdater* signalLogUpdater,
								 ILoggerStd& logFile) :
		m_hasRecentAppSignals{recentAppSignals != nullptr},
		m_pimpl{std::make_unique<AdsConnectionType>(signalUpdater, recentAppSignals, signalLogUpdater, logFile)}
	{
		return;
	}

	AdsConnection::~AdsConnection() = default;

	void AdsConnection::updateConnections(const ::Network::SoftwareInfo& softwareInfo, const std::vector<ServiceEndpoint>& appDataServices)
	{
		return m_pimpl->updateConnections(softwareInfo, appDataServices);
	}

	std::vector<ServiceConnectionState> AdsConnection::connectionStates() const
	{
		return m_pimpl->connectionStates();
	}

	int AdsConnection::connectionsPerServer() const
	{
		// Returns 2 for TcpConnection and 1 for Grpc.
		//
		int result = 0;

		if constexpr (std::is_same_v<::ClientLib::AdsConnection::AdsConnectionType, ::ClientLib::AdsConnectionPrivate>)
		{
			result = m_hasRecentAppSignals ? 2 : 1;
		}

		if constexpr (std::is_same_v<::ClientLib::AdsConnection::AdsConnectionType, ::ClientLib::AdsConnectionPrivate2>)
		{
			result = 1;
		}

		assert(result != 0);
		return result;
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
