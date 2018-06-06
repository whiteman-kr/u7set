#pragma once

#include <QDateTime>

#include "../lib/CircularLogger.h"
#include "../lib/Types.h"

// ------------------------------------------------------------------------------------
//
// CfgCheckerWorker class declaration
//
// ------------------------------------------------------------------------------------

class CfgCheckerWorker : public SimpleThreadWorker
{
	Q_OBJECT

public:
	CfgCheckerWorker(const QString& workFolder,
					 const QString& autoloadBuildFolder,
					 int checkNewBuildInterval,
					 std::shared_ptr<CircularLogger> logger);

	bool getFileHash(const QString& filePath, QString& hash);
	bool copyPath(const QString& src, const QString& dst);
	bool checkBuild(const QString& buildDirectoryPath);

	int checkNewBuildAttemptQuantity() const { return m_checkNewBuildCounter; }
	E::ConfigCheckerState checkNewBuildStage() const { return m_state; }

signals:
	void buildPathChanged(QString newBuildPath);

public slots:
	bool updateBuildXml();
	void renameWorkToBackup(QString workDirectoryPathToLeave);
	bool renameWorkToBackupCorrupted(QString corruptedWorkDirectoryPath);

protected:
	void onThreadStarted();

private:
	QString m_workFolder;
	QString m_autoloadBuildFolder;
	QDateTime m_lastBuildXmlModifyTime;
	QString m_lastBuildXmlHash;
	int m_checkNewBuildInterval;

	E::ConfigCheckerState m_state = E::ConfigCheckerState::Unknown;
	int m_checkNewBuildCounter = 0;

	std::shared_ptr<CircularLogger> m_logger;
};

