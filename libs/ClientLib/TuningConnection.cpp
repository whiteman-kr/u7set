#ifndef CLIENT_LIB_DOMAIN
	#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "./include/ClientLib/TuningConnection.h"
#include "TuningConnectionPrivate.h"

namespace ClientLib
{
	TuningConnection::TuningConnection(ITuningSignalManager& tuningSignalManager,
									   ITuningSignalUpdater& tuningSignalUpdater,
									   IRecentAppSignals& recentTuningSignals,
									   ITuningAuthorization& tuningAuthorization,
									   ILogFile* logFile,
									   ITuningLog* tuningLog) :
		m_pimpl{std::make_unique<TuningConnectionPrivate>(tuningSignalManager,
														  tuningSignalUpdater,
														  recentTuningSignals,
														  tuningAuthorization,
														  logFile,
														  tuningLog)}
	{
		return;
	}

	TuningConnection::~TuningConnection() = default;

	void TuningConnection::updateConnections(const SoftwareInfo& softwareInfo,
											 const std::vector<SoftwareEndpoint::TuningService>& tuningServices,
											 bool autoApply,
											 TuningClientSettings::LmStatusFlagMode lmStatusFlagMode)
	{
		m_pimpl->updateConnections(softwareInfo, tuningServices, autoApply, lmStatusFlagMode);
	}

	std::vector<Tcp::ConnectionState> TuningConnection::tcpTuningConnStates() const
	{
		return m_pimpl->tcpTuningConnStates();
	}

	std::vector<TuningSource> TuningConnection::tuningSourcesInfo() const
	{
		return m_pimpl->tuningSourcesInfo();
	}

	std::vector<TuningSource> TuningConnection::tuningSourceInfo(Hash sourceHash) const
	{
		return m_pimpl->tuningSourceInfo(sourceHash);
	}

	int TuningConnection::tuningSourceStatesCount(Hash sourceHash) const
	{
		return m_pimpl->tuningSourceStatesCount(sourceHash);
	}

	int TuningConnection::activatedTuningSourceStatesCount(Hash sourceHash) const
	{
		return m_pimpl->activatedTuningSourceStatesCount(sourceHash);
	}

	bool TuningConnection::activateTuningSource(Hash sourceHash, bool activate) const
	{
		return m_pimpl->activateTuningSource(sourceHash, activate);
	}

	QString TuningConnection::clientControlInfo() const
	{
		return m_pimpl->clientControlInfo();
	}

	bool TuningConnection::takeClientControl(Hash sourceHash) const
	{
		return m_pimpl->takeClientControl(sourceHash);
	}

	bool TuningConnection::writeTuningSignals(const std::vector<TuningWriteCommand>& writeCommands)
	{
		return m_pimpl->writeTuningSignals(writeCommands);
	}

	bool TuningConnection::writeTuningSignal(const QString& appSignalId, const TuningValue& tuningValue)
	{
		return m_pimpl->writeTuningSignal(appSignalId, tuningValue);
	}

	bool TuningConnection::writeTuningSignal(const QString& appSignalId, QVariant value)
	{
		return m_pimpl->writeTuningSignal(appSignalId, value);
	}

	bool TuningConnection::signalStatesLoaded() const
	{
		return m_pimpl->signalStatesLoaded();
	}

	void TuningConnection::applyTuningSignals(const std::vector<Hash>& signalHashes)
	{
		m_pimpl->applyTuningSignals(signalHashes);
	}

	void TuningConnection::applyTuningSignals()
	{
		m_pimpl->applyTuningSignals();
	}

} // namespace ClientLib
