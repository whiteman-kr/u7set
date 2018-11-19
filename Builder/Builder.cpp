#include "Stable.h"
#include "Builder.h"
#include "BuildWorkerThread.h"

namespace Builder
{
	void init()
	{
		qmlRegisterType<Hardware::OptoPort>();
		qmlRegisterType<JsVariantList>();

		qRegisterMetaType<RunOrder>("RunOrder");

		return;
	}

	void shutdown()
	{
	}

	Builder::Builder(BuildIssues* buildIssues, QObject* parent) :
		QObject(parent),
		m_log(buildIssues)
	{
		m_thread = new BuildWorkerThread;
		m_thread->setIssueLog(&m_log);

		connect(m_thread, &BuildWorkerThread::started, this, &Builder::started);
		connect(m_thread, &BuildWorkerThread::finished, this, &Builder::threadFinished);

		connect(m_thread, &BuildWorkerThread::runOrderReady, this, &Builder::runOrderReady);

		return;
	}

	Builder::~Builder()
	{
		if (isRunning() == true)
		{
			stop();
		}

		delete m_thread;
		return;
	}

	bool Builder::start(QString databaseAddress,
						int databasePort,
						QString databaseUserName,
						QString databasePassword,
						QString projectName,
						QString projectUserName,
						QString projectUserPassword,
						QString buildPath,
						BuildType buildType,
						bool expertMode)
	{
		qDebug() << "Build started\n" <<
					"\tdatabaseAddress: " << databaseAddress << "\n" <<
					"\tdatabasePort: " << databasePort << "\n" <<
					"\tdatabaseUserName: " << databaseUserName << "\n" <<
					"\tProjectName: " << projectName << "\n" <<
					"\tprojectUserName: " << projectUserName << "\n" <<
					"\tbuildPath: " << buildPath << "\n" <<
					"\tbuildType: " << (buildType == BuildType::Debug ? "Debug" : "Release") << "\n";

		if (isRunning() == true)
		{
			assert(isRunning() == false);
			stop();
		}

		m_thread->setProjectName(projectName);
		m_thread->setServerIpAddress(databaseAddress);
		m_thread->setServerPort(databasePort);
		m_thread->setServerUsername(databaseUserName);
		m_thread->setServerPassword(databasePassword);
		m_thread->setProjectUserName(projectUserName);
		m_thread->setProjectUserPassword(projectUserPassword);
		m_thread->setBuildOutputPath(buildPath);
		m_thread->setDebug(buildType == BuildType::Debug);
		m_thread->setExpertMode(expertMode);

		m_log.clear();

		// Ready? Go!
		//
		m_thread->start();

		return true;
	}

	bool Builder::stop()
	{
		if (isRunning() == false)
		{
			return true;
		}

		qDebug() << "Cancel build";

		m_thread->requestInterruption();
		bool result = m_thread->wait(60000);		// Wait for a minute.

		if (result == false)
		{
			qDebug() << "Building thread is not finished and forced to terminate.";
			m_thread->terminate();

			return false;
		}

		return true;
	}

	bool Builder::isRunning() const
	{
		return m_thread->isRunning();
	}

	IssueLogger& Builder::log()
	{
		return m_log;
	}

	void Builder::threadFinished()
	{
		emit finished(m_log.errorCount());
	}
}

