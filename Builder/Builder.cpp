#include "Builder.h"
#include "BuildWorkerThread.h"

namespace Builder
{
	void init()
	{
		qmlRegisterAnonymousType<Hardware::OptoPort>("OptoPort", 1);
		qmlRegisterAnonymousType<UnitsConverter>("UnitsConverter", 1);

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
						bool expertMode,
						BuildOptions buildOptions)
	{
		qDebug() << "Build started\n" <<
					"\tdatabaseAddress: " << databaseAddress << "\n" <<
					"\tdatabasePort: " << databasePort << "\n" <<
					"\tdatabaseUserName: " << databaseUserName << "\n" <<
					"\tProjectName: " << projectName << "\n" <<
					"\tprojectUserName: " << projectUserName << "\n" <<
		            "\tbuildPath: " << buildPath << "\n";

		if (isRunning() == true)
		{
			return false;
		}

		m_thread->setProjectName(projectName);
		m_thread->setServerIpAddress(databaseAddress);
		m_thread->setServerPort(databasePort);
		m_thread->setServerUsername(databaseUserName);
		m_thread->setServerPassword(databasePassword);
		m_thread->setProjectUserName(projectUserName);
		m_thread->setProjectUserPassword(projectUserPassword);
		m_thread->setBuildOutputPath(buildPath);
		m_thread->setExpertMode(expertMode);
		m_thread->setBuildOptions(buildOptions);

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

		m_thread->requestInterruption();
		bool result = m_thread->wait(180000);		// Wait for a couple minutes.

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

	int Builder::progress() const
	{
		if (isRunning() == false)
		{
			return 0;
		}

		return m_thread->progress();
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


