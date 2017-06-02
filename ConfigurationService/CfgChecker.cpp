#include <QStandardPaths>
#include <QTimer>
#include <QCryptographicHash>
#include <QDir>

#include "CfgChecker.h"
#include "../lib/XmlHelper.h"

// ------------------------------------------------------------------------------------
//
// CfgCheckerWorker class implementation
//
// ------------------------------------------------------------------------------------

QStringList redundantFileExtensions = QStringList() << ".alb" << ".asm" << ".mcb" << ".mct" << ".mem" << ".mif" << ".tub" << ".tun" << ".txt";

CfgCheckerWorker::CfgCheckerWorker(const QString& workFolder,
								   const QString& autoloadBuildFolder,
								   int checkNewBuildInterval,
								   std::shared_ptr<CircularLogger> logger) :
	m_workFolder(workFolder),
	m_autoloadBuildFolder(autoloadBuildFolder),
	m_checkNewBuildInterval(checkNewBuildInterval),
	m_logger(logger)
{
	m_workFolder.replace("\\", "/");
	m_autoloadBuildFolder.replace("\\", "/");
}


bool CfgCheckerWorker::getFileHash(const QString& filePath, QString& hash)
{
	QFile file(filePath);

	bool opened = file.open(QIODevice::ReadOnly);		// open in binary mode

	if (opened == false)
	{
		DEBUG_LOG_MSG(m_logger, "Could not open " + filePath + " for reading");
		return false;
	}

	QCryptographicHash md5Generator(QCryptographicHash::Md5);

	md5Generator.addData(&file);

	hash = md5Generator.result().toHex();

	return true;
}


bool CfgCheckerWorker::copyPath(const QString& src, const QString& dst)
{
	QDir dir(src);

	if (dir.exists() == false)
	{
		return false;
	}

	auto&& dirEntryList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

	for (QString& directoryName : dirEntryList)
	{
		QString dst_path = dst + "/" + directoryName;

		if (dir.mkpath(dst_path) == false)
		{
			return false;
		}

		if (copyPath(src + "/" + directoryName, dst_path) == false)
		{
			return false;
		}
	}

	auto&& fileEntryList = dir.entryList(QDir::Files);

	for (QString fileName : fileEntryList)
	{
		bool isRedundant = false;

		for (QString& redundantFileExtension : redundantFileExtensions)
		{
			if (redundantFileExtension == fileName.right(redundantFileExtension.length()))
			{
				isRedundant = true;
				break;
			}
		}

		if (isRedundant == true)
		{
			continue;
		}

		if (!QFile::copy(src + "/" + fileName, dst + "/" + fileName))
		{
			return false;
		}
	}

	return true;
}

bool CfgCheckerWorker::checkBuild(const QString& buildDirectoryPath)
{
	QString buildXmlPath = buildDirectoryPath + "/build.xml";
	QFile buildXmlFile(buildXmlPath);

	if (!buildXmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		DEBUG_LOG_MSG(m_logger, "Could not open " + buildXmlPath + " has been changed");

		return false;
	}

	QByteArray xmlFileData = buildXmlFile.readAll();
	XmlReadHelper xml(xmlFileData);

	if (xml.findElement("Build") == false)
	{
		return false;
	}

	if (xml.findElement("Files") == false)
	{
		return false;
	}

	int fileCount = 0;

	if (xml.readIntAttribute("Count", &fileCount) == false)
	{
		return false;
	}

	bool result = true;

	for(int count = 0; count < fileCount; count++)
	{
		if(xml.findElement("File") == false)
		{
			return false;
		}

		QString fileName = "";
		int fileSize = 0;
		QString fileMd5Hash = "";

		if (xml.readStringAttribute("Name", &fileName) == false)
		{
			return false;
		}

		if (xml.readIntAttribute("Size", &fileSize) == false)
		{
			return false;
		}

		// Check if file is redundant
		bool isRedundant = false;

		for (QString& redundantFileExtension : redundantFileExtensions)
		{
			if (redundantFileExtension == fileName.right(redundantFileExtension.length()))
			{
				isRedundant = true;
				break;
			}
		}

		if (isRedundant == true)
		{
			continue;
		}

		QFileInfo fileInfo(buildDirectoryPath + fileName);

		if (fileInfo.size() != fileSize)
		{
			DEBUG_LOG_MSG(m_logger, "File " + fileName + " has size " + QString::number(fileInfo.size()) + " expected " + QString::number(fileSize));
			result = false;
		}

		if (xml.readStringAttribute("MD5", &fileMd5Hash) == false)
		{
			return false;
		}

		QString realFileMd5Hash;

		if (getFileHash(buildDirectoryPath + fileName, realFileMd5Hash) == false)
		{
			return false;
		}

		if (realFileMd5Hash != fileMd5Hash)
		{
			DEBUG_LOG_WRN(m_logger, "File " + fileName + " has MD5 hash " + realFileMd5Hash + " expected " + fileMd5Hash);
			result = false;
		}
	}

	if (xml.findElement("BuildResult") == false)
	{
		return false;
	}

	int errors = 0;

	if (xml.readIntAttribute("Errors", &errors) == false)
	{
		return false;
	}

	if (errors != 0)
	{
		DEBUG_LOG_MSG(m_logger, "Build has " + QString::number(errors) + " errors");
		return false;
	}

	if (result == false)
	{
		return false;
	}

	return true;
}


bool CfgCheckerWorker::updateBuildXml()
{
	if (m_workFolder.isEmpty() || m_autoloadBuildFolder.isEmpty())
	{
		return false;
	}

	m_checkNewBuildCounter++;
	m_state = E::ConfigCheckerState::Check;

	// Has build.xml been changed?
	//
	QString buildXmlPath = m_autoloadBuildFolder + "/build.xml";
	QFileInfo buildXmlInfo(buildXmlPath);

	if (buildXmlInfo.lastModified() == m_lastBuildXmlModifyTime)
	{
		m_state = E::ConfigCheckerState::Actual;
		return false;
	}

	QString newBuildXmlHash;

	if (getFileHash(buildXmlPath, newBuildXmlHash) == false)
	{
		m_state = E::ConfigCheckerState::Actual;
		return false;
	}

	if (newBuildXmlHash == m_lastBuildXmlHash)
	{
		m_state = E::ConfigCheckerState::Actual;
		return false;
	}

	DEBUG_LOG_MSG(m_logger, buildXmlPath + " has been changed");

	m_state = E::ConfigCheckerState::Copy;

	m_lastBuildXmlModifyTime = buildXmlInfo.lastModified();
	m_lastBuildXmlHash = newBuildXmlHash;

	// Copying into workDirectory/check-date
	//
	QString workStorage = m_workFolder + "/CfgSrvStorage";
	QDir workDirectory(workStorage);
	QString newCheckDirectoryName = "check-" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss");
	QString newCheckDirectoryPath = workStorage + "/" + newCheckDirectoryName;

	if (workDirectory.mkpath(newCheckDirectoryPath) == false)
	{
		m_state = E::ConfigCheckerState::Actual;
		DEBUG_LOG_ERR(m_logger, "Could not create directory " + newCheckDirectoryPath);
		return false;
	}

	if (copyPath(m_autoloadBuildFolder, newCheckDirectoryPath) == false)
	{
		m_state = E::ConfigCheckerState::Actual;

		DEBUG_LOG_ERR(m_logger, "Could not copy content from " + m_autoloadBuildFolder + " to " + newCheckDirectoryPath);

		QDir newCheckDirectory(newCheckDirectoryPath);

		newCheckDirectory.removeRecursively();

		return false;
	}

	m_state = E::ConfigCheckerState::Verification;

	// Checking copied build folder
	//
	if (checkBuild(newCheckDirectoryPath) == false)
	{
		m_state = E::ConfigCheckerState::Actual;

		DEBUG_LOG_ERR(m_logger, "Build in " + newCheckDirectoryPath + " is not consistent");

		QDir newCheckDirectory(newCheckDirectoryPath);
		newCheckDirectory.removeRecursively();

		return false;
	}

	DEBUG_LOG_MSG(m_logger, "Build in " + newCheckDirectoryPath + " is correct");

	m_state = E::ConfigCheckerState::Switch;

	// Renaming to workDirectory/work-date
	QString date = newCheckDirectoryPath.right(19);	// 19 symbols in date YYYY-DD-MM-hh-mm-ss
	QString newWorkDirectoryPath = workStorage + "/work-" + date;

	if (workDirectory.rename(newCheckDirectoryPath, newWorkDirectoryPath) == false)
	{
		m_state = E::ConfigCheckerState::Actual;

		DEBUG_LOG_MSG(m_logger, "Could not rename " + newCheckDirectoryPath + " to " + newWorkDirectoryPath);

		return false;
	}

	DEBUG_LOG_MSG(m_logger, newCheckDirectoryPath + " renamed to " + newWorkDirectoryPath);

	emit buildPathChanged(newWorkDirectoryPath);

	m_state = E::ConfigCheckerState::Actual;

	return true;
}

void CfgCheckerWorker::renameWorkToBackup(QString workDirectoryPathToLeave)
{
	QString workDirectoryNameToLeave = workDirectoryPathToLeave.right(24);	// 24 symbols in directory name work-YYYY-DD-MM-hh-mm-ss
	QString workStorage = m_workFolder + "/CfgSrvStorage";
	QDir workDirectory(workStorage);

	QStringList&& workBuildDirectoryList = workDirectory.entryList(QStringList() << "work-?\?\?\?-?\?-?\?-?\?-?\?-?\?", QDir::Dirs | QDir::NoSymLinks, QDir::Name);

	for (QString& workBuildDirectory : workBuildDirectoryList)
	{
		if (workBuildDirectory == workDirectoryNameToLeave)
		{
			continue;
		}

		QString fullPath = workStorage + "/" + workBuildDirectory;
		QString date = workBuildDirectory.right(19);
		QString backupName = workStorage + "/backup-" + date;

		if (workDirectory.rename(fullPath, backupName) == false)
		{
			DEBUG_LOG_MSG(m_logger, "Could not rename " + fullPath + " to " + backupName);

			return;
		}

		DEBUG_LOG_MSG(m_logger, fullPath + " renamed to " + backupName);
	}
}


void CfgCheckerWorker::onThreadStarted()
{
	if (m_workFolder.isEmpty() == true)
	{
		m_workFolder = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	}

	QDir workDirectory(m_workFolder);
	QString workStorage = m_workFolder + "/CfgSrvStorage";

	if (workDirectory.exists("CfgSrvStorage") == false && !workDirectory.mkpath(workStorage) == false)
	{
		m_workFolder.clear();
	}

	if (m_workFolder.isEmpty() || m_autoloadBuildFolder.isEmpty())
	{
		DEBUG_LOG_WRN(m_logger, "Work directory is empty, autoupdating is off");
		return;
	}
	else
	{
		DEBUG_LOG_MSG(m_logger, "Autoupdate is working at " + workStorage);
	}

	QDir storageDirectory(workStorage);

	QStringList workBuildDirectoryList = storageDirectory.entryList(QStringList() << "work-?\?\?\?-?\?-?\?-?\?-?\?-?\?", QDir::Dirs | QDir::NoSymLinks, QDir::Name);
	QString workBuildPath;

	if (workBuildDirectoryList.isEmpty() == false)
	{
		workBuildPath = workStorage + "/" + workBuildDirectoryList[0];
		QString workBuildFileName = workBuildPath + "/build.xml";

		QFileInfo buildXmlInfo(workBuildFileName);

		m_lastBuildXmlModifyTime = buildXmlInfo.lastModified();
		getFileHash(workBuildFileName, m_lastBuildXmlHash);
	}

	if (updateBuildXml() == false)
	{
		if (workBuildPath.isEmpty() == false)
		{
			emit buildPathChanged(workBuildPath);
			DEBUG_LOG_MSG(m_logger, "Work build directory is " + workBuildPath);
		}
		else
		{
			DEBUG_LOG_WRN(m_logger, "There is no work build directory");
		}
	}

	if (m_checkNewBuildInterval > 0)
	{
		QTimer* checkBuildXmlTimer = new QTimer(this);

		connect(checkBuildXmlTimer, &QTimer::timeout, this, &CfgCheckerWorker::updateBuildXml);

		checkBuildXmlTimer->start(m_checkNewBuildInterval);
	}
}