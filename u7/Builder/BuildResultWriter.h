#pragma once

#include <QObject>
#include <QFile>
#include <QXmlStreamWriter>
#include <QVector>
#include <QHash>

#include "../lib/OutputLog.h"
#include "../lib/OrderedHash.h"
#include "../lib/DeviceObject.h"
#include "../lib/BuildInfo.h"
#include "../Builder/ModuleFirmwareWriter.h"


class DbController;

namespace Builder
{
	class IssueLogger;
	class BuildResultWriter;


	class BuildFile : public QObject
	{
		Q_OBJECT

	private:
		QString m_fileName;			// filename only, like "filename.xml"

		BuildFileInfo m_info;

		QFile m_file;

		void getFileInfo();

		static QString removeHeadTailSeparator(const QString& str);

	protected:
		BuildFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag);

		bool open(const QString& fullBuildPath, bool textMode, IssueLogger* log);

		bool write(const QString& fullBuildPath, const QByteArray& data, IssueLogger* log);
		bool write(const QString& fullBuildPath, const QString& dataString, IssueLogger* log);
		bool write(const QString& fullBuildPath, const QStringList& stringList, IssueLogger* log);

		QString fileName() const { return m_fileName; }
		QString pathFileName() const { return m_info.pathFileName; }

		qint64 size() const { return m_info.size; }
		QString md5() const { return m_info.md5; }

		QFile& file() { return m_file; }

		BuildFileInfo getBuildFileInfo() const { return m_info; }

		static QString constructPathFileName(const QString& subDir, const QString& fileName);

		friend class BuildResultWriter;
		friend class ConfigurationXmlFile;

	public:
		void addMetadata(const QString& name, const QString& value);
		void addMetadata(QList<StringPair>& nameValueList);
	};


	class ConfigurationXmlFile : public QObject
	{
		Q_OBJECT

	private:
		BuildResultWriter& m_buildResultWriter;
		QByteArray m_fileData;
		QXmlStreamWriter m_xmlWriter;
		OutputLog* m_log = nullptr;
		QString m_subDir;

		QList<BuildFile*> m_linkedFiles;

	public:
		ConfigurationXmlFile(BuildResultWriter& buildResultWriter, const QString& subDir);

		QXmlStreamWriter& xmlWriter() { return m_xmlWriter; }

		bool addLinkToFile(BuildFile* buildFile);
		bool addLinkToFile(const QString& subDir, const QString& fileName);

		void finalize();

		const QByteArray& getFileData() { return m_fileData; }
		QString subDir() const { return m_subDir; }
	};


	class MultichannelFile
	{
	private:
		BuildResultWriter& m_buildResultWriter;
		IssueLogger* m_log = nullptr;
		QString m_subsysStrID;
		int m_subsysID = 0;
		QString m_lmEquipmentID;
		QString m_lmCaption;

		Hardware::ModuleFirmwareWriter m_moduleFirmware;

	public:
		MultichannelFile(BuildResultWriter& buildResultWriter, QString subsysStrID, int subsysID, QString lmEquipmentID, QString lmCaption, int frameSize, int frameCount, const QStringList &descriptionFields);

		bool setChannelData(int channel, int frameSize, int frameCount, const QByteArray& appLogicBinCode, const std::vector<QVariantList> &descriptionData);

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

		QFile m_buildXmlFile;
		QXmlStreamWriter m_xmlWriter;

		BuildInfo m_buildInfo;

		IssueLogger* m_log = nullptr;
		DbController* m_dbController = nullptr;

		HashedVector<QString, BuildFile*> m_buildFiles;

		HashedVector<QString, ConfigurationXmlFile*> m_cfgFiles;

		HashedVector<QString, MultichannelFile*> m_multichannelFiles;

		QMap<QString, QString> m_buildFileIDMap;

		bool m_runBuild = true;

	private:
		BuildFile* createBuildFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag);

		bool createFile(const QString &pathFileName, QFile& file, bool textMode);

		bool createBuildDirectory();
		bool clearDirectory(QString directory);

		bool createBuildXml();
		bool writeBuildXmlFilesSection();
		bool closeBuildXml();

	public:
		BuildResultWriter(QObject *parent = 0);
		~BuildResultWriter();

		bool start(DbController *db, IssueLogger* log, bool release, int changesetID);
		bool finish();

		BuildFile* addFile(const QString& subDir, const QString& fileName, const QByteArray& data);
		BuildFile* addFile(const QString& subDir, const QString& fileName, const QString& dataString);
		BuildFile* addFile(const QString& subDir, const QString& fileName, const QStringList& stringList);

		BuildFile* addFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, const QByteArray& data);
		BuildFile* addFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, const QString& dataString);
		BuildFile* addFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, const QStringList& stringList);

		ConfigurationXmlFile* createConfigurationXmlFile(const QString& subDir);

		MultichannelFile* createMutichannelFile(QString subsysStrID, int subsysID, QString lmEquipmentID, QString lmCaption, int frameSize, int frameCount, const QStringList &descriptionFields);
		bool writeMultichannelFiles();

		bool writeConfigurationXmlFiles();

/*		QString project() const { return m_buildInfo.project; }
		QString user() const { return m_buildInfo.user; }
		int changeset() const { return m_buildInfo.changeset; }
		int id() const { return m_buildInfo.id; }
		QString buildType() const { return m_buildInfo.release ? "release" : "debug"; }
		QString workstation() const { return m_buildInfo.workstation; }
		QDateTime buildDateTime() const { return m_buildInfo.dateTime; }*/

		BuildInfo buildInfo() const { return m_buildInfo; }

		IssueLogger* log() { return m_log; }

		BuildFile* getBuildFile(const QString& pathFileName) const;
		bool checkBuildFilePtr(const BuildFile* buildFile) const;

		bool isDebug() const;
		bool isRelease() const;
	};
}


