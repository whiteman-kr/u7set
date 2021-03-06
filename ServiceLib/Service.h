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
#include "../OnlineLib/UdpSocket.h"
#include "../OnlineLib/CircularLogger.h"
#include "../UtilsLib/SimpleThread.h"
#include "CommandLineParser.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../lib/SoftwareSettings.h"
#include "../Proto/network.pb.h"

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
	ServiceInfo();
	ServiceInfo(E::SoftwareType _softwareType, quint16 _port, QString _name, QString _shortName);

	E::SoftwareType softwareType = E::SoftwareType::Unknown;
	quint16 port = 0;
	QString name;
	QString shortName;
};

class ServicesInfo : public HashedVector<E::SoftwareType, ServiceInfo>
{
public:
	ServicesInfo();
};

static ServicesInfo servicesInfo;

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
	ServiceWorker(const SoftwareInfo& softwareInfo,
				  const QString& serviceInstanceName,
				  int& argc,
				  char** argv,
				  CircularLoggerShared logger,
				  E::ServiceRunMode runMode);

	virtual ~ServiceWorker();

	int& argc() const;
	char** argv() const;

	QString appPath() const;
	QString cmdLine() const;

	QString serviceName() const;

	const SoftwareInfo& softwareInfo() const;
	E::SoftwareType softwareType() const;

	void initAndProcessCmdLineSettings();

	void setService(Service* service);
	Service* service();

	CommandLineParser& cmdLineParser();

	virtual ServiceWorker* createInstance() const = 0;	// Must be implemented in derived class as:
														//
														// ServiceWorker* createInstance() const override
														// {
														//		DerivedServiceWorker* newInstance = new DerivedServiceWorker(serviceName(), argc(), argv());
														//		newInstance->init();
														//		return newInstance;
														// }

	virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const = 0;

	QString equipmentID() const { return m_equipmentID; }

	HostAddressPort cfgServiceIP1() const { return m_cfgServiceIP1; }
	HostAddressPort cfgServiceIP2() const { return m_cfgServiceIP2; }

	bool clearSettings();								// clear all service settings

	CircularLoggerShared logger() const { return m_logger; }

	QString getStrSetting(const QString& settingName);
	QString getCmdLineSetting(const QString& settingName);

	QString getSoftwareInfoStr() const;

	SoftwareSettingsSet& softwareSettingsSet() { return m_softwareSettingsSet; }
	const SoftwareSettingsSet& softwareSettingsSet() const { return m_softwareSettingsSet; }

	void setSessionParams(const SessionParams& sp);
	SessionParams sessionParams() const;

	void setServiceRunMode(E::ServiceRunMode srm);
	E::ServiceRunMode serviceRunMode() const;

signals:
	void work();
	void stopped();

protected:
	void init();

	void processCmdLineSettings();					// override to process service-specific cmd line settings

	virtual void initCmdLineParser() = 0;			// override to add service-specific options to m_cmdLineParser
	virtual void loadSettings() = 0;				// override to load service-specific settings
	virtual void initialize() = 0;					// calls on ServiceWorker's thread start
	virtual void shutdown() = 0;					// calls on ServiceWorker's thread shutdown

private:
	void onThreadStarted() final;
	void onThreadFinished() final;

private:
	QString m_equipmentID;

	QString m_cfgServiceIP1Str;
	HostAddressPort m_cfgServiceIP1;

	QString m_cfgServiceIP2Str;
	HostAddressPort m_cfgServiceIP2;

	SoftwareInfo m_softwareInfo;

	SoftwareSettingsSet m_softwareSettingsSet;

	QString m_serviceName;

	int& m_argc;
	char** m_argv = nullptr;
	CircularLoggerShared m_logger;
	E::ServiceRunMode m_serviceRunMode = E::ServiceRunMode::ConsoleApp;

	QSettings m_settings;

	CommandLineParser m_cmdLineParser;

	Service* m_service = nullptr;

	mutable QMutex m_spMutex;
	SessionParams m_sessionParams;

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

	static QString getInstanceID(const QStringList& serviceArgs);
	static QString getInstanceID(int argc, char* argv[]);

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


// -------------------------------------------------------------------------------------
//
// DaemonServiceStarter class declaration
//
// -------------------------------------------------------------------------------------

class DaemonServiceStarter : private QtService
{
public:
	DaemonServiceStarter(QCoreApplication& app, ServiceWorker& serviceWorker, std::shared_ptr<CircularLogger> logger);
	virtual ~DaemonServiceStarter();

	int exec();

private:
	void start() final;				// override QtService::start
	void stop() final;				// override QtService::stop

	void stopAndDeleteService();

private:
	QCoreApplication& m_app;
	ServiceWorker& m_serviceWorker;
	std::shared_ptr<CircularLogger> m_logger;

	Service* m_service = nullptr;
};


// -------------------------------------------------------------------------------------
//
// ServiceStarter class declaration
//
// -------------------------------------------------------------------------------------

class ServiceStarter : public QObject
{
	Q_OBJECT

public:
	ServiceStarter(QCoreApplication& app, ServiceWorker& m_serviceWorker, std::shared_ptr<CircularLogger> logger);

	int exec();

private:
	int privateRun();

	void processCmdLineArguments(bool& pauseAndExit, bool& startAsRegularApp);

	int runAsRegularApplication();

private:
	class KeyReaderThread : public RunOverrideThread
	{
	public:
		KeyReaderThread();
		virtual void run() override;
		void stop();
	};

private:
	QCoreApplication& m_app;
	ServiceWorker& m_serviceWorker;
	std::shared_ptr<CircularLogger> m_logger;
};
