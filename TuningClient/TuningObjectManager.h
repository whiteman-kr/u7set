#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "Stable.h"
#include "../lib/Tuning/TuningObject.h"
#include "TuningObjectManager.h"

#include "../Proto/network.pb.h"
#include "../lib/Tcp.h"
#include "../lib/Hash.h"
#include "ConfigController.h"

#include <queue>

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

class TuningObjectManager : public Tcp::Client
{
    Q_ENUM(NetworkError)

    Q_OBJECT

public:
    TuningObjectManager(ConfigController* configController, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
    virtual ~TuningObjectManager();

    bool loadDatabase(const QByteArray& data, QString *errorCode);

    TuningObjectStorage objectStorage();

    bool objectExists(Hash hash) const; // WARNING!!! Lock the mutex before calling this function!!!

    TuningObject* objectPtrByHash(Hash hash) const; // WARNING!!! Lock the mutex before calling this function!!!

    // Tuning sources

    QStringList tuningSourcesEquipmentIds();

    std::vector<TuningSource> tuningSourcesInfo();

    bool tuningSourceInfo(quint64 id, TuningSource& result);

    // Writing states

    void writeTuningSignal(Hash hash, float value);

    void writeModifiedTuningObjects(std::vector<TuningObject>& objects);

	QString getStateToolTip();

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

protected slots:

    void slot_configurationArrived(ConfigSettings configuration);
    void slot_signalsUpdated();

signals:

    void tuningSourcesArrived();
    void connectionFailed();

private:

    QString networkErrorStr(NetworkError error);

public:

    QMutex m_mutex;

private:

    ConfigController* m_cfgController = nullptr;

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

    TuningObjectStorage m_objects;  // WARNING!!! Use this object only with m_mutex locked!!!

    // Tuning sources
    //

    QStringList m_tuningSourcesList;

    std::map<quint64, TuningSource> m_tuningSources;

    // Processing
    //

    std::queue<WriteCommand> m_writeQueue;

    int m_readTuningSignalIndex = 0;
    int m_readTuningSignalCount = 0;

};


#endif // OBJECTMANAGER_H
