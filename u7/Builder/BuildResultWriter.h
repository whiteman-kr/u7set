#pragma once

#include <QObject>
#include <QFile>
#include <QXmlStreamWriter>
#include <QVector>
#include <QHash>

#include "../include/OutputLog.h"
#include "../include/OrderedHash.h"
#include "../include/DeviceObject.h"


class DbController;

namespace Builder
{

	struct BuildFileInfo
	{
		QString pathFileName;		// path and file name from build root directory, like "/subdir/filename.xml"
		qint64 size = 0;			// size of file
		QString md5;				// MD5 hash of file
	};


	class BuildFile : public QObject
	{
		Q_OBJECT

	private:
		QString m_fileName;			// filename only, like "filename.xml"

		BuildFileInfo m_info;

		QFile m_file;

		static QString m_separator;

		void getFileInfo();

		static QString removeHeadTailSeparator(const QString& str);

	public:
		BuildFile(const QString& subDir, const QString& fileName);

		bool open(const QString& fullBuildPath, bool textMode, OutputLog* log);

		bool write(const QString& fullBuildPath, const QByteArray& data, OutputLog* log);
		bool write(const QString& fullBuildPath, const QString& dataString, OutputLog* log);
		bool write(const QString& fullBuildPath, const QStringList& stringList, OutputLog* log);

		QString fileName() const { return m_fileName; }
		QString pathFileName() const { return m_info.pathFileName; }

		qint64 size() const { return m_info.size; }
		QString md5() const { return m_info.md5; }

		QFile& file() { return m_file; }

		BuildFileInfo getInfo() const { return m_info; }

		static QString constructPathFileName(const QString& subDir, const QString& fileName);
	};


	class BuildResultWriter;


	class ConfigurationXmlFile : public QObject
	{
		Q_OBJECT

	private:
		BuildResultWriter& m_buildResultWriter;
		QByteArray m_fileData;
		QXmlStreamWriter m_xmlWriter;
		OutputLog* m_log = nullptr;
		QString m_subDir;

		HashedVector<QString, BuildFileInfo> m_linkedFilesInfo;

	public:
		ConfigurationXmlFile(BuildResultWriter& buildResultWriter, const QString& subDir);

		QXmlStreamWriter& xmlWriter() { return m_xmlWriter; }
		bool addLinkToFile(const QString& subDir, const QString& fileName);
		void finalize();
		const QByteArray& getFileData() { return m_fileData; }
		QString subDir() const { return m_subDir; }
	};


	class MultichannelFile
	{
	private:
		BuildResultWriter& m_buildResultWriter;
		OutputLog* m_log = nullptr;
		QString m_subsysStrID;
		int m_subsysID = 0;
		QString m_lmCaption;

		Hardware::ModuleFirmware m_moduleFirmware;

	public:
		MultichannelFile(BuildResultWriter& buildResultWriter, QString subsysStrID, int subsysID, QString lmCaption, int frameSize, int frameCount);

		bool setChannelData(int channel, int frameSize, int frameCount, const QByteArray& appLogicBinCode);

		bool getFileData(QByteArray& fileData);

		QString subsysStrID() const { return m_subsysStrID; }
		QString lmCaption() const { return m_lmCaption; }
		int subsysID() const { return m_subsysID; }
	};


	class BuildResultWriter : public QObject
	{
		Q_OBJECT

	private:
		QString msg;

		QString m_buildDirectory;
		QString m_buildFullPath;
		QString m_separator;

		QFile m_buildXmlFile;
		QXmlStreamWriter m_xmlWriter;

		QString m_projectName;
		QString m_userName;
		QString m_workstation;
		QDateTime m_buildDateTime;

		bool m_release = false;
		int m_changesetID = 0;
		int m_buildNo = -1;

		OutputLog* m_log = nullptr;
		DbController* m_dbController = nullptr;

		HashedVector<QString, BuildFile*> m_buildFiles;

		HashedVector<QString, ConfigurationXmlFile*> m_cfgFiles;

		HashedVector<QString, MultichannelFile*> m_multichannelFiles;

		bool m_runBuild = true;

	private:
		BuildFile* createBuildFile(const QString& subDir, const QString& fileName);

		bool createFile(const QString &pathFileName, QFile& file, bool textMode);

		bool createBuildDirectory();

		bool createBuildXml();
		bool writeBuildXmlFilesSection();
		bool closeBuildXml();

	public:
		BuildResultWriter(QObject *parent = 0);
		~BuildResultWriter();

		bool start(DbController *db, OutputLog *log, bool release, int changesetID);
		bool finish();

		bool addFile(const QString& subDir, const QString& fileName, const QByteArray& data);
		bool addFile(const QString& subDir, const QString& fileName, const QString& dataString);
		bool addFile(const QString& subDir, const QString& fileName, const QStringList& stringList);

		ConfigurationXmlFile* createConfigurationXmlFile(const QString& subDir);

		MultichannelFile* createMutichannelFile(QString subsysStrID, int subsysID, QString lmCaption, int frameSize, int frameCount);
		bool writeMultichannelFiles();

		bool writeConfigurationXmlFiles();

		QString projectName() const { return m_projectName; }
		QString userName() const { return m_userName; }
		int changesetID() const { return m_changesetID; }
		int buildNo() const { return m_buildNo; }
		QString buildType() const { return m_release ? "release" : "debug"; }
		QString workstation() const { return m_workstation; }
		QDateTime buildDateTime() const { return m_buildDateTime; }

		OutputLog* log() { return m_log; }

		BuildFile* getBuildFile(const QString& pathFileName);
	};
}


