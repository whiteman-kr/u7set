#include "BuildResultWriter.h"
#include "../include/DbController.h"
#include <QHostInfo>
#include <QStandardPaths>
#include <QDir>

namespace Builder
{
	BuildResultWriter::BuildResultWriter(QObject *parent) :
		QObject(parent)
	{
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

		if (m_dbController->buildStart(QHostInfo::localHostName(), m_release, m_changesetID, &m_buildNo, nullptr) == false)
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
			m_log->writeWarning(tr("WARNING: The workcopies of the checked out files will be compiled!"), false, false);
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
		/*if (m_runBuild == false)
		{
			return false;

		}*/
		if (m_buildNo == -1)
		{
			return false;
		}

		closeBuildXML();

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
		QString fullPath = m_buildFullPath + "/" + subDir;

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
		QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);

		m_buildDirectory = QString("%1-%2-%3")
				.arg(m_dbController->currentProject().projectName())
				.arg(m_release ? "release" : "debug").arg(m_buildNo);

		m_buildFullPath = appDataPath + "/" + m_buildDirectory;

		if (createDirectory(m_buildFullPath) == false)
		{
			m_runBuild = false;
			return false;
		}
		return true;
	}


	bool BuildResultWriter::createFile(QString subDir, QString fileName, QFile& file, bool textMode)
	{
		/*if (subDir.isEmpty())
		{
			file.setFileName(m_fullBuildPath +  "/") + fileName;
		}
		else
		{
			if (createSubdirectory(subDir) == false)
			{
				return false;
			}
			file.setFileName(m_fullBuildPath + "/" + subDir + "/" + fileName);
		}

		if (file.open(QIODevice::ReadWrite | QIODevice::Text) == false)
		{
			if (subDir.isEmpty())
			{
				msg = tr("Can't create file: ") + fileName;
			}
			else
			{
				msg = tr("Can't create file: ") + subDir + "/" + fileName;
			}
			m_log->writeError(msg, true, true);

			qDebug() << msg;

			m_runBuild = false;
			return false;
		}

		msg = tr("File was created: build.xml");

		m_log->writeMessage(msg, false);

		qDebug() << msg;
*/

		return true;

	}

	bool BuildResultWriter::createBuildXML()
	{
		m_buildXML.setFileName(m_buildFullPath + "/build.xml");

		if (m_buildXML.open(QIODevice::ReadWrite | QIODevice::Text) == false)
		{
			msg = tr("Can't create file: build.xml");
			m_log->writeError(msg, true, true);

			qDebug() << msg;

			m_runBuild = false;
			return false;

		}

		msg = tr("File was created: build.xml");

		m_log->writeMessage(msg, false);

		qDebug() << msg;


		return true;
	}


	bool BuildResultWriter::closeBuildXML()
	{
		return true;
	}
}
