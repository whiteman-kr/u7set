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
	class BuildResult;
	class BuildResultWriter;

	class BuildFile : public QObject
	{
		Q_OBJECT

	public:

	private:
		QString m_fileName;			// filename only, like "filename.xml"

		BuildFileInfo m_info;

		QFile m_file;

		bool getFileInfo(IssueLogger* log);

		static QString removeHeadTailSeparator(const QString& str);

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

		friend class BuildResult;
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
		bool addLinkToFile(const QString& subDir, const QString& fileName, const QString& metadataName, const QString& metadataValue);

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
		MultichannelFile(BuildResultWriter& buildResultWriter, QString subsysStrID, int subsysID, QString lmEquipmentID, QString lmCaption, int frameSize, int frameCount, int lmDescriptionNumber, int descriptionFieldsVersion, const QStringList &descriptionFields);

		bool setChannelData(int channel, int frameSize, int frameCount, quint64 uniqueID, const QByteArray& appLogicBinCode, const std::vector<QVariantList> &descriptionData);

		bool getFileData(QByteArray& fileData);

		QString subsysStrID() const { return m_subsysStrID; }
		QString lmCaption() const { return m_lmCaption; }
		QString lmEquipmentID() const { return m_lmEquipmentID; }
		int subsysID() const { return m_subsysID; }

		quint64 getFirmwareUniqueId(int lmNumber);
		void setGenericUniqueId(int lmNumber, quint64 genericUniqueId);
	};


	class BuildResult : public QObject
	{
	private:
		QString m_directory;
		QString m_fullPath;

		QFile m_buildXmlFile;
		QXmlStreamWriter m_xmlWriter;

		IssueLogger* m_log = nullptr;

		bool m_enableMessages = true;

		bool createBuildDirectory();
		bool clearDirectory(const QString &directory);

		bool createBuildXml(const BuildInfo& buildInfo);
		bool writeBuildXmlFilesSection(const HashedVector<QString, BuildFile*>& buildFiles);
		bool closeBuildXml();

	public:
		BuildResult();

		bool create(const QString& buildDir, const QString& fullPath, const BuildInfo& buildInfo, IssueLogger* log);
		bool finalize(const HashedVector<QString, BuildFile*>& buildFiles);

		bool enableMessages() const { return m_enableMessages; }
		void setEnableMessages(bool enable) { m_enableMessages = enable; }

		QString fullPath() const { return m_fullPath; }
	};


	class BuildResultWriter : public QObject
	{
		Q_OBJECT

	private:
		static const int BUILD_RESULT_COUNT = 2;

		QString msg;

		BuildResult m_buildResults[BUILD_RESULT_COUNT];

		BuildInfo m_buildInfo;

		IssueLogger* m_log = nullptr;
		DbController* m_dbController = nullptr;

		HashedVector<QString, BuildFile*> m_buildFiles;

		HashedVector<QString, ConfigurationXmlFile*> m_cfgFiles;

		HashedVector<QString, MultichannelFile*> m_multichannelFiles;

		QMap<QString, QString> m_buildFileIDMap;

	private:
		BuildFile* createBuildFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, bool compress);

		bool createFile(const QString &pathFileName, QFile& file, bool textMode);

		bool createBuildResults();

	public:
		BuildResultWriter(QObject *parent = 0);
		~BuildResultWriter();

		bool start(DbController *db, IssueLogger* log, bool release, int changesetID);
		bool finish();

		BuildFile* addFile(const QString& subDir, const QString& fileName, const QByteArray& data, bool compress = false);
		BuildFile* addFile(const QString& subDir, const QString& fileName, const QString& dataString, bool compress = false);
		BuildFile* addFile(const QString& subDir, const QString& fileName, const QStringList& stringList, bool compress = false);

		BuildFile* addFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, const QByteArray& data, bool compress = false);
		BuildFile* addFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, const QString& dataString, bool compress = false);
		BuildFile* addFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, const QStringList& stringList, bool compress = false);

		ConfigurationXmlFile* createConfigurationXmlFile(const QString& subDir);

		MultichannelFile* createMutichannelFile(QString subsysStrID, int subsysID, QString lmEquipmentID, QString lmCaption, int frameSize, int frameCount, int lmDescriptionNumber, int descriptionFieldsVersion, const QStringList &descriptionFields);
		bool writeMultichannelFiles();

		bool writeConfigurationXmlFiles();

		BuildInfo buildInfo() const { return m_buildInfo; }

		IssueLogger* log() { return m_log; }

		BuildFile* getBuildFile(const QString& pathFileName) const;
		BuildFile* getBuildFileByID(const QString& buildFileID) const;

		bool checkBuildFilePtr(const BuildFile* buildFile) const;

		bool isDebug() const;
		bool isRelease() const;

		quint64 getAppUniqueId(const QString& subsystemID, int lmNumber);

		void setGenericUniqueId(const QString& subsystemID, int lmNumber, quint64 genericUniqueId);

		static const QString COMMON_DIR;
		static const QString REPORTS_DIR;
		static const QString OPTO_VHD_DIR;
	};
}


