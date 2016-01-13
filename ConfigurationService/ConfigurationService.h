#pragma once

#include "../include/BaseService.h"
#include "../include/CfgServerLoader.h"


class ConfigurationServiceMainFunctionWorker : public MainFunctionWorker
{
	Q_OBJECT

private:
	Tcp::ServerThread* m_cfgServerThread = nullptr;

	void onGetSettings(UdpRequest& request);

public:
	virtual void initialize() override;
	virtual void shutdown() override;

signals:
	void ackInformationRequest(UdpRequest request);

public slots:
	void onInformationRequest(UdpRequest request);
};


class ConfigurationService : public BaseService
{
public:
	ConfigurationService(int argc, char ** argv);
	~ConfigurationService();
};
