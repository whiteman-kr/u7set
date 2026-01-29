#pragma once

#include "../../qtservice/src/qtservice.h"
#include "Service.h"

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
	void start() override final;			// override QtService::start
	void stop() override final;				// override QtService::stop

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

	bool processCommonCmdLineArgs(bool& startAsRegularApp);

	int runAsRegularApplication();

private:
	QCoreApplication& m_app;
	ServiceWorker& m_serviceWorker;
	std::shared_ptr<CircularLogger> m_logger;
};
