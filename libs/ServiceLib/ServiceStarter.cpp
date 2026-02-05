#ifndef SERVICE_LIB_DOMAIN
	#error Do not include this file in the project! Link ServiceLib instead.
#endif

#include <ServiceLib/Service.h>
#include <ServiceLib/ServiceStarter.h>
#include <QMetaObject>
#include <QCoreApplication>

#include "../UtilsLib/WUtils.h"
#include "WaitSignalOrKbInputThread.h"

#if defined(Q_OS_LINUX)
	#include <QTimer>
	#include <thread>
	#include <csignal>
	#include <signal.h>
#endif

#if defined(Q_OS_WIN)
	#include <windows.h>
	#include <conio.h>
#endif

// -------------------------------------------------------------------------------------
//
// DaemonServiceStarter class implementation
//
// -------------------------------------------------------------------------------------

DaemonServiceStarter::DaemonServiceStarter(QCoreApplication& app, ServiceWorker& serviceWorker, std::shared_ptr<CircularLogger> logger) :
	QtService(serviceWorker.argc(), serviceWorker.argv(), &app, serviceWorker.serviceName(), logger),
	m_app(app),
	m_serviceWorker(serviceWorker),
	m_logger(logger)
{
}

DaemonServiceStarter::~DaemonServiceStarter()
{
	stopAndDeleteService();
}

int DaemonServiceStarter::exec()
{
	setServiceFlags(QtServiceBase::ServiceFlag::NeedsStopOnShutdown);

	int result = QtService::exec();

	return result;
}

void DaemonServiceStarter::start()
{
	m_service = new Service(m_serviceWorker, m_logger);
	m_service->start();
}

void DaemonServiceStarter::stop()
{
	stopAndDeleteService();
}

void DaemonServiceStarter::stopAndDeleteService()
{
	if (m_service == nullptr)
	{
		return;
	}

	m_service->stop();

	delete m_service;
	m_service = nullptr;
}

// -------------------------------------------------------------------------------------
//
// ServiceStarter class implementation
//
// -------------------------------------------------------------------------------------

ServiceStarter::ServiceStarter(QCoreApplication& app, ServiceWorker& serviceWorker, std::shared_ptr<CircularLogger> logger) :
	m_app(app),
	m_serviceWorker(serviceWorker),
	m_logger(logger)
{
	app.setOrganizationName(Manufacturer::RADIY);
	app.setApplicationName(serviceWorker.serviceName());
}

int ServiceStarter::exec()
{
	LOG_MSG(m_logger, QString("Run: %1").arg(m_serviceWorker.cmdLine()));

	int result = privateRun();

	LOG_MSG(m_logger, QString("Exit: %1, result = %2").arg(m_serviceWorker.appPath()).arg(result));

	QThread::msleep(500); // not delete! wait while logger flush buffers

	return result;
}

int ServiceStarter::privateRun()
{
	QStringList swInfo = m_serviceWorker.getSoftwareInfo();

	DEBUG_LOG_MSG(m_logger, QString());
	DEBUG_LOG_MSG(m_logger, Separator::LINE);
	DEBUG_LOG_MSG(m_logger, QString());
	for (const QString& str : swInfo)
	{
		DEBUG_LOG_MSG(m_logger, str);
	}
	DEBUG_LOG_MSG(m_logger, QString());
	DEBUG_LOG_MSG(m_logger, Separator::LINE);
	DEBUG_LOG_MSG(m_logger, QString());

	// 1. init CommanLineParser
	// 2. process cmd line args
	// 3. update and store service settings
	//
	bool continueRun = m_serviceWorker.initInstance1();

	if (continueRun == false)
	{
		return 0;
	}

	bool startAsRegularApp = false;

	continueRun = processCommonCmdLineArgs(startAsRegularApp);

	if (continueRun == false)
	{
		return 0;
	}

	continueRun = m_serviceWorker.processServiceSpecificCmdLineArgs();

	if (continueRun == false)
	{
		return 0;
	}

	int result = 0;

	if (startAsRegularApp == true)
	{
		m_serviceWorker.setServiceRunMode(E::ServiceRunMode::ConsoleApp);

		if (m_serviceWorker.getSettingValue(SoftwareSetting::EQUIPMENT_ID).isEmpty() == true)
		{
			DEBUG_LOG_MSG(m_logger, "");
			DEBUG_LOG_ERR(m_logger, QString(tr("EquipmentID of service is NOT SET !!!")));
			return 7;
		}

		result = runAsRegularApplication();
	}
	else
	{
		m_serviceWorker.setServiceRunMode(E::ServiceRunMode::Service);

		DaemonServiceStarter daemonStarter(m_app, m_serviceWorker, m_logger);

		result = daemonStarter.exec();
	}

	return result;
}

// return 'true' for continue service running
// return 'false' for exit
//
bool ServiceStarter::processCommonCmdLineArgs(bool& startAsRegularApp)
{
	startAsRegularApp = false;

	// print Help and exit if "-h" is set
	//
	if (m_serviceWorker.cmdLineArgIsSet(CmdLineArg::HELP) == true)
	{
		QString helpText = m_serviceWorker.helpText();

		helpText += QString(tr("Run program without options to start service.\n"));

		std::cout << C_STR(helpText);

		LOG_MSG(m_logger, QString(tr("Help printed.")))

		return false;
	}

	// print Version and exit if "-v" is set
	//
	if (m_serviceWorker.cmdLineArgIsSet(CmdLineArg::VERSION) == true)
	{
		// service version info already printed
		return false;
	}

	// clear settings and exit if "-clr" is set
	//
	if (m_serviceWorker.cmdLineArgIsSet(CmdLineArg::CLEAR) == true)
	{
		bool res = m_serviceWorker.clearSettings();

		if (res == true)
		{
			DEBUG_LOG_MSG(m_logger, QString(tr("\nService settings has been cleaned.\n\n")));
		}
		else
		{
			DEBUG_LOG_ERR(m_logger, QString(tr("\nService settings cleaning error. Administrative rights required.\n\n")));
		}

		return false;
	}

	// run service as a regular application if "-e" is set
	//
	if (m_serviceWorker.cmdLineArgIsSet(CmdLineArg::EXEC_AS_APP) == true)
	{
		startAsRegularApp = true;
	}

	return true;
}

#if defined(Q_OS_LINUX)
	static volatile std::sig_atomic_t exitByPosixSignal = 0;

	extern "C" void PosixSignalHandler([[maybe_unused]] int signum) noexcept
	{
		exitByPosixSignal = 1;
	}
#endif

#if defined(Q_OS_WIN)
	static BOOL WINAPI ConsoleCtrlHandler(DWORD type)
	{
		switch (type)
		{
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_SHUTDOWN_EVENT:
		{
			if (QCoreApplication::instance() != nullptr)
			{
				QMetaObject::invokeMethod(
					QCoreApplication::instance(),
					"quit",
					Qt::QueuedConnection);
			}

			return TRUE; // мы обработали
		}

		default:
		{
			return FALSE; // пусть система разбирается
		}
		}
	}
#endif

int ServiceStarter::runAsRegularApplication()
{
#if defined(Q_OS_LINUX)
	struct sigaction sa{};

	sa.sa_handler = &PosixSignalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	(void)::sigaction(SIGTERM, &sa, nullptr);
	(void)::sigaction(SIGINT, &sa, nullptr);
#endif

#if defined(Q_OS_WIN)

	AttachConsole(ATTACH_PARENT_PROCESS);

	if (GetConsoleWindow() != nullptr)
	{
		SetConsoleCtrlHandler(&ConsoleCtrlHandler, TRUE);
	}

#endif

	KeyReaderThread keyReaderThread;

	keyReaderThread.start();

	// run service
	//
	Service service(m_serviceWorker, m_logger);

	service.start();

	int result = m_app.exec();

	keyReaderThread.stop();

	service.stop();

	return result;
}

ServiceStarter::KeyReaderThread::KeyReaderThread()
{
}

void ServiceStarter::KeyReaderThread::run()
{
#if defined(Q_OS_LINUX)
	while (exitByPosixSignal == 0 &&
		   !m_stop.load(std::memory_order_acquire))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
#else
	while (!m_stop.load(std::memory_order_acquire))
	{
		if (_kbhit())
		{
			(void)_getch(); // любую клавишу
			break;
		}

		QThread::msleep(200);
	}
#endif
	qDebug() << "KeyReaderThread: exit signal received.";

	QMetaObject::invokeMethod(
		QCoreApplication::instance(),
		"quit",
		Qt::QueuedConnection);
}

void ServiceStarter::KeyReaderThread::stop()
{
	m_stop.store(true, std::memory_order_release);
	wait();
}
