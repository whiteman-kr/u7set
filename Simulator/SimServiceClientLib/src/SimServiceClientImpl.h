#pragma once

#include "../include/SimServiceClientLib/SimServiceClient.h"

#include <grpcpp/channel.h>

namespace Sim
{
	class SimServiceClientImpl final : public std::enable_shared_from_this<SimServiceClientImpl>
	{
	public:
		explicit SimServiceClientImpl(QString address, bool only_as_shared_ptr);
		~SimServiceClientImpl();

	public:
		SimServiceClient::ChannelState channelState();
		bool connected();

		tl::expected<QByteArray, QString> Ping(const QByteArray& data);

		tl::expected<SimServiceClient::SimulatorStatus, QString> GetStatus();
		tl::expected<SimServiceClient::State, QString> CommandStart(int64_t durationMcs = -1, const QStringList& logicModules = {});
		tl::expected<SimServiceClient::State, QString> CommandPause();
		tl::expected<SimServiceClient::State, QString> CommandStop();

		tl::expected<std::vector<Sim::SimServiceModule>, QString> GetModule(const QStringList& equipmentIds);
		[[nodiscard]] tl::expected<::RpctGrpc::SetModuleFlagReply, QString> SetModuleFlag(const QString& equipmentId,
																						  int32_t flagId,
																						  bool value);

		tl::expected<QStringList, QString> GetSignalList();
		tl::expected<std::vector<::AppSignalParam>, QString> GetSignalParam(std::span<const Hash> signalHashes);
		tl::expected<std::vector<::AppSignalState>, QString> GetSignalState(std::span<const Hash> signalHashes);

		tl::expected<void, QStringList> OverrideSignals(const std::vector<SimServiceClient::OverrideSignalPair>& overrideSignals);
		[[nodiscard]] tl::expected<QStringList, QString> RemoveOverrideSignals(const QStringList& appSignalIds);

	private:
		const std::shared_ptr<::grpc::Channel> m_channel;
		std::unique_ptr<RpctGrpc::SimService::Stub> m_stub;
	};
} // namespace Sim