#ifndef TCPTUNINGCLIENT_H
#define TCPTUNINGCLIENT_H

#include "../lib/Tcp.h"
#include "../lib/Hash.h"
#include "../Proto/network.pb.h"
#include "ObjectManager.h"
#include "ConfigController.h"

struct TuningSource
{
    ::Network::DataSourceInfo m_info;
    ::Network::TuningSourceState m_state;
};


class TcpTuningClient : public Tcp::Client
{
	Q_OBJECT
public:
	TcpTuningClient(ConfigController* configController, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
	~TcpTuningClient();

	std::vector<TuningSource> tuningSourcesInfo();

public:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

protected:
	void resetToGetTuningSources();
	void resetToGetTuningState();

	void requestTuningSourcesInfo();
	void processTuningSourcesInfo(const QByteArray& data);

	void requestTuningSourcesState();
	void processTuningSourcesState(const QByteArray& data);

protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

signals:
	void tuningSourcesArrived();
	void connectionFailed();



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

	std::map<quint64, TuningSource> m_tuningSources;

	std::vector<QString> m_signalList;


};

extern TcpTuningClient* theTcpTuningClient;

#endif // TCPTUNINGCLIENT_H
