#pragma once

#include <QObject>
#include <QFile>
#include <QXmlStreamWriter>
#include <QVector>
#include <QHash>

#include "../include/OutputLog.h"
#include "../include/OrderedHash.h"


class DbController;

namespace Builder
{

class BuildFile : public QObject
	{
		Q_OBJECT

	private:
		QString m_fileName;			// filename only, like "filename.xml"
		QString m_pathFileName;		// path and file name from build root directory, like "/subdir/filename.xml"
		qint64 m_size = 0;			// size of file
		QString m_md5;				// MD5 hash of file

		QFile m_file;

		static QString m_separator;

		QString removeHeadTailSeparator(const QString& str);

		void getFileInfo();

	public:
		BuildFile(const QString& subDir, const QString& fileName);

		bool open(const QString& fullBuildPath, bool textMode, OutputLog* log);

		bool write(const QString& fullBuildPath, const QByteArray& data, OutputLog* log);
		bool write(const QString& fullBuildPath, const QString& dataString, OutputLog* log);
		bool write(const QString& fullBuildPath, const QStringList& stringList, OutputLog* log);

		QString fileName() const { return m_fileName; }
		QString pathFileName() const { return m_pathFileName; }

		qint64 size() const { return m_size; }
		QString md5() const { return m_md5; }

		QFile& file() { return m_file; }

		void setInfo(qint64 size, const QString& md5) { m_size = size; m_md5 = md5; }
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

		HashedVector<QString, BuildFile*> m_buildFiles;

		bool m_runBuild = true;

	private:
		BuildFile* createBuildFile(const QString& subDir, const QString& fileName);

		bool createFile(const QString &pathFileName, QFile& file, bool textMode);

		bool createBuildDirectory();

		bool createBuildXML();
		bool writeBuildXML();
		bool closeBuildXML();

		bool writeFilesSection();

	public:
		BuildResultWriter(QObject *parent = 0);
		~BuildResultWriter();

		bool start(DbController *db, OutputLog *log, bool release, int changesetID);
		bool finish();

		bool addFile(const QString& subDir, const QString& fileName, const QByteArray& data);
		bool addFile(const QString& subDir, const QString& fileName, const QString& dataString);
		bool addFile(const QString& subDir, const QString& fileName, const QStringList& stringList);

		QString projectName() const;
		QString userName() const;
		int changesetID() const { return m_changesetID; }
	};
}


