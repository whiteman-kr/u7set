#pragma once

#include "../include/Service.h"
#include "../include/CfgServerLoader.h"


class ConfigurationServiceWorker : public ServiceWorker
{
	Q_OBJECT

private:
	UdpSocketThread* m_infoSocketThread = nullptr;

	Tcp::ServerThread* m_cfgServerThread = nullptr;

	QString m_buildFolder;

	void startCfgServerThread();
	void stopCfgServerThread();

	void startUdpThreads();
	void stopUdpThreads();

	void onGetInfo(UdpRequest& request);
	void onGetSettings(UdpRequest& request);
	void onSetSettings(UdpRequest& request);

public:
	ConfigurationServiceWorker(const QString& serviceStrID, const QString& buildFolder);

	virtual void initialize() override;
	virtual void shutdown() override;

	ServiceWorker* createInstance() override
	{
		return new ConfigurationServiceWorker(serviceStrID(), m_buildFolder);
	}

signals:
	void ackInformationRequest(UdpRequest request);

public slots:
	void onInformationRequest(UdpRequest request);
};

