#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include <queue>

#include "../lib/Tuning/TuningSignalState.h"
#include "../lib/Tuning/TuningController.h"
#include "../lib/Tuning/TuningLog.h"

#include "../Proto/network.pb.h"
#include "../lib/Tcp.h"
#include "../lib/Hash.h"

struct TuningSource
{
	::Network::DataSourceInfo m_info;
	::Network::TuningSourceState m_state;
};

struct WriteCommand
{
	Hash m_hash = 0;
	float m_value = 0;

	WriteCommand(Hash hash, float value)
	{
		m_hash = hash;
		m_value = value;
	}
};

struct TuningModelRecord;

class TuningSignalManager : public Tcp::Client
{
	Q_ENUM(NetworkError)

	Q_OBJECT

public:
	TuningSignalManager(E::SoftwareType softwareType, const QString equipmentID, int majorVersion, int minorVersion, int commitNo, TuningLog::TuningLog* tuningLog);
	virtual ~TuningSignalManager();

	void setInstanceId(const QString& instanceId);

	void setRequestInterval(int requestInterval);

	bool loadDatabase(const QByteArray& data, QString* errorCode);

	TuningSignalStorage signalsStorage();

	// Reading signals and states

	bool signalExists(Hash hash) const; // WARNING!!! Lock the m_signalsMutex before calling this function!!!

	TuningSignalState stateByHash(Hash hash) const; // WARNING!!! Lock the m_statesMutex before calling this function!!!

	void updateStates(std::vector<TuningModelRecord>& items);

	// Tuning sources

	QStringList tuningSourcesEquipmentIds();

	std::vector<TuningSource> tuningSourcesInfo();

	bool tuningSourceInfo(quint64 id, TuningSource* result);

	// Writing states

	void writeTuningSignals(std::vector<std::pair<Hash, float>>& data);

	// Information

	QString getStateToolTip();

	// Controller

	void connectTuningController(TuningController* controller);

	//
	// Status and counters
	//
	int getLMErrorsCount();
	int getLMErrorsCount(const std::vector<QString>& equipmentHashes);

	int getSORCount();
	int getSORCount(const std::vector<QString>& equipmentHashes);


private:

	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	void invalidateSignals();

	TuningSignalState* statePtrByHash(Hash hash); // WARNING!!! Lock the m_statesMutex before calling this function!!!

protected:
	void resetToGetTuningSources();
	void resetToGetTuningSourcesState();

	void requestTuningSourcesInfo();
	void processTuningSourcesInfo(const QByteArray& data);

	void requestTuningSourcesState();
	void processTuningSourcesState(const QByteArray& data);

	void processTuningSignals();

	void requestReadTuningSignals();
	void processReadTuningSignals(const QByteArray& data);

	void requestWriteTuningSignals();
	void processWriteTuningSignals(const QByteArray& data);

	virtual void writeLogAlert(const QString& message);
	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);


public slots:
	void slot_signalsUpdated(QByteArray data);
	void slot_serversArrived(HostAddressPort address1, HostAddressPort address2);


private slots:
	void slot_writeValue(QString appSignalID, float value, bool* ok);

	void slot_signalParam(QString appSignalID, AppSignalParam* result, bool* ok);
	void slot_signalState(QString appSignalID, TuningSignalState* result, bool* ok);

signals:

	void tuningSourcesArrived();
	void connectionFailed();



private:

	QString networkErrorStr(NetworkError error);

private:
	// Cache protobug messages
	//
	::Network::GetTuningSourcesStates m_getTuningSourcesStates;
	::Network::GetDataSourcesInfoReply m_tuningDataSourcesInfoReply;

	::Network::GetTuningSourcesInfo m_getTuningSourcesInfo;
	::Network::GetTuningSourcesStatesReply m_tuningDataSourcesStatesReply;

	::Network::TuningSignalsRead m_readTuningSignals;
	::Network::TuningSignalsReadReply m_readTuningSignalsReply;

	::Network::TuningSignalsWrite m_writeTuningSignals;
	::Network::TuningSignalsWriteReply m_writeTuningSignalsReply;

	// Objects storage
	//

	TuningSignalStorage m_signals;  // WARNING!!! Use this object only with m_mutex locked!!!

	// States storage

	std::map<Hash, int> m_statesMap;
	std::vector<TuningSignalState> m_states;

	// Tuning sources
	//

	QStringList m_tuningSourcesList;

	std::map<quint64, TuningSource> m_tuningSources;

	// Processing
	//

	TuningLog::TuningLog* m_tuningLog = nullptr;

	std::queue<WriteCommand> m_writeQueue;

	int m_readTuningSignalIndex = 0;
	int m_readTuningSignalCount = 0;

	QString m_instanceId;
	int m_requestInterval = 10;

	std::map<TuningController*, bool> m_tuningControllersMap;

	QMutex m_tuningSourcesMutex;

public:
	QMutex m_signalsMutex;
	QMutex m_statesMutex;


};


#endif // OBJECTMANAGER_H
