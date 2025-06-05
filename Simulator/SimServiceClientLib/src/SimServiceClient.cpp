#include "../include/SimServiceClientLib/SimServiceClient.h"
#include "SimServiceClientImpl.h"


namespace Sim
{
	//
	// SimServiceClient
	//
	SimServiceClient::SimServiceClient(QString address) :
		m_impl{std::make_shared<SimServiceClientImpl>(address, true)}
	{
	}

	SimServiceClient::~SimServiceClient() = default;

	SimServiceClient::ChannelState SimServiceClient::channelState()
	{
		return m_impl->channelState();
	}

	bool SimServiceClient::connected()
	{
		return m_impl->connected();
	}

	tl::expected<QByteArray, QString> SimServiceClient::Ping(const QByteArray& data)
	{
		return m_impl->Ping(data);
	}

	tl::expected<SimServiceClient::SimulatorStatus, QString> SimServiceClient::GetStatus()
	{
		return m_impl->GetStatus();
	}

	tl::expected<SimServiceClient::State, QString> SimServiceClient::CommandStart(int64_t durationMcs, const QStringList& logicModules)
	{
		return m_impl->CommandStart(durationMcs, logicModules);
	}

	tl::expected<SimServiceClient::State, QString> SimServiceClient::CommandPause()
	{
		return m_impl->CommandPause();
	}

	tl::expected<SimServiceClient::State, QString> SimServiceClient::CommandStop()
	{
		return m_impl->CommandStop();
	}

	[[nodiscard]] tl::expected<std::vector<Sim::SimServiceModule>, QString> SimServiceClient::GetModule(const QStringList& equipmentIds)
	{
		return m_impl->GetModule(equipmentIds);
	}

	tl::expected<void, QString> SimServiceClient::SetModuleFlag(const QString& equipmentId, int32_t flagId, bool value)
	{
		auto result = m_impl->SetModuleFlag(equipmentId, flagId, value);
		if (result.has_value() == false)
		{
			return tl::unexpected(result.error());
		}

		return {};
	}

	tl::expected<QStringList, QString> SimServiceClient::GetSignalList()
	{
		return m_impl->GetSignalList();
	}

	tl::expected<std::vector<::AppSignalParam>, QString> SimServiceClient::GetSignalParam()
	{
		std::span<const Hash> signalHashes;
		return m_impl->GetSignalParam(signalHashes);
	}

	tl::expected<std::vector<::AppSignalParam>, QString> SimServiceClient::GetSignalParam(std::span<const Hash> signalHashes)
	{
		return m_impl->GetSignalParam(signalHashes);
	}

	tl::expected<std::vector<::AppSignalState>, QString> SimServiceClient::GetSignalState(std::span<const Hash> signalHashes)
	{
		return m_impl->GetSignalState(signalHashes);
	}

	tl::expected<void, QStringList> SimServiceClient::OverrideSignals(
		const std::vector<Sim::SimServiceClient::OverrideSignalPair>& overrideSignals)
	{
		return m_impl->OverrideSignals(overrideSignals);
	}

	tl::expected<QStringList, QString> SimServiceClient::RemoveOverrideSignals(const QStringList& appSignalIds)
	{
		return m_impl->RemoveOverrideSignals(appSignalIds);
	}

	tl::expected<QStringList, QString> SimServiceClient::GetOverriddenSignals()
	{
		return m_impl->RemoveOverrideSignals({});
	}
} // namespace Sim