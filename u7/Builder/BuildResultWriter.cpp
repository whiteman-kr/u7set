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

		if (subDir.isEmpty())
		{
			m_pathFileName = m_separator + m_fileName;
		}
		else
		{
			m_pathFileName = m_separator + removeHeadTailSeparator(subDir) + m_separator + m_fileName;
		}
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


	bool BuildFile::open(const QString& fullBuildPath, bool textMode, OutputLog* log)
	{
		QString fullPathFileName = fullBuildPath + m_pathFileName;

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

		LOG_MESSAGE(log, QString(tr("File was created: %1")).arg(m_pathFileName));

		return true;
	}


	void BuildFile::getFileInfo()
	{
		QFileInfo fi(m_file);

		m_size = fi.size();

		QCryptographicHash md5Generator(QCryptographicHash::Md5);

		md5Generator.addData(&m_file);

		m_md5 = QString(md5Generator.result().toHex());
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
			LOG_ERROR(log, QString(tr("Write error of file: ")).arg(m_pathFileName));
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


	QString	BuildResultWriter::projectName() const
	{
		if (m_dbController == nullptr)
		{
			assert(false);
			return QString();
		}

		return m_dbController->currentProject().projectName();
	}


	QString BuildResultWriter::userName() const
	{
		if (m_dbController == nullptr)
		{
			assert(false);
			return QString();
		}

		return m_dbController->currentUser().username();
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

		m_workstation = QHostInfo::localHostName();

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

		if (createBuildXML() == false)
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

		writeBuildXML();

		closeBuildXML();

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


	bool BuildResultWriter::createBuildXML()
	{
		m_buildXMLFile.setFileName(m_buildFullPath + m_separator + "build.xml");

		if (m_buildXMLFile.open(QIODevice::ReadWrite | QIODevice::Text) == false)
		{
			m_runBuild = false;
			return false;
		}

		m_buildXML.setDevice(&m_buildXMLFile);

		m_buildXML.setAutoFormatting(true);

		m_buildXML.writeStartDocument();

		m_buildXML.writeStartElement("build");

		m_buildXML.writeAttribute("id", QString("%1").arg(m_buildNo));
		m_buildXML.writeAttribute("type", m_release ? "release" : "debug");

		QDateTime now = QDateTime::currentDateTime();

		m_buildXML.writeAttribute("date", now.toString("dd.MM.yyyy"));
		m_buildXML.writeAttribute("time", now.toString("hh:mm:ss"));

		m_buildXML.writeAttribute("changeset", QString("%1").arg(m_changesetID));

		m_buildXML.writeAttribute("user", m_dbController->currentUser().username());
		m_buildXML.writeAttribute("workstation", m_workstation);

		return true;
	}


	bool BuildResultWriter::writeBuildXML()
	{
		return writeFilesSection();
	}


	bool BuildResultWriter::writeFilesSection()
	{
		m_buildXML.writeStartElement("files");
		m_buildXML.writeAttribute("count", QString("%1").arg(m_buildFiles.size()));

		for(BuildFile* buildFile : m_buildFiles)
		{
			if (buildFile == nullptr)
			{
				assert(false);
				continue;
			}

			m_buildXML.writeStartElement("file");

			m_buildXML.writeAttribute("name", buildFile->pathFileName());
			m_buildXML.writeAttribute("size", QString("%1").arg(buildFile->size()));
			m_buildXML.writeAttribute("md5", buildFile->md5());

			m_buildXML.writeEndElement();		// file
		}

		return true;
	}


	bool BuildResultWriter::closeBuildXML()
	{
		m_buildXML.writeEndElement();			// build
		m_buildXML.writeEndDocument();

		m_buildXMLFile.close();
		return true;
	}
}
