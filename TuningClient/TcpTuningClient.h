#ifndef TCPTUNINGCLIENT_H
#define TCPTUNINGCLIENT_H

#include "../lib/Tcp.h"
#include "../lib/Hash.h"
#include "../Proto/network.pb.h"
#include "ObjectManager.h"
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
    double m_value = 0;

    WriteCommand(Hash hash, double value)
    {
        m_hash = hash;
        m_value = value;
    }
};


class TcpTuningClient : public Tcp::Client
{
    Q_ENUM(NetworkError)

    Q_OBJECT
public:
	TcpTuningClient(ConfigController* configController, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
	~TcpTuningClient();

	std::vector<TuningSource> tuningSourcesInfo();

    bool tuningSourceInfo(quint64 id, TuningSource& result);

    void writeTuningSignal(Hash hash, double value);

    void writeTuningSignals(std::vector<WriteCommand> signalsArray);

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

private:
	ConfigController* m_cfgController = nullptr;


private:

	QMutex m_mutex;

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

    std::map<quint64, TuningSource> m_tuningSources;

	std::vector<QString> m_signalList;

    std::queue<WriteCommand> m_writeQueue;

    int m_readTuningSignalIndex = 0;
    int m_readTuningSignalCount = 0;



};

extern TcpTuningClient* theTcpTuningClient;

#endif // TCPTUNINGCLIENT_H
