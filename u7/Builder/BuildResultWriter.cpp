#include <QHostInfo>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>

#include "BuildResultWriter.h"
#include "Settings.h"
#include "../include/DbController.h"

namespace Builder
{

	// --------------------------------------------------------------------------------------
	//
	//	BuildFile class implementation
	//
	// --------------------------------------------------------------------------------------

	QString BuildFile::m_separator = QDir::separator();


	BuildFile::BuildFile(const QString& subDir, const QString& fileName)
	{
		m_fileName = removeHeadTailSeparator(fileName);

		m_info.pathFileName = constructPathFileName(subDir, fileName);
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
		if (result.startsWith(m_separator) == true)
		{
			result = result.remove(0, 1);
		}

		if (result.isEmpty())
		{
			return result;
		}

		// remove tail separator
		//
		if (result.endsWith(m_separator) == true)
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
			pathFileName = BuildFile::m_separator + fName;
		}
		else
		{
			pathFileName = BuildFile::m_separator + removeHeadTailSeparator(subDir) + m_separator + fName;
		}

		return pathFileName;
	}


	bool BuildFile::open(const QString& fullBuildPath, bool textMode, OutputLog* log)
	{
		QString fullPathFileName = fullBuildPath + m_info.pathFileName;

		QFileInfo fi(fullPathFileName);

		QString fullPath = fi.path();

		QDir dir(fullPath);

		if (dir.mkpath(fullPath) == false)
		{
			LOG_ERROR(log, QString(tr("Can't create directory: %1")).arg(fullPath));
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
			LOG_ERROR(log, QString(tr("Can't create file: %1")).arg(fullPathFileName));

			return false;
		}

		LOG_MESSAGE(log, QString(tr("File was created: %1")).arg(m_info.pathFileName));

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


	bool BuildFile::write(const QString& fullBuildPath, const QByteArray& data, OutputLog* log)
	{
		if (open(fullBuildPath, false, log) == false)
		{
			return false;
		}

		bool result = true;

		qint64 written = m_file.write(data);

		if (written == -1)
		{
			LOG_ERROR(log, QString(tr("Write error of file: ")).arg(m_info.pathFileName));
			result = false;
		}

		m_file.flush();

		getFileInfo();

		m_file.close();

		return result;
	}


	bool BuildFile::write(const QString& fullBuildPath, const QString& dataString, OutputLog* log)
	{
		if (open(fullBuildPath, true, log) == false)
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


	bool BuildFile::write(const QString& fullBuildPath, const QStringList& stringList, OutputLog* log)
	{
		if (open(fullBuildPath, true, log) == false)
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

		m_xmlWriter.writeStartElement("build");

		m_xmlWriter.writeAttribute("id", QString("%1").arg(m_buildResultWriter.buildNo()));
		m_xmlWriter.writeAttribute("type", m_buildResultWriter.buildType());

		m_xmlWriter.writeAttribute("date", m_buildResultWriter.buildDateTime().toString("dd.MM.yyyy"));
		m_xmlWriter.writeAttribute("time", m_buildResultWriter.buildDateTime().toString("hh:mm:ss"));

		m_xmlWriter.writeAttribute("changeset", QString("%1").arg(m_buildResultWriter.changesetID()));

		m_xmlWriter.writeAttribute("user", m_buildResultWriter.userName());
		m_xmlWriter.writeAttribute("workstation", m_buildResultWriter.workstation());
	}


	bool ConfigurationXmlFile::addLinkToFile(const QString& subDir, const QString& fileName)
	{
		QString pathFileName = BuildFile::constructPathFileName(subDir, fileName);

		BuildFile* buildFile = m_buildResultWriter.getBuildFile(pathFileName);

		if (buildFile == nullptr)
		{
			LOG_ERROR(m_log, QString(tr("Build file '%1' is not found, can't link to '%2/configuration.xml'")).
					  arg(pathFileName).arg(m_subDir));
			return false;
		}

		m_linkedFilesInfo.insert(pathFileName, buildFile->getInfo());

		return true;
	}


	void ConfigurationXmlFile::finalize()
	{
		m_xmlWriter.writeStartElement("files");

		m_xmlWriter.writeAttribute("count", QString("%1").arg(m_linkedFilesInfo.count()));

		for(const BuildFileInfo& buildFileInfo : m_linkedFilesInfo)
		{
			m_xmlWriter.writeStartElement("file");

			m_xmlWriter.writeAttribute("name", buildFileInfo.pathFileName);
			m_xmlWriter.writeAttribute("size", QString("%1").arg(buildFileInfo.size));
			m_xmlWriter.writeAttribute("md5", buildFileInfo.md5);

			m_xmlWriter.writeEndElement();		// file
		}

		m_xmlWriter.writeEndElement();			// files

		m_xmlWriter.writeEndElement();			// build

		m_xmlWriter.writeEndDocument();
	}


	// --------------------------------------------------------------------------------------
	//
	//	MultichannelFile class implementation
	//
	// --------------------------------------------------------------------------------------

	MultichannelFile::MultichannelFile(BuildResultWriter& buildResultWriter, QString subsysStrID, int subsysID,
									   QString lmCaption, int frameSize, int frameCount) :
		m_buildResultWriter(buildResultWriter),
		m_subsysStrID(subsysStrID),
		m_subsysID(subsysID),
		m_lmCaption(lmCaption)
	{
		m_moduleFirmware.init(lmCaption, subsysStrID, subsysID, 0x0101, frameSize, frameCount,
						 m_buildResultWriter.projectName(), m_buildResultWriter.userName(), m_buildResultWriter.changesetID());
	}


	bool MultichannelFile::setChannelData(int channel, int frameSize, int frameCount, const QByteArray& appLogicBinCode)
	{
		QString errorMsg;

		if (m_moduleFirmware.setChannelData(channel, frameSize, frameCount, appLogicBinCode, &errorMsg) == false)
		{
			LOG_ERROR(m_log, errorMsg);
			return false;
		}

		return true;
	}


	bool MultichannelFile::getFileData(QByteArray& fileData)
	{
		QString errorMsg;

		if (m_moduleFirmware.save(fileData, &errorMsg) == false)
		{
			LOG_ERROR(m_log, errorMsg);
			return false;
		}

		return true;
	}


	// --------------------------------------------------------------------------------------
	//
	//	BuildResultWriter class implementation
	//
	// --------------------------------------------------------------------------------------


	BuildResultWriter::BuildResultWriter(QObject *parent) :
		QObject(parent),
		m_separator(QDir::separator())
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


	bool BuildResultWriter::start(DbController* db, OutputLog* log, bool release, int changesetID)
	{
		m_dbController = db;
		m_log = log;
		m_release = release;
		m_changesetID = changesetID;

		if (m_dbController == nullptr || m_log == nullptr)
		{
			assert(m_dbController != nullptr);
			assert(m_log != nullptr);

			if (m_log != nullptr)
			{
				LOG_ERROR(log, QString(tr("%1: Invalid build params. Build aborted.")).arg(__FUNCTION__));
			}

			m_runBuild = false;
			return m_runBuild;
		}

		m_projectName = m_dbController->currentProject().projectName();
		m_userName = m_dbController->currentUser().username();
		m_workstation = QHostInfo::localHostName();
		m_buildDateTime = QDateTime::currentDateTime();

		if (m_dbController->buildStart(m_workstation, m_release, m_changesetID, &m_buildNo, nullptr) == false)
		{
			LOG_ERROR(log, QString(tr("%1: Build start error.")).arg(__FUNCTION__));
			m_runBuild = false;
			return m_runBuild;
		}

		msg = QString(tr("%1 building #%2 was started. User - %3, host - %4, changeset - %5."))
				.arg(m_release ? "RELEASE" : "DEBUG")
				.arg(m_buildNo).arg(m_dbController->currentUser().username())
				.arg(QHostInfo::localHostName()).arg(m_changesetID);

		LOG_MESSAGE(log, msg);

		if (m_release == true)
		{
			LOG_ERROR(m_log, QString(tr("RELEASE BUILD IS UNDER CONSTRUCTION!")));
			m_runBuild = false;
			return m_runBuild;
		}
		else
		{
			LOG_WARNING(m_log, QString(tr("WARNING: The workcopies of the checked out files will be compiled!")));
		}

		if (createBuildDirectory() == false)
		{
			return false;
		}

		if (createBuildXml() == false)
		{
			return false;
		}

		return true;
	}


	bool BuildResultWriter::finish()
	{
		if (m_buildNo == -1)
		{
			return false;
		}

		LOG_EMPTY_LINE(m_log)

		int errors = m_log->errorCount();
		int warnings = m_log->warningCount();

		msg = QString(tr("%1 building #%2 was finished. Errors - %3. Warnings - %4."))
				.arg(m_release ? "RELEASE" : "DEBUG")
				.arg(m_buildNo).arg(errors).arg(warnings);

		if (errors)
		{
			LOG_ERROR(m_log, msg);
		}
		else
		{
			if (warnings)
			{
				LOG_WARNING(m_log, msg);
			}
			else
			{
				LOG_SUCCESS(m_log, msg);
			}
		}

		QString buildLogStr = m_log->finishStrLogging();

		addFile("", "build.log", buildLogStr);


		writeBuildXmlFilesSection();

		closeBuildXml();

		m_dbController->buildFinish(m_buildNo, errors, warnings, buildLogStr, nullptr);

		return true;
	}


	bool BuildResultWriter::createBuildDirectory()
	{
		QString appDataPath = QDir::toNativeSeparators(theSettings.buildOutputPath());

		if (appDataPath.endsWith(m_separator) == true)
		{
			appDataPath.truncate(appDataPath.length() - 1);
		}

		QString buildNoStr;

		buildNoStr.sprintf("%06d", m_buildNo);

		m_buildDirectory = QString("%1-%2-%3")
				.arg(m_dbController->currentProject().projectName())
				.arg(m_release ? "release" : "debug").arg(buildNoStr);

		m_buildFullPath = appDataPath + m_separator + m_buildDirectory;

		if (QDir().mkpath(m_buildFullPath) == false)
		{
			LOG_ERROR(m_log, QString(tr("Can't create build directory: %1")).arg(m_buildFullPath));
			m_runBuild = false;
			return false;
		}

		LOG_MESSAGE(m_log, QString(tr("Build directory was created: %1")).arg(m_buildFullPath));

		return true;
	}


	BuildFile* BuildResultWriter::createBuildFile(const QString& subDir, const QString& fileName)
	{
		assert(fileName.isEmpty() == false);

		BuildFile* buildFile = new BuildFile(subDir, fileName);

		QString pathFileName = buildFile->pathFileName();

		if (m_buildFiles.contains(pathFileName))
		{
			LOG_ERROR(m_log, QString(tr("File already exists: %1")).arg(pathFileName));

			delete buildFile;

			return nullptr;
		}

		m_buildFiles.insert(pathFileName, buildFile);

		return buildFile;
	}


	bool BuildResultWriter::addFile(const QString& subDir, const QString& fileName, const QByteArray& data)
	{
		BuildFile* buildFile = createBuildFile(subDir, fileName);

		if (buildFile == nullptr)
		{
			return false;
		}

		return buildFile->write(m_buildFullPath, data, m_log);
	}


	bool BuildResultWriter::addFile(const QString& subDir, const QString& fileName, const QString& dataString)
	{
		BuildFile* buildFile = createBuildFile(subDir, fileName);

		if (buildFile == nullptr)
		{
			return false;
		}

		return buildFile->write(m_buildFullPath, dataString, m_log);
	}


	bool BuildResultWriter::addFile(const QString& subDir, const QString& fileName, const QStringList& stringList)
	{
		BuildFile* buildFile = createBuildFile(subDir, fileName);

		if (buildFile == nullptr)
		{
			return false;
		}

		return buildFile->write(m_buildFullPath, stringList, m_log);
	}


	bool BuildResultWriter::createBuildXml()
	{
		m_buildXmlFile.setFileName(m_buildFullPath + m_separator + "build.xml");

		if (m_buildXmlFile.open(QIODevice::ReadWrite | QIODevice::Text) == false)
		{
			m_runBuild = false;
			return false;
		}

		m_xmlWriter.setDevice(&m_buildXmlFile);

		m_xmlWriter.setAutoFormatting(true);

		m_xmlWriter.writeStartDocument();

		m_xmlWriter.writeStartElement("build");

		m_xmlWriter.writeAttribute("id", QString("%1").arg(m_buildNo));
		m_xmlWriter.writeAttribute("type", buildType());

		m_xmlWriter.writeAttribute("date", m_buildDateTime.toString("dd.MM.yyyy"));
		m_xmlWriter.writeAttribute("time", m_buildDateTime.toString("hh:mm:ss"));

		m_xmlWriter.writeAttribute("changeset", QString("%1").arg(m_changesetID));

		m_xmlWriter.writeAttribute("user", m_userName);
		m_xmlWriter.writeAttribute("workstation", m_workstation);

		return true;
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

			result &= addFile(cfgFile->subDir(), "configuration.xml", cfgFile->getFileData());

			delete cfgFile;		// is no longer needed
		}

		m_cfgFiles.clear();

		return result;
	}


	bool BuildResultWriter::writeBuildXmlFilesSection()
	{
		m_xmlWriter.writeStartElement("files");
		m_xmlWriter.writeAttribute("count", QString("%1").arg(m_buildFiles.size()));

		for(BuildFile* buildFile : m_buildFiles)
		{
			if (buildFile == nullptr)
			{
				assert(false);
				continue;
			}

			m_xmlWriter.writeStartElement("file");

			m_xmlWriter.writeAttribute("name", buildFile->pathFileName());
			m_xmlWriter.writeAttribute("size", QString("%1").arg(buildFile->size()));
			m_xmlWriter.writeAttribute("md5", buildFile->md5());

			m_xmlWriter.writeEndElement();		// file
		}

		m_xmlWriter.writeEndElement();			// files

		return true;
	}


	bool BuildResultWriter::closeBuildXml()
	{
		m_xmlWriter.writeEndElement();			// build
		m_xmlWriter.writeEndDocument();

		m_buildXmlFile.close();
		return true;
	}


	MultichannelFile* BuildResultWriter::createMutichannelFile(QString subsysStrID, int subsysID, QString lmCaption, int frameSize, int frameCount)
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
				LOG_ERROR(m_log, QString(tr("Different subsysID (%1 & %2) for subsysStrID = '%3'")).
						  arg(multichannelFile->subsysID()).arg(subsysID).arg(subsysStrID));

				return nullptr;
			}
		}
		else
		{
			multichannelFile = new MultichannelFile(*this, subsysStrID, subsysID, lmCaption, frameSize, frameCount);

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
				result &= addFile(multichannelFile->subsysStrID(), multichannelFile->lmCaption() + ".alb", fileData);
			}
			else
			{
				assert(false);
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


	BuildFile* BuildResultWriter::getBuildFile(const QString& pathFileName)
	{
		if (m_buildFiles.contains(pathFileName))
		{
			return m_buildFiles[pathFileName];
		}

		return nullptr;
	}

}
