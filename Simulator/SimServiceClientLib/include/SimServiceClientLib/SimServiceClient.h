#pragma once
#include "SimServiceModule.h"

// #include "../../AppSignalLib/AppSignalParam.h" // This must be included via precompiled header!!!
// #include "../../AppSignalLib/AppSignalState.h" // This must be included via precompiled header!!!

#include <CommonLib/expected.hpp>

#include <memory>
#include <variant>
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

		/**
		 * @brief Represents a value used to override a signal in the simulator.
		 *
		 * The value can be one of the following types, depending on the signal type:
		 * - bool:    For discrete (boolean) signals.
		 * - double:  For floating-point signals (float or double).
		 * - int32_t: For signed 32-bit integer signals.
		 * - QString: For script-based signals.
		 */
		using OverrideValueT = std::variant<bool, double, int32_t, QString>;

		/**
		 * @brief Associates an application signal ID with an override value.
		 *
		 * This struct is used to specify a signal to override and the value to set.
		 * The value type must match the signal's expected type.
		 */
		struct OverrideSignalPair
		{
			/**
			 * @brief The application signal identifier.
			 */
			QString appSignalId;

			/**
			 * @brief The value to override the signal with.
			 *
			 * The type of the value should correspond to the signal's type or be a script.
			 */
			OverrideValueT value; // Set according to signal type or set script.
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

		[[nodiscard]] tl::expected<std::vector<Sim::SimServiceModule>, QString> GetModuleList();
		[[nodiscard]] tl::expected<std::vector<Sim::SimServiceModule>, QString> GetModule(const QStringList& equipmentIds);
		tl::expected<void, QString> SetModuleFlag(const QString& equipmentId, int32_t flagId, bool value);

		[[nodiscard]] tl::expected<QStringList, QString> GetSignalList();
		[[nodiscard]] tl::expected<std::vector<::AppSignalParam>, QString> GetSignalParam();
		[[nodiscard]] tl::expected<std::vector<::AppSignalParam>, QString> GetSignalParam(std::span<const Hash> signalHashes);
		[[nodiscard]] tl::expected<std::vector<::AppSignalState>, QString> GetSignalState(std::span<const Hash> signalHashes);

		tl::expected<void, QStringList> OverrideSignals(const std::vector<Sim::SimServiceClient::OverrideSignalPair>& overrideSignals);

		[[nodiscard]] tl::expected<QStringList, QString> RemoveOverrideSignals(const QStringList& appSignalIds);
		[[nodiscard]] tl::expected<QStringList, QString> GetOverriddenSignals();

	private:
		std::shared_ptr<SimServiceClientImpl> m_impl;
	};
} // namespace Sim