#include "AdsSourceStateConnectionPrivate2.h"
#include "ClientGrpc.h"

namespace
{
	constexpr std::chrono::milliseconds UpdateStateInterval{100};
}

namespace ClientLib
{
	// AdsClientGrpc
	//
	class AdsSourceStateClientGrpc : public ClientGrpc<Grpc::AppDataSrv>
	{
	public:
		explicit AdsSourceStateClientGrpc(const SoftwareInfo& softwareInfo, const SoftwareEndpoint::AppDataService& ads, ILogFile& logFile);

		virtual ~AdsSourceStateClientGrpc();

		// Implementing ClientConnectionStatistics
		//
	public:
		virtual QString statsObjectName() override;

	public:
		std::expected<void, QString> getAppDataSourcesInfo();
		std::expected<void, QString> getAppDataSourcesState();

	private:
		virtual void clientCommunicationLoop(std::stop_token stoken) override;
		void clientCommunicationLoopImpl(std::stop_token stoken);

	public:
		const SoftwareEndpoint::AppDataService& ads() const;

		std::vector<ClientLib::AppDataSourceState> appDataSourceStates() const;

	private:
		const SoftwareEndpoint::AppDataService m_ads;

		mutable QReadWriteLock m_appDataSourceStatesLock;            // For access to m_appDataSourceStates
		std::map<quint64, AppDataSourceState> m_appDataSourceStates; // Key is source unique id
	};

	AdsSourceStateClientGrpc::AdsSourceStateClientGrpc(const SoftwareInfo& softwareInfo,
													   const SoftwareEndpoint::AppDataService& ads,
													   ILogFile& logFile) :
		ClientGrpc{softwareInfo, ads.equipmentId, ads.address, logFile, ads.shortenId},
		m_ads{ads}
	{
		m_tcpState.name = "AdsSourceStateClientGrpc " + ads.shortenId;
		return;
	}

	AdsSourceStateClientGrpc::~AdsSourceStateClientGrpc()
	{
		shutUp();
	}

	QString AdsSourceStateClientGrpc::statsObjectName()
	{
		return "AdsSourceStateClientGrpc";
	}

	std::expected<void, QString> AdsSourceStateClientGrpc::getAppDataSourcesInfo()
	{
		grpc::ClientContext context;
		createAuthContext(context, std::chrono::seconds(5));

		Grpc::GetAppDataSourcesInfoRequest request;
		Grpc::GetAppDataSourcesInfoReply reply;

		auto status = m_stub->GetAppDataSourcesInfo(&context, request, &reply);
		incRequestCount();

		if (status.ok() == false)
		{
			return std::unexpected<QString>(QString{"Failed to get app data source info, error %1"}.arg(statusToString(status)));
		}

		incReplyCount();

		QWriteLocker l{&m_appDataSourceStatesLock};
		m_appDataSourceStates.clear();

		for (const auto& sourceInfo : reply.appdatasourceinfo())
		{
			AppDataSourceState adsss;
			adsss.info = sourceInfo;

			quint64 id = sourceInfo.id();

			assert(m_appDataSourceStates.count(id) == 0);
			m_appDataSourceStates[id] = adsss;
		}

		return {};
	}

	std::expected<void, QString> AdsSourceStateClientGrpc::getAppDataSourcesState()
	{
		grpc::ClientContext context;
		createAuthContext(context, std::chrono::seconds(5));

		Grpc::GetAppDataSourcesStateRequest request;
		Grpc::GetAppDataSourcesStateReply reply;

		auto status = m_stub->GetAppDataSourcesState(&context, request, &reply);
		incRequestCount();

		if (status.ok() == false)
		{
			return std::unexpected<QString>(QString{"Failed to get app data source states, error %1"}.arg(statusToString(status)));
		}

		incReplyCount();

		QWriteLocker l{&m_appDataSourceStatesLock};
		for (const auto& sourceState : reply.appdatasourcestate())
		{
			quint64 id = sourceState.id();

			if (m_appDataSourceStates.contains(id) == true)
			{
				m_appDataSourceStates[id].setNewState(sourceState);
			}
			else
			{
				m_log.writeWarning(QString{"Received app data source state for unknown source id %1 from ADS %2 at address %3"}
									   .arg(id)
									   .arg(m_ads.equipmentId)
									   .arg(m_ads.address.toString()));
			}
		}

		return {};
	}

	void AdsSourceStateClientGrpc::clientCommunicationLoop(std::stop_token stoken)
	{
		{
			QWriteLocker locker{&m_appDataSourceStatesLock};
			m_appDataSourceStates.clear();
		}

		try
		{
			clientCommunicationLoopImpl(stoken);
		}
		catch (std::exception& e)
		{
			m_log.writeError(m_logPrefix +
							 QString{"Exception in AdsSourceStateClientGrpc::clientCommunicationLoopImpl for ADS %1 at address %2: %3"}
								 .arg(m_ads.equipmentId)
								 .arg(m_ads.address.toString())
								 .arg(e.what()));
		}

		{
			QWriteLocker locker{&m_appDataSourceStatesLock};
			m_appDataSourceStates.clear();
		}

		m_log.writeMessage(m_logPrefix + QString{"AdsSourceStateClientGrpc, Worker for ADS %1 gRPC client exiting."}.arg(m_ads.shortenId));
		return;
	}

	void AdsSourceStateClientGrpc::clientCommunicationLoopImpl(std::stop_token stoken)
	{
		auto result = getAppDataSourcesInfo();
		if (result.has_value() == false)
		{
			m_log.writeError(m_logPrefix +
							 QString{"AdsSourceStateClientGrpc, Failed to get app data source info from ADS %1 at address %2: %3"}
								 .arg(m_ads.equipmentId)
								 .arg(m_ads.address.toString())
								 .arg(result.error()));
			return;
		}

		while (stoken.stop_requested() == false)
		{
			auto resultState = getAppDataSourcesState();
			if (resultState.has_value() == false)
			{
				m_log.writeError(m_logPrefix +
								 QString{"AdsSourceStateClientGrpc, Failed to get app data source states from ADS %1 at address %2: %3"}
									 .arg(m_ads.equipmentId)
									 .arg(m_ads.address.toString())
									 .arg(resultState.error()));
			}

			std::this_thread::sleep_for(UpdateStateInterval);
		}
	}

	const SoftwareEndpoint::AppDataService& AdsSourceStateClientGrpc::ads() const
	{
		return m_ads;
	}

	std::vector<ClientLib::AppDataSourceState> AdsSourceStateClientGrpc::appDataSourceStates() const
	{
		QReadLocker locker{&m_appDataSourceStatesLock};

		std::vector<ClientLib::AppDataSourceState> result;
		result.reserve(m_appDataSourceStates.size());

		for (const auto& [sourceId, state] : m_appDataSourceStates)
		{
			result.emplace_back(state);
		}

		return result;
	}

	// AdsSourceStateClientGrpc
	//
	AdsSourceStateConnectionPrivate2::Connection::Connection(const SoftwareInfo& softwareInfo,
															 const SoftwareEndpoint::AppDataService& ads,
															 ILogFile& logFile) :
		m_client{std::make_unique<ClientLib::AdsSourceStateClientGrpc>(softwareInfo, ads, logFile)}
	{
		return;
	}

	HostAddressPort AdsSourceStateConnectionPrivate2::Connection::address() const
	{
		return m_client->ads().address;
	}

	const SoftwareEndpoint::AppDataService& AdsSourceStateConnectionPrivate2::Connection::server() const
	{
		return m_client->ads();
	}

	Tcp::ConnectionState AdsSourceStateConnectionPrivate2::Connection::tcpConnectionState() const
	{
		return m_client->statsConnectionState();
	}

	std::vector<ClientLib::AppDataSourceState> AdsSourceStateConnectionPrivate2::Connection::appDataSourceStates() const
	{
		return m_client->appDataSourceStates();
	}

	AdsSourceStateConnectionPrivate2::AdsSourceStateConnectionPrivate2(ILogFile* logFile) :
		m_logFile(logFile, "AdsConnection")
	{
		m_logFile.writeMessage("AdsSourceStateConnectionPrivate::AdsSourceStateConnectionPrivate()");
		return;
	}

	AdsSourceStateConnectionPrivate2::~AdsSourceStateConnectionPrivate2()
	{
		m_logFile.writeMessage("AdsSourceStateConnectionPrivate::~AdsSourceStateConnectionPrivate2()");
		m_conns.clear();
		return;
	}

	void AdsSourceStateConnectionPrivate2::updateConnections(const SoftwareInfo& softwareInfo,
															 const std::vector<SoftwareEndpoint::AppDataService>& appDataService)
	{
		m_logFile.writeMessage(QString("updateConnections(), %1 app data services").arg(appDataService.size()));

		m_conns.clear(); // it will stop all connection threads and destroy them

		for (const SoftwareEndpoint::AppDataService& ads : appDataService)
		{
			m_conns.emplace_back(softwareInfo, ads, *m_logFile.logFile());
		}

		return;
	}

	std::vector<Tcp::ConnectionState> AdsSourceStateConnectionPrivate2::adsConnectionStates() const
	{
		std::vector<Tcp::ConnectionState> states;
		states.reserve(m_conns.size());

		for (const Connection& c : m_conns)
		{
			states.emplace_back(c.tcpConnectionState());
		}

		return states;
	}

	std::vector<ClientLib::AppDataSourceState> AdsSourceStateConnectionPrivate2::appDataSourceStates() const
	{
		std::vector<ClientLib::AppDataSourceState> result;
		result.reserve(m_conns.size() * 10); // We do not know exact number of data sources, but assume average 10 per ADS

		for (const Connection& c : m_conns)
		{
			auto states = c.appDataSourceStates();
			result.insert(result.end(), states.begin(), states.end());
		}

		return result;
	}
} // namespace ClientLib