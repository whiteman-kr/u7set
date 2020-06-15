#pragma once

#include "IssueLogger.h"
#include "RunOrder.h"

namespace Builder
{
	class BuildWorkerThread;

	enum class BuildType
	{
		Debug,
		Release
	};

	void init();
	void shutdown();

	class Builder : public QObject
	{
		Q_OBJECT

	public:
		Builder(BuildIssues* buildIssues, QObject* parent = nullptr);
		virtual ~Builder();

	public:
		bool start(QString databaseAddress,
				   int databasePort,
				   QString databaseUserName,
				   QString databasePassword,
				   QString projectName,
				   QString projectUserName,
				   QString projectUserPassword,
				   QString buildPath,
				   BuildType buildType,
				   bool expertMode);
		bool stop();
		bool isRunning() const;

		int progress() const;	// return 0 - 100

		IssueLogger& log();

	protected slots:
		void threadFinished();

	signals:
		void started();
		void finished(int errorCount);			// Finished or canceled (if canceled errorCount > 0)

		void progressChanged(int value);		// Always from 0 to 100

		void runOrderReady(RunOrder runOrder);

	private:
		BuildWorkerThread* m_thread = nullptr;
		IssueLogger m_log;
	};
}


