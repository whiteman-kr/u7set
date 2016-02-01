#pragma once

#include "../include/Service.h"
#include "../include/CfgServerLoader.h"


class ConfigurationServiceWorker : public ServiceWorker
{
	Q_OBJECT

private:
	UdpSocketThread* m_infoSocketThread = nullptr;

	Tcp::ServerThread* m_cfgServerThread = nullptr;

	void startCfgServerThread();
	void stopCfgServerThread();

	void startUdpThreads();
	void stopUdpThreads();

	void onGetInfo(UdpRequest& request);
	void onGetSettings(UdpRequest& request);
	void onSetSettings(UdpRequest& request);

public:
	ConfigurationServiceWorker() : ServiceWorker(ServiceType::Configuration) {}
	virtual void initialize() override;
	virtual void shutdown() override;

	ServiceWorker* createInstance() override { return new ConfigurationServiceWorker; }

signals:
	void ackInformationRequest(UdpRequest request);

public slots:
	void onInformationRequest(UdpRequest request);
};

