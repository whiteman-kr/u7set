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

	QString m_equipmentId = 0;

	TuningValue m_value;

	TuningWriteCommandType m_type = TuningWriteCommandType::WriteValue;

	bool m_enableControl = false;

	// Write constructor

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

	TuningWriteCommand()
	{
		m_type = TuningWriteCommandType::Apply;
	}

	// Activate LM constructor

	TuningWriteCommand(const QString& equipmentId, bool enableControl)
	{
		m_type = TuningWriteCommandType::ActivateLm;

		m_equipmentId = equipmentId;
		m_enableControl = enableControl;
	}

	// Serializing

	bool save(Network::TuningWriteCommand* message) const;
	bool load(const Network::TuningWriteCommand& message);
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
#ifdef Q_DEBUG
	void setSimulationMode(bool value);
#endif

	// Tuning sources
	//
	std::vector<Hash> tuningSourcesEquipmentHashes() const;
	std::vector<TuningSource> tuningSourcesInfo() const;
	bool tuningSourceInfo(Hash equipmentHash, TuningSource* result) const;

	bool tuningSourceCounters(Hash equipmentHash, int* errorsCount, int* sorCount) const;
	bool tuningSourceStatus(Hash equipmentHash, int* errorsCount, int* sorCount, QString* status) const;

	bool activateTuningSourceControl(const QString& equipmentId, bool enableControl);

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

	void requestActivateTuningSource(const QString& equipmentId, bool enableControl);
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
	void slot_configurationArrived(HostAddressPort address1, HostAddressPort address2, bool autoApply);

signals:
	void tuningSourcesArrived();

private:
	QString networkErrorStr(NetworkError error);

	// Properties
	//
public:
	QString instanceId() const;
	void setInstanceId(const QString& instanceId);

	int requestInterval() const;
	void setRequestInterval(int requestInterval);

	bool autoApply() const;
	void setAutoApply(bool value);

	bool singleLmControlMode() const;

	// Data
	//
private:
	QString m_instanceId;
	Hash m_instanceIdHash;
	int m_requestInterval = 100;
	bool m_autoApply = true;

	TuningSignalManager* m_signals = nullptr;

protected:
	// Tuning sources
	//
	mutable QMutex m_tuningSourcesMutex;				// For access to m_tuningSources, m_equipmentToSignalMap
	std::map<Hash, TuningSource> m_tuningSources;		// Key is hash of EquipmentID

	std::multimap<Hash, Hash> m_equipmentToSignalMap;	// Key is hash of EquipmentID, values are hashes of signals

private:
	// Processing
	//
	mutable QMutex m_writeQueueMutex;					// For access to m_writeQueue
	std::queue<TuningWriteCommand> m_writeQueue;

	int m_readTuningSignalIndex = 0;
	int m_readTuningSignalCount = 0;

	std::vector<Hash> m_signalHashes;

	bool m_singleLmControlMode = false;

#ifdef Q_DEBUG
	bool m_simulationMode = false;
#endif

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
