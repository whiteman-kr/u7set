#pragma once

#include "../lib/Service.h"
#include "../lib/CfgServerLoader.h"


class ConfigurationServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	ConfigurationServiceWorker();

signals:
	void ackInformationRequest(UdpRequest request);

public slots:
	void onInformationRequest(UdpRequest request);

private:
	virtual void initCmdLineParser() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	void startCfgServerThread();
	void stopCfgServerThread();

	void startUdpThreads();
	void stopUdpThreads();

	void onGetInfo(UdpRequest& request);
	void onGetSettings(UdpRequest& request);
	void onSetSettings(UdpRequest& request);

private:
	UdpSocketThread* m_infoSocketThread = nullptr;
	Tcp::ServerThread* m_cfgServerThread = nullptr;

	QString m_clientIPStr;
	QString m_buildPath;
};

