#pragma once

#include "../OnlineLib/SoftwareSettings.h"
#include <ServiceLib/Service.h>

class UdpModelLinkThread;
class SimLinkThread;

class ModelSimBridgeWorker : public ServiceWorker
{
	Q_OBJECT

public:
	ModelSimBridgeWorker(const SoftwareInfo& softwareInfo,
						 const QString& serviceInstanceName,
						 int argc,
						 char** argv,
						 CircularLoggerShared logger,
						 CircularLoggerShared simLog);

	ModelSimBridgeWorker(const ModelSimBridgeWorker* worker);

	~ModelSimBridgeWorker();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const override;

	CircularLoggerShared simLog() const;

private:
	virtual void initServiceSpecificCmdLineArgs() override;
	virtual void loadServiceSpecificSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	void runUdpModelLinkThread();
	void stopUdpModelLinkThread();

	void runSimulatorLinkThread();
	void stopSimulatorLinkThread();

private:
	CircularLoggerShared m_simLog;

	mutable QMutex m_startStopMutex;

	QString m_modelIP = "0.0.0.0";
	int m_modelRequestPort = 9999;
	int m_modelReplyPort = 9998;

	QString m_simIP = "127.0.0.1";
	int m_simPort = 50051;

	UdpModelLinkThread* m_udpModelLinkThread = nullptr;
	SimLinkThread* m_simLinkThread = nullptr;
};
