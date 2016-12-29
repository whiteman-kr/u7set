#pragma once

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <iostream>
#include "../qtservice/src/qtservice.h"
#include "../lib/UdpSocket.h"
#include "../lib/CircularLogger.h"
#include "../lib/SimpleThread.h"
#include "../lib/CommandLineParser.h"
#include "../Proto/network.pb.h"


struct ServiceInfo
{
	ServiceType serviceType;
	quint16 port;
	const char* name;
};


const ServiceInfo serviceInfo[] =
{
	{ ServiceType::BaseService, PORT_BASE_SERVICE, "Base Service" },
	{ ServiceType::ConfigurationService, PORT_CONFIGURATION_SERVICE, "Configuration Service" },
	{ ServiceType::AppDataService, PORT_APP_DATA_SERVICE, "Application Data Service" },
	{ ServiceType::TuningService, PORT_TUNING_SERVICE, "Tuning Service" },
	{ ServiceType::ArchivingService, PORT_ARCHIVING_SERVICE, "Data Archiving Service" },
	{ ServiceType::DiagDataService, PORT_DIAG_DATA_SERVICE, "Diagnostics Data Service" },
};

const int SERVICE_TYPE_COUNT = sizeof(serviceInfo) / sizeof (ServiceInfo);

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
	ServiceWorker(ServiceType serviceType, const QString& serviceName, int& argc, char** argv);
	virtual ~ServiceWorker();

	int& argc() const;
	char** argv() const;

	ServiceType serviceType() const;
	QString serviceName() const;
	QString serviceEquipmentID() const;

	void init();

	void setService(Service* service);
	Service* service();

	const CommandLineParser& cmdLineParser() const;

	virtual ServiceWorker* createInstance() = 0;	// Must be implemented in derived class as:
													//
													// ServiceWorker* createInstance() override
													// {
													//		ServiceWorker* newInstance = new DerivedServiceWorker(argc(), argv());
													//		newInstance->init();
													//		return newInstance;
													// }

	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const = 0;

signals:
	void work();
	void stopped();

protected:
	virtual void initCmdLineParser() = 0;			// add service-specific options to m_cmdLineParser

	virtual void initialize() = 0;					// calls on ServiceWorker's thread start
	virtual void shutdown() = 0;					// calls on ServiceWorker's thread shutdown

private:
	void baseInitCmdLineParser();

	void onThreadStarted() final;
	void onThreadFinished() final;

private:
	ServiceType m_serviceType = ServiceType::BaseService;
	QString m_serviceName;
	int& m_argc;
	char** m_argv = nullptr;

	CommandLineParser m_cmdLineParser;

	QString m_serviceEquipmentID;

	Service* m_service = nullptr;
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
	Service(ServiceWorker *serviceWorker);
	virtual ~Service();

	void start();
	void stop();

//	HostAddressPort cfgServiceAddressPort() const { return m_cfgServiceAddressPort; }
//	HostAddressPort setCfgServiceAddressPort(const HostAddressPort& addressPort) { return m_cfgServiceAddressPort = addressPort; }

signals:
	void ackBaseRequest(UdpRequest request);

private slots:
	void onTimer500ms();

	void onServiceWork();
	void onServiceStopped();

	void onBaseRequest(UdpRequest request);

private:
	void checkMainFunctionState();

	void startServiceThread();
	void stopServiceThread();

	void startBaseRequestSocketThread();
	void stopBaseRequestSocketThread();

	void getServiceInfo(Network::ServiceInfo& serviceInfo);

private:
	QMutex m_mutex;

	quint32 m_majorVersion = 0;
	quint32 m_minorVersion = 0;
	quint32 m_buildNo = 0;
	quint32 m_crc = 0;

	qint64 m_serviceStartTime = 0;
	qint64 m_serviceWorkerStartTime = 0;

	ServiceState m_state = ServiceState::Stopped;

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

class DaemonServiceStarter : private QtService<QCoreApplication>
{
private:
	ServiceWorker* m_serviceWorker = nullptr;
	Service* m_service = nullptr;

	bool m_serviceWorkerDeleted = false;

	void start() final;				// override QtService::start
	void stop() final;				// override QtService::stop

public:
	DaemonServiceStarter(ServiceWorker* serviceWorker);
	virtual ~DaemonServiceStarter();

	int exec() { return QtService<QCoreApplication>::exec(); }
};


// -------------------------------------------------------------------------------------
//
// ConsoleService Starter class declaration
//
// -------------------------------------------------------------------------------------

class ConsoleServiceStarter : private QCoreApplication
{
private:
	QString m_name;

	ServiceWorker* m_serviceWorker = nullptr;
	Service* m_service = nullptr;

public:
	ConsoleServiceStarter(ServiceWorker* serviceWorker);

	int exec(QCommandLineParser& cmdLineParser);
};


// -------------------------------------------------------------------------------------
//
// ConsoleServiceKeyReaderThread Starter class declaration
//
// -------------------------------------------------------------------------------------

class ConsoleServiceKeyReaderThread : public QThread
{
	Q_OBJECT

public:
	virtual void run()
	{
		char c = 0;
		std::cin >> c;
		QCoreApplication::exit(0);
	}
};


// -------------------------------------------------------------------------------------
//
// ServiceStarter class declaration
//
// -------------------------------------------------------------------------------------

class ServiceStarter
{
public:
	ServiceStarter(ServiceWorker* m_serviceWorker);

	int exec();

private:

	ServiceWorker* m_serviceWorker = nullptr;
};






