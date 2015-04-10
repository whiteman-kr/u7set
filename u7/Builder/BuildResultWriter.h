#pragma once

#include <QObject>
#include <QFile>
#include <QXmlStreamWriter>

#include "../include/OutputLog.h"


class DbController;

namespace Builder
{
	class BuildResultWriter : public QObject
	{
		Q_OBJECT

	private:
		QString msg;

		QString m_buildDirectory;
		QString m_buildFullPath;
		QString m_separator;

		QFile m_buildXMLFile;
		QXmlStreamWriter m_buildXML;

		bool m_release = false;
		int m_changesetID = 0;
		int m_buildNo = -1;
		QString m_workstation;

		OutputLog* m_log = nullptr;
		DbController* m_dbController = nullptr;

		bool m_runBuild = true;

	private:
		bool createDirectory(QString dir);			// create full path directory
		bool createSubdirectory(QString subDir);		// create subDirectory in build directory

		bool createFile(QString subDir, QString fileName, QFile& file, bool textMode);

		bool createBuildDirectory();
		bool createBuildXML();
		bool closeBuildXML();

	public:
		explicit BuildResultWriter(QObject *parent = 0);

		bool start(DbController *db, OutputLog *log, bool release, int changesetID);
		bool finish();

	signals:

	public slots:

	};
}


