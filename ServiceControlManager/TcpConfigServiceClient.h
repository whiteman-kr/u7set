#pragma once

#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/SocketIO.h"
#include "../CommonLib/Hash.h"
#include "../lib/OrderedHash.h"
#include "../AppSignalLib/AppSignal.h"
#include "../AppDataService/AppDataSource.h"
#include "../Proto/network.pb.h"


class QTimer;

class TcpConfigServiceClient : public Tcp::Client
{
	Q_OBJECT

public:
	TcpConfigServiceClient(const SoftwareInfo& softwareInfo,
						   const HostAddressPort& serverAddressPort);

	TcpConfigServiceClient(const SoftwareInfo& softwareInfo,
						   const HostAddressPort& serverAddressPort1,
						   const HostAddressPort& serverAddressPort2);

	virtual ~TcpConfigServiceClient();

	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	const Builder::BuildInfo& buildInfo() { return m_buildInfo; }
	bool buildInfoIsReady() { return m_buildInfoIsReady; }

	const Network::ConfigurationServiceState& serviceState() { return m_configurationServiceStateMessage; }
	bool serviceStateIsReady() { return m_serviceStateIsReady; }

	const Network::ServiceClients& clients() { return m_serviceClientsMessage; }
	bool clientsIsReady() { return m_clientsIsReady; }

	bool settingsIsReady() { return m_settingsIsReady; }

	QString equipmentID() { return m_equipmentID; }
	QString autoloadBuildPath() {return m_autoloadBuildPath; }
	QString workDirectory() { return m_workDirectory; }

signals:
	void serviceStateLoaded();
	void clientsLoaded();
	void buildInfoLoaded();
	void settingsLoaded();

	void disconnected();

private slots:
	void updateState();

private:
	void onGetConfigurationServiceState(const char* replyData, quint32 replyDataSize);
	void onGetConfigurationServiceClientList(const char* replyData, quint32 replyDataSize);
	void onGetConfigurationServiceLoadedBuildInfoReply(const char* replyData, quint32 replyDataSize);
	void onGetConfigurationServiceSettingsReply(const char* replyData, quint32 replyDataSize);

	QTimer* m_updateStatesTimer = nullptr;
	Builder::BuildInfo m_buildInfo;
	Network::ConfigurationServiceState m_configurationServiceStateMessage;
	Network::ServiceClients m_serviceClientsMessage;

	QString m_equipmentID;
	QString m_autoloadBuildPath;
	QString m_workDirectory;

	bool m_serviceStateIsReady = false;
	bool m_clientsIsReady = false;
	bool m_buildInfoIsReady = false;
	bool m_settingsIsReady = false;
};
