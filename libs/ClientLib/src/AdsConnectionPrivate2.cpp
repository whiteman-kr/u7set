#ifndef CLIENT_LIB_DOMAIN
	#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "AdsConnectionPrivate2.h"
#include "ClientGrpc.h"

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
							   IRecentAppSignals* recentAppSignals,
							   ISignalLogUpdater* signalLogUpdater,
							   ILogFile& logFile);

		virtual ~AdsClientGrpc();

		// Implementing ClientConnectionStatistics
		//
	public:
		virtual QString statsObjectName() override;

	public:
		std::expected<QStringList, QString> requestSignalList();
		std::expected<std::vector<AppSignalParam>, QString> requestSignalParams(std::span<Hash> signalHashes = {});
		std::expected<std::vector<AppSignalState>, QString> requestSignalStates(std::span<Hash> signalHashes);
		void requestSignalStatesChanges(std::stop_token stoken, IAppSignalUpdater::SourceIdType sourceId);
		void requestSignalLog(std::stop_token stoken);
		std::expected<void, QString> requestAckSignalLog();
		std::expected<void, QString> requestServerTime();

	private:
		virtual void clientCommunicationLoop(std::stop_token stoken) override;
		void clientCommunicationLoopImpl(std::stop_token stoken);

		void stateChangesThreadFunc(std::stop_token stoken, IAppSignalUpdater::SourceIdType sourceId);
		void recentlyUsedThreadFunc(std::stop_token stoken);
		void signalLogThreadFunc(std::stop_token stoken);

	public:
		const SoftwareEndpoint::AppDataService& ads() const;

		bool signalParamsLoaded() const;
		bool signalStatesLoaded() const;

	private:
		const SoftwareEndpoint::AppDataService m_ads;
		IAppSignalUpdater& m_signalUpdater;
		IRecentAppSignals* m_recentAppSignals = nullptr; // If nullptr, then is not used.
		ISignalLogUpdater* m_signalLogUpdater = nullptr; // If nullptr, then is not used.

		// --
		//
		std::atomic<bool> m_signalParamsLoaded{false};
		std::atomic<bool> m_signalStatesLoaded{false};
	};


	AdsClientGrpc::AdsClientGrpc(const SoftwareInfo& softwareInfo,
								 const SoftwareEndpoint::AppDataService& ads,
								 IAppSignalUpdater& signalUpdater,
								 IRecentAppSignals* recentAppSignals,
								 ISignalLogUpdater* signalLogUpdater,
								 ILogFile& logFile) :
		ClientGrpc{softwareInfo, ads.equipmentId, ads.address, logFile, ads.shortenId},
		m_ads{ads},
		m_signalUpdater{signalUpdater},
		m_recentAppSignals{recentAppSignals},
		m_signalLogUpdater{signalLogUpdater}
	{
		m_tcpState.name = "AdsClientGrpc " + ads.shortenId;
		return;
	}

	AdsClientGrpc::~AdsClientGrpc()
	{
		shutUp();
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

	void AdsClientGrpc::requestSignalStatesChanges(std::stop_token stoken, IAppSignalUpdater::SourceIdType sourceId)
	{
		m_log.writeMessage(m_logPrefix + QString{"Enter AdsClientGrpc::requestSignalStatesChanges for ADS %1 at address %2"}
											 .arg(m_ads.equipmentId)
											 .arg(m_ads.address.toString()));

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

			m_signalUpdater.setStates(std::span(states), ::calcHash(m_serviceEquipmentId), sourceId);
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

		if (status.ok() == false && status.error_code() != grpc::StatusCode::CANCELLED)
		{
			m_log.writeError(m_logPrefix + QString{"Stream error in requestSignalStatesChanges: %1, ADS %2, Address %3"}
											   .arg(statusToString(status))
											   .arg(m_ads.equipmentId)
											   .arg(m_ads.address.toString()));
		}
		else
		{
			m_log.writeMessage(m_logPrefix + QString{"AdsClientGrpc::requestSignalStatesChanges exited normally, ADS %1, Address %2"}
												 .arg(m_ads.equipmentId)
												 .arg(m_ads.address.toString()));
		}

		return;
	}

	void AdsClientGrpc::requestSignalLog(std::stop_token stoken)
	{
		m_log.writeMessage(
			m_logPrefix +
			QString{"Enter AdsClientGrpc::requestSignalLog for ADS %1 at address %2"}.arg(m_ads.equipmentId).arg(m_ads.address.toString()));

		grpc::ClientContext context;
		createAuthContext(context);

		std::stop_callback stopCallback{stoken,
										[&context]()
										{
											// Trigger gRPC cancellation, this makes Read() to return false.
											//
											context.TryCancel();
										}};

		Grpc::GetDiscretesLogRequest request;
		Grpc::GetDiscretesLogReply reply;

		auto replyReader = m_stub->GetDiscretesLog(&context, request);
		incRequestCount();

		while (m_signalLogUpdater != nullptr && m_signalLogUpdater->enabled() == true && stoken.stop_requested() == false &&
			   replyReader->Read(&reply) == true)
		{
			incReplyCount();

			m_signalLogUpdater->add(m_ads.equipmentId.toStdString(), reply.discreteslogrecord().begin(), reply.discreteslogrecord().end());
			m_signalLogUpdater->deleteUpTo(m_ads.equipmentId.toStdString(), reply.logfirstrecordid());

			reply.Clear();
		}

		auto status = replyReader->Finish();

		if (status.ok() == false && status.error_code() != grpc::StatusCode::CANCELLED)
		{
			m_log.writeError(m_logPrefix + QString{"Stream error in requestSignalLog: %1, ADS %2, Address %3"}
											   .arg(statusToString(status))
											   .arg(m_ads.equipmentId)
											   .arg(m_ads.address.toString()));
		}
		else
		{
			m_log.writeMessage(m_logPrefix + QString{"AdsClientGrpc::requestSignalLog exited normally, ADS %1, Address %2"}
												 .arg(m_ads.equipmentId)
												 .arg(m_ads.address.toString()));
		}

		return;
	}

	std::expected<void, QString> AdsClientGrpc::requestAckSignalLog()
	{
		if (m_signalLogUpdater == nullptr || m_signalLogUpdater->enabled() == false)
		{
			return {};
		}

		auto plantTimeToAck = m_signalLogUpdater->getNextAckUpTo();
		if (plantTimeToAck.has_value() == false)
		{
			return {};
		}

		auto dt = TimeStamp{plantTimeToAck.value()}.toDateTime();
		if (dt.isValid() == false)
		{
			QString message = QString("Invalid plantTime to ack discrete logs for ADS %1.").arg(m_ads.equipmentId);
			return std::unexpected<QString>{message};
		}

		m_log.writeMessage(m_logPrefix + QString("Acknowledging discrete logs up to plantTime %1, ADS %2.")
											 .arg(dt.toString("dd MMM yyyy hh:mm:ss.zzz"))
											 .arg(m_ads.equipmentId));

		grpc::ClientContext context;
		createAuthContext(context, std::chrono::seconds(15));

		Grpc::AckDiscretesLogRequest request;
		Grpc::AckDiscretesLogReply reply;

		request.set_acksource(m_softwareInfo.equipmentID().toStdString());
		request.set_ackuser("User"); // ???
		request.set_ackuptoplanttime(plantTimeToAck.value());

		auto status = m_stub->AckDiscretesLog(&context, request, &reply);
		incRequestCount();

		if (status.ok() == false)
		{
			return std::unexpected<QString>{QString("Failed to ack discrete logs up to plantTime %1 for ADS %2, error %3")
												.arg(dt.toString("dd MMM yyyy hh:mm:ss.zzz"))
												.arg(m_ads.equipmentId)
												.arg(statusToString(status))};
		}
		else
		{
			incReplyCount();
		}

		return {};
	}

	std::expected<void, QString> AdsClientGrpc::requestServerTime()
	{
		grpc::ClientContext context;
		createAuthContext(context, std::chrono::seconds(5));

		Grpc::GetServerTimeRequest request;
		Grpc::GetServerTimeReply reply;

		auto status = m_stub->GetServerTime(&context, request, &reply);

		// The earliest possible point of getting local time after sending request.
		//
		const auto clientUtcMs = currentMSecsSinceEpoch();
		const auto clientLocalMs = currentMSecsLocal();

		incRequestCount();

		if (status.ok() == false)
		{
			return std::unexpected<QString>{QString("Failed to get server time for ADS %1, address %2, error %3")
												.arg(m_ads.equipmentId)
												.arg(m_ads.address.toString())
												.arg(statusToString(status))};
		}

		incReplyCount();

		const auto serverUtcTimeMs = reply.servertimeutc();
		const auto serverLocalTimeMs = reply.servertimelocal();

		const auto utcDiff = clientUtcMs - serverUtcTimeMs;
		const auto localDiff = clientLocalMs - serverLocalTimeMs;

		m_log.writeMessage(m_logPrefix + QString{"ADS %1 at address %2 server time received. UTC diff: %3 ms, Local diff: %4 ms"}
											 .arg(m_ads.equipmentId)
											 .arg(m_ads.address.toString())
											 .arg(utcDiff)
											 .arg(localDiff));


		// 1. UTC time is different?
		//
		if (const qint64 limitMs = static_cast<qint64>(3 * 1'000) * 60; // 3 minutes.
			utcDiff > limitMs)
		{
			const auto clientUtcDateTime = QDateTime::fromMSecsSinceEpoch(clientUtcMs, QTimeZone::UTC);
			const auto serverUtcDateTime = QDateTime::fromMSecsSinceEpoch(serverUtcTimeMs, QTimeZone::UTC);

			m_log.writeWarning(QString("UTC time discrepancy detected (%1 seconds). Client UTC time %2, server UTC time %3.")
								   .arg(utcDiff / 1000)
								   .arg(clientUtcDateTime.toString("dd MMM yyyy hh:mm:ss.zzz"))
								   .arg(serverUtcDateTime.toString("dd MMM yyyy hh:mm:ss.zzz")));
		}

		// 2. Time zone is different?
		//
		{
			const qint64 serverTimeZoneDiff = serverLocalTimeMs - serverUtcTimeMs;
			const qint64 clientTimeZoneDiff = clientLocalMs - clientUtcMs;
			const qint64 timeZoneDiff = std::abs(serverTimeZoneDiff - clientTimeZoneDiff);

			if (timeZoneDiff != 0)
			{
				auto clientLocalDateTime = QDateTime::fromMSecsSinceEpoch(clientLocalMs, QTimeZone::UTC);
				auto serverLocalDateTime = QDateTime::fromMSecsSinceEpoch(serverLocalTimeMs, QTimeZone::UTC);

				m_log.writeWarning(QString("TimeZone discrepancy detected. Client local time %1, server local time %2.")
									   .arg(clientLocalDateTime.toString("dd MMM yyyy hh:mm:ss.zzz"))
									   .arg(serverLocalDateTime.toString("dd MMM yyyy hh:mm:ss.zzz")));
			}
		}

		return {};
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

				// We have not received any states yet, but still invalidate any previous states, just to be sure.
				//
				m_signalUpdater.invalidateSignalStates(sourceId());
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

				m_signalUpdater.invalidateSignalStates(sourceId());
				return;
			}

			m_signalUpdater.setStates(std::span(result.value()), ::calcHash(m_serviceEquipmentId), sourceId());

			m_signalStatesLoaded.store(true);
		}

		// Start separate thread for getting streamed signal state changes.
		//
		std::jthread stateChangesThread{[this](std::stop_token stoken, IAppSignalUpdater::SourceIdType sourceId)
										{
											return stateChangesThreadFunc(stoken, sourceId);
										},
										sourceId()};

		std::jthread signalLogThread{[this](std::stop_token stoken)
									 {
										 return signalLogThreadFunc(stoken);
									 }};

		// Start separate thread for updating recently used signals.
		//
		std::jthread recentSignalsThread;
		if (m_recentAppSignals != nullptr)
		{
			recentSignalsThread = std::jthread{[this](std::stop_token stoken)
											   {
												   recentlyUsedThreadFunc(stoken);
											   }};
		}

		// Loop, getting signal states periodically.
		//
		constexpr size_t RepeatedlyGetStateCount =
			ADS_GET_APP_SIGNAL_STATE_MAX / 16; // 312 states * 10 requests/sec = 3120 states/sec, or 187'200 states/minute.
		size_t statePart = 0;

		std::chrono::days todayDays{0};

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
					break;
				}

				m_signalUpdater.setStates(std::span(result.value()), ::calcHash(m_serviceEquipmentId), sourceId());

				statePart++;
				if (statePart * RepeatedlyGetStateCount >= signalList.size())
				{
					statePart = 0;
				}
			}

			// Send SignalLock ACKs
			//
			if (auto ackResult = requestAckSignalLog(); //
				ackResult.has_value() == false)
			{
				m_log.writeError(m_logPrefix + QString{"Failed to ack discrete logs from ADS %1, address %2, error: %3"}
												   .arg(m_ads.equipmentId)
												   .arg(m_ads.address.toString())
												   .arg(ackResult.error()));
			}

			// Control server time discrepancy, once a day.
			//
			if (auto nowDays = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()).time_since_epoch(); //
				nowDays != todayDays)
			{
				todayDays = nowDays;

				auto result = requestServerTime();
				if (result.has_value() == false)
				{
					m_log.writeWarning(m_logPrefix + QString{"Failed to get server time from ADS %1, address %2, error: %3"}
														 .arg(m_ads.equipmentId)
														 .arg(m_ads.address.toString())
														 .arg(result.error()));
				}
			}
		}

		stateChangesThread.request_stop();
		stateChangesThread.join();

		signalLogThread.request_stop();
		signalLogThread.join();

		if (recentSignalsThread.joinable() == true)
		{
			Q_ASSERT(m_recentAppSignals != nullptr);
			recentSignalsThread.request_stop();
			recentSignalsThread.join();
		}

		m_signalUpdater.invalidateSignalStates(sourceId());

		return;
	}

	void AdsClientGrpc::stateChangesThreadFunc(std::stop_token stoken, IAppSignalUpdater::SourceIdType sourceId)
	{
		m_log.writeMessage(m_logPrefix + QString{"Enter listening for signal state changes for ADS %1 at address %2: %3"}
											 .arg(m_ads.equipmentId)
											 .arg(m_ads.address.toString()));

		try
		{
			requestSignalStatesChanges(stoken, sourceId);
		}
		catch (std::exception& e)
		{
			m_log.writeError(m_logPrefix + QString{"Exception in AdsClientGrpc::requestSignalStatesChanges for ADS %1 at address %2: %3"}
											   .arg(m_ads.equipmentId)
											   .arg(m_ads.address.toString())
											   .arg(e.what()));
		}

		m_log.writeMessage(m_logPrefix + QString{"Leaving listening for signal state changes for ADS %1 at address %2: %3"}
											 .arg(m_ads.equipmentId)
											 .arg(m_ads.address.toString()));
		return;
	}

	void AdsClientGrpc::recentlyUsedThreadFunc(std::stop_token stoken)
	{
		// Set separate source ID for this thread.
		//
		auto sourceId = static_cast<IAppSignalUpdater::SourceIdType>(std::hash<std::thread::id>{}(std::this_thread::get_id()));

		m_log.writeMessage(
			m_logPrefix +
			QString{"Enter recently used signals thread for ADS %1 at address %2"}.arg(m_ads.equipmentId).arg(m_ads.address.toString()));

		while (stoken.stop_requested() == false)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			try
			{
				auto hashes = m_recentAppSignals->recentlyUsedAppSignals(m_ads.equipmentId);
				auto reply = requestSignalStates(hashes);

				if (reply.has_value() == false)
				{
					m_log.writeError(m_logPrefix + QString{"Failed to get recently used signal states from ADS %1, address %2, error: %3"}
													   .arg(m_ads.equipmentId)
													   .arg(m_ads.address.toString())
													   .arg(reply.error()));
					continue;
				}

				m_signalUpdater.setStates(std::span(reply.value()), ::calcHash(m_ads.equipmentId), sourceId);
			}
			catch (std::exception& e)
			{
				m_signalUpdater.invalidateSignalStates(sourceId);

				m_log.writeError(m_logPrefix + QString{"Exception in AdsClientGrpc::recentlyUsedThreadFunc for ADS %1 at address %2: %3"}
												   .arg(m_ads.equipmentId)
												   .arg(m_ads.address.toString())
												   .arg(e.what()));
				continue;
			}
		}

		m_signalUpdater.invalidateSignalStates(sourceId);

		m_log.writeMessage(
			m_logPrefix +
			QString{"Leaving recently used signals thread for ADS %1 at address %2"}.arg(m_ads.equipmentId).arg(m_ads.address.toString()));

		return;
	}

	void AdsClientGrpc::signalLogThreadFunc(std::stop_token stoken)
	{
		if (m_signalLogUpdater == nullptr)
		{
			return;
		}

		while (stoken.stop_requested() == false)
		{
			if (m_signalLogUpdater->enabled() == false)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}

			try
			{
				requestSignalLog(stoken);
			}
			catch (std::exception& e)
			{
				m_log.writeError(m_logPrefix + QString{"Exception in AdsClientGrpc::signalLogThreadFunc for ADS %1 at address %2: %3"}
												   .arg(m_ads.equipmentId)
												   .arg(m_ads.address.toString())
												   .arg(e.what()));
				continue;
			}
		}

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
												  IRecentAppSignals* recentAppSignals,
												  ISignalLogUpdater* signalLogUpdater,
												  ILogFile& logFile) :
		m_client{std::make_unique<ClientLib::AdsClientGrpc>(softwareInfo, ads, signalUpdater, recentAppSignals, signalLogUpdater, logFile)}
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
	AdsConnectionPrivate2::AdsConnectionPrivate2(IAppSignalUpdater& signalUpdater,
												 IRecentAppSignals* recentAppSignals,
												 ISignalLogUpdater* signalLogUpdater,
												 ILogFile* logFile) :
		m_signalUpdater{signalUpdater},
		m_recentAppSignals{recentAppSignals},
		m_signalLogUpdater{signalLogUpdater},
		m_logFile{logFile, "AdsConnectionPrivate2"}
	{
		m_logFile.writeMessage("AdsConnectionPrivate2::AdsConnectionPrivate2()");
		return;
	}

	AdsConnectionPrivate2::~AdsConnectionPrivate2()
	{
		m_logFile.writeMessage("AdsConnectionPrivate2::~AdsConnectionPrivate2()");
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
				m_conns.emplace_back(softwareInfo, ads, m_signalUpdater, m_recentAppSignals, m_signalLogUpdater, *m_logFile);
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
} // namespace ClientLib