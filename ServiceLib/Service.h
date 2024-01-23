#pragma once

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QSettings>
#include <iostream>

#include "./qtservice/src/qtservice.h"
#include "../UtilsLib/SimpleThread.h"
#include "../OnlineLib/UdpSocket.h"
#include "../OnlineLib/CircularLogger.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../Proto/network.pb.h"
#include "CommandLineParser.h"

enum ServiceState
{
	Stopped,
	Starts,
	Work,
	Stops,

	Undefined,			// this states used by 'Service Control Manager' only
	Unavailable,
};

struct ServiceInfo
{
	E::SoftwareType softwareType = E::SoftwareType::Unknown;
	quint16 port = 0;
	QString name;
	QString shortName;
};

inline const std::vector<ServiceInfo> servicesInfo =
{
	{ E::SoftwareType::BaseService, PORT_BASE_SERVICE, "Base Service", "BaseSrv" },
	{ E::SoftwareType::ConfigurationService, PORT_CONFIGURATION_SERVICE, "Configuration Service", "CfgSrv" },
	{ E::SoftwareType::AppDataService, PORT_APP_DATA_SERVICE, "Application Data Service", "AppDataSrv" },
	{ E::SoftwareType::TuningService, PORT_TUNING_SERVICE, "Tuning Service", "TuningSrv" },
	{ E::SoftwareType::ArchiveService, PORT_ARCHIVING_SERVICE, "Data Archiving Service", "DataArchSrv" },
	{ E::SoftwareType::DiagDataService, PORT_DIAG_DATA_SERVICE, "Diagnostics Data Service", "DiagDataSrv" },
	{ E::SoftwareType::GatewayService, PORT_GATEWAY_SERVICE, "Gateway Service", "GatewaySrv" },
};

class Service;

// -------------------------------------------------------------------------------------
//
// ServiceWorker class declaration
//
// -------------------------------------------------------------------------------------

class ServiceWorker : public SimpleThreadWorker
{
	Q_OBJECT

public:
	// FIRST instance of ServiceWorker constructor
	//
	ServiceWorker(const SoftwareInfo& softwareInfo,
				  const QString& serviceInstanceName,
				  int argc,
				  char** argv,
				  CircularLoggerShared logger);

protected:
	// SECOND instance of ServiceWorker constructor
	// should be called in createInstance function of derived class only!
	//
	ServiceWorker(const ServiceWorker* prevInstance);

public:
	virtual ~ServiceWorker();

	int argc() const { return m_argc; }
	const char** argv() const { return m_argv; }

	QString appPath() const;
	QString cmdLine() const;

	QString serviceName() const;

	const SoftwareInfo& softwareInfo() const;
	E::SoftwareType softwareType() const;

	void setService(Service* service);
	Service* service();

	// createInstance() must be implemented in derived class as:
	//
	// ServiceWorker* DerivedServiceWorker::createInstance() const override
	// {
	//		DerivedServiceWorker* newInstance =
	//				new DerivedServiceWorker(this);
	//
	//		return newInstance;
	// }
	//
	virtual ServiceWorker* createInstance() const = 0;

	virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const = 0;

	QString equipmentID() const { return m_equipmentID; }

	HostAddressPort cfgServiceIP1() const { return m_cfgServiceIP1; }
	HostAddressPort cfgServiceIP2() const { return m_cfgServiceIP2; }

	bool clearSettings();								// clear all service settings

	CircularLoggerShared logger() const { return m_logger; }

	QString getSettingValue(const QString& settingName);
	bool getBoolSettingValue(const QString& settingName);

	QStringList getSoftwareInfo() const;

	SoftwareSettingsSet& softwareSettingsSet() { return m_softwareSettingsSet; }
	const SoftwareSettingsSet& softwareSettingsSet() const { return m_softwareSettingsSet; }

	void setSessionParams(const SessionParams& sp);
	SessionParams sessionParams() const;

	void setServiceRunMode(E::ServiceRunMode srm);
	E::ServiceRunMode serviceRunMode() const;

	bool addSimpleNoWritableCmdLineArg(	const QString& cmdLineArgName,
										const QString& description);

	bool addSimpleCmdLineArg(const QString& cmdLineArgName,
							 const QString& settingName,
							 const QString& description);

	bool addValueNoWritebleCmdLineArg(const QString& cmdLineArgName,
									  const QString& description,
									  const QString& paramExample);

	bool addValueCmdLineArg(const QString& cmdLineArgName,
							const QString& settingName,
							const QString& description,
							const QString& paramExample);

	bool addBoolCmdLineArg(const QString& cmdLineArgName,
							const QString& settingName,
							const QString& description);

	bool cmdLineArgIsSet(const QString& cmdLineArgName) const;
	QString helpText() const;

	int thisInstanceNo() const { return m_thisInstanceNo; }

signals:
	void work();
	void stopped();

protected:
	virtual void initialize() = 0;						// calls on ServiceWorker's thread start
	virtual void shutdown() = 0;						// calls on ServiceWorker's thread shutdown

	virtual void initServiceSpecificCmdLineArgs() = 0;	// override to add service-specific options to m_cmdLineParser

	virtual bool processServiceSpecificCmdLineArgs();	// override to process service-specific cmd line settings
														// return true - to continue service running
														// return false - to exit service
	virtual void loadServiceSpecificSettings()
	{
		DEBUG_LOG_MSG(m_logger, QString("ServiceWorker::loadServiceSpecificSettings() override to load service-specific settings"));
	}

	const CommandLineParser& commandLineParser() const { return m_cmdLineParser; }

private:
	void setThisInstanceNo();

	void copyCmdLineArgs(int argc, const char** argv);
	const QStringList cmdLineArgs() const { return m_cmdLineArgs; }

	bool initInstance1();					// called by ServiceStarter only for instance 1 of ServiceWorker derived class

	void onThreadStarted() override final;
	void onThreadFinished() override final;

	void loadCommonServicesSettings();

	friend class ServiceStarter;

private:
	QString m_equipmentID;

	QString m_cfgServiceIP1Str;
	HostAddressPort m_cfgServiceIP1;

	QString m_cfgServiceIP2Str;
	HostAddressPort m_cfgServiceIP2;

	SoftwareInfo m_softwareInfo;

	SoftwareSettingsSet m_softwareSettingsSet;

	QString m_serviceName;

	CircularLoggerShared m_logger;

	int m_argc = 0;
	const char** m_argv = nullptr;

	QStringList m_cmdLineArgs;

	E::ServiceRunMode m_serviceRunMode = E::ServiceRunMode::ConsoleApp;

	CommandLineParser m_cmdLineParser;

	Service* m_service = nullptr;

	mutable QRecursiveMutex m_spMutex;
	SessionParams m_sessionParams;

	int m_thisInstanceNo = 0;

	static int m_instanceNo;
};

// -------------------------------------------------------------------------------------
//
// Service class declaration
//
// -------------------------------------------------------------------------------------

class Service : public QObject
{
	Q_OBJECT

public:
	Service(ServiceWorker& serviceWorker, std::shared_ptr<CircularLogger> logger);
	virtual ~Service();

	void start();
	void stop();

	static QString getServiceInstanceName(const QString& serviceName, const QString& instanceID);
	static QString getServiceInstanceName(const QString& serviceName, int argc, char* argv[]);

signals:
	void ackBaseRequest(UdpRequest request);

private slots:
	void onServiceWork();
	void onServiceStopped();

	void onBaseRequest(UdpRequest request);

private:
	void startServiceWorkerThread();
	void stopServiceWorkerThread();

	void startBaseRequestSocketThread();
	void stopBaseRequestSocketThread();

	void getServiceInfo(Network::ServiceInfo& servicesInfo);

private:
	QMutex m_mutex;
	std::shared_ptr<CircularLogger> m_logger;

	quint32 m_crc = 0;

	qint64 m_serviceStartTime = 0;
	qint64 m_serviceWorkerStartTime = 0;

	ServiceState m_state = ServiceState::Stopped;

	ServiceWorker& m_serviceWorkerFactory;
	ServiceWorker* m_serviceWorker = nullptr;

	SimpleThread* m_serviceWorkerThread = nullptr;
	UdpSocketThread* m_baseRequestSocketThread = nullptr;

	bool m_mainFunctionNeedRestart = false;
	bool m_mainFunctionStopped = false;

	QTimer m_timer500ms;
};


