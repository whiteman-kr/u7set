#pragma once

#include "../lib/Service.h"
#include "../lib/CfgServerLoader.h"


class ConfigurationServiceWorker : public ServiceWorker
{
	Q_OBJECT

private:
	UdpSocketThread* m_infoSocketThread = nullptr;

	Tcp::ServerThread* m_cfgServerThread = nullptr;

	QString m_clientIPStr;

	void startCfgServerThread();
	void stopCfgServerThread();

	void startUdpThreads();
	void stopUdpThreads();

	void onGetInfo(UdpRequest& request);
	void onGetSettings(UdpRequest& request);
	void onSetSettings(UdpRequest& request);

public:
	ConfigurationServiceWorker(const QString& serviceEquipmentID, const QString& buildFolder, const QString& ipStr);

	virtual void initialize() override;
	virtual void shutdown() override;

	ServiceWorker* createInstance() override
	{
		return new ConfigurationServiceWorker(serviceEquipmentID(), buildPath(), m_clientIPStr);
	}

signals:
	void ackInformationRequest(UdpRequest request);

public slots:
	void onInformationRequest(UdpRequest request);
};

