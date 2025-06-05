#include "SimulatorPrivate.h"
#include <SimulatorLib/SimService.h>

#include "../AppSignalLib/IAppSignalManager.h"
#include "SimScopedLog.h"


#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4267)
#endif
#include <SimService.grpc.pb.h>

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

#include <grpcpp/server_builder.h>

#include <span>
#include <thread>

namespace
{
	constexpr int MaxSignalParamCount = 256;

	::RpctGrpc::SimulatorState getProtoControlState(Sim::SimControlState state)
	{
		auto result = ::RpctGrpc::SimulatorState::STATE_STOPPED;

		switch (state)
		{
		case Sim::SimControlState::Stop:
			result = ::RpctGrpc::SimulatorState::STATE_STOPPED;
			break;
		case Sim::SimControlState::Run:
			result = ::RpctGrpc::SimulatorState::STATE_RUNNING;
			break;
		case Sim::SimControlState::Pause:
			result = ::RpctGrpc::SimulatorState::STATE_PAUSED;
			break;
		}

		return result;
	}
} // namespace


namespace Sim
{
	class ServiceImpl final : public RpctGrpc::SimService::Service
	{
	public:
		explicit ServiceImpl(SimulatorPrivate& simulator);
		~ServiceImpl();

	public:
		grpc::Status Ping(grpc::ServerContext* context, const RpctGrpc::PingRequest* request, RpctGrpc::PongReply* response) override;

		grpc::Status GetStatus(::grpc::ServerContext* context,
							   const ::RpctGrpc::GetStatusRequest* request,
							   ::RpctGrpc::GetStatusReply* response) override;

		grpc::Status CommandStart(::grpc::ServerContext* context,
								  const ::RpctGrpc::CommandStartRequest* request,
								  ::RpctGrpc::CommandStartReply* response) override;

		grpc::Status CommandPause(::grpc::ServerContext* context,
								  const ::RpctGrpc::CommandPauseRequest* request,
								  ::RpctGrpc::CommandPauseReply* response) override;

		grpc::Status CommandStop(::grpc::ServerContext* context,
								 const ::RpctGrpc::CommandStopRequest* request,
								 ::RpctGrpc::CommandStopReply* response) override;

		grpc::Status GetModule(::grpc::ServerContext* context,
							   const ::RpctGrpc::GetModuleRequest* request,
							   ::RpctGrpc::GetModuleReply* response) override;

		grpc::Status SetModuleFlag(::grpc::ServerContext* context,
								   const ::RpctGrpc::SetModuleFlagRequest* request,
								   ::RpctGrpc::SetModuleFlagReply* response) override;

		grpc::Status GetSignalList(::grpc::ServerContext* context,
								   const ::RpctGrpc::GetSignalListRequest* request,
								   ::RpctGrpc::GetSignalListReply* response) override;

		grpc::Status GetSignalParam(::grpc::ServerContext* context,
									const ::RpctGrpc::GetSignalParamRequest* request,
									::grpc::ServerWriter<::RpctGrpc::GetSignalParamReply>* writer) override;

		grpc::Status GetSignalState(grpc::ServerContext* context,
									const RpctGrpc::GetSignalStateRequest* request,
									RpctGrpc::GetSignalStateReply* response) override;

		grpc::Status OverrideSignal(::grpc::ServerContext* context,
									const ::RpctGrpc::OverrideSignalRequest* request,
									::RpctGrpc::OverrideSignalReply* response) override;

		grpc::Status RemoveOverrideSignal(::grpc::ServerContext* context,
										  const ::RpctGrpc::RemoveOverrideSignalRequest* request,
										  ::RpctGrpc::RemoveOverrideSignalReply* response) override;

	private:
		SimulatorPrivate& m_simulator;
		mutable ScopedLog m_log;

		std::jthread m_thread;
		std::unique_ptr<grpc::Server> m_server;
	};


	ServiceImpl::ServiceImpl(SimulatorPrivate& simulator) :
		m_simulator{simulator},
		m_log{m_simulator.log().logInterface(), true, "Service"}
	{
		// Start GRPC Server
		//
		QString address{"0.0.0.0:50051"};

		grpc::ServerBuilder builder;
		builder.AddListeningPort(address.toStdString(), grpc::InsecureServerCredentials());
		builder.RegisterService(this);

		m_server = builder.BuildAndStart();

		if (m_server == nullptr)
		{
			m_log.writeError(QString{"Failed to start gRPC server, address %1."}.arg(address));
		}
		else
		{
			m_thread = std::jthread{[this](std::stop_token stoken, grpc::Server* server)
									{
										std::stop_callback stop_cb{stoken,
																   [server]()
																   {
																	   server->Shutdown(); // Triggers Server::Wait() to exit.
																   }};
										try
										{
											server->Wait(); // Blocking call, unblocked by stop_token callback.
										}
										catch (std::exception& e)
										{
											m_log.writeError("Server thread exception: " + QString{e.what()});
										}
									},
									m_server.get()};

			m_log.writeMessage(QString{"Server listening on %1."}.arg(address));
		}

		return;
	}

	ServiceImpl::~ServiceImpl()
	{
		if (m_server)
		{
			// Join thread explicitly, to log message only after thread has stopped.
			//
			m_thread.request_stop();
			m_thread.join();
		}

		m_log.writeMessage("Finished.");
		return;
	}

	grpc::Status ServiceImpl::Ping([[maybe_unused]] grpc::ServerContext* context,
								   const RpctGrpc::PingRequest* request,
								   RpctGrpc::PongReply* response)
	{
		if (request->payload().size() > 2048)
		{
			return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Payload too large");
		}

		response->set_payload(request->payload());
		return ::grpc::Status::OK;
	}

	grpc::Status ServiceImpl::GetStatus([[maybe_unused]] ::grpc::ServerContext* context,
										[[maybe_unused]] const ::RpctGrpc::GetStatusRequest* request,
										::RpctGrpc::GetStatusReply* response)
	{
		if (m_simulator.isLoaded() == false)
		{
			response->set_project({});
		}
		else
		{
			response->set_project(m_simulator.projectName().toStdString());
		}

		auto state = getProtoControlState(m_simulator.control().state());
		response->set_state(state);

		response->set_lefttimemcs(m_simulator.control().leftTime().count());
		response->set_durationmcs(m_simulator.control().duration().count());

		return grpc::Status::OK;
	}

	grpc::Status ServiceImpl::CommandStart([[maybe_unused]] ::grpc::ServerContext* context,
										   const ::RpctGrpc::CommandStartRequest* request,
										   ::RpctGrpc::CommandStartReply* response)
	{
		if (m_simulator.isLoaded() == false)
		{
			return grpc::Status{grpc::StatusCode::FAILED_PRECONDITION, "Project is not loaded."};
		}

		if (m_simulator.control().isRunning() == true)
		{
			return grpc::Status{grpc::StatusCode::FAILED_PRECONDITION, "Simulation is already running."};
		}

		QStringList modules;
		std::transform(request->logicmodules().begin(),
					   request->logicmodules().end(),
					   std::back_inserter(modules),
					   [](const std::string& lm)
					   {
						   return QString::fromStdString(lm);
					   });
		m_simulator.control().setRunList(modules);

		m_simulator.control().setSpeedFactor(request->speedfactor());
		m_simulator.control().startSimulation(std::chrono::microseconds{request->durationmcs()});

		auto state = getProtoControlState(m_simulator.control().state());
		response->set_state(state);

		return grpc::Status::OK;
	}

	grpc::Status ServiceImpl::CommandPause([[maybe_unused]] ::grpc::ServerContext* context,
										   [[maybe_unused]] const ::RpctGrpc::CommandPauseRequest* request,
										   ::RpctGrpc::CommandPauseReply* response)
	{
		if (m_simulator.isLoaded() == false)
		{
			return grpc::Status{grpc::StatusCode::FAILED_PRECONDITION, "Project is not loaded."};
		}

		if (m_simulator.control().state() != Sim::SimControlState::Run)
		{
			return grpc::Status{grpc::StatusCode::FAILED_PRECONDITION, "Simulation is not running."};
		}

		m_simulator.control().pause();

		auto state = getProtoControlState(m_simulator.control().state());
		response->set_state(state);

		return grpc::Status::OK;
	}

	grpc::Status ServiceImpl::CommandStop([[maybe_unused]] ::grpc::ServerContext* context,
										  [[maybe_unused]] const ::RpctGrpc::CommandStopRequest* request,
										  ::RpctGrpc::CommandStopReply* response)
	{
		if (m_simulator.isLoaded() == false)
		{
			return grpc::Status{grpc::StatusCode::FAILED_PRECONDITION, "Project is not loaded."};
		}

		if (m_simulator.control().state() == Sim::SimControlState::Stop)
		{
			return grpc::Status{grpc::StatusCode::FAILED_PRECONDITION, "Simulation is already stopped."};
		}

		m_simulator.control().stop();

		auto state = getProtoControlState(m_simulator.control().state());
		response->set_state(state);

		return grpc::Status::OK;
	}

	grpc::Status ServiceImpl::GetModule([[maybe_unused]] ::grpc::ServerContext* context,
										const ::RpctGrpc::GetModuleRequest* request,
										::RpctGrpc::GetModuleReply* response)
	{
		// Get modules, now only LogicModules.
		//
		std::vector<std::shared_ptr<LogicModuleImpl>> modules;
		modules.reserve(request->equipmentid_size());
		std::transform(request->equipmentid().begin(),
					   request->equipmentid().end(),
					   std::back_inserter(modules),
					   [this](const std::string& equipmentId)
					   {
						   return m_simulator.logicModule(QString::fromStdString(equipmentId));
					   });

		// Format response.
		//
		for (const auto& lm : modules)
		{
			::RpctGrpc::ModuleState* protoModuleState = response->add_modules();

			if (lm != nullptr)
			{
				// Fill message ModuleState.
				//
				protoModuleState->set_equipmentid(lm->equipmentId().toStdString());

				auto addBoolFlagFunc = [protoModuleState](::RpctGrpc::ModuleFlagId flagId, bool value)
				{
					::RpctGrpc::ModuleFlagValue protoValue{};
					protoValue.set_boolvalue(value);
					protoModuleState->mutable_flags()->emplace(static_cast<int32_t>(flagId), std::move(protoValue));
				};

				addBoolFlagFunc(::RpctGrpc::ModuleFlagId::MF_LM_BOOL_FAULT, lm->runtimeMode() == Sim::RuntimeMode::FaultedMode);
				addBoolFlagFunc(::RpctGrpc::ModuleFlagId::MF_LM_BOOL_POWER_ON, !lm->isPowerOff());
				addBoolFlagFunc(::RpctGrpc::ModuleFlagId::MF_LM_BOOL_ARMING_KEY, lm->armingKey());
				addBoolFlagFunc(::RpctGrpc::ModuleFlagId::MF_LM_BOOL_TUNING_KEY, lm->tuningKey());
				addBoolFlagFunc(::RpctGrpc::ModuleFlagId::MF_LM_BOOL_SOR_IS_SET, lm->sorIsSet());
				addBoolFlagFunc(::RpctGrpc::ModuleFlagId::MF_LM_BOOL_SOR_SET_SWITCH_1, lm->sorSetSwitch1());
				addBoolFlagFunc(::RpctGrpc::ModuleFlagId::MF_LM_BOOL_SOR_SET_SWITCH_2, lm->sorSetSwitch2());
				addBoolFlagFunc(::RpctGrpc::ModuleFlagId::MF_LM_BOOL_SOR_SET_SWITCH_3, lm->sorSetSwitch3());

				// Fill message oneof ModuleOneOf -> LogicModule
				//
				::RpctGrpc::LogicModule* protoLm = protoModuleState->mutable_logicmodule();
				protoLm->set_subsystemid(lm->logicModuleExtraInfo().subsystemID.toStdString());
				protoLm->set_lmnumber(lm->lmNumber());
				protoLm->set_channel(static_cast<int32_t>(lm->channel()));
			}
		}

		return grpc::Status::OK;
	}

	grpc::Status ServiceImpl::SetModuleFlag([[maybe_unused]] ::grpc::ServerContext* context,
											const ::RpctGrpc::SetModuleFlagRequest* request,
											::RpctGrpc::SetModuleFlagReply* response)
	{
		response->Clear();

		// Get module, now only LogicModules.
		//
		QString equipmentId = QString::fromStdString(request->equipmentid());
		std::shared_ptr<LogicModuleImpl> module = m_simulator.logicModule(equipmentId);

		if (module == nullptr)
		{
			return grpc::Status(grpc::StatusCode::OUT_OF_RANGE, QString{"Module %1 not found"}.arg(equipmentId).toStdString());
		}

		for (const auto& f : request->flags())
		{
			switch (f.id())
			{
			case ::RpctGrpc::MF_LM_BOOL_FAULT:
				return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Flag MF_LM_BOOL_FAULT is ReadOnly");

			case ::RpctGrpc::MF_LM_BOOL_POWER_ON:
				module->setPowerOff(!f.value().boolvalue());
				response->mutable_updatedvalue()->set_boolvalue(module->isPowerOff() == false);
				break;

			case ::RpctGrpc::MF_LM_BOOL_ARMING_KEY:
				module->setArmingKey(f.value().boolvalue());
				response->mutable_updatedvalue()->set_boolvalue(module->armingKey());
				break;

			case ::RpctGrpc::MF_LM_BOOL_TUNING_KEY:
				module->setTuningKey(f.value().boolvalue());
				response->mutable_updatedvalue()->set_boolvalue(module->tuningKey());
				break;

			case ::RpctGrpc::MF_LM_BOOL_SOR_IS_SET:
				return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Flag MF_LM_BOOL_SOR_IS_SET is ReadOnly");

			case ::RpctGrpc::MF_LM_BOOL_SOR_SET_SWITCH_1:
				module->setSorSetSwitch1(f.value().boolvalue());
				response->mutable_updatedvalue()->set_boolvalue(module->sorSetSwitch1());
				break;

			case ::RpctGrpc::MF_LM_BOOL_SOR_SET_SWITCH_2:
				module->setSorSetSwitch2(f.value().boolvalue());
				response->mutable_updatedvalue()->set_boolvalue(module->sorSetSwitch2());
				break;

			case ::RpctGrpc::MF_LM_BOOL_SOR_SET_SWITCH_3:
				module->setSorSetSwitch3(f.value().boolvalue());
				response->mutable_updatedvalue()->set_boolvalue(module->sorSetSwitch3());
				break;

			case ::RpctGrpc::MF_LM_BOOL_SOR_RESET:
				module->testSorResetSwitch(f.value().boolvalue());
				response->mutable_updatedvalue()->set_boolvalue(false);
				break;

			default:
				return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, QString{"Flag %1 is not implemented"}.arg(f.id()).toStdString());
			}
		}

		return grpc::Status::OK;
	}

	grpc::Status ServiceImpl::GetSignalList([[maybe_unused]] ::grpc::ServerContext* context,
											[[maybe_unused]] const ::RpctGrpc::GetSignalListRequest* request,
											::RpctGrpc::GetSignalListReply* response)
	{
		const auto allSignals = m_simulator.appSignalManager().signalList();

		auto ms = response->mutable_appsignalids();
		ms->Reserve(static_cast<int>(allSignals.size()));

		for (const auto& signalParam : allSignals)
		{
			ms->Add(signalParam.appSignalId().toStdString());
		}

		return ::grpc::Status::OK;
	}

	grpc::Status ServiceImpl::GetSignalParam([[maybe_unused]] ::grpc::ServerContext* context,
											 const ::RpctGrpc::GetSignalParamRequest* request,
											 ::grpc::ServerWriter<::RpctGrpc::GetSignalParamReply>* writer)
	{
		bool found = false;

		::RpctGrpc::GetSignalParamReply reply;
		uint32_t totalSize = 0;
		uint32_t replyIndex = 0;

		auto sendFunc = [&writer, &reply](uint32_t totalSize, uint32_t replyIndex)
		{
			reply.set_totalsize(totalSize);
			reply.set_replysignalindex(replyIndex);

			writer->Write(std::move(reply));
			reply.Clear();
		};

		if (request->signalhashes_size() == 0)
		{
			auto allSignals = m_simulator.appSignalManager().signalList();
			totalSize = static_cast<uint32_t>(allSignals.size());

			// If empty then get all signals.
			//
			for (const auto& signalParam : allSignals)
			{
				if (context->IsCancelled() == true)
				{
					return grpc::Status(grpc::StatusCode::CANCELLED, "Client cancelled");
				}

				signalParam.save(reply.mutable_signalparams()->Add());
				if (reply.signalparams_size() >= MaxSignalParamCount)
				{
					sendFunc(totalSize, static_cast<int32_t>(replyIndex - (reply.signalparams_size())));
				}
			}

			replyIndex++;
		}
		else
		{
			totalSize = static_cast<uint32_t>(request->signalhashes_size());
			for (auto hash : request->signalhashes())
			{
				if (context->IsCancelled() == true)
				{
					return grpc::Status(grpc::StatusCode::CANCELLED, "Client cancelled");
				}

				AppSignalParam signalParam = m_simulator.appSignalManager().signalParam(static_cast<Hash>(hash), &found);
				signalParam.save(reply.mutable_signalparams()->Add());

				if (reply.signalparams_size() >= MaxSignalParamCount)
				{
					sendFunc(totalSize, static_cast<int32_t>(replyIndex - (reply.signalparams_size())));
				}

				replyIndex++;
			}
		}

		if (reply.signalparams_size() > 0)
		{
			sendFunc(totalSize, static_cast<int32_t>(replyIndex - (reply.signalparams_size())));
		}

		return grpc::Status::OK;
	}

	grpc::Status ServiceImpl::GetSignalState([[maybe_unused]] ::grpc::ServerContext* context,
											 const ::RpctGrpc::GetSignalStateRequest* request,
											 ::RpctGrpc::GetSignalStateReply* response)
	{
		// Get signal states.
		//
#ifdef _MSC_VER
		std::span<const Hash> hashes{request->signalhashes().data(), static_cast<size_t>(request->signalhashes_size())};
#else
		// GCC and CLANG threat quint64 and uint64_t as different types (c++ standard).
		// Potential UB here.
		//
		std::span<const Hash> hashes{reinterpret_cast<const Hash*>(request->signalhashes().data()),
									 static_cast<size_t>(request->signalhashes_size())};
#endif
		thread_local std::vector<AppSignalState> states;
		states.clear();
		int found = 0;

		m_simulator.appSignalManager().signalState(hashes, &states, &found);

		// Save states to the reply.
		//
		auto protoStates = response->mutable_states();
		protoStates->Reserve(states.size());

		for (const ::AppSignalState& state : states)
		{
			::Proto::AppSignalState protoState;
			state.save(&protoState);

			protoStates->Add(std::move(protoState));
		}

		return grpc::Status::OK;
	}

	grpc::Status ServiceImpl::OverrideSignal([[maybe_unused]] ::grpc::ServerContext* context,
											 const ::RpctGrpc::OverrideSignalRequest* request,
											 ::RpctGrpc::OverrideSignalReply* response)
	{
		// Add signals to override if they were noy yet.
		//
		{
			QStringList appSignalIds;
			appSignalIds.reserve(request->appsignals_size());
			std::transform(request->appsignals().begin(),
						   request->appsignals().end(),
						   std::back_inserter(appSignalIds),
						   [](const auto& protoAppSignal)
						   {
							   return QString::fromStdString(protoAppSignal.appsignalid());
						   });

			QStringList alreadyOverriddenSignals = m_simulator.overrideSignals().overrideSignalIds();
			std::unordered_set<QString> alreadyOverriddenSignalSet{alreadyOverriddenSignals.begin(), alreadyOverriddenSignals.end()};

			QStringList signalsToAdd;
			std::copy_if(appSignalIds.cbegin(),
						 appSignalIds.cend(),
						 std::back_inserter(signalsToAdd),
						 [&alreadyOverriddenSignalSet](const QString& appSignalId)
						 {
							 return alreadyOverriddenSignalSet.contains(appSignalId) == false;
						 });

			int added = m_simulator.overrideSignals().addSignals(signalsToAdd);
			if (added != signalsToAdd.size())
			{
				// Error, to form an error we need to understand which signal was not added.
				//
				QStringList currentSignals = m_simulator.overrideSignals().overrideSignalIds();
				std::unordered_set<QString> currentSet{currentSignals.begin(), currentSignals.end()};

				for (const QString& signalId : signalsToAdd)
				{
					if (currentSet.contains(signalId) == false)
					{
						auto* error = response->mutable_errors()->Add();
						error->set_appsignalid(signalId.toStdString());
						error->set_error("Failed to add signal");
					}
				}
			}
		}

		// Overriding signal values.
		//
		{
			std::vector<Sim::OverrideSetValueData> overrides;
			overrides.reserve(request->appsignals_size());

			for (const auto& protoAppSignal : request->appsignals())
			{
				QString appSignalId = QString::fromStdString(protoAppSignal.appsignalid());
				QVariant value;

				auto method = Sim::OverrideSignalMethod::Value;

				switch (protoAppSignal.ValueOneOf_case())
				{
				case ::RpctGrpc::OverrideSignal::ValueOneOfCase::kBoolValue:
					value.setValue<int>(protoAppSignal.boolvalue());
					break;
				case ::RpctGrpc::OverrideSignal::ValueOneOfCase::kDoubleValue:
					value.setValue<double>(protoAppSignal.doublevalue());
					break;
				case ::RpctGrpc::OverrideSignal::ValueOneOfCase::kInt32Value:
					value.setValue<int>(protoAppSignal.int32value());
					break;
				case ::RpctGrpc::OverrideSignal::ValueOneOfCase::kScript:
					value = QString::fromStdString(protoAppSignal.script());
					method = Sim::OverrideSignalMethod::Script;
					break;
				case ::RpctGrpc::OverrideSignal::ValueOneOfCase::VALUEONEOF_NOT_SET:
					{
						auto r = response->mutable_errors()->Add();
						r->set_appsignalid(protoAppSignal.appsignalid());
						r->set_error("Override data not provided");
					}
					break;
				}

				if (value.isNull() == false)
				{
					overrides.emplace_back(appSignalId, method, value);
				}
			}

			m_simulator.overrideSignals().setValues(overrides);
		}

		// Response is already formed by processing case VALUEONEOF_NOT_SET.
		//

		return grpc::Status::OK;
	}

	grpc::Status ServiceImpl::RemoveOverrideSignal(::grpc::ServerContext* context,
												   const ::RpctGrpc::RemoveOverrideSignalRequest* request,
												   ::RpctGrpc::RemoveOverrideSignalReply* response)
	{
		QStringList appSignalIds;
		appSignalIds.reserve(request->appsignalids_size());

		std::transform(request->appsignalids().cbegin(),
					   request->appsignalids().cend(),
					   std::back_inserter(appSignalIds),
					   [](const auto& appSignalId)
					   {
						   return QString::fromStdString(appSignalId);
					   });

		m_simulator.overrideSignals().removeSignals(appSignalIds);

		for (const auto& current = m_simulator.overrideSignals().overrideSignalIds(); const QString& appSignalId : current)
		{
			response->add_overriddenappsignalids(appSignalId.toStdString());
		}

		return grpc::Status::OK;
	}

	//
	//
	// Service
	//
	//
	Service::Service(SimulatorPrivate& simulator) :
		m_simulator{simulator}
	{
	}

	Service::~Service() = default;

	bool Service::enabled() const
	{
		return m_impl != nullptr;
	}

	void Service::setEnabled(bool enable)
	{
		if (enable == true && m_impl == nullptr)
		{
			m_impl = std::make_unique<ServiceImpl>(m_simulator);
		}

		if (enable == false)
		{
			m_impl.reset();
		}

		return;
	}

} // namespace Sim
