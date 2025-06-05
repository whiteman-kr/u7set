#include "SimServiceClientImpl.h"

#include <grpcpp/create_channel.h>

#include <chrono>


namespace
{
	std::chrono::milliseconds RequestTimeOuts{10'000};

	QString formatErrorMessage(::grpc::Status status)
	{
		QString result;

		static const std::unordered_map<grpc::StatusCode, QString> errorCodeMap = {
			{grpc::StatusCode::OK, QStringLiteral("OK")},
			{grpc::StatusCode::CANCELLED, QStringLiteral("CANCELLED")},
			{grpc::StatusCode::UNKNOWN, QStringLiteral("UNKNOWN")},
			{grpc::StatusCode::INVALID_ARGUMENT, QStringLiteral("INVALID_ARGUMENT")},
			{grpc::StatusCode::DEADLINE_EXCEEDED, QStringLiteral("DEADLINE_EXCEEDED")},
			{grpc::StatusCode::NOT_FOUND, QStringLiteral("NOT_FOUND")},
			{grpc::StatusCode::ALREADY_EXISTS, QStringLiteral("ALREADY_EXISTS")},
			{grpc::StatusCode::PERMISSION_DENIED, QStringLiteral("PERMISSION_DENIED")},
			{grpc::StatusCode::RESOURCE_EXHAUSTED, QStringLiteral("RESOURCE_EXHAUSTED")},
			{grpc::StatusCode::FAILED_PRECONDITION, QStringLiteral("FAILED_PRECONDITION")},
			{grpc::StatusCode::ABORTED, QStringLiteral("ABORTED")},
			{grpc::StatusCode::OUT_OF_RANGE, QStringLiteral("OUT_OF_RANGE")},
			{grpc::StatusCode::UNIMPLEMENTED, QStringLiteral("UNIMPLEMENTED")},
			{grpc::StatusCode::INTERNAL, QStringLiteral("INTERNAL")},
			{grpc::StatusCode::UNAVAILABLE, QStringLiteral("UNAVAILABLE")},
			{grpc::StatusCode::DATA_LOSS, QStringLiteral("DATA_LOSS")},
			{grpc::StatusCode::UNAUTHENTICATED, QStringLiteral("UNAUTHENTICATED")}};

		auto it = errorCodeMap.find(status.error_code());
		if (it != errorCodeMap.end())
		{
			result = it->second;
		}
		else
		{
			result = QStringLiteral("UNKNOWN_ERROR_CODE");
		}

		result += QString{" %1"}.arg(QString::fromStdString(status.error_message()));
		return result;
	}
} // namespace

namespace Sim
{
	SimServiceClientImpl::SimServiceClientImpl(QString address, [[maybe_unused]] bool only_as_shared_ptr) :
		m_channel{grpc::CreateChannel(address.toStdString(), grpc::InsecureChannelCredentials())},
		m_stub{RpctGrpc::SimService::NewStub(m_channel)}
	{
	}

	SimServiceClientImpl::~SimServiceClientImpl() = default;

	SimServiceClient::ChannelState SimServiceClientImpl::channelState()
	{
		auto state = m_channel->GetState(true);
		using StateType = decltype(state);

		static_assert(static_cast<StateType>(GRPC_CHANNEL_IDLE) == StateType::GRPC_CHANNEL_IDLE);
		static_assert(static_cast<StateType>(GRPC_CHANNEL_CONNECTING) == StateType::GRPC_CHANNEL_CONNECTING);
		static_assert(static_cast<StateType>(GRPC_CHANNEL_READY) == StateType::GRPC_CHANNEL_READY);
		static_assert(static_cast<StateType>(GRPC_CHANNEL_TRANSIENT_FAILURE) == StateType::GRPC_CHANNEL_TRANSIENT_FAILURE);
		static_assert(static_cast<StateType>(GRPC_CHANNEL_SHUTDOWN) == StateType::GRPC_CHANNEL_SHUTDOWN);

		return static_cast<SimServiceClient::ChannelState>(state);
	}

	bool SimServiceClientImpl::connected()
	{
		return channelState() == SimServiceClient::GRPC_CHANNEL_READY;
	}

	tl::expected<QByteArray, QString> SimServiceClientImpl::Ping(const QByteArray& data)
	{
		grpc::ClientContext context;
		auto deadline = std::chrono::system_clock::now() + RequestTimeOuts;
		context.set_deadline(deadline);

		static thread_local RpctGrpc::PingRequest request;
		request.Clear();
		request.set_payload(data.toStdString());

		static thread_local RpctGrpc::PongReply reply;
		reply.Clear();

		grpc::Status status = m_stub->Ping(&context, request, &reply);
		if (status.ok() == false)
		{
			return tl::unexpected{formatErrorMessage(status)};
		}

		return QByteArray(reply.payload().data(), reply.payload().size());
	}

	tl::expected<SimServiceClient::SimulatorStatus, QString> SimServiceClientImpl::GetStatus()
	{
		grpc::ClientContext context;
		auto deadline = std::chrono::system_clock::now() + RequestTimeOuts;
		context.set_deadline(deadline);

		static thread_local RpctGrpc::GetStatusRequest request;
		static thread_local RpctGrpc::GetStatusReply reply;
		request.Clear();
		reply.Clear();

		grpc::Status status = m_stub->GetStatus(&context, request, &reply);
		if (status.ok() == false)
		{
			return tl::unexpected{formatErrorMessage(status)};
		}

		SimServiceClient::SimulatorStatus result;

		result.project = QString::fromStdString(reply.project());
		result.state = static_cast<SimServiceClient::State>(reply.state());

		static_assert(SimServiceClient::STATE_STOPPED == static_cast<SimServiceClient::State>(::RpctGrpc::SimulatorState::STATE_STOPPED));
		static_assert(SimServiceClient::STATE_RUNNING == static_cast<SimServiceClient::State>(::RpctGrpc::SimulatorState::STATE_RUNNING));
		static_assert(SimServiceClient::STATE_PAUSED == static_cast<SimServiceClient::State>(::RpctGrpc::SimulatorState::STATE_PAUSED));

		return result;
	}

	tl::expected<SimServiceClient::State, QString> SimServiceClientImpl::CommandStart(int64_t durationMcs, const QStringList& logicModules)
	{
		grpc::ClientContext context;
		auto deadline = std::chrono::system_clock::now() + RequestTimeOuts;
		context.set_deadline(deadline);

		static thread_local RpctGrpc::CommandStartRequest request;
		static thread_local RpctGrpc::CommandStartReply reply;
		request.Clear();
		reply.Clear();

		for (const auto& logicModule : logicModules)
		{
			request.mutable_logicmodules()->Add(logicModule.toStdString());
		}
		request.set_durationmcs(durationMcs);
		request.set_speedfactor(1.0);

		grpc::Status status = m_stub->CommandStart(&context, request, &reply);
		if (status.ok() == false)
		{
			return tl::unexpected{formatErrorMessage(status)};
		}

		SimServiceClient::State state = static_cast<SimServiceClient::State>(reply.state());
		return state;
	}

	tl::expected<SimServiceClient::State, QString> SimServiceClientImpl::CommandPause()
	{
		grpc::ClientContext context;
		auto deadline = std::chrono::system_clock::now() + RequestTimeOuts;
		context.set_deadline(deadline);

		static thread_local RpctGrpc::CommandPauseRequest request;
		static thread_local RpctGrpc::CommandPauseReply reply;
		request.Clear();
		reply.Clear();

		grpc::Status status = m_stub->CommandPause(&context, request, &reply);
		if (status.ok() == false)
		{
			return tl::unexpected{formatErrorMessage(status)};
		}

		SimServiceClient::State state = static_cast<SimServiceClient::State>(reply.state());
		return state;
	}

	tl::expected<SimServiceClient::State, QString> SimServiceClientImpl::CommandStop()
	{
		grpc::ClientContext context;
		auto deadline = std::chrono::system_clock::now() + RequestTimeOuts;
		context.set_deadline(deadline);

		static thread_local RpctGrpc::CommandStopRequest request;
		static thread_local RpctGrpc::CommandStopReply reply;
		request.Clear();
		reply.Clear();

		grpc::Status status = m_stub->CommandStop(&context, request, &reply);
		if (status.ok() == false)
		{
			return tl::unexpected{formatErrorMessage(status)};
		}

		SimServiceClient::State state = static_cast<SimServiceClient::State>(reply.state());
		return state;
	}

	tl::expected<std::vector<Sim::SimServiceModule>, QString> SimServiceClientImpl::GetModule(const QStringList& equipmentIds)
	{
		grpc::ClientContext context;
		RpctGrpc::GetModuleRequest request;
		RpctGrpc::GetModuleReply reply;

		for (const auto& equipmentId : equipmentIds)
		{
			request.add_equipmentid()->assign(equipmentId.toStdString());
		}

		grpc::Status status = m_stub->GetModule(&context, request, &reply);
		if (status.ok() == false)
		{
			return tl::unexpected{formatErrorMessage(status)};
		}

		std::vector<Sim::SimServiceModule> result;
		result.reserve(reply.modules_size());

		for (const auto moduleData : reply.modules())
		{
			SimServiceModule module{shared_from_this(), std::make_shared<::RpctGrpc::ModuleState>(std::move(moduleData))};
			result.emplace_back(std::move(module));
		}

		return result;
	}

	tl::expected<::RpctGrpc::SetModuleFlagReply, QString> SimServiceClientImpl::SetModuleFlag(const QString& equipmentId,
																							  int32_t flagId,
																							  bool value)
	{
		grpc::ClientContext context;
		RpctGrpc::SetModuleFlagRequest request;
		RpctGrpc::SetModuleFlagReply reply;

		request.set_equipmentid(equipmentId.toStdString());

		auto f = request.add_flags();
		f->set_id(static_cast<::RpctGrpc::ModuleFlagId>(flagId));
		f->mutable_value()->set_boolvalue(value);

		grpc::Status status = m_stub->SetModuleFlag(&context, request, &reply);
		if (status.ok() == false)
		{
			return tl::unexpected{formatErrorMessage(status)};
		}

		return std::move(reply);
	}

	tl::expected<QStringList, QString> SimServiceClientImpl::GetSignalList()
	{
		static thread_local RpctGrpc::GetSignalListRequest request;
		static thread_local RpctGrpc::GetSignalListReply reply;

		grpc::ClientContext context;
		auto deadline = std::chrono::system_clock::now() + RequestTimeOuts;
		context.set_deadline(deadline);

		request.Clear();
		reply.Clear();

		grpc::Status status = m_stub->GetSignalList(&context, request, &reply);
		if (status.ok() == false)
		{
			return tl::unexpected{formatErrorMessage(status)};
		}

		QStringList signalIds;
		signalIds.reserve(reply.appsignalids_size());

		std::transform(reply.appsignalids().cbegin(),
					   reply.appsignalids().cend(),
					   std::back_inserter(signalIds),
					   [](const auto& sid)
					   {
						   return QString::fromStdString(sid);
					   });

		return signalIds;
	}

	tl::expected<std::vector<::AppSignalParam>, QString> SimServiceClientImpl::GetSignalParam(std::span<const Hash> signalHashes)
	{
		grpc::ClientContext context;
		auto deadline = std::chrono::system_clock::now() + RequestTimeOuts;
		context.set_deadline(deadline);

		static thread_local RpctGrpc::GetSignalParamRequest request;
		request.Clear();

#ifdef _MSC_VER
		request.mutable_signalhashes()->Assign(signalHashes.begin(), signalHashes.end());
#else
		// GCC and CLANG threat quint64 and uint64_t as different types (c++ standard).
		// Potential UB here.
		//
		request.mutable_signalhashes()->Assign(reinterpret_cast<const uint64_t*>(signalHashes.data()),
											   reinterpret_cast<const uint64_t*>(signalHashes.data()) + signalHashes.size());
#endif
		auto reader = m_stub->GetSignalParam(&context, request);

		std::vector<::AppSignalParam> result;
		result.reserve(signalHashes.size());

		RpctGrpc::GetSignalParamReply reply;
		while (reader->Read(&reply))
		{
			if (result.capacity() < reply.totalsize())
			{
				// If we send empty request, then all signals should be returned, and we need to allocate memory for them.
				//
				result.reserve(reply.totalsize());
			}

			for (const auto& protoSignal : reply.signalparams())
			{
				result.emplace_back().load(protoSignal);
			}
		}

		grpc::Status status = reader->Finish();
		if (status.ok() == false)
		{
			return tl::unexpected{formatErrorMessage(status)};
		}

		return result;
	}

	tl::expected<std::vector<::AppSignalState>, QString> SimServiceClientImpl::GetSignalState(std::span<const Hash> signalHashes)
	{
		grpc::ClientContext context;
		auto deadline = std::chrono::system_clock::now() + RequestTimeOuts;
		context.set_deadline(deadline);

		static thread_local RpctGrpc::GetSignalStateRequest request;
		request.Clear();

#ifdef _MSC_VER
		request.mutable_signalhashes()->Assign(signalHashes.begin(), signalHashes.end());
#else
		// GCC and CLANG threat quint64 and uint64_t as different types (c++ standard).
		// Potential UB here.
		//
		request.mutable_signalhashes()->Assign(reinterpret_cast<const uint64_t*>(signalHashes.data()),
											   reinterpret_cast<const uint64_t*>(signalHashes.data()) + signalHashes.size());
#endif

		static thread_local RpctGrpc::GetSignalStateReply reply;
		reply.Clear();

		grpc::Status status = m_stub->GetSignalState(&context, request, &reply);
		if (status.ok() == false)
		{
			return tl::unexpected{formatErrorMessage(status)};
		}

		std::vector<::AppSignalState> result;
		result.reserve(signalHashes.size());

		std::transform(reply.states().begin(),
					   reply.states().end(),
					   std::back_inserter(result),
					   [](const ::Proto::AppSignalState& protoState)
					   {
						   return ::AppSignalState{protoState};
					   });

		return result;
	}

	tl::expected<void, QStringList> SimServiceClientImpl::OverrideSignals(
		const std::vector<SimServiceClient::OverrideSignalPair>& overrideSignals)
	{
		grpc::ClientContext context;
		RpctGrpc::OverrideSignalRequest request;
		RpctGrpc::OverrideSignalReply reply;

		for (const SimServiceClient::OverrideSignalPair& s : overrideSignals)
		{
			auto protoSignal = request.add_appsignals();
			protoSignal->set_appsignalid(s.appSignalId.toStdString());

			std::visit(
				[protoSignal](auto&& value)
				{
					using T = std::decay_t<decltype(value)>;
					if constexpr (std::is_same_v<T, bool>)
					{
						protoSignal->set_boolvalue(value);
					}
					else if constexpr (std::is_same_v<T, double>)
					{
						protoSignal->set_doublevalue(value);
					}
					else if constexpr (std::is_same_v<T, int32_t>)
					{
						protoSignal->set_int32value(value);
					}
					else if constexpr (std::is_same_v<T, QString>)
					{
						protoSignal->set_script(value.toStdString());
					}
				},
				s.value);
		}

		grpc::Status status = m_stub->OverrideSignal(&context, request, &reply);
		if (status.ok() == false)
		{
			return tl::unexpected{QStringList{} << formatErrorMessage(status)};
		}

		// Processing reply.
		//
		QStringList errors;
		errors.reserve(reply.errors_size());

		std::transform(reply.errors().cbegin(),
					   reply.errors().cend(),
					   std::back_inserter(errors),
					   [](const auto& protoError)
					   {
						   return QString::fromStdString(protoError.appsignalid()) + ": " + QString::fromStdString(protoError.error());
					   });

		if (errors.isEmpty() == false)
		{
			return tl::unexpected{errors};
		}

		return {};
	}

	tl::expected<QStringList, QString> SimServiceClientImpl::RemoveOverrideSignals(const QStringList& appSignalIds)
	{
		grpc::ClientContext context;
		RpctGrpc::RemoveOverrideSignalRequest request;
		RpctGrpc::RemoveOverrideSignalReply reply;

		for (const auto& appSignalId : appSignalIds)
		{
			*request.add_appsignalids() = appSignalId.toStdString();
		}

		grpc::Status status = m_stub->RemoveOverrideSignal(&context, request, &reply);
		if (status.ok() == false)
		{
			return tl::unexpected{formatErrorMessage(status)};
		}

		QStringList currentlyOverriddenSignals;
		currentlyOverriddenSignals.reserve(reply.overriddenappsignalids_size());

		std::transform(reply.overriddenappsignalids().cbegin(),
					   reply.overriddenappsignalids().cend(),
					   std::back_inserter(currentlyOverriddenSignals),
					   [](const auto& appSignalId)
					   {
						   return QString::fromStdString(appSignalId);
					   });

		return currentlyOverriddenSignals;
	}
} // namespace Sim