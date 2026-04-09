#include "AdsSourceStateConnectionPrivate2.h"
#include "../include/ClientLib/LoggerStdAdapter.h"
#include <AdsConnectionLib/ClientGrpc.h>


namespace
{
	constexpr std::chrono::milliseconds UpdateStateInterval{100};
}

namespace ClientLib
{
	// AdsClientGrpc
	//
	class AdsSourceStateClientGrpc : public LoggerStdAdapter,
									 public ClientGrpc<Grpc::AppDataSrv>
	{
	public:
		explicit AdsSourceStateClientGrpc(const Network::SoftwareInfo& softwareInfo, const ServiceEndpoint& ads, ILogFile& logFile);

		virtual ~AdsSourceStateClientGrpc();

		// Implementing ClientConnectionStatistics
		//
	public:
		virtual std::string statsObjectName() override;

	public:
		std::expected<void, QString> getAppDataSourcesInfo();
		std::expected<void, QString> getAppDataSourcesState();

	private:
		virtual void clientCommunicationLoop(std::stop_token stoken) override;
		void clientCommunicationLoopImpl(std::stop_token stoken);

		QString logPrefix() { return QString::fromStdString(m_logPrefix); }

	public:
		const ServiceEndpoint& ads() const;

		std::vector<ClientLib::AppDataSourceState> appDataSourceStates() const;

	private:
		const ServiceEndpoint m_ads;

		mutable QReadWriteLock m_appDataSourceStatesLock;            // For access to m_appDataSourceStates
		std::map<quint64, AppDataSourceState> m_appDataSourceStates; // Key is source unique id
	};

	AdsSourceStateClientGrpc::AdsSourceStateClientGrpc(const Network::SoftwareInfo& softwareInfo,
													   const ServiceEndpoint& ads,
													   ILogFile& logFile) :
		LoggerStdAdapter{logFile},
		ClientGrpc{softwareInfo, ads, static_cast<LoggerStdAdapter&>(*this), ads.shortenId},
		m_ads{ads}
	{
		m_tcpState.name = "AdsSourceStateClientGrpc " + ads.shortenId;
		return;
	}

	AdsSourceStateClientGrpc::~AdsSourceStateClientGrpc()
	{
		shutUp();
	}

	std::string AdsSourceStateClientGrpc::statsObjectName()
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
			QString error = QString::fromStdString(statusToString(status));
			return std::unexpected<QString>(QString{"Failed to get app data source info, error %1"}.arg(error));
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
			QString error = QString::fromStdString(statusToString(status));
			return std::unexpected<QString>(QString{"Failed to get app data source states, error %1"}.arg(error));
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
				logFile().writeWarning(QString{"Received app data source state for unknown source id %1 from ADS %2"}.arg(id).arg(
					QString::fromStdString(m_ads.to_string())));
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
			logFile().writeError(logPrefix() + QString{"Exception in AdsSourceStateClientGrpc::clientCommunicationLoopImpl for ADS %1: %2"}
												   .arg(QString::fromStdString(m_ads.to_string()))
												   .arg(e.what()));
		}

		{
			QWriteLocker locker{&m_appDataSourceStatesLock};
			m_appDataSourceStates.clear();
		}

		logFile().writeMessage(logPrefix() + QString{"AdsSourceStateClientGrpc, Worker for ADS %1 gRPC client exiting."}.arg(
												 QString::fromStdString(m_ads.to_string())));
		return;
	}

	void AdsSourceStateClientGrpc::clientCommunicationLoopImpl(std::stop_token stoken)
	{
		auto result = getAppDataSourcesInfo();
		if (result.has_value() == false)
		{
			logFile().writeError(logPrefix() + QString{"AdsSourceStateClientGrpc, Failed to get app data source info from ADS %1: %2"}
												   .arg(QString::fromStdString(m_ads.to_string()))
												   .arg(result.error()));
			return;
		}

		while (stoken.stop_requested() == false)
		{
			auto resultState = getAppDataSourcesState();
			if (resultState.has_value() == false)
			{
				logFile().writeError(logPrefix() + QString{"AdsSourceStateClientGrpc, Failed to get app data source states from ADS %1: %2"}
													   .arg(QString::fromStdString(m_ads.to_string()))
													   .arg(resultState.error()));
			}

			std::this_thread::sleep_for(UpdateStateInterval);
		}
	}

	const ServiceEndpoint& AdsSourceStateClientGrpc::ads() const
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
	AdsSourceStateConnectionPrivate2::Connection::Connection(const Network::SoftwareInfo& softwareInfo,
															 const ServiceEndpoint& ads,
															 ILogFile& logFile) :
		m_client{std::make_unique<ClientLib::AdsSourceStateClientGrpc>(softwareInfo, ads, logFile)}
	{
		return;
	}

	// HostAddressPort AdsSourceStateConnectionPrivate2::Connection::address() const
	//{
	//	return m_client->ads().address;
	// }

	const ServiceEndpoint& AdsSourceStateConnectionPrivate2::Connection::server() const
	{
		return m_client->ads();
	}

	ServiceConnectionState AdsSourceStateConnectionPrivate2::Connection::tcpConnectionState() const
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

		Network::SoftwareInfo si;
		softwareInfo.serializeTo(&si);

		for (const SoftwareEndpoint::AppDataService& ads : appDataService)
		{
			m_conns.emplace_back(si, toServiceEndpoint(ads), *m_logFile.logFile());
		}

		return;
	}

	std::vector<ServiceConnectionState> AdsSourceStateConnectionPrivate2::adsConnectionStates() const
	{
		std::vector<ServiceConnectionState> states;
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