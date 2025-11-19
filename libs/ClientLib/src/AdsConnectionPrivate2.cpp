#ifndef CLIENT_LIB_DOMAIN
	#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "AdsConnectionPrivate2.h"
#include "ClientGrpc.h"

#include <grpcpp/grpcpp.h>

#include <expected>


namespace ClientLib
{
	// AdsClientGrpc
	//
	class AdsClientGrpc : public ClientGrpc<Grpc::AppDataSrv>
	{
	public:
		explicit AdsClientGrpc(const SoftwareInfo& softwareInfo,
							   const SoftwareEndpoint::AppDataService& ads,
							   IAppSignalUpdater& signalUpdater,
							   SignalLog& signalLog,
							   ILogFile& logFile);

		// Implementing ClientConnectionStatistics
		//
	public:
		virtual QString statsObjectName() override;

	public:
		std::expected<QStringList, QString> requestSignalList();
		std::expected<std::vector<AppSignalParam>, QString> requestSignalParams(std::span<Hash> signalHashes = {});
		std::expected<std::vector<AppSignalState>, QString> requestSignalStates(std::span<Hash> signalHashes);
		void requestSignalStatesChanges(std::stop_token stoken);

	private:
		virtual void clientCommunicationLoop(std::stop_token stoken) override;
		void clientCommunicationLoopImpl(std::stop_token stoken);

	public:
		const SoftwareEndpoint::AppDataService& ads() const;

		bool signalParamsLoaded() const;
		bool signalStatesLoaded() const;

	private:
		const SoftwareEndpoint::AppDataService m_ads;
		IAppSignalUpdater& m_signalUpdater;
		SignalLog& m_signalLog;

		// --
		//
		std::atomic<bool> m_signalParamsLoaded{false};
		std::atomic<bool> m_signalStatesLoaded{false};
	};


	AdsClientGrpc::AdsClientGrpc(const SoftwareInfo& softwareInfo,
								 const SoftwareEndpoint::AppDataService& ads,
								 IAppSignalUpdater& signalUpdater,
								 SignalLog& signalLog,
								 ILogFile& logFile) :
		ClientGrpc{softwareInfo, ads.equipmentId, ads.address, logFile, ads.shortenId},
		m_ads{ads},
		m_signalUpdater{signalUpdater},
		m_signalLog{signalLog}
	{
		m_tcpState.name = "AdsClientGrpc " + ads.shortenId;
		return;
	}

	QString AdsClientGrpc::statsObjectName()
	{
		return "AdsClientGrpc";
	}

	std::expected<QStringList, QString> AdsClientGrpc::requestSignalList()
	{
		grpc::ClientContext context;
		createAuthContext(context, std::chrono::seconds(20));

		QStringList appSignalIds;
		Grpc::GetAppSignalListRequest request;
		Grpc::GetAppSignalListReply reply;

		auto replyReader = m_stub->GetAppSignalList(&context, request);
		incRequestCount();

		while (replyReader->Read(&reply) == true)
		{
			incReplyCount();
			if (appSignalIds.size() == 0)
			{
				appSignalIds.reserve(reply.totalsize());
			}

			for (const auto& appSignalId : reply.appsignalids())
			{
				appSignalIds.append(QString::fromStdString(appSignalId));
			}
		}

		grpc::Status status = replyReader->Finish();
		if (status.ok() == false)
		{
			return std::unexpected<QString>{QString("Failed to get signal list, error %1").arg(statusToString(status))};
		}

		return appSignalIds;
	}

	std::expected<std::vector<AppSignalParam>, QString> AdsClientGrpc::requestSignalParams(std::span<Hash> signalHashes /*= {}*/)
	{
		grpc::ClientContext context;
		createAuthContext(context, std::chrono::seconds(60));

		Grpc::GetAppSignalParamRequest request;
		Grpc::GetAppSignalParamReply reply;

		request.mutable_signalhashes()->Reserve(static_cast<int>(signalHashes.size()));
		for (const auto& hash : signalHashes)
		{
			request.add_signalhashes(hash);
		}

		std::vector<AppSignalParam> result;

		auto replyReader = m_stub->GetAppSignalParam(&context, request);
		incRequestCount();

		while (replyReader->Read(&reply))
		{
			incReplyCount();

			if (result.capacity() == 0)
			{
				result.reserve(reply.totalsize());
			}

			for (const auto& paramProto : reply.signalparams())
			{
				AppSignalParam param;
				bool ok = param.load(paramProto);

				if (ok == true)
				{
					result.push_back(std::move(param));
				}
				else
				{
					m_log.writeError(m_logPrefix + QString{"Failed to load AppSignalParam for signal hash %1 from ADS %2, address %3"}
													   .arg(QString::fromStdString(paramProto.appsignalid()))
													   .arg(m_serviceEquipmentId)
													   .arg(m_serviceAddress));
				}
			}
		}

		auto status = replyReader->Finish();
		if (status.ok() == false)
		{
			return std::unexpected<QString>{QString("Failed to get signal params, error %1").arg(statusToString(status))};
		}

		return result;
	}

	std::expected<std::vector<AppSignalState>, QString> AdsClientGrpc::requestSignalStates(std::span<Hash> signalHashes)
	{
		Grpc::GetAppSignalStateRequest request;
		Grpc::GetAppSignalStateReply reply;

		// Calculate number of parts needed, rounding up.
		//
		const size_t parts = (signalHashes.size() + ADS_GET_APP_SIGNAL_STATE_MAX - 1) / ADS_GET_APP_SIGNAL_STATE_MAX;

		std::vector<AppSignalState> result;
		result.reserve(signalHashes.size());

		for (size_t part = 0; part < parts; ++part)
		{
			request.mutable_signalhashes()->Clear();

			// Calculate start and end indices for this part.
			//
			const size_t startIdx = part * ADS_GET_APP_SIGNAL_STATE_MAX;
			const size_t endIdx = std::min(startIdx + ADS_GET_APP_SIGNAL_STATE_MAX, signalHashes.size());
			const size_t partSize = endIdx - startIdx;

			request.mutable_signalhashes()->Reserve(static_cast<int>(partSize));
			for (size_t i = startIdx; i < endIdx; ++i)
			{
				request.add_signalhashes(signalHashes[i]);
			}

			grpc::ClientContext context;
			createAuthContext(context, std::chrono::seconds(30));

			auto status = m_stub->GetAppSignalState(&context, request, &reply);
			incRequestCount();

			if (status.ok() == false)
			{
				return std::unexpected<QString>{
					QString("Failed to get signal states (part %1/%2), error %3").arg(part + 1).arg(parts).arg(statusToString(status))};
			}
			else
			{
				incReplyCount();
			}

			for (const auto& stateProto : reply.appsignalstates())
			{
				result.emplace_back().load(stateProto);
			}
		}

		return result;
	}

	void AdsClientGrpc::requestSignalStatesChanges(std::stop_token stoken)
	{
		grpc::ClientContext context;
		createAuthContext(context);

		std::stop_callback stopCallback{stoken,
										[&context]()
										{
											// Trigger gRPC cancellation, this makes Read() to return false.
											//
											context.TryCancel();
										}};

		Grpc::GetAppSignalStateChangesRequest request;
		Grpc::GetAppSignalStateChangesReply reply;
		std::vector<AppSignalState> states;

		auto replyReader = m_stub->GetAppSignalStateChanges(&context, request);
		incRequestCount();

		while (stoken.stop_requested() == false && replyReader->Read(&reply) == true)
		{
			incReplyCount();

			states.clear();
			states.reserve(reply.appsignalstates_size());

			std::transform(reply.appsignalstates().begin(),
						   reply.appsignalstates().end(),
						   std::back_inserter(states),
						   [](const auto& stateProto)
						   {
							   return AppSignalState{stateProto};
						   });

			m_signalUpdater.setStates(std::span(states), ::calcHash(m_serviceEquipmentId), sourceId());
#if 0
			for (const auto& state : states)
			{
				if (state.hash() == ::calcHash(QString("#CT_LOG_LM11")))
				{
					qDebug() << "ADS gRPC client: State change " << m_ads.equipmentId << " at " << m_ads.address.toString()
							 << ", Value: " << state.value()
							 << ", Timestamp:" << state.time().localToDateTime()
							 << ", numerator: " << reply.numertator() << ", states: " << reply.appsignalstates_size()
							 << ", incudes: " << reply.includesthatfuckingsignal();
				}
			}
#endif
			reply.Clear();
		}

		auto status = replyReader->Finish();
		m_log.writeMessage(m_logPrefix +
						   QString{"AdsClientGrpc::requestSignalStatesChanges is about to exit with code %1, ADS %2, Address %3"}
							   .arg(statusToString(status))
							   .arg(m_ads.equipmentId)
							   .arg(m_ads.address.toString()));
		return;
	}

	void AdsClientGrpc::clientCommunicationLoop(std::stop_token stoken)
	{
		m_signalParamsLoaded.store(false);
		m_signalStatesLoaded.store(false);

		try
		{
			clientCommunicationLoopImpl(stoken);
		}
		catch (std::exception& e)
		{
			m_log.writeError(m_logPrefix + QString{"Exception in AdsClientGrpc::clientCommunicationLoopImpl for ADS %1 at address %2: %3"}
											   .arg(m_ads.equipmentId)
											   .arg(m_ads.address.toString())
											   .arg(e.what()));
		}

		m_signalParamsLoaded.store(false);
		m_signalStatesLoaded.store(false);

		m_log.writeMessage(m_logPrefix + QString{"Worker for ADS %1 gRPC client exiting."}.arg(m_ads.shortenId));
		return;
	}

	void AdsClientGrpc::clientCommunicationLoopImpl(std::stop_token stoken)
	{
		// QDate timeDiscrepancyCheckDate; // Check that the server and client time is the same.
		//  When was the last time the time discrepancy was checked?
#if 0
			auto listResult = requestSignalList();
			if (listResult.has_value() == false)
			{
				m_log.writeError(m_logPrefix + QString{"Failed to get signal list from ADS %1, address %2, error: %3"}
									.arg(m_ads.equipmentId)
									.arg(m_ads.address.toString())
									.arg(listResult.error()));
				continue;
			}

			std::vector<Hash> signalList;
			signalList.reserve(listResult.value().size());
			std::transform(listResult.value().begin(),
						   listResult.value().end(),
						   std::back_inserter(signalList),
						   [](const QString& signalId)
						   {
							   return calcHash(signalId);
						   });

			qDebug() << "ADS gRPC client: Retrieved" << signalList.size() << "signalIds from ADS" << m_ads.equipmentId << "at"
					 << m_ads.address.toString();
#endif
		// Get all signals
		//
		std::vector<Hash> signalList;

		{
			auto signalParamsResult = requestSignalParams();
			if (signalParamsResult.has_value() == false)
			{
				m_log.writeError(m_logPrefix + QString{"Failed to get signal params from ADS %1, address %2, error: %3"}
												   .arg(m_ads.equipmentId)
												   .arg(m_ads.address.toString())
												   .arg(signalParamsResult.error()));
				return;
			}

			// Filter out bus signals
			//
			std::vector<AppSignalParam> signalParams = std::move(signalParamsResult.value());

			auto signalParamToHash = [](const AppSignalParam& param) -> Hash
			{
				return param.hash();
			};

			auto isNotBusSignal = [](const AppSignalParam& param) -> bool
			{
				return param.isBus() == false;
			};

			auto signalListView = signalParams | std::views::filter(isNotBusSignal) | std::views::transform(signalParamToHash);
			signalList.reserve(signalParams.size()); // There are likely little bus signals.
			signalList.assign(signalListView.begin(), signalListView.end());

			// Update signal params in the signal updater and notify.
			//
			m_signalUpdater.addSignals(std::span(signalParams), m_ads.equipmentId);
			m_signalUpdater.notifySignalParamsUpdated();

			m_signalParamsLoaded.store(true);
		}

		// Get initial signal states.
		//
		{
			auto result = requestSignalStates(signalList);
			if (result.has_value() == false)
			{
				m_log.writeError(m_logPrefix + QString{"Failed to get signal states from ADS %1, address %2, error: %3"}
												   .arg(m_ads.equipmentId)
												   .arg(m_ads.address.toString())
												   .arg(result.error()));
				return;
			}

			m_signalUpdater.setStates(std::span(result.value()), ::calcHash(m_serviceEquipmentId), sourceId());

			m_signalStatesLoaded.store(true);
		}

		// Start separate thread for getting streamed signal state changes.
		//
		std::jthread stateChangesThread{
			[this](std::stop_token stoken)
			{
				m_log.writeMessage(m_logPrefix + QString{"Enter listening for signal state changes for ADS %1 at address %2: %3"}
													 .arg(m_ads.equipmentId)
													 .arg(m_ads.address.toString()));

				try
				{
					requestSignalStatesChanges(stoken);
				}
				catch (std::exception& e)
				{
					m_log.writeError(m_logPrefix +
									 QString{"Exception in AdsClientGrpc::requestSignalStatesChanges for ADS %1 at address %2: %3"}
										 .arg(m_ads.equipmentId)
										 .arg(m_ads.address.toString())
										 .arg(e.what()));
				}

				m_log.writeMessage(m_logPrefix + QString{"Leaving listening for signal state changes for ADS %1 at address %2: %3"}
													 .arg(m_ads.equipmentId)
													 .arg(m_ads.address.toString()));
			}};

		// Loop, getting signal states periodically.
		//
		constexpr size_t RepeatedlyGetStateCount =
			ADS_GET_APP_SIGNAL_STATE_MAX / 16; // 312 states * 10 requests/sec = 3120 states/sec, or 187'200 states/minute.
		size_t statePart = 0;

		auto lastCycleStart = std::chrono::steady_clock::now();
		constexpr auto StateRequestMinCycle = std::chrono::seconds(15);

		while (stoken.stop_requested() == false)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			// Get next part of signal states
			//
			{
				std::span<Hash> signalHashesPart{
					signalList.begin() + statePart * RepeatedlyGetStateCount,
					std::min<size_t>(RepeatedlyGetStateCount, signalList.size() - statePart * RepeatedlyGetStateCount)};

				auto result = requestSignalStates(signalHashesPart);
				if (result.has_value() == false)
				{
					m_log.writeError(m_logPrefix + QString{"Failed to get signal states from ADS %1, address %2, error: %3"}
													   .arg(m_ads.equipmentId)
													   .arg(m_ads.address.toString())
													   .arg(result.error()));
					return;
				}

				m_signalUpdater.setStates(std::span(result.value()), ::calcHash(m_serviceEquipmentId), sourceId());

				statePart++;
				if (statePart * RepeatedlyGetStateCount >= signalList.size())
				{
					statePart = 0;

					auto timeSinceLastCycle = std::chrono::steady_clock::now() - lastCycleStart;
					if (timeSinceLastCycle < StateRequestMinCycle)
					{
						// Throttle to not overload the ADS if all states were retrieved too fast.
						//
						for (auto sleepTime = StateRequestMinCycle - timeSinceLastCycle; sleepTime > std::chrono::milliseconds(0);
							 sleepTime -= std::chrono::milliseconds(100))
						{
							if (stoken.stop_requested() == true)
							{
								break;
							}

							std::this_thread::sleep_for(std::chrono::milliseconds(100));
						}
					}

					lastCycleStart = std::chrono::steady_clock::now();
				}
			}

			// Control server time discrepancy
			//
		}

		stateChangesThread.request_stop();
		stateChangesThread.join();

		return;
	}

	const SoftwareEndpoint::AppDataService& AdsClientGrpc::ads() const
	{
		return m_ads;
	}

	bool AdsClientGrpc::signalParamsLoaded() const
	{
		return m_signalParamsLoaded.load();
	}

	bool AdsClientGrpc::signalStatesLoaded() const
	{
		return m_signalStatesLoaded.load();
	}

	// AdsConnectionPrivate2::Connection
	//
	AdsConnectionPrivate2::Connection::Connection(const SoftwareInfo& softwareInfo,
												  const SoftwareEndpoint::AppDataService& ads,
												  IAppSignalUpdater& signalUpdater,
												  IRecentAppSignals* /*recentAppSignals*/,
												  SignalLog& signalLog,
												  ILogFile& logFile) :
		m_client{std::make_unique<ClientLib::AdsClientGrpc>(softwareInfo, ads, signalUpdater, signalLog, logFile)}
	{
		return;
	}

	HostAddressPort AdsConnectionPrivate2::Connection::address() const
	{
		return m_client->ads().address;
	}

	const SoftwareEndpoint::AppDataService& AdsConnectionPrivate2::Connection::server() const
	{
		return m_client->ads();
	}

	Tcp::ConnectionState AdsConnectionPrivate2::Connection::tcpConnectionState() const
	{
		return m_client->statsConnectionState();
	}

	bool AdsConnectionPrivate2::Connection::signalParamsLoaded() const
	{
		return m_client->signalParamsLoaded();
	}

	bool AdsConnectionPrivate2::Connection::signalStatesLoaded() const
	{
		return m_client->signalStatesLoaded();
	}

	// AdsConnectionPrivate2
	//
	AdsConnectionPrivate2::AdsConnectionPrivate2(IAppSignalUpdater& signalUpdater, IRecentAppSignals* recentAppSignals, ILogFile* logFile) :
		m_signalUpdater{signalUpdater},
		m_recentAppSignals{recentAppSignals},
		m_logFile{logFile, "AdsConnectionPrivate"}
	{
		return;
	}

	AdsConnectionPrivate2::~AdsConnectionPrivate2()
	{
		m_logFile.writeMessage("~AdsConnectionPrivate()");
	}

	void AdsConnectionPrivate2::updateConnections(const SoftwareInfo& softwareInfo,
												  const std::vector<SoftwareEndpoint::AppDataService>& appDataServices)
	{
		m_logFile.writeMessage(QString{"updateConnections(), %1 AppDataServices"}.arg(appDataServices.size()));
		for (const auto& ads : appDataServices)
		{
			m_logFile.writeMessage(QString{"updateConnections(),    %1 - %2"}.arg(ads.shortenId).arg(ads.address.toString()));
		}

		QWriteLocker locker{&m_connsMutex};

		// Number of AppDataServices has been changed or any address has been changed
		//
		bool connectionsChanged = (m_conns.size() != appDataServices.size()) ||
								  std::any_of(m_conns.begin(),
											  m_conns.end(),
											  [&appDataServices](const Connection& conn)
											  {
												  return std::none_of(appDataServices.begin(),
																	  appDataServices.end(),
																	  [&conn](const SoftwareEndpoint::AppDataService& ads)
																	  {
																		  return conn.server() == ads;
																	  });
											  });

		if (connectionsChanged == true)
		{
			m_conns.clear();
			m_signalUpdater.reset();

			// Create connections
			//
			for (const auto& ads : appDataServices)
			{
				m_conns.emplace_back(softwareInfo, ads, m_signalUpdater, m_recentAppSignals, m_signalLog, *m_logFile);
			}
		}

		return;
	}

	std::vector<Tcp::ConnectionState> AdsConnectionPrivate2::connectionStates() const
	{
		QReadLocker locker{&m_connsMutex};

		std::vector<Tcp::ConnectionState> states;
		states.reserve(m_conns.size());

		for (const Connection& c : m_conns)
		{
			states.emplace_back(c.tcpConnectionState());
		}

		return states;
	}

	bool AdsConnectionPrivate2::signalParamsLoaded() const
	{
		QReadLocker locker{&m_connsMutex};
		return std::all_of(m_conns.begin(),
						   m_conns.end(),
						   [](const Connection& c)
						   {
							   return c.signalParamsLoaded();
						   });
	}

	bool AdsConnectionPrivate2::signalStatesLoaded() const
	{
		QReadLocker locker{&m_connsMutex};
		return std::all_of(m_conns.begin(),
						   m_conns.end(),
						   [](const Connection& c)
						   {
							   return c.signalStatesLoaded();
						   });
	}

	SignalLog& AdsConnectionPrivate2::signalLog()
	{
		return m_signalLog;
	}

	const SignalLog& AdsConnectionPrivate2::signalLog() const
	{
		return m_signalLog;
	}

} // namespace ClientLib