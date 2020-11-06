#pragma once

#include "../lib/OutputLog.h"
#include "../lib/OrderedHash.h"
#include "../lib/DeviceObject.h"
#include "../lib/BuildInfo.h"
#include "CfgFiles.h"
#include "ModuleFirmwareWriter.h"
#include <QFile>

class DbController;

namespace Builder
{
	class IssueLogger;
	class BuildResult;
	class BuildResultWriter;

	class BuildFile : public QObject
	{
		Q_OBJECT

		friend class BuildResult;
		friend class BuildResultWriter;
		friend class ConfigurationXmlFile;

	protected:

		BuildFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, bool compress);

		bool open(const BuildResult& buildResult, bool textMode, IssueLogger* log);

		bool write(const BuildResult& buildResult, const QByteArray& data, IssueLogger* log);
		bool write(const BuildResult& buildResult, const QString& dataString, IssueLogger* log);
		bool write(const BuildResult& buildResult, const QStringList& stringList, IssueLogger* log);

		QString fileName() const { return m_fileName; }
		QString pathFileName() const { return m_info.pathFileName; }

		qint64 size() const { return m_info.size; }
		QString md5() const { return m_info.md5; }

		QFile& file() { return m_file; }

		BuildFileInfo getBuildFileInfo() const { return m_info; }

		static QString constructPathFileName(const QString& subDir, const QString& fileName);

		void addMetadata(const QString& name, const QString& value);
		void addMetadata(QList<StringPair>& nameValueList);

	private:
		bool getFileInfo(IssueLogger* log);

		static QString removeHeadTailSeparator(const QString& str);

	private:
		QString m_fileName;			// filename only, like "filename.xml"

		BuildFileInfo m_info;

		QFile m_file;
	};

	class ConfigurationXmlFile : public QObject
	{
		Q_OBJECT

	public:
		ConfigurationXmlFile(BuildResultWriter& buildResultWriter, const QString& subDir);

		QXmlStreamWriter& xmlWriter() { return m_xmlWriter; }

		bool addLinkToFile(BuildFile* buildFile);
		bool addLinkToFile(const QString& subDir, const QString& fileName);
		bool addLinkToFile(const QString& subDir, const QString& fileName, const QString& metadataName, const QString& metadataValue);

		void finalize();

		const QByteArray& getFileData() { return m_fileData; }
		QString subDir() const { return m_subDir; }
		QString fileName() const { return m_subDir + "/" + QString(FILE_CONFIGURATION_XML); }

	private:
		BuildResultWriter& m_buildResultWriter;
		QByteArray m_fileData;
		QXmlStreamWriter m_xmlWriter;
		IssueLogger* m_log = nullptr;
		QString m_subDir;

		QList<BuildFile*> m_linkedFiles;
	};

	class BuildResult : public QObject
	{
	public:
		BuildResult();

		bool create(const QString& buildDir, const QString& fullPath, const BuildInfo& buildInfo, IssueLogger* log);
		bool finalize(const HashedVector<QString, BuildFile*>& buildFiles);

		bool enableMessages() const { return m_enableMessages; }
		void setEnableMessages(bool enable) { m_enableMessages = enable; }

		QString fullPath() const { return m_fullPath; }

	private:
		bool createBuildDirectory();
		bool clearDirectory(const QString &directory);

		bool createBuildXml(const BuildInfo& buildInfo);
		bool writeBuildXmlFilesSection(const HashedVector<QString, BuildFile*>& buildFiles);
		bool closeBuildXml();

	private:
		QString m_directory;
		QString m_fullPath;

		QFile m_buildXmlFile;
		QXmlStreamWriter m_xmlWriter;

		IssueLogger* m_log = nullptr;

		bool m_enableMessages = true;
	};

	class BuildResultWriter : public QObject
	{
		Q_OBJECT

	public:
		BuildResultWriter(QObject* parent = nullptr);
		~BuildResultWriter();

		bool start(const QString& outputPath, DbController *db, IssueLogger* log, int changesetID);
		bool finish();

		BuildFile* addFile(const QString& subDir, const QString& fileName, const QByteArray& data, bool compress = false);
		BuildFile* addFile(const QString& subDir, const QString& fileName, const QString& dataString, bool compress = false);
		BuildFile* addFile(const QString& subDir, const QString& fileName, const QStringList& stringList, bool compress = false);

		BuildFile* addFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, const QByteArray& data, bool compress = false);
		BuildFile* addFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, const QString& dataString, bool compress = false);
		BuildFile* addFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, const QStringList& stringList, bool compress = false);

		ConfigurationXmlFile* createConfigurationXmlFile(const QString& subDir);

		bool writeFirmwareStatistics();

		bool writeBinaryFiles();

		bool writeConfigurationXmlFiles();

		BuildInfo buildInfo() const { return m_buildInfo; }

		IssueLogger* log() { return m_log; }

		Hardware::ModuleFirmwareWriter* firmwareWriter() { return &m_firmwareWriter; }

		BuildFile* getBuildFile(const QString& pathFileName) const;
		BuildFile* getBuildFileByID(const QString& subDir /* same as EquipmentID or common dirs */, const QString& buildFileID) const;

		bool checkBuildFilePtr(const BuildFile* buildFile) const;

		QString outputPath() const;
		QStringList fullOutputPathes() const;

	private:
		QString checkOutputPath(QString outputPath);
		bool isWritable(const QString& outputPath);

		BuildFile* createBuildFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, bool compress);

		bool createBuildResults();

	private:
		static const int BUILD_RESULT_COUNT = 2;

		QString m_outputPath;
		QString msg;

		BuildResult m_buildResults[BUILD_RESULT_COUNT];

		BuildInfo m_buildInfo;

		IssueLogger* m_log = nullptr;
		DbController* m_dbController = nullptr;

		HashedVector<QString, BuildFile*> m_buildFiles;

		HashedVector<QString, ConfigurationXmlFile*> m_cfgFiles;

		Hardware::ModuleFirmwareWriter m_firmwareWriter;

		QHash<QString, QHash<QString, QString>> m_buildFileIDMap;		// subDir (same as EquipmentID) => (FileID => FileName)
	};
}


