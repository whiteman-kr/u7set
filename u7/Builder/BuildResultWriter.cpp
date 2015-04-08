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

	bool BuildResultWriter::start(DbController *db, OutputLog *log, bool release, int changesetID)
	{
		m_dbController = db;
		m_log = log;
		m_release = release;
		m_changesetID = changesetID;

		if (m_dbController == nullptr || m_log == nullptr)
		{
			assert(m_dbController != nullptr);
			assert(m_log != nullptr);

			msg = tr("Invalid build params. Build aborted.");

			if (m_log != nullptr)
			{
				log->writeError(msg, true);
			}

			qDebug() << msg;

			m_runBuild = false;
			return m_runBuild;
		}

		if (m_dbController->buildStart(QHostInfo::localHostName(), m_release, m_changesetID, &m_buildNo, nullptr) == false)
		{
			msg = tr("Build start error.");

			log->writeError(msg, true);

			qDebug() << msg;

			m_runBuild = false;
			return m_runBuild;
		}

		msg = QString(tr("%1 building #%2 was started. User - %3, host - %4, changeset - %5."))
				.arg(m_release ? "RELEASE" : "DEBUG")
				.arg(m_buildNo).arg(m_dbController->currentUser().username())
				.arg(QHostInfo::localHostName()).arg(m_changesetID);

		log->writeMessage(msg, true);

		qDebug() << msg;

		if (m_release == true)
		{
			m_log->writeError(tr("RELEASE BUILD IS UNDER CONSTRACTION!"), true);

			m_runBuild = false;
			return m_runBuild;
		}
		else
		{
			m_log->writeWarning(tr("WARNING: The workcopies of the checked out files will be compiled!"), true);
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

#pragma message("Load correct Errors, Warnings counters & Build Log")
		int errors = 0;
		int warnings = 0;
		QString buildLogStr = "Build Log Str";

		msg = QString(tr("%1 building #%2 was finished. Errors - %3. Warnings - %4."))
				.arg(m_release ? "RELEASE" : "DEBUG")
				.arg(m_buildNo).arg(errors).arg(warnings);

		if (errors)
		{
			m_log->writeError(msg, true);
		}
		else
		{
			if (warnings)
			{
				m_log->writeWarning(msg, true);
			}
			else
			{
				m_log->writeSuccess(msg, true);
			}
		}

		m_dbController->buildFinish(m_buildNo, errors, warnings, buildLogStr,  nullptr);

		qDebug() << msg;

		return true;
	}


	bool BuildResultWriter::createBuildDirectory()
	{
		QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);

		m_buildDirectory = QString("%1-%2-%3")
				.arg(m_dbController->currentProject().projectName())
				.arg(m_release ? "release" : "debug").arg(m_buildNo);

		m_fullBuildPath = appDataPath + "/" + m_buildDirectory;

		if (QDir().mkdir(m_fullBuildPath) == false)
		{
			msg = tr("Can't create build directory: ") + m_fullBuildPath;
			m_log->writeError(msg, true);

			qDebug() << msg;

			m_runBuild = false;
			return false;
		}

		msg = tr("Build directory was created: ") + m_fullBuildPath;

		m_log->writeMessage(msg);

		qDebug() << msg;

		return true;
	}


	bool BuildResultWriter::createBuildXML()
	{
		m_buildXML.setFileName(m_fullBuildPath + "/build.xml");

		if (m_buildXML.open(QIODevice::ReadWrite | QIODevice::Text) == false)
		{
			msg = tr("Can't create file: build.xml");
			m_log->writeError(msg, true);

			qDebug() << msg;

			m_runBuild = false;
			return false;

		}

		msg = tr("File was created: build.xml");

		m_log->writeMessage(msg);

		qDebug() << msg;


		return true;
	}


	bool BuildResultWriter::closeBuildXML()
	{
		return true;
	}
}
