#pragma once

#include "../OnlineLib/CfgLoader.h"
#include "../OnlineLib/SoftwareSettings.h"
#include <ServiceLib/Service.h>

class UdpModelLinkThread;
class SimulatorLinkThread;

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

	void clear();

	virtual void initialize() override;
	virtual void shutdown() override;

	void runUdpModelLinkThread();
	void stopUdpModelLinkThread();

	void runSimulatorLinkThread();
	void stopSimulatorLinkThread();

private:
	CircularLoggerShared m_simLog;

	mutable QMutex m_startStopMutex;

	QString m_modelIP;
	int m_modelPort = 0;

	UdpModelLinkThread* m_udpModelLinkThread = nullptr;
	//SimulatorLinkThread* m_simulatorLinkThread = nullptr;
};
