#include "AdsConnectionPrivate2.h"
#include "../include/AdsConnectionLib/ClientGrpc.h"

#include <CommonStdLib/TimesStd.h>

#include <expected>
#include <ranges>

namespace
{
	const int ADS_GET_APP_SIGNAL_STATE_MAX = 5000; // Synchronize with SocketIO.h
}


namespace ClientLib
{
	// AdsClientGrpc
	//
	class AdsClientGrpc : public ClientGrpc<Grpc::AppDataSrv>
	{
	public:
		explicit AdsClientGrpc(const ::Network::SoftwareInfo& softwareInfo,
							   const ServiceEndpoint& ads,
							   IAppSignalUpdater& signalUpdater,
							   IRecentAppSignals* recentAppSignals,
							   ISignalLogUpdater* signalLogUpdater,
							   ILoggerStd& logFile);

		virtual ~AdsClientGrpc();

		// Implementing ClientConnectionStatistics
		//
	public:
		virtual std::string statsObjectName() override;

	public:
		std::expected<std::vector<std::string>, std::string> requestSignalList();
		std::expected<std::vector<::Proto::AppSignal>, std::string> requestSignalParams(std::span<Hash> signalHashes = {});
		std::expected<std::vector<::Proto::AppSignalState>, std::string> requestSignalStates(std::span<Hash> signalHashes);
		void requestSignalStatesChanges(std::stop_token stoken, grpc::ClientContext& context, IAppSignalUpdater::SourceIdType sourceId);
		void requestSignalLog(std::stop_token stoken, grpc::ClientContext& context);
		std::expected<void, std::string> requestAckSignalLog();
		std::expected<void, std::string> requestServerTime();

	private:
		virtual void clientCommunicationLoop(std::stop_token stoken) override;
		void clientCommunicationLoopImpl(std::stop_token stoken);

		void stateChangesThreadFunc(std::stop_token stoken, grpc::ClientContext& context, IAppSignalUpdater::SourceIdType sourceId);
		void recentlyUsedThreadFunc(std::stop_token stoken);
		void signalLogThreadFunc(std::stop_token stoken, grpc::ClientContext& context);

	public:
		const ServiceEndpoint& ads() const;

		bool signalParamsLoaded() const;
		bool signalStatesLoaded() const;

	private:
		IAppSignalUpdater& m_signalUpdater;
		IRecentAppSignals* m_recentAppSignals = nullptr; // If nullptr, then is not used.
		ISignalLogUpdater* m_signalLogUpdater = nullptr; // If nullptr, then is not used.

		// --
		//
		std::atomic<bool> m_signalParamsLoaded{false};
		std::atomic<bool> m_signalStatesLoaded{false};
	};


	AdsClientGrpc::AdsClientGrpc(const ::Network::SoftwareInfo& softwareInfo,
								 const ServiceEndpoint& ads,
								 IAppSignalUpdater& signalUpdater,
								 IRecentAppSignals* recentAppSignals,
								 ISignalLogUpdater* signalLogUpdater,
								 ILoggerStd& logFile) :
		ClientGrpc{softwareInfo, ads, logFile, ads.shortenId},
		m_signalUpdater{signalUpdater},
		m_recentAppSignals{recentAppSignals},
		m_signalLogUpdater{signalLogUpdater}
	{
		m_tcpState.name = std::string{"AdsClientGrpc "} + ads.shortenId;
		return;
	}

	AdsClientGrpc::~AdsClientGrpc()
	{
		shutUp();
	}

	std::string AdsClientGrpc::statsObjectName()
	{
		return "AdsClientGrpc";
	}

	std::expected<std::vector<std::string>, std::string> AdsClientGrpc::requestSignalList()
	{
		grpc::ClientContext context;
		createAuthContext(context, std::chrono::seconds(20));

		std::vector<std::string> appSignalIds;
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

			for (auto& appSignalId : *reply.mutable_appsignalids())
			{
				appSignalIds.push_back(std::move(appSignalId));
			}
		}

		grpc::Status status = replyReader->Finish();
		if (status.ok() == false)
		{
			return std::unexpected<std::string>{std::format("Failed to get signal list, error %1", statusToString(status))};
		}

		return appSignalIds;
	}

	std::expected<std::vector<::Proto::AppSignal>, std::string> AdsClientGrpc::requestSignalParams(std::span<Hash> signalHashes /*= {}*/)
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

		std::vector<::Proto::AppSignal> result;

		auto replyReader = m_stub->GetAppSignalParam(&context, request);
		incRequestCount();

		while (replyReader->Read(&reply))
		{
			incReplyCount();

			if (result.capacity() == 0)
			{
				result.reserve(reply.totalsize());
			}

			for (auto& paramProto : *reply.mutable_signalparams())
			{
				result.push_back(std::move(paramProto));
			}
		}

		auto status = replyReader->Finish();
		if (status.ok() == false)
		{
			return std::unexpected<std::string>{std::format("Failed to get signal params, error {}", statusToString(status))};
		}

		return result;
	}

	std::expected<std::vector<::Proto::AppSignalState>, std::string> AdsClientGrpc::requestSignalStates(std::span<Hash> signalHashes)
	{
		Grpc::GetAppSignalStateRequest request;
		Grpc::GetAppSignalStateReply reply;

		// Calculate number of parts needed, rounding up.
		//
		const size_t parts = (signalHashes.size() + ADS_GET_APP_SIGNAL_STATE_MAX - 1) / ADS_GET_APP_SIGNAL_STATE_MAX;

		std::vector<::Proto::AppSignalState> result;
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
				return std::unexpected<std::string>{std::format("Failed to get signal states (part {}/{})", part + 1, parts)};
			}
			else
			{
				incReplyCount();
			}

			for (auto& stateProto : *reply.mutable_appsignalstates())
			{
				result.push_back(stateProto);
			}
		}

		return result;
	}

	void AdsClientGrpc::requestSignalStatesChanges(std::stop_token stoken,
												   grpc::ClientContext& context, IAppSignalUpdater::SourceIdType sourceId)
	{
		m_log.writeMessage(m_logPrefix + std::format("Enter AdsClientGrpc::requestSignalStatesChanges for ADS {}", m_service.to_string()));

		std::stop_callback stopCallback{stoken,
										[&context]()
										{
											// Trigger gRPC cancellation, this makes Read() to return false.
											//
											context.TryCancel();
										}};

		Grpc::GetAppSignalStateChangesRequest request;
		Grpc::GetAppSignalStateChangesReply reply;
		std::vector<::Proto::AppSignalState> states;

		auto replyReader = m_stub->GetAppSignalStateChanges(&context, request);
		incRequestCount();

		while (stoken.stop_requested() == false && replyReader->Read(&reply) == true)
		{
			incReplyCount();

			states.clear();
			states.reserve(reply.appsignalstates_size());

			std::move(reply.mutable_appsignalstates()->begin(), reply.mutable_appsignalstates()->end(), std::back_inserter(states));

			m_signalUpdater.setStates(std::span(states), ::calcHash(m_service.equipmentId), sourceId);

			reply.Clear();
		}

		auto status = replyReader->Finish();

		if (status.ok() == false && status.error_code() != grpc::StatusCode::CANCELLED)
		{
			m_log.writeError(m_logPrefix + std::format("Stream error in requestSignalStatesChanges: {}, {}",
													   statusToString(status),
													   m_service.to_string()));
		}
		else
		{
			m_log.writeMessage(m_logPrefix +
							   std::format("AdsClientGrpc::requestSignalStatesChanges exited normally, ADS {}", m_service.to_string()));
		}

		return;
	}

	void AdsClientGrpc::requestSignalLog(std::stop_token stoken, grpc::ClientContext& context)
	{
		m_log.writeMessage(m_logPrefix + std::format("Enter AdsClientGrpc::requestSignalLog for ADS {}", m_service.to_string()));

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

			m_signalLogUpdater->add(m_service.equipmentId, reply.discreteslogrecord().begin(), reply.discreteslogrecord().end());
			m_signalLogUpdater->deleteUpTo(m_service.equipmentId, reply.logfirstrecordid());

			reply.Clear();
		}

		auto status = replyReader->Finish();

		if (status.ok() == false && status.error_code() != grpc::StatusCode::CANCELLED)
		{
			m_log.writeError(m_logPrefix +
							 std::format("Stream error in requestSignalLog: {}, ADS {}", statusToString(status), m_service.to_string()));
		}
		else
		{
			m_log.writeMessage(m_logPrefix + std::format("AdsClientGrpc::requestSignalLog exited normally, ADS {}", m_service.to_string()));
		}

		return;
	}

	std::expected<void, std::string> AdsClientGrpc::requestAckSignalLog()
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

		auto dt = std::chrono::system_clock::time_point{std::chrono::milliseconds{plantTimeToAck.value()}};
		if (dt == std::chrono::system_clock::time_point{})
		{
			std::string message = std::format("Invalid plantTime to ack discrete logs for ADS {}.", m_service.to_string());
			return std::unexpected<std::string>{message};
		}

		m_log.writeMessage(m_logPrefix + std::format("Acknowledging discrete logs up to plantTime {:%d %b %Y %H:%M:%S}, ADS {}.",
													 std::chrono::floor<std::chrono::milliseconds>(dt),
													 m_service.to_string()));

		grpc::ClientContext context;
		createAuthContext(context, std::chrono::seconds(15));

		Grpc::AckDiscretesLogRequest request;
		Grpc::AckDiscretesLogReply reply;

		request.set_acksource(m_softwareInfo.equipmentid());
		request.set_ackuser("User"); // ???
		request.set_ackuptoplanttime(plantTimeToAck.value());

		auto status = m_stub->AckDiscretesLog(&context, request, &reply);
		incRequestCount();

		if (status.ok() == false)
		{
			return std::unexpected<std::string>{
				std::format("Failed to ack discrete logs up to plantTime {:%d %b %Y %H:%M:%S} for ADS {}, error {}",
							std::chrono::floor<std::chrono::milliseconds>(dt),
							m_service.to_string(),
							statusToString(status))};
		}
		else
		{
			incReplyCount();
		}

		return {};
	}

	std::expected<void, std::string> AdsClientGrpc::requestServerTime()
	{
		using namespace std::chrono;

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
			return std::unexpected{
				std::format("Failed to get server time for ADS {}, error {}", m_service.to_string(), statusToString(status))};
		}

		incReplyCount();

		const auto serverUtcTimeMs = reply.servertimeutc();
		const auto serverLocalTimeMs = reply.servertimelocal();

		const auto utcDiff = clientUtcMs - serverUtcTimeMs;
		const auto localDiff = clientLocalMs - serverLocalTimeMs;

		m_log.writeMessage(
			m_logPrefix +
			std::format("ADS {} server time received. UTC diff: {} ms, Local diff: {} ms", m_service.to_string(), utcDiff, localDiff));

		// 1. UTC time is different?
		//
		if (auto limitMs = static_cast<const int64_t>(3 * 1'000) * 60; // 3 minutes.
			utcDiff > limitMs)
		{
			const auto clientUtcDateTime = system_clock::time_point{milliseconds{clientUtcMs}};
			const auto serverUtcDateTime = system_clock::time_point{milliseconds{serverUtcTimeMs}};

			m_log.writeWarning(m_logPrefix + std::format("UTC time discrepancy detected ({} seconds). Client UTC time {:%d %b %Y "
														 "%H:%M:%S}, server UTC time {:%d %b %Y %H:%M:%S}.",
														 utcDiff / 1000,
														 floor<milliseconds>(clientUtcDateTime),
														 floor<milliseconds>(serverUtcDateTime)));
		}

		// 2. Time zone is different?
		//
		{
			const int64_t serverTimeZoneDiff = serverLocalTimeMs - serverUtcTimeMs;
			const int64_t clientTimeZoneDiff = clientLocalMs - clientUtcMs;
			const int64_t timeZoneDiff = std::abs(serverTimeZoneDiff - clientTimeZoneDiff);

			if (timeZoneDiff != 0)
			{
				auto clientLocalDateTime = std::chrono::system_clock::time_point{std::chrono::milliseconds{clientLocalMs}};
				auto serverLocalDateTime = std::chrono::system_clock::time_point{std::chrono::milliseconds{serverLocalTimeMs}};

				m_log.writeWarning(
					m_logPrefix +
					std::format(
						"TimeZone discrepancy detected. Client local time {:%d %b %Y %H:%M:%S}, server local time {:%d %b %Y %H:%M:%S}.",
						std::chrono::floor<std::chrono::milliseconds>(clientLocalDateTime),
						std::chrono::floor<std::chrono::milliseconds>(serverLocalDateTime)));
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
			m_log.writeError(m_logPrefix + std::format("Exception in AdsClientGrpc::clientCommunicationLoopImpl for ADS {}: {}",
													   m_service.to_string(),
													   e.what()));
		}

		m_signalParamsLoaded.store(false);
		m_signalStatesLoaded.store(false);

		m_log.writeMessage(m_logPrefix + std::format("Worker for ADS {} gRPC client exiting.", m_service.to_string()));
		return;
	}

	void AdsClientGrpc::clientCommunicationLoopImpl(std::stop_token stoken)
	{
#if 0
		auto listResult = requestSignalList();
		if (listResult.has_value() == false)
		{
			m_log.writeError(m_logPrefix + std::format("Failed to get signal list from ADS {}, error: {}", m_service.to_string(), listResult.error()));
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
#endif
		// Get all signals
		//
		std::vector<Hash> signalList;

		{
			auto signalParamsResult = requestSignalParams();
			if (signalParamsResult.has_value() == false)
			{
				m_log.writeError(m_logPrefix + std::format("Failed to get signal params from ADS {}, error: {}",
														   m_service.to_string(),
														   signalParamsResult.error()));

				// We have not received any states yet, but still invalidate any previous states, just to be sure.
				//
				m_signalUpdater.invalidateSignalStates(sourceId());
				return;
			}

			// Filter out bus signals
			//
			std::vector<::Proto::AppSignal> signalParams = std::move(signalParamsResult.value());

			auto signalParamToHash = [](const ::Proto::AppSignal& param) -> Hash
			{
				return ::calcHash(param.appsignalid());
			};

			auto isNotBusSignal = [](const ::Proto::AppSignal& param) -> bool
			{
				return param.bustypeid().empty() == true; // Assume that bus signals have bustypeid set.
			};

			auto signalListView = signalParams | std::views::filter(isNotBusSignal) | std::views::transform(signalParamToHash);
			signalList.reserve(signalParams.size());      // There are likely little bus signals.
			signalList.assign(signalListView.begin(), signalListView.end());

			// Update signal params in the signal updater and notify.
			//
			m_signalUpdater.addSignals(std::span(signalParams), m_service.equipmentId);
			m_signalUpdater.notifySignalParamsUpdated();

			m_signalParamsLoaded.store(true);
		}

		// Get initial signal states.
		//
		{
			auto result = requestSignalStates(signalList);
			if (result.has_value() == false)
			{
				m_log.writeError(m_logPrefix +
								 std::format("Failed to get signal states from ADS {}, error: {}", m_service.to_string(), result.error()));

				m_signalUpdater.invalidateSignalStates(sourceId());
				return;
			}

			m_signalUpdater.setStates(std::span(result.value()), ::calcHash(m_service.equipmentId), sourceId());

			m_signalStatesLoaded.store(true);
		}

		// Start separate thread for getting streamed signal state changes.
		//
		grpc::ClientContext stateChangesContext;
		createAuthContext(stateChangesContext);

		std::jthread stateChangesThread{
			[this](std::stop_token stoken, grpc::ClientContext& context, IAppSignalUpdater::SourceIdType sourceId)
			{
				return stateChangesThreadFunc(stoken, context, sourceId);
			},
			std::ref(stateChangesContext),
			sourceId()};

		grpc::ClientContext signalLogContext;
		createAuthContext(signalLogContext);
		std::jthread signalLogThread{[this](std::stop_token stoken, grpc::ClientContext& context)
									 {
										 return signalLogThreadFunc(stoken, context);
									 },
									 std::ref(signalLogContext)};

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
					m_log.writeError(m_logPrefix + std::format("Failed to get signal states from ADS {}, error: {}",
															   m_service.to_string(),
															   result.error()));
					break;
				}

				m_signalUpdater.setStates(std::span(result.value()), ::calcHash(m_service.equipmentId), sourceId());

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
				m_log.writeError(m_logPrefix + std::format("Failed to ack discrete logs from ADS {}, error: {}",
														   m_service.to_string(),
														   ackResult.error()));
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
					m_log.writeWarning(m_logPrefix + std::format("Failed to get server time from ADS {}, error: {}",
																 m_service.to_string(),
																 result.error()));
				}
			}
		}

		stateChangesThread.request_stop();
		stateChangesThread.join();

		signalLogThread.request_stop();
		signalLogThread.join();

		if (recentSignalsThread.joinable() == true)
		{
			assert(m_recentAppSignals != nullptr);
			recentSignalsThread.request_stop();
			recentSignalsThread.join();
		}

		m_signalUpdater.invalidateSignalStates(sourceId());

		return;
	}

	void AdsClientGrpc::stateChangesThreadFunc(std::stop_token stoken,
											   grpc::ClientContext& context,
											   IAppSignalUpdater::SourceIdType sourceId)
	{
		m_log.writeMessage(m_logPrefix + std::format("Enter listening for signal state changes for ADS {}.", m_service.to_string()));

		try
		{
			requestSignalStatesChanges(stoken, context, sourceId);
		}
		catch (std::exception& e)
		{
			m_log.writeError(m_logPrefix + std::format("Exception in AdsClientGrpc::requestSignalStatesChanges for ADS {}: {}",
													   m_service.to_string(),
													   e.what()));
		}

		m_log.writeMessage(m_logPrefix + std::format("Leaving listening for signal state changes for ADS {}.", m_service.to_string()));
		return;
	}

	void AdsClientGrpc::recentlyUsedThreadFunc(std::stop_token stoken)
	{
		// Set separate source ID for this thread.
		//
		auto sourceId = static_cast<IAppSignalUpdater::SourceIdType>(std::hash<std::thread::id>{}(std::this_thread::get_id()));

		m_log.writeMessage(m_logPrefix + std::format("Enter recently used signals thread for ADS {}.", m_service.to_string()));

		while (stoken.stop_requested() == false)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			try
			{
				auto hashes = m_recentAppSignals->recentlyUsedAppSignals(m_service.equipmentId);
				auto reply = requestSignalStates(hashes);

				if (reply.has_value() == false)
				{
					m_log.writeError(m_logPrefix + std::format("Failed to get recently used signal states from ADS {}, error: {}",
															   m_service.to_string(),
															   reply.error()));
					continue;
				}

				m_signalUpdater.setStates(std::span(reply.value()), ::calcHash(m_service.equipmentId), sourceId);
			}
			catch (std::exception& e)
			{
				m_signalUpdater.invalidateSignalStates(sourceId);

				m_log.writeError(m_logPrefix + std::format("Exception in AdsClientGrpc::recentlyUsedThreadFunc for ADS {}: {}",
														   m_service.to_string(),
														   e.what()));
				continue;
			}
		}

		m_signalUpdater.invalidateSignalStates(sourceId);

		m_log.writeMessage(m_logPrefix + std::format("Leaving recently used signals thread for ADS {}.", m_service.to_string()));

		return;
	}

	void AdsClientGrpc::signalLogThreadFunc(std::stop_token stoken, grpc::ClientContext& context)
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
				requestSignalLog(stoken, context);
			}
			catch (std::exception& e)
			{
				m_log.writeError(m_logPrefix + std::format("Exception in AdsClientGrpc::signalLogThreadFunc for ADS {}: {}",
														   m_service.to_string(),
														   e.what()));
				continue;
			}
		}

		return;
	}

	const ServiceEndpoint& AdsClientGrpc::ads() const
	{
		return m_service;
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
	AdsConnectionPrivate2::Connection::Connection(const ::Network::SoftwareInfo& softwareInfo,
												  const ServiceEndpoint& ads,
												  IAppSignalUpdater& signalUpdater,
												  IRecentAppSignals* recentAppSignals,
												  ISignalLogUpdater* signalLogUpdater,
												  ILoggerStd& logFile) :
		m_logFile{logFile},
		m_client{std::make_unique<ClientLib::AdsClientGrpc>(softwareInfo, ads, signalUpdater, recentAppSignals, signalLogUpdater, logFile)}
	{
		return;
	}

	AdsConnectionPrivate2::Connection::~Connection()
	{
		return;
	}

	const ServiceEndpoint& AdsConnectionPrivate2::Connection::server() const
	{
		return m_client->ads();
	}

	ServiceConnectionState AdsConnectionPrivate2::Connection::tcpConnectionState() const
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
												 ILoggerStd& logFile) :
		m_signalUpdater{signalUpdater},
		m_recentAppSignals{recentAppSignals},
		m_signalLogUpdater{signalLogUpdater},
		m_logFile{logFile}
	{
		m_logFile.writeMessage("AdsConnectionPrivate2::AdsConnectionPrivate2()");
		return;
	}

	AdsConnectionPrivate2::~AdsConnectionPrivate2()
	{
		m_conns.clear();
		m_logFile.writeMessage("AdsConnectionPrivate2::~AdsConnectionPrivate2()");
	}

	void AdsConnectionPrivate2::updateConnections(const ::Network::SoftwareInfo& softwareInfo,
												  const std::vector<ServiceEndpoint>& appDataServices)
	{
		m_logFile.writeMessage(std::format("updateConnections(), {} AppDataServices", appDataServices.size()));
		for (const auto& ads : appDataServices)
		{
			m_logFile.writeMessage(std::format("updateConnections(), {}", ads.to_string()));
		}

		std::unique_lock lock{m_connsMutex};

		// Number of AppDataServices has been changed or any address has been changed
		//
		bool connectionsChanged =
			(m_conns.size() != appDataServices.size()) || std::any_of(m_conns.begin(),
																	  m_conns.end(),
																	  [&appDataServices](const Connection& conn)
																	  {
																		  return std::none_of(appDataServices.begin(),
																							  appDataServices.end(),
																							  [&conn](const ServiceEndpoint& ads)
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
				m_conns.emplace_back(softwareInfo, ads, m_signalUpdater, m_recentAppSignals, m_signalLogUpdater, m_logFile);
			}
		}

		return;
	}

	std::vector<ServiceConnectionState> AdsConnectionPrivate2::connectionStates() const
	{
		std::shared_lock lock{m_connsMutex};

		std::vector<ServiceConnectionState> states;
		states.reserve(m_conns.size());

		for (const auto& c : m_conns)
		{
			states.emplace_back(c.tcpConnectionState());
		}

		return states;
	}

	bool AdsConnectionPrivate2::signalParamsLoaded() const
	{
		std::shared_lock lock{m_connsMutex};
		return std::all_of(m_conns.begin(),
						   m_conns.end(),
						   [](const Connection& c)
						   {
							   return c.signalParamsLoaded();
						   });
	}

	bool AdsConnectionPrivate2::signalStatesLoaded() const
	{
		std::shared_lock lock{m_connsMutex};
		return std::all_of(m_conns.begin(),
						   m_conns.end(),
						   [](const Connection& c)
						   {
							   return c.signalStatesLoaded();
						   });
	}
} // namespace ClientLib