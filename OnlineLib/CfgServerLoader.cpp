#ifndef ONLINE_LIB_DOMAIN
#error Do not include this file in the project! Link OnlineLib instead.
#endif

#include "CfgServerLoader.h"
#include "CircularLogger.h"
#include "../lib/ConstStrings.h"


// -------------------------------------------------------------------------------------
//
// CfgServerLoaderBase class implementation
//
// -------------------------------------------------------------------------------------

bool CfgServerLoaderBase::m_typesRegistered = false;

CfgServerLoaderBase::CfgServerLoaderBase()
{
	if (m_typesRegistered == false)
	{
		qRegisterMetaType<BuildFileInfoArray>("BuildFileInfoArray");
		qRegisterMetaType<Tcp::FileTransferResult>("Tcp::FileTransferResult");
		qRegisterMetaType<std::shared_ptr<const SoftwareSettings>>("std::shared_ptr<const SoftwareSettings>");
		qRegisterMetaType<SessionParams>("SessionParams");

		m_typesRegistered = true;
	}
}

// -------------------------------------------------------------------------------------
//
// CfgServer class implementation
//
// -------------------------------------------------------------------------------------

CfgServer::CfgServer(const QString& buildFolder,
					 const SoftwareInfo& softwareInfo,
					 E::SecurityLevel securityLevel,
					 const SessionParams& sessionParams,
					 CircularLoggerShared logger) :
	Tcp::FileServer(buildFolder, softwareInfo, securityLevel, logger, "CfgServer"),
	m_sessionParams(sessionParams)
{
}

CfgServer* CfgServer::getNewInstance()
{
	return new CfgServer(m_rootFolder,
						 localSoftwareInfo(),
						 securityLevel(),
						 m_sessionParams,
						 log());
}

void CfgServer::processSuccessorRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	Q_UNUSED(requestData);
	Q_UNUSED(requestDataSize);

	switch(requestID)
	{
	case RQID_GET_SESSION_PARAMS:
		processGetSessionParamsRequest();
		break;

	default:
		logError(QString("unknown request ID = %1 (ignored)").arg(requestID));
	}
}

void CfgServer::onServerThreadStarted()
{
	m_buildXmlPathFileName = m_rootFolder + "/build.xml";

	readBuildXml();
}

void CfgServer::onServerThreadFinished()
{
}

void CfgServer::readBuildXml()
{
	QDir dir(m_buildXmlPathFileName);

	if (dir.exists(m_buildXmlPathFileName) == false)
	{
		m_errorCode = ErrorCode::BuildNotFound;
		logError(QString("file %1 not found!").arg(m_buildXmlPathFileName));
		return;
	}

	QFile buildXml(m_buildXmlPathFileName);

	if (buildXml.open(QIODevice::ReadOnly) == false)
	{
		m_errorCode = ErrorCode::BuildCantRead;
		return;
	}

	QByteArray data = buildXml.readAll();

	buildXml.close();

	if (data.isEmpty())
	{
		m_errorCode = ErrorCode::BuildCantRead;
		return;
	}

	QXmlStreamReader xmlReader(data);

	while(xmlReader.atEnd() == false)
	{
		if (xmlReader.readNextStartElement() == false)
		{
			continue;
		}

		if (xmlReader.name() == QLatin1String("BuildInfo"))
		{
			m_buildInfo.readFromXml(xmlReader);
			continue;
		}

		// find "file" element
		//
		if (xmlReader.name() != QLatin1String("File"))
		{
			continue;
		}

		OnlineLib::BuildFileInfo bfi;

		bfi.readFromXml(xmlReader);

		m_buildFileInfo.emplace(bfi.pathFileName, bfi);
	}

	logMessage(QString("file %1 has been read").arg(m_buildXmlPathFileName));
}

bool CfgServer::checkFile(QString& pathFileName, QByteArray& fileData)
{
	if (m_buildFileInfo.contains(pathFileName) == false)
	{
		return false;
	}

	QString fileMd5 = m_buildFileInfo[pathFileName].md5;
	QString calculatedMd5 = QCryptographicHash::hash(fileData, QCryptographicHash::Md5).toHex();

	if (fileMd5 != calculatedMd5)
	{
		return false;
	}

	return true;
}

void CfgServer::processGetSessionParamsRequest()
{
	Network::SessionParams sp;

	m_sessionParams.saveTo(&sp);

	sendReply(sp);
}

// -------------------------------------------------------------------------------------
//
// CfgLoader::FileDownloadRequest struct implementation
//
// -------------------------------------------------------------------------------------

void CfgLoader::FileDownloadRequest::clear()
{
	pathFileName = "";
	etalonMD5 = "";
	isAutoRequest = false;
	isTestCfgRequest = false;
	fileData = nullptr;
	errorCode = nullptr;
}

void CfgLoader::FileDownloadRequest::setErrorCode(Tcp::FileTransferResult result)
{
	if (errorCode != nullptr)
	{
		*errorCode = result;
	}
}

// -------------------------------------------------------------------------------------
//
// CfgLoader class implementation
//
// -------------------------------------------------------------------------------------

CfgLoader::CfgLoader(const SoftwareInfo& softwareInfo,
						int appInstance,
						const HostAddressPort& serverAddressPort1,
						const HostAddressPort& serverAddressPort2,
						bool enableDownloadCfg,
						std::shared_ptr<CircularLogger> logger) :
	Tcp::FileClient(softwareInfo, "", serverAddressPort1, serverAddressPort2, "CfgLoader"),
	TcpClientStatistics(this),
	m_enableDownloadConfiguration(enableDownloadCfg),
	m_timer(this)
{
	setObjectName("CfgLoader");
	setLogger(logger);
	changeApp(softwareInfo.equipmentID(), appInstance);
}

void CfgLoader::onClientThreadStarted()
{
	Tcp::FileClient::onClientThreadStarted();

	connect(&m_timer, &QTimer::timeout, this, &CfgLoader::slot_onTimer);
	connect(this, &CfgLoader::signal_getFile, this, &CfgLoader::slot_getFile);
	connect(this, &CfgLoader::signal_enableDownloadConfiguration, this, &CfgLoader::slot_enableDownloadConfiguration);

	m_timer.setInterval(2000);
	m_timer.start();
}

void CfgLoader::changeApp(const QString& appEquipmentID, int appInstance)
{
	shutdown();

	m_appEquipmentID = appEquipmentID;
	m_appInstance = appInstance;

	m_appDataPath = "/" + m_appEquipmentID + "-" + QString::number(m_appInstance);

	m_rootFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + m_appDataPath;

	logMessage(QString("app cfg data root folder - %1").arg(m_rootFolder));

	setRootFolder(m_rootFolder);

	m_configurationXmlPathFileName = "/" + m_appEquipmentID + "/" + File::CONFIGURATION_XML;

	readSavedConfiguration();

	resetStatuses();
}

bool CfgLoader::getFileBlocked(QString pathFileName, QByteArray* fileData, QString* errorStr)
{
	// execute in context of calling thread
	//
	TEST_PTR_RETURN_FALSE(fileData);
	TEST_PTR_RETURN_FALSE(errorStr);

	fileData->clear();
	errorStr->clear();

	bool result = false;

	m_getFileBlockedMutex.lock();

	m_localFileData.clear();

	emit signal_getFile(pathFileName, &m_localFileData);

	bool res = m_fileReadyCondition.wait(&m_getFileBlockedMutex, 6000);

	if (res == true)
	{
		*errorStr = getLastErrorStr();

		if (errorStr->isEmpty() == true)
		{
			fileData->swap(m_localFileData);
			result = true;
		}
		else
		{
			result = false;
		}
	}
	else
	{
		*errorStr = tr("File reading timeout");
		result = false;
	}

	m_getFileBlockedMutex.unlock();

	return result;
}

bool CfgLoader::getFile(QString pathFileName, QByteArray* fileData)
{
	// execute in context of calling thread
	//
	if (fileData == nullptr)
	{
		assert(false);
		return false;
	}

	fileData->clear();

	emit signal_getFile(pathFileName, fileData);

	return true;
}

bool CfgLoader::getFileBlockedByID(QString fileID, QByteArray* fileData, QString* errorStr)
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

bool CfgLoader::getFileByID(QString fileID, QByteArray* fileData)
{
	QString pathFileName = getFilePathNameByID(fileID);

	return getFile(pathFileName, fileData);
}

bool CfgLoader::hasFileID(QString fileID) const
{
	AUTO_LOCK(m_mutex);

	return m_fileIDPathMap.contains(fileID);
}

OnlineLib::BuildInfo CfgLoader::buildInfo()
{
	AUTO_LOCK(m_mutex);

	return m_buildInfo;
}

SessionParams CfgLoader::sessionParams() const
{
	AUTO_LOCK(m_mutex);

	return m_sessionParams;
}

QString CfgLoader::curSoftwareSettingsProfileName() const
{
	AUTO_LOCK(m_mutex);

	return m_sessionParams.currentSettingsProfile;
}

E::SoftwareRunMode CfgLoader::softwareRunMode() const
{
	AUTO_LOCK(m_mutex);

	return m_sessionParams.softwareRunMode;
}

QStringList CfgLoader::getSettingsProfiles() const
{
	AUTO_LOCK(m_mutex);

	QStringList profiles = m_settingsSet.getSettingsProfiles();

	return profiles;
}

void CfgLoader::onConnection()
{
	Client::onConnection();

	resetStatuses();

	sendGetSessionParamsRequest();
}

void CfgLoader::onStartDownload(const QString& fileName)
{
	logMessage(QString("start download: %1").arg(fileName));
}

void CfgLoader::onEndDownload(const QString& fileName, Tcp::FileTransferResult errorCode)
{
	if (errorCode == Tcp::FileTransferResult::Ok)
	{
		logMessage(QString("file %1 download Ok").arg(fileName));
	}
	else
	{
		logError(QString("file %1 download error - %2").arg(fileName).arg(getErrorStr(errorCode)));
	}
}

void CfgLoader::slot_enableDownloadConfiguration()
{
	m_enableDownloadConfiguration = true;
}

void CfgLoader::slot_getFile(QString fileName, QByteArray* fileData)
{
	if (fileData == nullptr)
	{
		assert(false);
		return;
	}

	fileData->clear();

	if (m_configurationXmlReady == false)
	{
		m_lastError = Tcp::FileTransferResult::ConfigurationIsNotReady;
		emitFileReady();
		return;
	}

	if (m_cfgFilesInfo.contains(fileName) == false)
	{
		m_lastError = Tcp::FileTransferResult::FileIsNotAccessible;
		emitFileReady();
		return;
	}

	const CfgFileInfo& cfgFileInfo = m_cfgFilesInfo.value(fileName);

	if (readCfgFileIfExists(fileName, fileData, cfgFileInfo.md5, cfgFileInfo.compressed) == true)
	{
		logMessage(QString("file %1 already exists, md5 = %2").
					arg(fileName).arg(cfgFileInfo.md5));

		m_lastError = Tcp::FileTransferResult::Ok;
		emitFileReady();
		return;
	}

	// file is not exists
	//

	FileDownloadRequest fdr;

	fdr.pathFileName = fileName;
	fdr.isAutoRequest = false;			// manual request
	fdr.fileData = fileData;
	fdr.errorCode = &m_lastError;
	fdr.etalonMD5 = cfgFileInfo.md5;
	fdr.needUncompress = cfgFileInfo.compressed;

	m_downloadQueue.append(fdr);

	if (isTransferInProgress() == false)
	{
		startDownload();
	}
}

void CfgLoader::slot_onTimer()
{
	if (isConnected() == false)
	{
		return;
	}

	if (m_configurationXmlReady == false)
	{
		startConfigurationXmlLoading();
		return;
	}

	if (m_autoDownloadIndex >= m_cfgFilesInfo.count() ||
		isConnected() == false ||
		m_configurationXmlReady == false ||
		isTransferInProgress() ||
		m_allFilesLoaded == true ||
		m_downloadQueue.isEmpty() == false)
	{
		return;
	}

	while(m_autoDownloadIndex < m_cfgFilesInfo.count())
	{
		CfgFileInfo& cfi = m_cfgFilesInfo[m_autoDownloadIndex];

		if (isCfgFileIsExists(cfi.pathFileName, cfi.md5) == true)
		{
			// file exists from previous downloads, nothing to do
			//
			logMessage(QString("file %1 already exists, md5 = %2").
										arg(cfi.pathFileName).arg(cfi.md5));
		}
		else
		{
			FileDownloadRequest fdr;

			fdr.pathFileName = m_cfgFilesInfo[m_autoDownloadIndex].pathFileName;
			fdr.etalonMD5 = m_cfgFilesInfo[m_autoDownloadIndex].md5;
			fdr.isAutoRequest = true;

			m_downloadQueue.append(fdr);

			startDownload();

			m_autoDownloadIndex++;

			break;
		}

		m_autoDownloadIndex++;
	}
}

void CfgLoader::processSuccessorReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	switch(requestID)
	{
	case RQID_GET_SESSION_PARAMS:
		processGetSessionParamsReply(replyData, replyDataSize);
		break;

	default:
		Q_ASSERT(false);
	}
}

void CfgLoader::processGetSessionParamsReply(const char* replyData, quint32 replyDataSize)
{
	Network::SessionParams sp;

	bool res = sp.ParseFromArray(replyData, replyDataSize);

	if (res == false)
	{
		Q_ASSERT(false);
		sendGetSessionParamsRequest();
		return;
	}

	m_mutex.lock();

	m_sessionParams.loadFrom(sp);

	m_mutex.unlock();

	logMessage(QString("current software settings profile - %1, run mode - %2").
				  arg(m_sessionParams.currentSettingsProfile).
				  arg(E::valueToString<E::SoftwareRunMode>(m_sessionParams.softwareRunMode)));

	startConfigurationXmlLoading();
}

void CfgLoader::sendGetSessionParamsRequest()
{
	sendRequest(RQID_GET_SESSION_PARAMS);
}

void CfgLoader::shutdown()
{
}

void CfgLoader::startDownload()
{
	if (m_downloadQueue.isEmpty())
	{
		assert(false);
		return;
	}

	m_currentDownloadRequest = m_downloadQueue.first();

	m_downloadQueue.removeFirst();

	onStartDownload(m_currentDownloadRequest.pathFileName);

	slot_downloadFile(m_currentDownloadRequest.pathFileName);	// TcpFileTransfer::slot_downloadFile
}

void CfgLoader::resetStatuses()
{
	m_downloadQueue.clear();
	m_currentDownloadRequest.clear();

	m_configurationXmlMd5 = "";

	m_configurationXmlReady = false;
	m_autoDownloadIndex = 1;		// index 0 - Configuration.xml
	m_allFilesLoaded = false;
}

void CfgLoader::onEndFileDownload(const QString fileName, Tcp::FileTransferResult errorCode, const QString md5)
{
	m_currentDownloadRequest.setErrorCode(errorCode);

	//

	emit signal_onEndFileDownload(fileName, errorCode);

	if (errorCode != Tcp::FileTransferResult::Ok)
	{
		emit signal_onEndFileDownloadError(fileName, errorCode);
	}

	//

	onEndDownload(fileName, errorCode);

	if (errorCode != Tcp::FileTransferResult::Ok)
	{
		emitFileReady();
		return;
	}

	if (m_currentDownloadRequest.etalonMD5.isEmpty() == true)
	{
		// can be empty for Configuration.xml file only!!!
		//
		assert(m_currentDownloadRequest.pathFileName == m_configurationXmlPathFileName);
	}
	else
	{
		if (m_currentDownloadRequest.etalonMD5 != md5)
		{
			assert(false);

			m_currentDownloadRequest.setErrorCode(Tcp::FileTransferResult::FileDataCorrupted);
			emitFileReady();
			return;
		}
	}

	if (fileName == m_configurationXmlPathFileName)
	{
		// Configuration.xml is loaded
		//
		if (m_currentDownloadRequest.isTestCfgRequest)
		{
			if (m_cfgFilesInfo[CONFIGURATION_XML_FILE_INDEX].md5 != md5)
			{
				// configuration changed !!!!!!!!!!!!
			}
		}
		else
		{
			bool result = true;

			if (readConfigurationXml() == true)
			{
				m_configurationXmlReady = true;

				BuildFileInfoArray bfiArray;

				for(const CfgFileInfo& cfi : m_cfgFilesInfo)
				{
					const OnlineLib::BuildFileInfo& bfi = cfi;

					bfiArray.emplace_back(bfi);
				}

				std::shared_ptr<const SoftwareSettings> curSettingsProfile = getCurrentSettingsProfile<SoftwareSettings>();

				if (curSettingsProfile != nullptr)
				{
					logMessage(QString("current software settings profile '%1' read - Ok").
												arg(m_sessionParams.currentSettingsProfile));

					logMessage("read Configuration.xml - Ok");

					emit signal_configurationReady(m_cfgFilesInfo[CONFIGURATION_XML_FILE_INDEX].fileData,
												   bfiArray,
												   m_sessionParams,
												   curSettingsProfile);
				}
				else
				{
					logError(QString("reading software settings profile '%1' - FAILED").
												arg(m_sessionParams.currentSettingsProfile));
					result = false;
				}
			}
			else
			{
				result = false;
			}

			if (result == false)
			{
				logError("reading Configuration.xml - FAILED");
			}
		}
	}
	else
	{
		logMessage(QString("downloaded %1 %2").
									arg(m_currentDownloadRequest.isAutoRequest ? "(auto) :" : "(manual) :").
									arg(fileName));

		if (m_currentDownloadRequest.isAutoRequest == false)
		{
			// emit signal_endFileDownload for "manual" requests only!
			//
			if(m_currentDownloadRequest.fileData == nullptr)
			{
				assert(false);
				m_currentDownloadRequest.setErrorCode(Tcp::FileTransferResult::InternalError);
				emitFileReady();
			}
			else
			{
				if (readCfgFile(fileName, m_currentDownloadRequest.fileData, m_currentDownloadRequest.needUncompress) == false)
				{
					m_currentDownloadRequest.setErrorCode(Tcp::FileTransferResult::LocalFileReadingError);
					emitFileReady();
				}
				else
				{
					emitFileReady();
				}
			}
		}
	}

	if (m_autoDownloadIndex == m_cfgFilesInfo.count())
	{
		m_allFilesLoaded = true;
	}

	if (m_downloadQueue.isEmpty() == false)
	{
		startDownload();
	}
}

bool CfgLoader::startConfigurationXmlLoading()
{
	if (m_enableDownloadConfiguration == false)
	{
		return true;
	}

	if (isConnected() == false)
	{
		return true;
	}

	if (m_transferInProgress == true)
	{
		return true;
	}

	m_configurationXmlReady = false;

	FileDownloadRequest fdr;

	fdr.pathFileName = m_configurationXmlPathFileName;

	m_downloadQueue.push_front(fdr);

	startDownload();

	return true;
}

bool CfgLoader::readConfigurationXml()
{
	QByteArray fileData;

	if (readCfgFile(m_configurationXmlPathFileName, &fileData, false) == false)
	{
		return false;
	}

	AUTO_LOCK(m_mutex);

	m_cfgFilesInfo.clear();
	m_fileIDPathMap.clear();

	CfgFileInfo cfi;

	cfi.pathFileName = m_configurationXmlPathFileName;
	cfi.size = fileData.size();
	cfi.md5 = Md5Hash::hashStr(fileData);
	cfi.fileData.swap(fileData);

	bool result = m_settingsSet.readFromXml(cfi.fileData);

	if (result == false)
	{
		logError("reading software settings set - FAILED!");
		return false;
	}

	// Configuration.xml info always first item in m_cfgFileInfo
	//
	m_cfgFilesInfo.insert(cfi.pathFileName, cfi);

	QXmlStreamReader xmlReader(cfi.fileData);

	while(xmlReader.atEnd() == false)
	{
		if (xmlReader.readNextStartElement() == false)
		{
			continue;
		}

		if (xmlReader.name() == QLatin1String("BuildInfo"))
		{
			m_buildInfo.readFromXml(xmlReader);
			continue;
		}

		// find "file" element
		//
		if (xmlReader.name() != QLatin1String("File"))
		{
			continue;
		}

		CfgFileInfo xcfi;

		xcfi.readFromXml(xmlReader);

		m_cfgFilesInfo.insert(xcfi.pathFileName, xcfi);

		if (xcfi.ID.isEmpty() == false)
		{
			m_fileIDPathMap.insert(xcfi.ID, xcfi.pathFileName);
		}
	}

	return true;
}

void CfgLoader::readSavedConfiguration()
{
	// if exists, read previously loaded configuration
	//

   // if ()
}

bool CfgLoader::readCfgFile(const QString& pathFileName, QByteArray* fileData, bool needUncompress)
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

bool CfgLoader::readCfgFileIfExists(const QString& filePathName, QByteArray* fileData, const QString& etalonMd5, bool needUncompress)
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

bool CfgLoader::isCfgFileIsExists(const QString& filePathName, const QString& etalonMd5)
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

void CfgLoader::configurationChanged()
{
	m_enableDownloadConfiguration = false;			// waiting for call slot_enableDownloadConfiguration

	emit signal_configurationChanged();
}

void CfgLoader::emitFileReady()
{
	emit signal_fileReady();

	m_fileReadyCondition.wakeAll();
}

QString CfgLoader::getFilePathNameByID(QString fileID) const
{
	AUTO_LOCK(m_mutex);

	return m_fileIDPathMap.value(fileID, QString());
}

// -------------------------------------------------------------------------------------
//
// CfgLoaderThread class implementation
//
// -------------------------------------------------------------------------------------

CfgLoaderThread::CfgLoaderThread(	const SoftwareInfo& softwareInfo,
									int appInstance,
									const HostAddressPort& serverAddressPort1,
									const HostAddressPort& serverAddressPort2,
									bool enableDownloadCfg,
									std::shared_ptr<CircularLogger> logger) :
	m_softwareInfo(softwareInfo),
	m_appInstance(appInstance),
	m_server1(serverAddressPort1),
	m_server2(serverAddressPort2),
	m_enableDownloadCfg(enableDownloadCfg),
	m_logger(logger)
{
	AUTO_LOCK(m_mutex);

	initThread();
}

CfgLoaderThread::~CfgLoaderThread()
{
	AUTO_LOCK(m_mutex);

	shutdownThread(nullptr);
}


void CfgLoaderThread::start()
{
	AUTO_LOCK(m_mutex);

	if (m_thread == nullptr || m_cfgLoader == nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	m_thread->start();
}

void CfgLoaderThread::quit()
{
	AUTO_LOCK(m_mutex);

	if (m_thread == nullptr || m_cfgLoader == nullptr)
	{
		assert(false);
		return;
	}

	m_thread->quit();
}

void CfgLoaderThread::quitAndWait()
{
	AUTO_LOCK(m_mutex);

	if (m_thread == nullptr || m_cfgLoader == nullptr)
	{
		assert(false);
		return;
	}

	m_thread->quitAndWait();
}

void CfgLoaderThread::enableDownloadConfiguration()
{
	AUTO_LOCK(m_mutex);

	m_cfgLoader->slot_enableDownloadConfiguration();
}

bool CfgLoaderThread::getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr)
{
	TEST_PTR_RETURN_FALSE(fileData);
	TEST_PTR_RETURN_FALSE(errorStr);

	errorStr->clear();

	return m_cfgLoader->getFileBlocked(pathFileName, fileData, errorStr);
}

bool CfgLoaderThread::getFile(const QString& pathFileName, QByteArray* fileData)
{
	TEST_PTR_RETURN_FALSE(fileData);

	AUTO_LOCK(m_mutex);

	return m_cfgLoader->getFile(pathFileName, fileData);
}

bool CfgLoaderThread::getFileBlockedByID(const QString& fileID, QByteArray* fileData, QString* errorStr)
{
	TEST_PTR_RETURN_FALSE(fileData);
	TEST_PTR_RETURN_FALSE(errorStr);

	errorStr->clear();

	return m_cfgLoader->getFileBlockedByID(fileID, fileData, errorStr);
}

bool CfgLoaderThread::getFileByID(const QString& fileID, QByteArray* fileData)
{
	TEST_PTR_RETURN_FALSE(fileData);

	AUTO_LOCK(m_mutex);

	return m_cfgLoader->getFileByID(fileID, fileData);
}

bool CfgLoaderThread::hasFileID(QString fileID) const
{
	AUTO_LOCK(m_mutex);

	return m_cfgLoader->hasFileID(fileID);
}

OnlineLib::BuildInfo CfgLoaderThread::buildInfo()
{
	AUTO_LOCK(m_mutex);

	return m_cfgLoader->buildInfo();
}

QString CfgLoaderThread::getLastErrorStr()
{
	AUTO_LOCK(m_mutex);

	return m_cfgLoader->getLastErrorStr();
}

Tcp::ConnectionState CfgLoaderThread::getConnectionState()
{
	AUTO_LOCK(m_mutex);

	return m_cfgLoader->getConnectionState();
}

HostAddressPort CfgLoaderThread::getCurrentServerAddressPort()
{
	AUTO_LOCK(m_mutex);

	return m_cfgLoader->currentServerAddressPort();
}

void CfgLoaderThread::setConnectionParams(const SoftwareInfo& softwareInfo,
										  const HostAddressPort& serverAddressPort1,
										  const HostAddressPort& serverAddressPort2,
										  bool enableDownloadConfiguration)
{
	m_softwareInfo = softwareInfo;
	m_server1 = serverAddressPort1;
	m_server2 = serverAddressPort2;
	m_enableDownloadCfg = enableDownloadConfiguration;

	bool restartThread = false;

	m_mutex.lock();

	shutdownThread(&restartThread);
	initThread();

	m_mutex.unlock();

	if (restartThread == true)
	{
		start();
	}
}

SessionParams CfgLoaderThread::sessionParams() const
{
	if (m_cfgLoader != nullptr)
	{
		return m_cfgLoader->sessionParams();
	}

	Q_ASSERT(false);

	return SessionParams();
}

void CfgLoaderThread::initThread()
{
	assert(m_cfgLoader == nullptr);
	assert(m_thread == nullptr);

	m_thread = new SimpleThread;

	m_cfgLoader = new CfgLoader(m_softwareInfo,
								m_appInstance,
								m_server1,
								m_server2,
								m_enableDownloadCfg,
								m_logger);

	m_thread->addWorker(m_cfgLoader); // this instance of CfgLoader will be deleted during SimpleThread destruction

	connect(m_cfgLoader, &CfgLoader::signal_configurationReady, this, &CfgLoaderThread::signal_configurationReady);

	connect(m_cfgLoader, &CfgLoader::signal_unknownClientID, this, &CfgLoaderThread::signal_unknownClientID);
	connect(m_cfgLoader, &CfgLoader::signal_wrongClientHostname, this, &CfgLoaderThread::signal_wrongClientHostname);

	connect(m_cfgLoader, &CfgLoader::signal_onEndFileDownload, this, &CfgLoaderThread::signal_onEndFileDownload);
	connect(m_cfgLoader, &CfgLoader::signal_onEndFileDownloadError, this, &CfgLoaderThread::signal_onEndFileDownloadError);
}

void CfgLoaderThread::shutdownThread(bool* restartThread)
{
	// restartThread can be == nullptr

	if (m_thread != nullptr)
	{
		bool threadIsRunning = m_thread->isRunning();

		if (threadIsRunning == true)
		{
			m_thread->quitAndWait();			// m_cfgLoader will be deleted here
		}

		delete m_thread;

		m_thread = nullptr;
		m_cfgLoader = nullptr;

		if (restartThread != nullptr)
		{
			*restartThread = threadIsRunning;
		}
	}
	else
	{
		assert(false);
	}
}
