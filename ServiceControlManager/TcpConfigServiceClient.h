#pragma once

#include "../lib/Tcp.h"
#include "../lib/SocketIO.h"
#include "../lib/Hash.h"
#include "../lib/OrderedHash.h"
#include "../lib/Signal.h"
#include "../lib/AppDataSource.h"
#include "../Proto/network.pb.h"


class QTimer;

class TcpConfigServiceClient : public Tcp::Client
{
	Q_OBJECT

private:
	QTimer* m_updateStatesTimer = nullptr;
	Builder::BuildInfo m_buildInfo;

	bool m_buildInfoIsReady = false;
	bool m_settingsIsReady = false;

	void onGetConfigurationSerivceLoadedBuildInfoReply(const char* replyData, quint32 replyDataSize);
	void onGetConfigurationSerivceSettingsReply(const char* replyData, quint32 replyDataSize);

private slots:
	void updateState();

signals:
	void buildInfoLoaded();
	void settingsLoaded(QString equipmentID, QString autoloadBuildPath, QString workDirectory);

	void disconnected();

public:
	TcpConfigServiceClient(const HostAddressPort& serverAddressPort);
	TcpConfigServiceClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
	virtual ~TcpConfigServiceClient();

	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	const Builder::BuildInfo& buildInfo() { return m_buildInfo; }
	bool buildInfoIsReady() { return m_buildInfoIsReady; }
};
