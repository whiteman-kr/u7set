#pragma once

#include <QObject>
#include <QFile>
#include <QXmlStreamWriter>
#include <QVector>
#include <QHash>

#include "../include/OutputLog.h"


class DbController;

namespace Builder
{

	const int MAX_FILE_SIZE = 1024 * 1024 * 10;			// 10 MBytes

	class BuildFile
	{
	private:
		QString m_name;
		qint64 m_size = 0;
		QString m_md5;

	public:
		BuildFile(const QString& fileName) : m_name(fileName) {}

		QString name() const { return m_name; }
		qint64 size() const { return m_size; }
		QString md5() const { return m_md5; }

		void setInfo(qint64 size, const QString& md5) { m_size = size; m_md5 = md5; }
	};

	class BuildSubdirectory
	{
	private:
		QString	m_name;
		QVector<BuildFile*> m_file;

	public:
		BuildSubdirectory(QString name);
		~BuildSubdirectory();

		int addFile(QString fileName);

		void setFileInfo(int fileIndex, const QFile& file, const QByteArray& data);

		int fileCount() const { return m_file.count(); }

		BuildFile* file(int index);
	};


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

		QHash<QString, BuildSubdirectory*> m_subdirectory;

		bool m_runBuild = true;

	private:
		bool createDirectory(QString dir);				// create full path directory
		bool createSubdirectory(QString subDir);		// create subDirectory in build directory

		QString formatFileName(const QString& subDir, const QString& fileName);

		bool createFile(QString subDir, QString fileName, QFile& file, bool textMode);

		bool createBuildDirectory();
		bool createBuildXML();
		bool closeBuildXML();

		bool writeFilesSection();

		BuildSubdirectory* getBuildSubdirectory(QString subDir);

	public:
		BuildResultWriter(QObject *parent = 0);
		~BuildResultWriter();

		bool start(DbController *db, OutputLog *log, bool release, int changesetID);
		bool finish();

		bool addFile(QString subDir, QString fileName, const QByteArray& data);
		bool addFile(QString subDir, QString fileName, const QStringList &stringList);

		QString projectName() const;
		QString userName() const;
		int changesetID() const { return m_changesetID; }

	signals:

	public slots:

	};
}


