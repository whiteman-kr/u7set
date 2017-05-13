#pragma once

#include <QDateTime>

#include "../lib/CircularLogger.h"

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

signals:
	void buildPathChanged(QString newBuildPath);

public slots:
	void updateBuildXml();
	void renameWorkToBackup(QString workDirectoryPathToLeave);

protected:
	void onThreadStarted();

private:
	QString m_workFolder;
	QString m_autoloadBuildFolder;
	QDateTime m_lastBuildXmlModifyTime;
	QString m_lastBuildXmlHash;
	int m_checkNewBuildInterval;

	std::shared_ptr<CircularLogger> m_logger;
};

