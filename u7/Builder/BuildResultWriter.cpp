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
	//	BuildSubdirectory class implementation
	//

	BuildSubdirectory::BuildSubdirectory(QString name) :
		m_name(name)
	{

	}


	BuildSubdirectory::~BuildSubdirectory()
	{
		for(int i = 0; i < m_file.count(); i++)
		{
			delete m_file[i];
		}

		m_file.clear();
	}


	int BuildSubdirectory::addFile(QString fileName)
	{
		for(int i = 0; i < m_file.count(); i++)
		{
			if (m_file[i]->name() == fileName)
			{
				return -1;			// file already exists
			}
		}

		BuildFile* newFile = new BuildFile(fileName);

		m_file.append(newFile);

		return m_file.count() - 1;
	}


	void BuildSubdirectory::setFileInfo(int fileIndex, const QFile& file, const QByteArray& data)
	{
		if (fileIndex < 0 || fileIndex >= m_file.count())
		{
			assert(false);
			return;
		}

		QFileInfo fi(file);

		qint64 fileSize = fi.size();

		QString md5 = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();

		m_file[fileIndex]->setInfo(fileSize, md5);
	}


	BuildFile* BuildSubdirectory::file(int index)
	{
		if (index < 0  || index >=m_file.count())
		{
			assert(false);
			return nullptr;
		}

		return m_file[index];
	}


	// --------------------------------------------------------------------------------------
	//	BuildResultWriter class implementation
	//

	BuildResultWriter::BuildResultWriter(QObject *parent) :
		QObject(parent),
		m_separator(QDir::separator())
	{
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

			msg = tr("%1: Invalid build params. Build aborted.").arg(__FUNCTION__);

			if (m_log != nullptr)
			{
				log->writeError(msg, true, true);
			}

			qDebug() << msg;

			m_runBuild = false;
			return m_runBuild;
		}

		m_workstation = QHostInfo::localHostName();

		if (m_dbController->buildStart(m_workstation, m_release, m_changesetID, &m_buildNo, nullptr) == false)
		{
			msg = tr("%1: Build start error.").arg(__FUNCTION__);

			log->writeError(msg, true, true);

			qDebug() << msg;

			m_runBuild = false;
			return m_runBuild;
		}

		m_log->resetErrorCount();
		m_log->resetWarningCount();

		m_log->startStrLogging();

		msg = QString(tr("%1 building #%2 was started. User - %3, host - %4, changeset - %5."))
				.arg(m_release ? "RELEASE" : "DEBUG")
				.arg(m_buildNo).arg(m_dbController->currentUser().username())
				.arg(QHostInfo::localHostName()).arg(m_changesetID);

		log->writeMessage(msg, true);

		qDebug() << msg;

		if (m_release == true)
		{
			m_log->writeError(tr("RELEASE BUILD IS UNDER CONSTRACTION!"), true, false);

			m_runBuild = false;
			return m_runBuild;
		}
		else
		{
			m_log->writeWarning(tr("WARNING: The workcopies of the checked out files will be compiled!"), true, false);
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

		closeBuildXML();

		m_log->writeEmptyLine();

		int errors = m_log->errorCount();
		int warnings = m_log->warningCount();
#pragma message("Load correct Build Log Str")
		QString buildLogStr = "Build Log Str";

		msg = QString(tr("%1 building #%2 was finished. Errors - %3. Warnings - %4."))
				.arg(m_release ? "RELEASE" : "DEBUG")
				.arg(m_buildNo).arg(errors).arg(warnings);

		if (errors)
		{
			m_log->writeError(msg, true, false);
		}
		else
		{
			if (warnings)
			{
				m_log->writeWarning(msg, true, false);
			}
			else
			{
				m_log->writeSuccess(msg, true);
			}
		}

		buildLogStr = m_log->finishStrLogging();

		m_dbController->buildFinish(m_buildNo, errors, warnings, buildLogStr,  nullptr);

		qDebug() << msg;

		return true;
	}


	// create full path directory
	//
	bool BuildResultWriter::createDirectory(QString dir)
	{
		if (QDir().exists(dir))
		{
			return true;
		}

		if (QDir().mkpath(dir) == false)
		{
			msg = tr("Can't create directory: ") + dir;
			m_log->writeError(msg, true, true);

			qDebug() << msg;
			return false;
		}

		msg = tr("Directory was created: ") + dir;

		m_log->writeMessage(msg, false);

		qDebug() << msg;

		return true;
	}


	// create subdirectory in build directory
	//
	bool BuildResultWriter::createSubdirectory(QString subDir)
	{
		QString fullPath = m_buildFullPath + m_separator + subDir;

		if (QDir().exists(fullPath))
		{
			return true;
		}

		if (QDir().mkpath(fullPath) == false)
		{
			msg = tr("Can't create subdirectory: ") + subDir;
			m_log->writeError(msg, true, true);

			qDebug() << msg;
			return false;
		}

		msg = tr("Subdirectory was created: ") + subDir;

		m_log->writeMessage(msg, false);

		qDebug() << msg;
		return true;
	}


	bool BuildResultWriter::createBuildDirectory()
	{
		//QStandardPaths::writableLocation(QStandardPaths::DataLocation)

		QString appDataPath = QDir::toNativeSeparators(theSettings.buildOutputPath());

		QString buildNoStr;

		buildNoStr.sprintf("%06d", m_buildNo);

		m_buildDirectory = QString("%1-%2-%3")
				.arg(m_dbController->currentProject().projectName())
				.arg(m_release ? "release" : "debug").arg(buildNoStr);

		m_buildFullPath = appDataPath + m_separator + m_buildDirectory;

		if (createDirectory(m_buildFullPath) == false)
		{
			m_runBuild = false;
			return false;
		}
		return true;
	}


	QString BuildResultWriter::formatFileName(const QString& subDir, const QString& fileName)
	{
		if (subDir.isEmpty())
		{
			return fileName;
		}

		return subDir + m_separator + fileName;
	}


	bool BuildResultWriter::createFile(QString subDir, QString fileName, QFile& file, bool textMode)
	{
		if (subDir.isEmpty())
		{
			file.setFileName(m_buildFullPath +  m_separator + fileName);
		}
		else
		{
			if (createSubdirectory(subDir) == false)
			{
				return false;
			}

			file.setFileName(m_buildFullPath + m_separator + subDir + m_separator + fileName);
		}

		bool res = false;

		if (textMode)
		{
			res = file.open(QIODevice::ReadWrite | QIODevice::Text);
		}
		else
		{
			res = file.open(QIODevice::ReadWrite);
		}

		if (res == false)
		{
			msg = tr("Can't create file: ") + formatFileName(subDir, fileName);;

			m_log->writeError(msg, true, true);

			qDebug() << msg;

			return false;
		}

		msg = tr("File was created: ") + formatFileName(subDir, fileName);;

		m_log->writeMessage(msg, false);

		qDebug() << msg;

		return true;
	}


	BuildSubdirectory* BuildResultWriter::getBuildSubdirectory(QString subDir)
	{
		BuildSubdirectory* buildSubdirectory = nullptr;

		if (m_subdirectory.contains(subDir))
		{
			buildSubdirectory = m_subdirectory.value(subDir);
		}
		else
		{
			buildSubdirectory = new BuildSubdirectory(subDir);
			m_subdirectory.insert(subDir, buildSubdirectory);
		}

		return buildSubdirectory;
	}


	bool BuildResultWriter::addFile(QString subDir, QString fileName, const QByteArray &data)
	{
		assert(!fileName.isEmpty());

		BuildSubdirectory* buildSubdirectory = getBuildSubdirectory(subDir);

		int fileIndex = buildSubdirectory->addFile(fileName);

		if (fileIndex == -1)
		{
			msg = tr("File already exists: ") + formatFileName(subDir, fileName);

			m_log->writeError(msg, true, true);

			return false;
		}

		QFile file;

		if (createFile(subDir, fileName, file, false) == false)
		{
			return false;
		}

		file.write(data);

		file.close();

		buildSubdirectory->setFileInfo(fileIndex, file, data);

		return true;
	}


	bool BuildResultWriter::addFile(QString subDir, QString fileName, const QStringList& stringList)
	{
		assert(!fileName.isEmpty());

		BuildSubdirectory* buildSubdirectory = getBuildSubdirectory(subDir);

		int fileIndex = buildSubdirectory->addFile(fileName);

		if (fileIndex == -1)
		{
			msg = tr("File already exists: ") + formatFileName(subDir, fileName);

			m_log->writeError(msg, true, true);

			return false;
		}

		QFile file;

		if (createFile(subDir, fileName, file, false) == false)
		{
			return false;
		}

		QTextStream textStream(&file);

		for(auto string : stringList)
		{
			textStream << string << "\n";
		}

		textStream.flush();

		file.seek(0);

		QByteArray data = file.read(MAX_FILE_SIZE);

		if (data.count() == MAX_FILE_SIZE)
		{
			assert(false);		// possible need increase MAX_FILE_SIZE
		}

		file.close();

		buildSubdirectory->setFileInfo(fileIndex, file, data);

		return true;
	}


	bool BuildResultWriter::createBuildXML()
	{
		if (!createFile("", "build.xml", m_buildXMLFile, true))
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


	bool BuildResultWriter::writeFilesSection()
	{
		QHashIterator<QString, BuildSubdirectory*> iterator(m_subdirectory);

		while (iterator.hasNext())
		{
			iterator.next();

			m_buildXML.writeStartElement("directory");
			m_buildXML.writeAttribute("name", iterator.key());

			BuildSubdirectory* buildDirectory = iterator.value();

			for(int i = 0; i < buildDirectory->fileCount(); i++)
			{
				BuildFile* file = buildDirectory->file(i);

				if (file == nullptr)
				{
					assert(false);
					continue;
				}

				m_buildXML.writeStartElement("file");

				m_buildXML.writeAttribute("name", file->name());
				m_buildXML.writeAttribute("size", QString("%1").arg(file->size()));
				m_buildXML.writeAttribute("md5", file->md5());

				m_buildXML.writeEndElement();		// file
			}

			m_buildXML.writeEndElement();			// directory
		}

		return true;
	}


	bool BuildResultWriter::closeBuildXML()
	{
		writeFilesSection();

		m_buildXML.writeEndElement();		// build
		m_buildXML.writeEndDocument();

		m_buildXMLFile.close();
		return true;
	}
}
