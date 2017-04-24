#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include <queue>

#include "Stable.h"
#include "../lib/Tuning/TuningSignal.h"
#include "../lib/Tuning/TuningController.h"

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

class TuningSignalManager : public Tcp::Client
{
    Q_ENUM(NetworkError)

    Q_OBJECT

public:
	TuningSignalManager();
    virtual ~TuningSignalManager();

	void setInstanceId(const QString& instanceId);

	void setRequestInterval(int requestInterval);

    bool loadDatabase(const QByteArray& data, QString *errorCode);

    TuningSignalStorage objectStorage();

    bool objectExists(Hash hash) const; // WARNING!!! Lock the mutex before calling this function!!!

    TuningSignal* objectPtrByHash(Hash hash) const; // WARNING!!! Lock the mutex before calling this function!!!

    // Tuning sources

    QStringList tuningSourcesEquipmentIds();

    std::vector<TuningSource> tuningSourcesInfo();

    bool tuningSourceInfo(quint64 id, TuningSource& result);

    // Writing states

	void writeTuningSignal(Hash hash, float value);

    void writeModifiedTuningSignals(std::vector<TuningSignal>& objects);

	// Information

	QString getStateToolTip();

	// Controller

	void connectTuningController(TuningController* controller);

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

    void requestTuningSourcesInfo();
    void processTuningSourcesInfo(const QByteArray& data);

    void requestTuningSourcesState();
    void processTuningSourcesState(const QByteArray& data);

    void processTuningSignals();

    void requestReadTuningSignals();
    void processReadTuningSignals(const QByteArray& data);

    void requestWriteTuningSignals();
    void processWriteTuningSignals(const QByteArray& data);

	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);


public slots:
	void slot_signalsUpdated(QByteArray data);
	void slot_serversArrived(HostAddressPort address1, HostAddressPort address2);


private slots:
	void slot_exists(QString appSignalID, bool* result, bool* ok);
	void slot_valid(QString appSignalID, bool* result, bool* ok);
	void slot_analog(QString appSignalID, bool* result, bool* ok);

	void slot_highLimit(QString appSignalID, float* result, bool* ok);
	void slot_lowLimit(QString appSignalID, float* result, bool* ok);

	void slot_decimalPlaces(QString appSignalID, float* result, bool* ok);

	void slot_value(QString appSignalID, float *result, bool* ok);
	void slot_setValue(QString appSignalID, float value, bool* ok);
signals:

	void tuningSourcesArrived();
	void connectionFailed();



private:

    QString networkErrorStr(NetworkError error);

public:

    QMutex m_mutex;

private:

	//ConfigController* m_cfgController = nullptr;

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

    TuningSignalStorage m_objects;  // WARNING!!! Use this object only with m_mutex locked!!!

	// Tuning sources
    //

    QStringList m_tuningSourcesList;

    std::map<quint64, TuningSource> m_tuningSources;

    // Processing
    //

    std::queue<WriteCommand> m_writeQueue;

    int m_readTuningSignalIndex = 0;
    int m_readTuningSignalCount = 0;

	QString m_instanceId;
	int m_requestInterval = 10;

	std::map<TuningController*, bool> m_tuningControllersMap;
};


#endif // OBJECTMANAGER_H
