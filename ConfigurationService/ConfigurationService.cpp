#include <QXmlStreamReader>
#include <QMetaProperty>
#include <QStandardPaths>
#include <QTimer>

#include "ConfigurationService.h"
#include "../lib/XmlHelper.h"

// ------------------------------------------------------------------------------------
//
// ConfigurationServiceWorker class implementation
//
// ------------------------------------------------------------------------------------

ConfigurationServiceWorker::ConfigurationServiceWorker(const QString& serviceName,
													   int& argc, char** argv,
													   const VersionInfo& versionInfo,
													   std::shared_ptr<CircularLogger> logger) :
	ServiceWorker(ServiceType::ConfigurationService, serviceName, argc, argv, versionInfo, logger),
	m_logger(logger)
{
}


ServiceWorker* ConfigurationServiceWorker::createInstance() const
{
	ConfigurationServiceWorker* newInstance = new ConfigurationServiceWorker(serviceName(), argc(), argv(), versionInfo(), m_logger);
	return newInstance;
}


void ConfigurationServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	Q_UNUSED(serviceInfo)
}


void ConfigurationServiceWorker::onInformationRequest(UdpRequest request)
{
	switch(request.ID())
	{
	case RQID_GET_CONFIGURATION_SERVICE_INFO:
		onGetInfo(request);
		break;

	case RQID_SET_CONFIGURATION_SERVICE_SETTINGS:
		onSetSettings(request);
		break;

	case RQID_GET_CONFIGURATION_SERVICE_SETTINGS:
		onGetSettings(request);
		break;

	default:
		assert(false);
	}
}


void ConfigurationServiceWorker::initCmdLineParser()
{
	CommandLineParser& cp = cmdLineParser();

	cp.addSingleValueOption("id", "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption("b", "Path to RPCT project's build  for auto load.", "PathToBuild");
	cp.addSingleValueOption("ip", "Client request IP.", "IPv4");
	cp.addSingleValueOption("w", "Work directory of Configuration Service.", "Path");
}


void ConfigurationServiceWorker::processCmdLineSettings()
{
	CommandLineParser& cp = cmdLineParser();

	if (cp.optionIsSet("id") == true)
	{
		setStrSetting("EquipmentID", cp.optionValue("id"));
	}

	if (cp.optionIsSet("b") == true)
	{
		setStrSetting("AutoloadBuildPath", cp.optionValue("b"));
	}

	if (cp.optionIsSet("ip") == true)
	{
		setStrSetting("ClientRequestIP", cp.optionValue("ip"));
	}

	if (cp.optionIsSet("w") == true)
	{
		setStrSetting("WorkDirectory", cp.optionValue("w"));
	}
}


void ConfigurationServiceWorker::loadSettings()
{
	m_equipmentID = getStrSetting("EquipmentID");
	m_autoloadBuildPath = getStrSetting("AutoloadBuildPath");
	m_clientIPStr = getStrSetting("ClientRequestIP");
	m_workDirectory = getStrSetting("WorkDirectory");

	m_clientIP = HostAddressPort(m_clientIPStr, PORT_CONFIGURATION_SERVICE_REQUEST);

	DEBUG_LOG_MSG(m_logger, QString("Load settings:"));
	DEBUG_LOG_MSG(m_logger, QString("EquipmentID = %1").arg(m_equipmentID));
	DEBUG_LOG_MSG(m_logger, QString("AutoloadBuildPath = %1").arg(m_autoloadBuildPath));
	DEBUG_LOG_MSG(m_logger, QString("ClientRequestIP = %1 (%2)").arg(m_clientIPStr).arg(m_clientIP.addressPortStr()));
	DEBUG_LOG_MSG(m_logger, QString("WorkDirectory = %1").arg(m_workDirectory));
}


void ConfigurationServiceWorker::initialize()
{
	startCfgServerThread();

	startCfgCheckerThread();

	DEBUG_LOG_MSG(m_logger, QString(tr("ServiceWorker is initialized")));
}


void ConfigurationServiceWorker::shutdown()
{
	stopCfgCheckerThread();

	stopCfgServerThread();

	DEBUG_LOG_MSG(m_logger, QString(tr("ServiceWorker is shutting down")));
}


void ConfigurationServiceWorker::startCfgServerThread()
{
	CfgServer* cfgServer = new CfgServer(m_autoloadBuildPath, m_logger);

	Tcp::Listener* listener = new Tcp::Listener(m_clientIP, cfgServer, m_logger);

	m_cfgServerThread = new Tcp::ServerThread(listener);

	m_cfgServerThread->start();
}


void ConfigurationServiceWorker::stopCfgServerThread()
{
	m_cfgServerThread->quit();
	delete m_cfgServerThread;
}


void ConfigurationServiceWorker::startCfgCheckerThread()
{
	CfgCheckerWorker* cfgCheckerWorker = new CfgCheckerWorker(m_workDirectory, m_autoloadBuildPath, 30, m_logger);
	m_cfgCheckerThread = new SimpleThread(cfgCheckerWorker);

	m_cfgCheckerThread->start();
}


void ConfigurationServiceWorker::stopCfgCheckerThread()
{
	m_cfgCheckerThread->quit();
	delete m_cfgCheckerThread;
}


void ConfigurationServiceWorker::startUdpThreads()
{
	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, PORT_CONFIGURATION_SERVICE_INFO, m_logger);

	connect(serverSocket, &UdpServerSocket::receiveRequest, this, &ConfigurationServiceWorker::onInformationRequest);
	connect(this, &ConfigurationServiceWorker::ackInformationRequest, serverSocket, &UdpServerSocket::sendAck);

	m_infoSocketThread = new UdpSocketThread(serverSocket);

	m_infoSocketThread->start();
}


void ConfigurationServiceWorker::stopUdpThreads()
{
	m_infoSocketThread->quitAndWait();

	delete m_infoSocketThread;
}


void ConfigurationServiceWorker::onGetInfo(UdpRequest& request)
{
	UdpRequest ack;

	ack.initAck(request);

	ackInformationRequest(ack);
}


void ConfigurationServiceWorker::onSetSettings(UdpRequest& request)
{
	UdpRequest ack;

	ack.initAck(request);

	ackInformationRequest(ack);
}


void ConfigurationServiceWorker::onGetSettings(UdpRequest& request)
{
	UdpRequest ack;

	ack.initAck(request);

	ackInformationRequest(ack);
}


// ------------------------------------------------------------------------------------
//
// CfgCheckerWorker class implementation
//
// ------------------------------------------------------------------------------------

CfgCheckerWorker::CfgCheckerWorker(const QString& workFolder,
								   const QString& autoloadBuildFolder,
								   int checkNewBuildInterval,
								   std::shared_ptr<CircularLogger> logger) :
	m_workFolder(workFolder),
	m_autoloadBuildFolder(autoloadBuildFolder),
	m_checkNewBuildInterval(checkNewBuildInterval),
	m_logger(logger)
{
}


QString CfgCheckerWorker::getFileHash(const QString& filePath)
{
	QFile file(filePath);

	bool opened = file.open(QIODevice::ReadOnly);		// open in binary mode

	if (!opened)
	{
		return "";
	}

	QCryptographicHash md5Generator(QCryptographicHash::Md5);

	md5Generator.addData(&file);

	return QString(md5Generator.result().toHex());
}


bool CfgCheckerWorker::copyPath(const QString& src, const QString& dst)
{
	QDir dir(src);
	if (!dir.exists())
	{
		return false;
	}

	auto&& entryList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (QString directoryName : entryList)
	{
		QString dst_path = dst + QDir::separator() + directoryName;
		if (!dir.mkpath(dst_path))
		{
			return false;
		}

		if (!copyPath(src+ QDir::separator() + directoryName, dst_path))
		{
			return false;
		}
	}

	foreach (QString f, dir.entryList(QDir::Files))
	{
		if (!QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f))
		{
			return false;
		}
	}

	return true;
}

bool CfgCheckerWorker::checkBuild(const QString& buildDirectoryPath)
{
	QFile buildXmlFile(buildDirectoryPath + "/build.xml");

	if (!buildXmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		DEBUG_LOG_MSG(m_logger, "Could not open " + buildDirectoryPath + "/build.xml has been changed");
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

		QString realFileMd5Hash = getFileHash(buildDirectoryPath + fileName);

		if (realFileMd5Hash != fileMd5Hash)
		{
			DEBUG_LOG_MSG(m_logger, "File " + fileName + " has MD5 hash " + realFileMd5Hash + " expected " + fileMd5Hash);
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


void CfgCheckerWorker::updateBuildXml()
{
	if (m_workFolder.isEmpty() || m_autoloadBuildFolder.isEmpty())
	{
		return;
	}

	// Has build.xml been changed?
	//
	QFileInfo buildXmlInfo(m_autoloadBuildFolder + "/build.xml");
	if (buildXmlInfo.lastModified() == m_lastBuildXmlModifyTime)
	{
		return;
	}

	QString newBuildXmlHash = getFileHash(m_autoloadBuildFolder + "/build.xml");

	if (newBuildXmlHash == m_lastBuildXmlHash)
	{
		return;
	}

	DEBUG_LOG_MSG(m_logger, "build.xml has been changed");

	m_lastBuildXmlModifyTime = buildXmlInfo.lastModified();
	m_lastBuildXmlHash = newBuildXmlHash;

	// Copying into workDirectory/check-date
	//
	QDir workDirectory(m_workFolder + "/CfgSrvStorage");
	QString newCheckDirectoryName = "check-" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss");
	QString newCheckDirectoryPath = m_workFolder + "/CfgSrvStorage/" + newCheckDirectoryName;
	if (!workDirectory.mkpath(newCheckDirectoryPath))
	{
		DEBUG_LOG_ERR(m_logger, "Could not create directory " + newCheckDirectoryPath);
		return;
	}

	if (!copyPath(m_autoloadBuildFolder, newCheckDirectoryPath))
	{
		DEBUG_LOG_ERR(m_logger, "Could not copy content from " + m_autoloadBuildFolder + " to " + newCheckDirectoryPath);

		QDir newCheckDirectory(newCheckDirectoryPath);
		newCheckDirectory.removeRecursively();

		return;
	}

	// Checking copied build folder
	//
	if (!checkBuild(newCheckDirectoryPath))
	{
		DEBUG_LOG_ERR(m_logger, "Build in " + newCheckDirectoryPath + " is not consistent");

		QDir newCheckDirectory(newCheckDirectoryPath);
		newCheckDirectory.removeRecursively();

		return;
	}

	DEBUG_LOG_MSG(m_logger, "Build in " + newCheckDirectoryPath + " is correct");

	// Renaming to workDirectory/work-date
	QString date = newCheckDirectoryPath.right(19);
	QString newWorkDirectoryPath = m_workFolder + "/CfgSrvStorage/work-" + date;

	if (workDirectory.rename(newCheckDirectoryPath, newWorkDirectoryPath) == false)
	{
		DEBUG_LOG_MSG(m_logger, "Could not rename " + newCheckDirectoryPath + " to " + newWorkDirectoryPath);

		return;
	}

	DEBUG_LOG_MSG(m_logger, newCheckDirectoryPath + " renamed to " + newWorkDirectoryPath);

	// Renaming previous work-date directory to backup-date
	//
	QStringList&& workBuildDirectoryList = workDirectory.entryList(QStringList() << "work-?\?\?\?-?\?-?\?-?\?-?\?-?\?", QDir::Dirs | QDir::NoSymLinks, QDir::Name);

	if (!workBuildDirectoryList.isEmpty())
	{
		for (int i = 0; i < workBuildDirectoryList.count(); i++)
		{
			QString fullPath = m_workFolder + "/CfgSrvStorage/" + workBuildDirectoryList[i];
			if (fullPath != newWorkDirectoryPath)
			{
				QString date = workBuildDirectoryList[i].right(19);
				QString backupName = m_workFolder + "/CfgSrvStorage/backup-" + date;

				if (workDirectory.rename(fullPath, backupName) == false)
				{
					DEBUG_LOG_MSG(m_logger, "Could not rename " + fullPath + " to " + backupName);

					return;
				}
			}
		}
	}

	emit buildPathChanged(newWorkDirectoryPath);
}


void CfgCheckerWorker::onThreadStarted()
{
	if (m_workFolder.isEmpty())
	{
		m_workFolder = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	}

	QDir workDirectory(m_workFolder);
	if (!workDirectory.exists("CfgSrvStorage") && !workDirectory.mkpath(m_workFolder + "/CfgSrvStorage"))
	{
		m_workFolder.clear();
	}

	if (m_workFolder.isEmpty() || m_autoloadBuildFolder.isEmpty())
	{
		DEBUG_LOG_WRN(m_logger, "Work directory is empty, autoupdating is off")
		return;
	}
	else
	{
		DEBUG_LOG_MSG(m_logger, "Autoupdate is working at " + m_workFolder + "/CfgSrvStorage");
	}

	QDir storageDirectory(m_workFolder + "/CfgSrvStorage");

	QStringList workBuildDirectoryList = storageDirectory.entryList(QStringList() << "work-?\?\?\?-?\?-?\?-?\?-?\?-?\?", QDir::Dirs | QDir::NoSymLinks, QDir::Name);

	if (!workBuildDirectoryList.isEmpty())
	{
		QString workBuildFileName = m_workFolder + "/CfgSrvStorage/" + workBuildDirectoryList[0] + "/build.xml";

		QFileInfo buildXmlInfo(workBuildFileName);

		m_lastBuildXmlModifyTime = buildXmlInfo.lastModified();
		m_lastBuildXmlHash = getFileHash(workBuildFileName);
	}

	updateBuildXml();

	if (m_checkNewBuildInterval > 0)
	{
		QTimer* checkBuildXmlTimer = new QTimer(this);
		connect(checkBuildXmlTimer, &QTimer::timeout, this, &CfgCheckerWorker::updateBuildXml);
		checkBuildXmlTimer->start(m_checkNewBuildInterval);
	}
}
