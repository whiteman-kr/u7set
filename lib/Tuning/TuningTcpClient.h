#ifndef TUNINGTCPCLIENT_H
#define TUNINGTCPCLIENT_H

#include <queue>
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/ITuningTcpClient.h"
#include "../lib/Tuning/TuningSourceState.h"
#include "../lib/Tcp.h"
#include "../lib/Hash.h"
#include "../Proto/network.pb.h"

struct TuningWriteCommand
{
	enum class TuningWriteCommandType
	{
		WriteValue,
		Apply,
		ActivateLm
	};

	// Data

	Hash m_hash = 0;
	QString m_equipmentId;		// Used only for activation/deactivation LM
	TuningValue m_value;

	TuningWriteCommandType m_type = TuningWriteCommandType::WriteValue;

	bool m_enableControl = false;
	bool m_forceTakeControl = false;

	// Write constructor
	//
	TuningWriteCommand(const QString& appSignalId, const TuningValue& value) :
		TuningWriteCommand(::calcHash(appSignalId), value)
	{
	}

	TuningWriteCommand(Hash hash, const TuningValue& value)
	{
		m_type = TuningWriteCommandType::WriteValue;

		m_hash = hash;
		m_value = value;
	}

	// Apply constructor
	//
	TuningWriteCommand(bool apply)
	{
		Q_UNUSED(apply);
		m_type = TuningWriteCommandType::Apply;
	}

	// Activate LM constructor
	//
	TuningWriteCommand(const QString& equipmentId, bool enableControl, bool forceTakeControl)
	{
		m_type = TuningWriteCommandType::ActivateLm;

		m_equipmentId = equipmentId;
		m_enableControl = enableControl;
		m_forceTakeControl = forceTakeControl;
	}

	// Serializing

	bool save(Network::TuningWriteCommand* message) const;
	bool load(const Network::TuningWriteCommand& message);
};

enum class LmStatusFlagMode
{
	None,
	SOR,
	AccessKey
};

class TuningTcpClient : public Tcp::Client, public ITuningTcpClient
{
	Q_OBJECT

	Q_ENUM(NetworkError)

public:
	TuningTcpClient(const SoftwareInfo& softwareInfo,
					TuningSignalManager* signalManager);

	virtual ~TuningTcpClient();

public:
	void setSimulationMode(bool value);

	// Tuning sources
	//
	std::vector<Hash> tuningSourcesEquipmentHashes() const;
	std::vector<TuningSource> tuningSourcesInfo() const;
	bool tuningSourceInfo(Hash equipmentHash, TuningSource* result) const;

	bool activateTuningSourceControl(const QString& equipmentId, bool enableControl, bool forceTakeControl);

	// Writing states
	//
	void writeTuningSignal(const TuningWriteCommand& data);
	void writeTuningSignal(const std::vector<TuningWriteCommand>& data);

	// Apply states
	//
	void applyTuningSignals();

	// ITuningTcpClient implementation
	//
public:
	virtual bool writeTuningSignal(QString appSignalId, TuningValue value) override;

private:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

protected:
	void resetToGetTuningSources();
	void resetToGetTuningSourcesState();
	void resetToProcessTuningSignals();

	void requestTuningSourcesInfo();
	void processTuningSourcesInfo(const QByteArray& data);

	void requestTuningSourcesState();
	void processTuningSourcesState(const QByteArray& data);

	void requestActivateTuningSource(const QString& equipmentId, bool enableControl, bool forceTakeControl);
	void processActivateTuningSource(const QByteArray& data);

	void requestReadTuningSignals();
	void processReadTuningSignals(const QByteArray& data);

	void requestWriteTuningSignals();
	void processWriteTuningSignals(const QByteArray& data);

	void requestApplyTuningSignals();
	void processApplyTuningSignals(const QByteArray& data);

	virtual void writeLogAlert(const QString& message);
	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);

	virtual void writeLogSignalChange(const AppSignalParam& param, const TuningValue& oldValue, const TuningValue& newValue);
	virtual void writeLogSignalChange(const QString& message);

public slots:
	void slot_signalsUpdated();
	void slot_configurationArrived(HostAddressPort address, bool autoApply, LmStatusFlagMode lmStatusFlagMode);

signals:
	void tuningSourcesArrived();

private:
	QString networkErrorStr(NetworkError error);

	// Properties
	//
public:
	QString instanceId() const;
	void setInstanceId(const QString& instanceId);

	Hash instanceIdHash() const;

	int requestInterval() const;
	void setRequestInterval(int requestInterval);

	bool autoApply() const;
	void setAutoApply(bool value);

	// LM Control functions

	bool singleLmControlMode() const;

	bool clientIsActive() const;

	QString activeClientId() const;
	QString activeClientIp() const;

	int activeTuningSourceCount() const;

	QString singleActiveTuningSource() const;

	LmStatusFlagMode lmStatusFlagMode() const;

	// Data
	//
private:
	QString m_instanceId;
	Hash m_instanceIdHash;
	int m_requestInterval = 100;
	bool m_autoApply = true;

	LmStatusFlagMode m_lmStatusFlagMode = LmStatusFlagMode::SOR;

	TuningSignalManager* m_signals = nullptr;

protected:

	// Tuning sources
	//
	mutable QMutex m_tuningSourcesMutex;				// For access to m_tuningSources, m_equipmentToSignalMap
	std::map<Hash, TuningSource> m_tuningSources;		// Key is hash of EquipmentID

private:
	// Processing
	//
	mutable QMutex m_writeQueueMutex;					// For access to m_writeQueue
	std::queue<TuningWriteCommand> m_writeQueue;

	int m_readTuningSignalIndex = 0;
	int m_readTuningSignalCount = 0;

	mutable QMutex m_signalHashesMutex;					// For access to m_signalHashes
	std::vector<Hash> m_signalHashes;

	bool m_singleLmControlMode = false;

	mutable QMutex m_activeClientMutex;				// For access to m_activeClientId, m_activeClientIp
	QString m_activeClientId;
	QString m_activeClientIp;

	bool m_currentClientIsActive = false;

protected:
	bool m_simulationMode = false;

private:
	// Cached protobuf messages
	//
	::Network::GetTuningSourcesStates m_getTuningSourcesStates;
	::Network::GetTuningSourcesStatesReply m_tuningSourcesStatesReply;

	::Network::GetTuningSourcesInfo m_getTuningSourcesInfo;
	::Network::GetTuningSourcesInfoReply m_tuningSourcesInfoReply;

	::Network::ChangeConrolledTuningSourceRequest m_activateTuningSource;
	::Network::ChangeConrolledTuningSourceReply m_activateTuningSourceReply;

	::Network::TuningSignalsRead m_readTuningSignals;
	::Network::TuningSignalsReadReply m_readTuningSignalsReply;

	::Network::TuningSignalsWrite m_writeTuningSignals;
	::Network::TuningSignalsWriteReply m_writeTuningSignalsReply;

	::Network::TuningSignalsApply m_applyTuningSignals;
	::Network::TuningSignalsApplyReply m_applyTuningSignalsReply;
};


#endif // TUNINGTCPCLIENT_H
