#pragma once

#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/SocketIO.h"
#include "../OnlineLib/ApertureRecord.h"
#include "../AppDataService/AppDataSource.h"

class QTimer;

class ScmServiceClient : public Tcp::Client
{
	Q_OBJECT

public:
	ScmServiceClient(const SoftwareInfo& softwareInfo,
						   const HostAddressPort& serverAddressPort);

	ScmServiceClient(const SoftwareInfo& softwareInfo,
						   const HostAddressPort& serverAddressPort1,
						   const HostAddressPort& serverAddressPort2);

	virtual ~ScmServiceClient();

	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	void enqueueRequest(int requestID);
	void checkRequestQueue();

	void overrideApertures(const std::vector<ApertureRecord>& apertures);

signals:
	void serviceInfoUpdated(QByteArray replyData);
	void requestEnqueued();

	void serviceStateLoaded();
	void clientsLoaded();
	void buildInfoLoaded();
	void settingsLoaded();

	void socketDisconnected();

private slots:
	void sendSrvGetInfoRequest();

private:
	void onGetServiceInfo(const char* replyData, quint32 replyDataSize);

	QTimer* m_timer = nullptr;

	Network::ServiceInfo m_srvInfo;

	QMutex m_apertureRecordsMutex;
	std::vector<ApertureRecord> m_apertureRecords;

	QMutex m_requestQueueMutex;
	std::queue<int> m_requestQueue;
};
