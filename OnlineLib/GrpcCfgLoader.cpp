#ifndef ONLINE_LIB_DOMAIN
#error Do not include this file in the project! Link OnlineLib instead.
#endif

#include <algorithm>

#include <CommonLib/ConstStrings.h>

#include "GrpcCfgLoader.h"
#include "CircularLogger.h"
#include "../UtilsLib/XmlHelper.h"

// -------------------------------------------------------------------------------------
//
// GrpcCfgLoader class implementation
//
// -------------------------------------------------------------------------------------

GrpcCfgLoader::GrpcCfgLoader(const SoftwareInfo& softwareInfo,
						int appInstance,
						const std::vector<HostAddressPort>& serverAddrs,
						CircularLoggerShared logger) :
	LogWrapper(logger),
	m_swInfo(softwareInfo),
	m_appInstance(appInstance),
	m_serverAddrs(serverAddrs)
{
	if (m_typesRegistered.load(std::memory_order::seq_cst) == false)
	{
		qRegisterMetaType<BuildFileInfoArray>("BuildFileInfoArray");
		qRegisterMetaType<Tcp::FileTransferResult>("Tcp::FileTransferResult");
		qRegisterMetaType<std::shared_ptr<const SoftwareSettings>>("std::shared_ptr<const SoftwareSettings>");
		qRegisterMetaType<SessionParams>("SessionParams");

		m_typesRegistered.store(true, std::memory_order::seq_cst);
	}

	setObjectName("GrpcCfgLoader");
}

void GrpcCfgLoader::changeAppAndInitPaths(const QString& appEquipmentID, int appInstance)
{
	m_appEquipmentID = appEquipmentID;
	m_appInstance = appInstance;

	m_appDataPath = Separator::DIR + m_appEquipmentID + Separator::MINUS + QString::number(m_appInstance);

	m_rootFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + m_appDataPath;

	logMsg(QString("app cfg data root folder - %1").arg(m_rootFolder));

	m_cfgXmlFileName = Separator::DIR + m_appEquipmentID + Separator::DIR + File::CONFIGURATION_XML;

	resetStatuses();
}

bool GrpcCfgLoader::getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr)
{
	return readFile(pathFileName, fileData, errorStr);
}

bool GrpcCfgLoader::getFileBlockedByID(const QString& fileID, QByteArray* fileData, QString* errorStr)
{
	TEST_PTR_RETURN_FALSE(fileData);
	TEST_PTR_RETURN_FALSE(errorStr);

	QString pathFileName = getFilePathNameByID(fileID);

	if (pathFileName.isEmpty())
	{
		*errorStr = QString(tr("File with ID = '%1' not found.")).arg(fileID);
		return false;
	}

	return getFileBlocked(pathFileName, fileData, errorStr);
}

HostAddressPort GrpcCfgLoader::getServerAddr() const
{
	if (m_grpcFileClient != nullptr)
	{
		return m_grpcFileClient->getConnectedServerAddr();
	}

	return HostAddressPort();
}

Tcp::ConnectionState GrpcCfgLoader::getConnectionState() const
{
	if (m_grpcFileClient != nullptr)
	{
		return m_grpcFileClient->getConnectionState();
	}

	return Tcp::ConnectionState();
}

bool GrpcCfgLoader::hasFileID(const QString& fileID) const
{
	return m_fileIDPathMap.contains(fileID);
}

OnlineLib::BuildInfo GrpcCfgLoader::buildInfo() const
{
	return m_buildInfo;
}

SoftwareInfo GrpcCfgLoader::softwareInfo() const
{
	return m_swInfo;
}

int GrpcCfgLoader::appInstance() const
{
	return m_appInstance;
}

SessionParams GrpcCfgLoader::sessionParams() const
{
	return m_sessionParams;
}

QString GrpcCfgLoader::curSoftwareSettingsProfileName() const
{
	return m_sessionParams.currentSettingsProfile;
}

E::SoftwareRunMode GrpcCfgLoader::softwareRunMode() const
{
	return m_sessionParams.softwareRunMode;
}

QStringList GrpcCfgLoader::getSettingsProfiles() const
{
	QStringList profiles = m_settingsSet.getSettingsProfiles();

	return profiles;
}

// void GrpcCfgLoader::onEndDownload(const QString& fileName, Tcp::FileTransferResult errorCode)
// {
// 	if (errorCode == Tcp::FileTransferResult::Ok)
// 	{
// 		logMessage(QString("file %1 download Ok").arg(fileName));
// 	}
// 	else
// 	{
// 		logError(QString("file %1 download error - %2").arg(fileName).arg(getErrorStr(errorCode)));
// 	}
// }

void GrpcCfgLoader::slot_setConnection()
{
	resetStatuses();

	m_grpcFileClient->downloadSessionParams();
}

void GrpcCfgLoader::slot_sessionParamsReady(Tcp::FileTransferResult result, SessionParams params)
{
	if (result != Tcp::FileTransferResult::Ok)
	{
		restartGrpcFileClient();
		return;
	}

	m_sessionParams = params;

	m_grpcFileClient->downloadFile(m_cfgXmlFileName);
}

void GrpcCfgLoader::slot_fileReady(FileReady fileReady)
{
	if (fileReady.errorCode != Tcp::FileTransferResult::Ok)
	{
		restartGrpcFileClient();
		return;
	}

	if (fileReady.fileName == m_cfgXmlFileName)
	{
		processCfgXmlFile(fileReady);
	}
	else
	{
		processOtherFiles(fileReady);
	}
}

void GrpcCfgLoader::onThreadStarted()
{
	Q_ASSERT(m_grpcFileClient == nullptr);

	changeAppAndInitPaths(m_swInfo.equipmentID(), m_appInstance);

	startGrpcFileClient();
}

void GrpcCfgLoader::onThreadFinished()
{
	stopGrpcFileClient();
}

void GrpcCfgLoader::startGrpcFileClient()
{
	if (m_grpcFileClient != nullptr)
	{
		stopGrpcFileClient();
	}

	m_grpcFileClient = std::make_unique<GrpcFileClient>(m_swInfo, m_serverAddrs, m_rootFolder,
														QStringLiteral("GrpcCfgLoader"), getLog(), false);

	m_grpcFileClient->setEmitFileReady(true);

	connect(m_grpcFileClient.get(), &GrpcFileClient::signal_setConnection, this, &GrpcCfgLoader::slot_setConnection);
	connect(m_grpcFileClient.get(), &GrpcFileClient::signal_unknownClientID, this, &GrpcCfgLoader::signal_unknownClientID);
	connect(m_grpcFileClient.get(), &GrpcFileClient::signal_wrongClientHostname, this, &GrpcCfgLoader::signal_wrongClientHostname);
	connect(m_grpcFileClient.get(), &GrpcFileClient::signal_sessionParamsReady, this, &GrpcCfgLoader::slot_sessionParamsReady);
	connect(m_grpcFileClient.get(), &GrpcFileClient::signal_fileReady, this, &GrpcCfgLoader::slot_fileReady);

	m_grpcFileClient->start();
}

void GrpcCfgLoader::stopGrpcFileClient()
{
	if (m_grpcFileClient != nullptr)
	{
		m_grpcFileClient->stop();
		m_grpcFileClient.reset();
	}
}

void GrpcCfgLoader::restartGrpcFileClient()
{
	stopGrpcFileClient();
	startGrpcFileClient();
}

void GrpcCfgLoader::processCfgXmlFile(FileReady& fr)
{
	if (saveFile(fr) == false)
	{
		Q_ASSERT(false);
		return;
	}

	if (readCfgXmlFile(fr.fileData) == false)
	{
		logErr("Error reading Configuration.xml file!");
		restartGrpcFileClient();
		return;
	}

	m_cfgXmlFileData.swap(fr.fileData);

	checkExistsBuildFiles();

	downloadNextFile();
}

void GrpcCfgLoader::processOtherFiles(const FileReady& fr)
{
	auto it = m_filesToDownload.find(fr.fileName);

	if (it == m_filesToDownload.end())
	{
		Q_ASSERT(false);
		downloadNextFile();
		return;
	}

	if (it->second != fr.md5)
	{
		Q_ASSERT(false);
		downloadNextFile();
		return;
	}

	saveFile(fr);

	m_filesToDownload.erase(it);

	downloadNextFile();
}

bool GrpcCfgLoader::saveFile(const FileReady& fr)
{
	QString fileName = m_rootFolder + fr.fileName;

	QFile file(fileName);

	if (file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate) == false)
	{
		return false;
	}

	file.write(fr.fileData);

	file.close();

	return true;
}

void GrpcCfgLoader::checkExistsBuildFiles()
{
	m_filesToDownload.clear();

	for(const OnlineLib::BuildFileInfo& bfi : m_buildFilesInfo)
	{
		QString fileName = m_rootFolder + bfi.pathFileName;

		QFile file(fileName);

		if (file.open(QIODeviceBase::ReadOnly) == false)
		{
			m_filesToDownload.emplace(bfi.pathFileName, bfi.md5);
			continue;
		}

		QByteArray fileData = file.readAll();

		if (Md5Hash::hashStr(fileData) != bfi.md5)
		{
			m_filesToDownload.emplace(bfi.pathFileName, bfi.md5);
		}

		file.close();
	}
}

void GrpcCfgLoader::downloadNextFile()
{
	if (m_filesToDownload.empty())
	{
		emit signal_configurationReady(m_cfgXmlFileData, m_buildFilesInfo,
									   m_sessionParams, getCurrentSettingsProfile<SoftwareSettings>());
		return;
	}

	QString fileName = m_filesToDownload.begin()->first;

	m_grpcFileClient->downloadFile(fileName);
}

bool GrpcCfgLoader::readFile(const QString& pathFileName, QByteArray* fileData, QString* errorStr) const
{
	TEST_PTR_RETURN_FALSE(fileData);
	TEST_PTR_RETURN_FALSE(errorStr);

	fileData->clear();
	errorStr->clear();

	OnlineLib::BuildFileInfo bfi;

	if (findBuildFileInfo(pathFileName, bfi) == false)
	{
		*errorStr = QString("File %1 not found in configuration files").arg(pathFileName);
		logErr(*errorStr);
		return false;
	}

	QFile f(m_rootFolder + pathFileName);

	if (f.open(QIODeviceBase::ReadOnly) == false)
	{
		*errorStr = QString("File %1 open error").arg(pathFileName);
		logErr(*errorStr);
		return false;
	}

	*fileData = f.readAll();

	f.close();

	if (fileData->size() != bfi.size)
	{
		*errorStr = QString("File %1 read error").arg(pathFileName);
		logErr(*errorStr);
		return false;
	}

	if (Md5Hash::hashStr(*fileData) != bfi.md5)
	{
		*errorStr = QString("File %1 corrupted").arg(pathFileName);
		logErr(*errorStr);
		return false;
	}

	if (bfi.compressed == true)
	{
		*fileData = qUncompress(*fileData);
	}

	return true;
}

bool GrpcCfgLoader::findBuildFileInfo(const QString& pathFileName, OnlineLib::BuildFileInfo& bfi) const
{
	bfi.clear();

	auto it = std::find_if(m_buildFilesInfo.begin(), m_buildFilesInfo.end(),
						   [&pathFileName](const OnlineLib::BuildFileInfo& b)
							{
								return b.pathFileName == pathFileName;
							});

	if (it == m_buildFilesInfo.end())
	{
		return false;
	}

	bfi = *it;
	return true;
}

void GrpcCfgLoader::resetStatuses()
{
	m_cfgXmlMd5.clear();
	m_configurationXmlReady = false;
	m_allFilesLoaded = false;
}

bool GrpcCfgLoader::readCfgXmlFile(const QByteArray& fileData)
{
	m_buildFilesInfo.clear();
	m_fileIDPathMap.clear();

	XmlReadHelper xmlReader(fileData);

	bool res = m_buildInfo.readFromXml(xmlReader);

	if (res == false)
	{
		logErr(QString("Can't read <BuildInfo> section in file %1!").arg(m_cfgXmlFileName));
		return false;
	}

	res = m_settingsSet.readFromXml(xmlReader);

	if (res == false)
	{
		logErr("reading software settings set - FAILED!");
		return false;
	}

	res = xmlReader.findElement(XmlElement::FILES);

	if (res == false)
	{
		logErr(QString("can't read <Files> section in file %1!").arg(m_cfgXmlFileName));
		return false;
	}

	while(xmlReader.findElement(XmlElement::FILE) != false)
	{
		OnlineLib::BuildFileInfo bfi;

		bfi.readFromXml(xmlReader, false);

		m_buildFilesInfo.emplace_back(bfi);

		if (bfi.ID.isEmpty() == false)
		{
			m_fileIDPathMap.emplace(bfi.ID, bfi.pathFileName);
		}
	}

	return true;
}

bool GrpcCfgLoader::readCfgFile(const QString& pathFileName, QByteArray* fileData, bool needUncompress) const
{
	if (fileData == nullptr)
	{
		assert(false);
		return false;
	}

	QFile file(m_rootFolder + pathFileName);

	if (file.open(QIODevice::ReadOnly) == false)
	{
		assert(false);
		return false;
	}

	if (needUncompress == true)
	{
		*fileData = qUncompress(file.readAll());
	}
	else
	{
		*fileData = file.readAll();
	}

	file.close();

	return true;
}

bool GrpcCfgLoader::readCfgFileIfExists(const QString& filePathName, QByteArray* fileData, const QString& etalonMd5, bool needUncompress)
{
	if (fileData == nullptr)
	{
		assert(false);
		return false;
	}

	QString fileName = m_rootFolder + filePathName;

	QFile file(fileName);

	if (file.exists() == false)
	{
		return false;
	}

	if (file.open(QIODevice::ReadOnly) == false)
	{
		return false;
	}

	*fileData = file.readAll();

	file.close();

	Md5Hash md5Hash;

	md5Hash.addData(*fileData);

	if (md5Hash.resultStr() == etalonMd5)
	{
		if (needUncompress == true)
		{
			*fileData = qUncompress(*fileData);
		}

		return true;
	}

	fileData->clear();

	return false;
}

bool GrpcCfgLoader::isCfgFileIsExists(const QString& filePathName, const QString& etalonMd5)
{
	QString fileName = m_rootFolder + filePathName;

	QFile file(fileName);

	if (file.exists() == false)
	{
		return false;
	}

	if (file.open(QIODevice::ReadOnly) == false)
	{
		return false;
	}

	Md5Hash md5Hash;

	md5Hash.addData(&file);

	file.close();

	if (md5Hash.resultStr() == etalonMd5)
	{
		return true;
	}

	return false;
}

QString GrpcCfgLoader::getFilePathNameByID(const QString& fileID) const
{
	auto it = m_fileIDPathMap.find(fileID);

	if (it != m_fileIDPathMap.end())
	{
		return it->second;
	}

	return QString();
}

// -------------------------------------------------------------------------------------
//
// GrpcCfgLoaderThread class implementation
//
// -------------------------------------------------------------------------------------

GrpcCfgLoaderThread::GrpcCfgLoaderThread(	const SoftwareInfo& softwareInfo,
									int appInstance,
									const HostAddressPort& serverAddressPort1,
									const HostAddressPort& serverAddressPort2,
									std::shared_ptr<CircularLogger> logger) :
	m_softwareInfo(softwareInfo),
	m_appInstance(appInstance),
	m_logger(logger)
{
	addServerAddrs(serverAddressPort1, serverAddressPort2);

	initThread();
}

GrpcCfgLoaderThread::~GrpcCfgLoaderThread()
{
	shutdownThread();
}

void GrpcCfgLoaderThread::start()
{
	std::lock_guard lg(m_mutex);

	if (m_thread == nullptr || m_grpcCfgLoader == nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	m_thread->start();
}

void GrpcCfgLoaderThread::quitAndWait()
{
	std::lock_guard lg(m_mutex);
	shutdownThread();
}

void GrpcCfgLoaderThread::setConnectionParams(const SoftwareInfo& softwareInfo,
	const HostAddressPort& serverAddressPort1,
	const HostAddressPort& serverAddressPort2)
{
	{
		std::lock_guard lg(m_mutex);

		m_softwareInfo = softwareInfo;

		addServerAddrs(serverAddressPort1, serverAddressPort2);
	}

	shutdownThread();
	initThread();
	start();
}

bool GrpcCfgLoaderThread::getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr)
{
	TEST_PTR_RETURN_FALSE(fileData);
	TEST_PTR_RETURN_FALSE(errorStr);

	errorStr->clear();

	return m_grpcCfgLoader->getFileBlocked(pathFileName, fileData, errorStr);
}

bool GrpcCfgLoaderThread::getFileBlockedByID(const QString& fileID, QByteArray* fileData, QString* errorStr)
{
	TEST_PTR_RETURN_FALSE(fileData);
	TEST_PTR_RETURN_FALSE(errorStr);

	errorStr->clear();

	return m_grpcCfgLoader->getFileBlockedByID(fileID, fileData, errorStr);
}

HostAddressPort GrpcCfgLoaderThread::getServerAddr() const
{
	if (m_grpcCfgLoader != nullptr)
	{
		return m_grpcCfgLoader->getServerAddr();
	}

	return HostAddressPort();
}

bool GrpcCfgLoaderThread::hasFileID(const QString& fileID) const
{
	std::lock_guard lg(m_mutex);

	return m_grpcCfgLoader->hasFileID(fileID);
}

OnlineLib::BuildInfo GrpcCfgLoaderThread::buildInfo() const
{
	std::lock_guard lg(m_mutex);

	return m_grpcCfgLoader->buildInfo();
}

Tcp::ConnectionState GrpcCfgLoaderThread::getConnectionState() const
{
	std::lock_guard lg(m_mutex);

	if (m_grpcCfgLoader != nullptr)
	{
		return m_grpcCfgLoader->getConnectionState();
	}

	return Tcp::ConnectionState{};
}

/*HostAddressPort GrpcCfgLoaderThread::getCurrentServerAddressPort()
{
	AUTO_LOCK(m_mutex);

//	return m_grpcCfgLoader->currentServerAddressPort();
	return HostAddressPort{};
}*/

SessionParams GrpcCfgLoaderThread::sessionParams() const
{
	std::lock_guard lg(m_mutex);

	if (m_grpcCfgLoader != nullptr)
	{
		return m_grpcCfgLoader->sessionParams();
	}

	Q_ASSERT(false);

	return SessionParams();
}

void GrpcCfgLoaderThread::addServerAddrs(const HostAddressPort& addr1, const HostAddressPort& addr2)
{
	m_serverAddrs.clear();

	if (addr1.isNull() == false)
	{
		m_serverAddrs.push_back(addr1);
	}

	if (addr2.isNull() == false)
	{
		m_serverAddrs.push_back(addr2);
	}
}

void GrpcCfgLoaderThread::initThread()
{
	std::lock_guard lg(m_mutex);

	Q_ASSERT(m_grpcCfgLoader == nullptr);
	Q_ASSERT(m_thread == nullptr);

	m_grpcCfgLoader = new GrpcCfgLoader(m_softwareInfo,
								m_appInstance,
								m_serverAddrs,
								m_logger);

	m_thread = new SimpleThread;

	m_thread->addWorker(m_grpcCfgLoader); // this instance of CfgLoader will be deleted during SimpleThread destruction

	connect(m_grpcCfgLoader, &GrpcCfgLoader::signal_configurationReady, this, &GrpcCfgLoaderThread::signal_configurationReady);
	connect(m_grpcCfgLoader, &GrpcCfgLoader::signal_unknownClientID, this, &GrpcCfgLoaderThread::signal_unknownClientID);
	connect(m_grpcCfgLoader, &GrpcCfgLoader::signal_wrongClientHostname, this, &GrpcCfgLoaderThread::signal_wrongClientHostname);
}

void GrpcCfgLoaderThread::shutdownThread()
{
	if (m_thread == nullptr)
	{
		return;
	}

	m_thread->quitAndWait();			// m_cfgLoader will be deleted here

	qDebug() << "GrpcCfgLoaderThread quited";

	delete m_thread;

	m_thread = nullptr;
	m_grpcCfgLoader = nullptr;
}

