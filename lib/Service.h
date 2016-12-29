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


class Service;
class ServiceWorker;



// -------------------------------------------------------------------------------------
//
// ServiceStarter class declaration
//
// -------------------------------------------------------------------------------------

class ServiceStarter
{
public:
	ServiceStarter(int argc, char ** argv, const QString& name, ServiceWorker* m_serviceWorker);

	int exec();

private:
	void initCmdLineParser();

private:
	int m_argc = 0;
	char **m_argv = nullptr;
	QString m_name;
	ServiceWorker* m_serviceWorker = nullptr;

	QStringList m_cmdLineArgs;
	CommandLineParser m_cmdLineParser;
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
	DaemonServiceStarter(int argc, char ** argv, const QString& name, ServiceWorker* serviceWorker);
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
	ConsoleServiceStarter(int argc, char ** argv, const QString& name, ServiceWorker* serviceWorker);

	int exec(QCommandLineParser& cmdLineParser);
};


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
// Service class declaration
//
// -------------------------------------------------------------------------------------

class Service : public QObject
{
	Q_OBJECT

public:

private:
	QMutex m_mutex;

	ServiceType m_type = ServiceType::BaseService;

	quint32 m_majorVersion = 0;
	quint32 m_minorVersion = 0;
	quint32 m_buildNo = 0;
	quint32 m_crc = 0;

	HostAddressPort m_cfgServiceAddressPort;
	QString m_serviceStrID;

	qint64 m_startTime = 0;
	qint64 m_serviceStartTime = 0;

	ServiceState m_state = ServiceState::Stopped;

	ServiceWorker* m_serviceWorker = nullptr;
	SimpleThread* m_serviceThread = nullptr;

	UdpSocketThread* m_baseRequestSocketThread = nullptr;

	bool m_mainFunctionNeedRestart = false;
	bool m_mainFunctionStopped = false;

	QTimer m_timer500ms;

	void checkMainFunctionState();

	void startServiceThread();
	void stopServiceThread();

	void startBaseRequestSocketThread();
	void stopBaseRequestSocketThread();

	void getServiceInfo(Network::ServiceInfo& serviceInfo);

private slots:
	void onTimer500ms();

	void onServiceWork();
	void onServiceStopped();

	void onBaseRequest(UdpRequest request);

signals:
	void ackBaseRequest(UdpRequest request);

public:
	Service(ServiceWorker *serviceWorker);
	virtual ~Service();

	void start();
	void stop();

	HostAddressPort cfgServiceAddressPort() const { return m_cfgServiceAddressPort; }
	HostAddressPort setCfgServiceAddressPort(const HostAddressPort& addressPort) { return m_cfgServiceAddressPort = addressPort; }

	friend class DaemonServiceStarter;
};



// -------------------------------------------------------------------------------------
//
// ServiceWorker class declaration
//
// -------------------------------------------------------------------------------------

class ServiceWorker : public SimpleThreadWorker
{
	Q_OBJECT

public:
	ServiceWorker(ServiceType serviceType);
	virtual ~ServiceWorker();

	virtual void initialize() = 0;					// calls on ServiceWorker's thread start
	virtual void shutdown() = 0;					// calls on ServiceWorker's thread shutdoen

	virtual void initCmdLineParser() = 0;			// add service-specific options to m_cmdLineParser

signals:
	void work();
	void stopped();

protected:
	void baseInitCmdLineParser(CommandLineParser* cmdLineParser);

	CommandLineParser* cmdLineParser();

	QString serviceEquipmentID() const { return m_serviceEquipmentID; }

	ServiceType serviceType() const { return m_serviceType; }

	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) { Q_UNUSED(serviceInfo); }



private:
	ServiceType m_serviceType = ServiceType::BaseService;

	CommandLineParser* m_cmdLineParser = nullptr;

	QString m_serviceEquipmentID;

	Service* m_service = nullptr;

	void onThreadStarted() final;
	void onThreadFinished() final;

	void setService(Service* service) { m_service = service; }
	Service& service() { assert(m_service != nullptr); return *m_service; }

	friend class Service;
	friend class ServiceStarter;
};


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
