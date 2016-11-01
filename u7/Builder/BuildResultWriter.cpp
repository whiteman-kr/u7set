#include <QHostInfo>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>

#include "BuildResultWriter.h"
#include "IssueLogger.h"
#include "../u7/Settings.h"
#include "../lib/DbController.h"

namespace Builder
{


	// --------------------------------------------------------------------------------------
	//
	//	BuildFile class implementation
	//
	// --------------------------------------------------------------------------------------

	BuildFile::BuildFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag)
	{
		m_fileName = removeHeadTailSeparator(fileName);

		m_info.pathFileName = constructPathFileName(subDir, fileName);
		m_info.ID = id;
		m_info.tag = tag;
	}


	QString BuildFile::removeHeadTailSeparator(const QString& str)
	{
		QString result = str;

		if (result.isEmpty())
		{
			return result;
		}

		// remove head separator
		//
		if (result.startsWith("/") == true || result.startsWith("\\") == true)
		{
			result = result.remove(0, 1);
		}

		if (result.isEmpty())
		{
			return result;
		}

		// remove tail separator
		//
		if (result.endsWith("/") == true || result.endsWith("\\") == true)
		{
			result.truncate(result.length() - 1);
		}

		return result;
	}


	QString BuildFile::constructPathFileName(const QString& subDir, const QString& fileName)
	{
		QString pathFileName;

		QString fName = removeHeadTailSeparator(fileName);

		if (subDir.isEmpty())
		{
			pathFileName = "/" + fName;
		}
		else
		{
			pathFileName = "/" + removeHeadTailSeparator(subDir) + "/" + fName;
		}

		return pathFileName;
	}


	void BuildFile::addMetadata(const QString& name, const QString& value)
	{
		m_info.metadata.insert(name, value);
	}


	void BuildFile::addMetadata(QList<StringPair>& nameValueList)
	{
		for(const StringPair& p : nameValueList)
		{
			m_info.metadata.insert(p.first, p.second);
		}
	}


	bool BuildFile::open(const BuildResult& buildResult, bool textMode, IssueLogger* log)
	{
		QString fullPathFileName = buildResult.fullPath() + m_info.pathFileName;

		QFileInfo fi(fullPathFileName);

		QString fullPath = fi.path();

		QDir dir(fullPath);

		if (dir.mkpath(fullPath) == false)
		{
			log->errCMN0011(fullPath);
			return false;
		}

		m_file.setFileName(fullPathFileName);

		bool result = false;

		if (textMode)
		{
			result = m_file.open(QIODevice::ReadWrite | QIODevice::Text);
		}
		else
		{
			result = m_file.open(QIODevice::ReadWrite);
		}

		if (result == false)
		{
			log->errCMN0012(fullPathFileName);
			return false;
		}

		if (buildResult.enableMessages())
		{
			LOG_MESSAGE(log, QString(tr("File was created: %1")).arg(m_info.pathFileName));
		}

		return true;
	}


	void BuildFile::getFileInfo()
	{
		QFileInfo fi(m_file);

		m_info.size = fi.size();

		QCryptographicHash md5Generator(QCryptographicHash::Md5);

		m_file.seek(0);

		md5Generator.addData(&m_file);

		m_info.md5 = QString(md5Generator.result().toHex());

	}


	bool BuildFile::write(const BuildResult& buildResult, const QByteArray& data, IssueLogger* log)
	{
		if (open(buildResult, false, log) == false)
		{
			return false;
		}

		bool result = true;

		qint64 written = m_file.write(data);

		if (written == -1)
		{
			log->errCMN0013(m_info.pathFileName);
			result = false;
		}

		m_file.flush();

		getFileInfo();

		m_file.close();

		return result;
	}


	bool BuildFile::write(const BuildResult& buildResult, const QString& dataString, IssueLogger* log)
	{
		if (open(buildResult, false, log) == false)
		{
			return false;
		}

		QTextStream textStream(&m_file);

		textStream << dataString;

		textStream.flush();

		getFileInfo();

		m_file.close();

		return true;
	}


	bool BuildFile::write(const BuildResult& buildResult, const QStringList& stringList, IssueLogger* log)
	{
		if (open(buildResult, true, log) == false)
		{
			return false;
		}

		QTextStream textStream(&m_file);

		for(auto string : stringList)
		{
			textStream << string << "\n";
		}

		textStream.flush();

		getFileInfo();

		m_file.close();

		return true;
	}


	// --------------------------------------------------------------------------------------
	//
	//	ConfigurationXmlFile class implementation
	//
	// --------------------------------------------------------------------------------------

	ConfigurationXmlFile::ConfigurationXmlFile(BuildResultWriter& buildResultWriter, const QString& subDir) :
		m_buildResultWriter(buildResultWriter),
		m_xmlWriter(&m_fileData),
		m_log(buildResultWriter.log()),
		m_subDir(subDir)
	{
		m_xmlWriter.setAutoFormatting(true);

		m_xmlWriter.writeStartDocument();

		m_xmlWriter.writeStartElement("Configuration");

		BuildInfo bi = buildResultWriter.buildInfo();

		bi.writeToXml(m_xmlWriter);
	}


	bool ConfigurationXmlFile::addLinkToFile(BuildFile* buildFile)
	{
		if (buildFile == nullptr)
		{
			assert(false);
			return false;
		}

		bool result = m_buildResultWriter.checkBuildFilePtr(buildFile);

		if (result == false)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  QString(tr("Build file '%1' is not found")).
					  arg(buildFile->pathFileName()).arg(m_subDir));
			return false;
		}

		m_linkedFiles.append(buildFile);

		return true;
	}


	bool ConfigurationXmlFile::addLinkToFile(const QString& subDir, const QString& fileName)
	{
		QString pathFileName = BuildFile::constructPathFileName(subDir, fileName);

		BuildFile* buildFile = m_buildResultWriter.getBuildFile(pathFileName);

		if (buildFile == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  QString(tr("Build file '%1' is not found")).
					  arg(pathFileName).arg(m_subDir));
			return false;
		}

		m_linkedFiles.append(buildFile);

		return true;
	}

	bool ConfigurationXmlFile::addLinkToFile(const QString& subDir, const QString& fileName, const QString& metadataName, const QString& metadataValue)
	{
		QString pathFileName = BuildFile::constructPathFileName(subDir, fileName);

		BuildFile* buildFile = m_buildResultWriter.getBuildFile(pathFileName);

		if (buildFile == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  QString(tr("Build file '%1' is not found")).
					  arg(pathFileName).arg(m_subDir));
			return false;
		}

		buildFile->addMetadata(metadataName, metadataValue);

		return addLinkToFile(buildFile);

		return true;
	}


	void ConfigurationXmlFile::finalize()
	{
		m_xmlWriter.writeStartElement("Files");

		m_xmlWriter.writeAttribute("Count", QString("%1").arg(m_linkedFiles.count()));

		for(const BuildFile* buildFile : m_linkedFiles)
		{
			if (buildFile == nullptr)
			{
				assert(false);
				continue;
			}

			buildFile->getBuildFileInfo().writeToXml(m_xmlWriter);
		}

		m_xmlWriter.writeEndElement();		// </Files>

		m_xmlWriter.writeEndElement();		// </Configuration>

		m_xmlWriter.writeEndDocument();
	}


	// --------------------------------------------------------------------------------------
	//
	//	MultichannelFile class implementation
	//
	// --------------------------------------------------------------------------------------

	MultichannelFile::MultichannelFile(BuildResultWriter& buildResultWriter, QString subsysStrID, int subsysID, QString lmEquipmentID,
									   QString lmCaption, int frameSize, int frameCount, const QStringList& descriptionFields) :
		m_buildResultWriter(buildResultWriter),
		m_log(buildResultWriter.log()),
		m_subsysStrID(subsysStrID),
		m_subsysID(subsysID),
		m_lmEquipmentID(lmEquipmentID),
		m_lmCaption(lmCaption)
	{
		BuildInfo bi = m_buildResultWriter.buildInfo();

		m_moduleFirmware.init(lmCaption, subsysStrID, subsysID, 0x0101, frameSize, frameCount,
						 bi.project, bi.user, bi.id, bi.typeStr(), bi.changeset, descriptionFields);
	}


	bool MultichannelFile::setChannelData(int channel, int frameSize, int frameCount, quint64 uniqueID, const QByteArray& appLogicBinCode, const std::vector<QVariantList>& descriptionData)
	{
		if (m_moduleFirmware.setChannelData(m_lmEquipmentID, channel, frameSize, frameCount, uniqueID, appLogicBinCode, descriptionData, m_log) == false)
		{
			return false;
		}

		return true;
	}


	bool MultichannelFile::getFileData(QByteArray& fileData)
	{
		QString errorMsg;

		if (m_moduleFirmware.save(fileData, m_log) == false)
		{
			return false;
		}

		return true;
	}


	// --------------------------------------------------------------------------------------
	//
	//	BuildResult class implementation
	//
	// --------------------------------------------------------------------------------------

	BuildResult::BuildResult()
	{

	}


	bool BuildResult::create(const QString& buildDir, const QString& fullPath, const BuildInfo& buildInfo, IssueLogger* log)
	{
		m_directory = buildDir;
		m_fullPath = fullPath;
		m_log = log;

		bool result = true;

		result &= createBuildDirectory();

		result &= createBuildXml(buildInfo);

		return result;
	}


	bool BuildResult::finalize(const HashedVector<QString, BuildFile*>& buildFiles)
	{
		bool result = true;

		result &= writeBuildXmlFilesSection(buildFiles);
		result &= closeBuildXml();

		return result;
	}


	bool BuildResult::createBuildDirectory()
	{
		if (QDir().mkpath(m_fullPath) == false)
		{
			m_log->errCMN0011(m_fullPath);
			return false;
		}

		clearDirectory(m_fullPath);

		LOG_MESSAGE(m_log, QString(tr("Build directory was created: %1")).arg(m_fullPath));

		return true;
	}


	bool BuildResult::clearDirectory(const QString &directory)
	{
		bool result = true;
		QDir dir(directory);

		if (dir.exists(directory))
		{
			QFileInfoList dirContent = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
			foreach(QFileInfo entry, dirContent)
			{
				if (entry.isDir())
				{
					result = clearDirectory(entry.absoluteFilePath());
					result = dir.rmdir(entry.absoluteFilePath());
				}
				else
				{
					result = QFile::remove(entry.absoluteFilePath());
				}

				if (!result)
				{
						return result;
				}
			}
		}
		return result;
	}


	bool BuildResult::createBuildXml(const BuildInfo& buildInfo)
	{
		m_buildXmlFile.setFileName(m_fullPath + "/build.xml");

		if (m_buildXmlFile.open(QIODevice::ReadWrite | QIODevice::Text) == false)
		{
			return false;
		}

		m_xmlWriter.setDevice(&m_buildXmlFile);

		m_xmlWriter.setAutoFormatting(true);

		m_xmlWriter.writeStartDocument();

		m_xmlWriter.writeStartElement("Build");

		buildInfo.writeToXml(m_xmlWriter);

		return true;
	}


	bool BuildResult::writeBuildXmlFilesSection(const HashedVector<QString, BuildFile *>& buildFiles)
	{
		m_xmlWriter.writeStartElement("Files");
		m_xmlWriter.writeAttribute("Count", QString("%1").arg(buildFiles.size()));

		for(BuildFile* buildFile : buildFiles)
		{
			if (buildFile == nullptr)
			{
				assert(false);
				continue;
			}

			buildFile->getBuildFileInfo().writeToXml(m_xmlWriter);
		}

		m_xmlWriter.writeEndElement();			// </Files>

		return true;
	}


	bool BuildResult::closeBuildXml()
	{
		m_xmlWriter.writeStartElement("BuildResult");

		m_xmlWriter.writeAttribute("Errors", QString::number(m_log->errorCount()));
		m_xmlWriter.writeAttribute("Warnings", QString::number(m_log->warningCount()));

		m_xmlWriter.writeEndElement();			// </BuildResult>

		m_xmlWriter.writeEndElement();			// </Build>
		m_xmlWriter.writeEndDocument();

		m_buildXmlFile.close();

		return true;
	}

	// --------------------------------------------------------------------------------------
	//
	//	BuildResultWriter class implementation
	//
	// --------------------------------------------------------------------------------------


	BuildResultWriter::BuildResultWriter(QObject *parent) :
		QObject(parent)
	{
	}


	BuildResultWriter::~BuildResultWriter()
	{
		for(BuildFile* file : m_buildFiles)
		{
			delete file;
		}

		m_buildFiles.clear();
	}


	bool BuildResultWriter::start(DbController* db, IssueLogger* log, bool release, int changesetID)
	{
		m_dbController = db;
		m_log = log;

		m_buildInfo.release = release;
		m_buildInfo.changeset = changesetID;

		if (m_dbController == nullptr || m_log == nullptr)
		{
			assert(m_dbController != nullptr);
			assert(m_log != nullptr);

			if (m_log != nullptr)
			{
				LOG_ERROR_OBSOLETE(log, IssuePrexif::NotDefined, QString(tr("%1: Invalid build params. Build aborted.")).arg(__FUNCTION__));
			}

			return false;
		}

		m_buildInfo.project = m_dbController->currentProject().projectName();
		m_buildInfo.user = m_dbController->currentUser().username();
		m_buildInfo.workstation = QHostInfo::localHostName();
		m_buildInfo.date = QDateTime::currentDateTime();

		if (m_dbController->buildStart(m_buildInfo.workstation, m_buildInfo.release, m_buildInfo.changeset, &m_buildInfo.id, nullptr) == false)
		{
			LOG_ERROR_OBSOLETE(log, IssuePrexif::NotDefined, QString(tr("%1: Build start error.")).arg(__FUNCTION__));
			return false;
		}

		msg = QString(tr("%1 building #%2 was started. User - %3, host - %4, changeset - %5."))
				.arg(m_buildInfo.release ? "RELEASE" : "DEBUG")
				.arg(m_buildInfo.id).arg(m_dbController->currentUser().username())
				.arg(QHostInfo::localHostName()).arg(m_buildInfo.changeset);

		LOG_MESSAGE(log, msg);

		if (m_buildInfo.release == true)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("RELEASE BUILD IS UNDER CONSTRUCTION!")));
			return false;
		}
		else
		{
			// The workcopies of the checked out files will be compiled.
			//
			m_log->wrnPDB2000();
		}

		if (createBuildResults() == false)
		{
			return false;
		}

		return true;
	}


	bool BuildResultWriter::finish()
	{
		if (m_buildInfo.id == -1)
		{
			return false;
		}

		LOG_EMPTY_LINE(m_log)

		int errors = m_log->errorCount();
		int warnings = m_log->warningCount();

		msg = QString(tr("%1 building #%2 was finished. Errors - %3. Warnings - %4."))
				.arg(m_buildInfo.release ? "RELEASE" : "DEBUG")
				.arg(m_buildInfo.id).arg(errors).arg(warnings);

		if (errors || warnings)
		{
			LOG_MESSAGE(m_log, msg);
		}
		else
		{
			LOG_SUCCESS(m_log, msg);
		}

		QString buildLogStr = m_log->finishStrLogging();

		BuildFile* buildLogFile = addFile("", "build.log", buildLogStr);

		if (buildLogFile == nullptr)
		{
			return false;
		}

		bool result = true;

		for(BuildResult& buildResult : m_buildResults)
		{
			result &= buildResult.finalize(m_buildFiles);
		}

		result &= m_dbController->buildFinish(m_buildInfo.id, errors, warnings, buildLogStr, nullptr);

		return result;
	}


	bool BuildResultWriter::createBuildResults()
	{
		QString appDataPath = QDir::fromNativeSeparators(theSettings.buildOutputPath());

		if (appDataPath.endsWith("/") == true)
		{
			appDataPath.truncate(appDataPath.length() - 1);
		}

		bool result = true;

		QString buildNoStr;

		buildNoStr.sprintf("%06d", m_buildInfo.id);

		QString buildDir = QString("%1-%2/build-%3")
				.arg(m_dbController->currentProject().projectName())
				.arg(m_buildInfo.typeStr()).arg(buildNoStr);

		QString buildFullPath = appDataPath + "/" + buildDir;

		result &= m_buildResults[0].create(buildDir, buildFullPath, m_buildInfo, m_log);

		// create build in fixed path
		//
		buildDir = QString("%1-%2/build")
					.arg(m_dbController->currentProject().projectName())
					.arg(m_buildInfo.typeStr());

		buildFullPath = appDataPath + "/" + buildDir;

		result &= m_buildResults[1].create(buildDir, buildFullPath, m_buildInfo, m_log);

		m_buildResults[1].setEnableMessages(false);

		return result;
	}


	BuildFile* BuildResultWriter::createBuildFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag)
	{
		assert(fileName.isEmpty() == false);

		BuildFile* buildFile = new BuildFile(subDir, fileName, id, tag);

		QString pathFileName = buildFile->pathFileName();

		if (m_buildFiles.contains(pathFileName))
		{
			// File '%1' already exists.
			//
			m_log->errCMN0014(pathFileName);

			delete buildFile;

			return nullptr;
		}

		m_buildFiles.insert(pathFileName, buildFile);


		if (id.isEmpty() == false)
		{
			if (m_buildFileIDMap.contains(id))
			{
				QString file1 = m_buildFileIDMap[id];

				// '%1' and '%2' files have the same ID = '%3'.
				//
				m_log->wrnCMN0015(file1, pathFileName, id);
			}
			else
			{
				m_buildFileIDMap.insert(id, pathFileName);
			}
		}

		return buildFile;
	}


	BuildFile* BuildResultWriter::addFile(const QString& subDir, const QString& fileName, const QByteArray& data)
	{
		return addFile(subDir, fileName, "", "", data);
	}


	BuildFile* BuildResultWriter::addFile(const QString& subDir, const QString& fileName, const QString& dataString)
	{
		return addFile(subDir, fileName, "", "", dataString);
	}


	BuildFile* BuildResultWriter::addFile(const QString& subDir, const QString& fileName, const QStringList& stringList)
	{
		return addFile(subDir, fileName, "", "", stringList);
	}


	BuildFile* BuildResultWriter::addFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, const QByteArray& data)
	{
		BuildFile* buildFile = createBuildFile(subDir, fileName, id, tag);

		if (buildFile == nullptr)
		{
			return nullptr;
		}

		for(BuildResult& buildResult : m_buildResults)
		{
			if (buildFile->write(buildResult, data, m_log) == false)
			{
				return nullptr;
			}
		}

		return buildFile;
	}


	BuildFile* BuildResultWriter::addFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, const QString& dataString)
	{
		BuildFile* buildFile = createBuildFile(subDir, fileName, id, tag);

		if (buildFile == nullptr)
		{
			return nullptr;
		}

		for(BuildResult& buildResult : m_buildResults)
		{
			if (buildFile->write(buildResult, dataString, m_log) == false)
			{
				return nullptr;
			}
		}

		return buildFile;
	}


	BuildFile* BuildResultWriter::addFile(const QString& subDir, const QString& fileName, const QString& id, const QString& tag, const QStringList& stringList)
	{
		BuildFile* buildFile = createBuildFile(subDir, fileName, id, tag);

		if (buildFile == nullptr)
		{
			return nullptr;
		}

		for(BuildResult& buildResult : m_buildResults)
		{
			if (buildFile->write(buildResult, stringList, m_log) == false)
			{
				return nullptr;
			}
		}

		return buildFile;
	}


	bool BuildResultWriter::writeConfigurationXmlFiles()
	{
		if (m_cfgFiles.isEmpty())
		{
			return true;
		}

		bool result = true;

		LOG_MESSAGE(m_log, QString(tr("Software configuration files writing...")));

		for(ConfigurationXmlFile* cfgFile : m_cfgFiles)
		{
			if (cfgFile == nullptr)
			{
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			cfgFile->finalize();

			BuildFile* buildFile = addFile(cfgFile->subDir(), "configuration.xml", cfgFile->getFileData());

			if (buildFile == nullptr)
			{
				result = false;
			}

			delete cfgFile;		// is no longer needed
		}

		m_cfgFiles.clear();

		return result;
	}


	MultichannelFile* BuildResultWriter::createMutichannelFile(QString subsysStrID, int subsysID, QString lmEquipmentID, QString lmCaption, int frameSize, int frameCount, const QStringList& descriptionFields)
	{
		MultichannelFile* multichannelFile = nullptr;

		if (m_multichannelFiles.contains(subsysStrID) == true)
		{
			multichannelFile = m_multichannelFiles[subsysStrID];

			if (multichannelFile == nullptr)
			{
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				return nullptr;
			}

			if (multichannelFile->subsysID() != subsysID)
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
						  QString(tr("Different subsysID (%1 & %2) for subsysStrID = '%3'")).
						  arg(multichannelFile->subsysID()).arg(subsysID).arg(subsysStrID));

				return nullptr;
			}
		}
		else
		{
			multichannelFile = new MultichannelFile(*this, subsysStrID, subsysID, lmEquipmentID, lmCaption, frameSize, frameCount, descriptionFields);

			m_multichannelFiles.insert(subsysStrID, multichannelFile);
		}

		return multichannelFile;
	}


	bool BuildResultWriter::writeMultichannelFiles()
	{
		bool result = true;

		if (m_multichannelFiles.isEmpty() == false)
		{
			LOG_EMPTY_LINE(m_log);
		}

		for(MultichannelFile* multichannelFile : m_multichannelFiles)
		{
			if (multichannelFile == nullptr)
			{
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			QByteArray fileData;

			if (multichannelFile->getFileData(fileData) == true)
			{
				BuildFile* buildFile = addFile(multichannelFile->subsysStrID(), multichannelFile->lmCaption() + ".alb", fileData);

				if (buildFile == nullptr)
				{
					result = false;
				}
			}
			else
			{
				//assert(false);
				result = false;
			}

			delete multichannelFile;		// is no longer needed
		}

		m_multichannelFiles.clear();

		return result;
	}


	ConfigurationXmlFile* BuildResultWriter::createConfigurationXmlFile(const QString& subDir)
	{
		if (m_cfgFiles.contains(subDir))
		{
			return m_cfgFiles[subDir];
		}

		ConfigurationXmlFile* cfgFile = new ConfigurationXmlFile(*this, subDir);

		m_cfgFiles.insert(subDir, cfgFile);

		return cfgFile;
	}


	BuildFile* BuildResultWriter::getBuildFile(const QString& pathFileName) const
	{
		if (m_buildFiles.contains(pathFileName))
		{
			return m_buildFiles[pathFileName];
		}

		return nullptr;
	}


	bool BuildResultWriter::checkBuildFilePtr(const BuildFile* buildFile) const
	{
		for(const BuildFile* bFile : m_buildFiles)
		{
			if (bFile == buildFile)
			{
				return true;
			}
		}
		return false;
	}

	bool BuildResultWriter::isDebug() const
	{
		return !m_buildInfo.release;
	}

	bool BuildResultWriter::isRelease() const
	{
		return m_buildInfo.release;
	}

}
