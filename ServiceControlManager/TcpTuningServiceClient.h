#pragma once

#include "../lib/Tcp.h"
#include "../lib/SocketIO.h"
#include "../Proto/network.pb.h"
#include "../lib/Tuning/TuningSourceState.h"


class QTimer;

class TcpTuningServiceClient : public Tcp::Client
{
	Q_OBJECT

public:
	TcpTuningServiceClient(const SoftwareInfo& softwareInfo,
					 const HostAddressPort& serverAddressPort);

	TcpTuningServiceClient(const SoftwareInfo& softwareInfo,
					 const HostAddressPort& serverAddressPort1,
					 const HostAddressPort& serverAddressPort2);
	virtual ~TcpTuningServiceClient();

	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	const Network::ServiceClients& clients() { return m_serviceClientsMessage; }
	bool clientsIsReady() { return m_clientsIsReady; }
	bool stateIsReady() { return m_stateIsReady; }
	bool settingsIsReady() { return m_settingsIsReady; }
	bool tuningSourcesInfoIsReady() { return m_tuningSourcesInfoIsReady; }
	bool tuningSourcesStateIsReady() { return m_tuningSourcesStateIsReady; }

	QString equipmentID() { return m_equipmentID; }
	QString configIP1() {return m_configIP1; }
	QString configIP2() { return m_configIP2; }

	const QList<TuningSource>& tuningSources() { return m_tuningSources; }

signals:
	void clientsLoaded();

	void settingsLoaded();

	void stateLoaded();

	void tuningSourcesInfoLoaded();
	void tuningSoursesStateUpdated();

	void disconnected();

private:
	void init();

	void onGetClientList(const char* replyData, quint32 replyDataSize);

	void onGetServiceSettings(const char* replyData, quint32 replyDataSize);

	void onGetTuningSourcesInfo(const char* replyData, quint32 replyDataSize);
	void onGetTuningSourcesStates(const char* replyData, quint32 replyDataSize);

	void startStateUpdating();

private slots:
	void updateStates();

private:
	QList<TuningSource> m_tuningSources;

	QTimer* m_updateStatesTimer = nullptr;

	// reused protobuf messages
	//
	Network::ServiceClients m_serviceClientsMessage;

	Network::ServiceSettings m_getServiceSettings;

	Network::GetTuningSourcesStates m_getTuningSourcesStates;
	Network::GetTuningSourcesStatesReply m_tuningSourcesStatesReply;

	Network::GetTuningSourcesInfo m_getTuningSourcesInfo;
	Network::GetTuningSourcesInfoReply m_tuningSourcesInfoReply;

	QString m_equipmentID;
	QString m_configIP1;
	QString m_configIP2;

	//

	bool m_clientsIsReady = false;
	bool m_settingsIsReady = false;
	bool m_stateIsReady = false;
	bool m_tuningSourcesInfoIsReady = false;
	bool m_tuningSourcesStateIsReady = false;
};
