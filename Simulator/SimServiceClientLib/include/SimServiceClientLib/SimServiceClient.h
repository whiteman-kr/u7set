#pragma once
#include "SimServiceModule.h"

// #include "../../AppSignalLib/AppSignalParam.h" // This must be included via precompiled header!!!
// #include "../../AppSignalLib/AppSignalState.h" // This must be included via precompiled header!!!

#include <CommonLib/expected.hpp>

#include <memory>
#include <vector>


namespace Sim
{
	class SimServiceClientImpl;

	class SimServiceClient
	{
	public:
		explicit SimServiceClient(QString address);
		virtual ~SimServiceClient();

	public:
		/**
		 * @brief Represents the state of the gRPC channel.
		 */
		enum ChannelState
		{
			/** channel is idle */
			GRPC_CHANNEL_IDLE,
			/** channel is connecting */
			GRPC_CHANNEL_CONNECTING,
			/** channel is ready for work */
			GRPC_CHANNEL_READY,
			/** channel has seen a failure but expects to recover */
			GRPC_CHANNEL_TRANSIENT_FAILURE,
			/** channel has seen a failure that it cannot recover from */
			GRPC_CHANNEL_SHUTDOWN
		};


		/**
		 * @brief Represents the state of the simulator.
		 *
		 * This enum describes the possible states of the simulator:
		 * - STATE_STOPPED: The simulator is stopped and not running.
		 * - STATE_RUNNING: The simulator is currently running.
		 * - STATE_PAUSED:  The simulator is paused and can be resumed.
		 */
		enum State
		{
			/** The simulator is stopped and not running. */
			STATE_STOPPED = 0,
			/** The simulator is currently running. */
			STATE_RUNNING = 1,
			/** The simulator is paused and can be resumed. */
			STATE_PAUSED = 2
		};

		/**
		 * @brief Represents the status of the simulator.
		 *
		 * This struct contains information about the current simulator project and its state.
		 */
		struct SimulatorStatus
		{
			/**
			 * @brief The name of the currently loaded project in the simulator.
			 */
			QString project;

			/**
			 * @brief The current state of the simulator.
			 *
			 * See SimServiceClient::State for possible values.
			 */
			State state{STATE_STOPPED};
		};

	public:
		/**
		 * @brief Returns the current state of the gRPC channel.
		 * @return The current channel state as a value of SimServiceClient::ChannelState.
		 */
		[[nodiscard]] SimServiceClient::ChannelState channelState();

		/**
		 * @brief Checks if the client is currently connected to the simulator service.
		 * @return true if the client is connected; otherwise, false.
		 */
		[[nodiscard]] bool connected();

		tl::expected<QByteArray, QString> Ping(const QByteArray& data);

		[[nodiscard]] tl::expected<SimServiceClient::SimulatorStatus, QString> GetStatus();

		tl::expected<SimServiceClient::State, QString> CommandStart(int64_t durationMcs = -1, const QStringList& logicModules = {});
		tl::expected<SimServiceClient::State, QString> CommandPause();
		tl::expected<SimServiceClient::State, QString> CommandStop();

		[[nodiscard]] tl::expected<std::vector<Sim::SimServiceModule>, QString> GetModule(const QStringList& equipmentIds);
		[[nodiscard]] tl::expected<void, QString> SetModuleFlag(const QString& equipmentId, int32_t flagId, bool value);

		[[nodiscard]] tl::expected<QStringList, QString> GetSignalList();
		[[nodiscard]] tl::expected<std::vector<::AppSignalParam>, QString> GetSignalParam();
		[[nodiscard]] tl::expected<std::vector<::AppSignalParam>, QString> GetSignalParam(std::span<const Hash> signalHashes);
		[[nodiscard]] tl::expected<std::vector<::AppSignalState>, QString> GetSignalState(std::span<const Hash> signalHashes);

	private:
		std::shared_ptr<SimServiceClientImpl> m_impl;
	};
} // namespace Sim