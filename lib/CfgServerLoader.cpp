#include "../lib/CfgServerLoader.h"
#include "../lib/CircularLogger.h"

#include "../Builder/CfgFiles.h"

#include <QXmlStreamReader>
#include <QStandardPaths>


// -------------------------------------------------------------------------------------
//
// CfgServerLoaderBase class implementation
//
// -------------------------------------------------------------------------------------

bool CfgServerLoaderBase::m_BuildFileInfoArrayRegistered = false;

CfgServerLoaderBase::CfgServerLoaderBase()
{
	if (m_BuildFileInfoArrayRegistered == false)
	{
		qRegisterMetaType<BuildFileInfoArray>("BuildFileInfoArray");
		m_BuildFileInfoArrayRegistered = true;
	}
}

// -------------------------------------------------------------------------------------
//
// CfgServer class implementation
//
// -------------------------------------------------------------------------------------

CfgServer::CfgServer(const SoftwareInfo& softwareInfo, const QString& buildFolder, std::shared_ptr<CircularLogger> logger) :
	Tcp::FileServer(buildFolder, softwareInfo, logger),
	m_logger(logger)
{
}

CfgServer* CfgServer::getNewInstance()
{
	return new CfgServer(localSoftwareInfo(), m_rootFolder, m_logger);
}

void CfgServer::onServerThreadStarted()
{
	m_buildXmlPathFileName = m_rootFolder + "/build.xml";

	readBuildXml();
}

void CfgServer::onServerThreadFinished()
{
}

void CfgServer::onConnection()
{
	DEBUG_LOG_MSG(m_logger, QString(tr("CfgServer new connection #%1 accepted from %2")).arg(id()).arg(peerAddr().addressStr()));
}

void CfgServer::onDisconnection()
{
	DEBUG_LOG_MSG(m_logger, QString(tr("CfgServer connection #%1 closed")).arg(id()));
}

void CfgServer::readBuildXml()
{
	QDir dir(m_buildXmlPathFileName);

	if (dir.exists(m_buildXmlPathFileName) == false)
	{
		m_errorCode = ErrorCode::BuildNotFound;
		qDebug() << "File not found: " << m_buildXmlPathFileName;
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

		if (xmlReader.name() == "BuildInfo")
		{
			m_buildInfo.readFromXml(xmlReader);
			continue;
		}

		// find "file" element
		//
		if (xmlReader.name() != "File")
		{
			continue;
		}

		Builder::BuildFileInfo bfi;

		bfi.readFromXml(xmlReader);

		m_buildFileInfo.insert(bfi.pathFileName, bfi);
	}

	QString str = QString("File %1 has been read").arg(m_buildXmlPathFileName);

	qDebug() << C_STR(str);
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

bool CfgLoader::m_registerTypes = true;

CfgLoader::CfgLoader(const SoftwareInfo& softwareInfo,
						int appInstance,
						const HostAddressPort& serverAddressPort1,
						const HostAddressPort& serverAddressPort2,
						bool enableDownloadCfg,
						std::shared_ptr<CircularLogger> logger) :
	Tcp::FileClient(softwareInfo, "", serverAddressPort1, serverAddressPort2),
	m_enableDownloadConfiguration(enableDownloadCfg),
	m_logger(logger),
	m_timer(this)
{
	if (m_registerTypes == true)
	{
		qRegisterMetaType<Tcp::FileTransferResult>("Tcp::FileTransferResult");
		m_registerTypes = false;
	}

	changeApp(softwareInfo.equipmentID(), appInstance);

	// DELETE after periodic CFG requests to be added
	//
	enableWatchdogTimer(false);
	//
	// DELETE after periodic CFG requests to be added
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

	qDebug() << "App cfg data root folder: " << C_STR(m_rootFolder);

	//m_rootFolder = "d:/cfgloader" + m_appDataPath;		// for debugging only!!!

	setRootFolder(m_rootFolder);

	m_configurationXmlPathFileName = "/" + m_appEquipmentID + "/" + Builder::FILE_CONFIGURATION_XML;

	readSavedConfiguration();

	resetStatuses();
}

bool CfgLoader::getFileBlocked(QString pathFileName, QByteArray* fileData, QString* errorStr)
{
	// execute in context of calling thread
	//
	if (fileData == nullptr || errorStr == nullptr)
	{
		assert(false);
		return false;
	}

	fileData->clear();

	WaitForSignalHelper wsh(this, SIGNAL(signal_fileReady()));

	m_localFileData.clear();

	emit signal_getFile(pathFileName, &m_localFileData);

	if (wsh.wait(6000) == true)
	{
		*errorStr = getLastErrorStr();

		if (errorStr->isEmpty())
		{
			fileData->swap(m_localFileData);
			return true;
		}
		else
		{
			return false;
		}
	}

	*errorStr = tr("File reading timeout");

	return false;
}

bool CfgLoader::getFile(QString pathFileName, QByteArray* fileData)
{
	// execute in context of calling thread
	//
	setFileReady(false);

	if (fileData == nullptr)
	{
		assert(false);
		return false;
	}

	fileData->clear();

	emit signal_getFile(pathFileName, fileData);

	return true;
}

bool CfgLoader::getFileBlockedByID(QString fileID, QByteArray* fileData, QString *errorStr)
{
	if (fileData == nullptr || errorStr == nullptr)
	{
		assert(false);
		return false;
	}

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
	QMutexLocker l(&m_mutex);

	return m_fileIDPathMap.contains(fileID);
}

bool CfgLoader::isFileReady()
{
	m_mutex.lock();

	bool result = m_fileReady;

	m_mutex.unlock();

	return result;
}

Builder::BuildInfo CfgLoader::buildInfo()
{
	Builder::BuildInfo buildInfo;

	m_mutex.lock();

	buildInfo = m_buildInfo;

	m_mutex.unlock();

	return buildInfo;
}

void CfgLoader::onTryConnectToServer(const HostAddressPort& serverAddr)
{
	DEBUG_LOG_MSG(m_logger, QString(tr("Try connect to server %1").arg(serverAddr.addressPortStr())));
}

void CfgLoader::onConnection()
{
	DEBUG_LOG_MSG(m_logger, QString(tr("CfgLoader connected to server %1").arg(peerAddr().addressStr())));

	resetStatuses();

	startConfigurationXmlLoading();
}

void CfgLoader::onDisconnection()
{
	DEBUG_LOG_MSG(m_logger, QString(tr("CfgLoader disconnected from server %1")).arg(peerAddr().addressStr()));
}


void CfgLoader::onStartDownload(const QString& fileName)
{
	DEBUG_LOG_MSG(m_logger, QString(tr("Start download: %1")).arg(fileName));
}

void CfgLoader::onEndDownload(const QString& fileName, Tcp::FileTransferResult errorCode)
{
	if (errorCode == Tcp::FileTransferResult::Ok)
	{
		DEBUG_LOG_MSG(m_logger, QString("File %1 download Ok").arg(fileName));
	}
	else
	{
		DEBUG_LOG_ERR(m_logger, QString("File %1 download error - %2").arg(fileName).arg(getErrorStr(errorCode)));
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
		setFileReady(true);
		return;
	}

	if (m_cfgFilesInfo.contains(fileName) == false)
	{
		m_lastError = Tcp::FileTransferResult::NotFoundRemoteFile;
		setFileReady(true);
		return;
	}

	if (readCfgFileIfExists(fileName, fileData, m_cfgFilesInfo[fileName].md5, m_cfgFilesInfo[fileName].compressed) == true)
	{
		qDebug() << "File " << fileName << " already exists, md5 = " << m_cfgFilesInfo[fileName].md5;

		m_lastError = Tcp::FileTransferResult::Ok;
		setFileReady(true);
		return;
	}

	// file is not exists
	//

	FileDownloadRequest fdr;

	fdr.pathFileName = fileName;
	fdr.isAutoRequest = false;			// manual request
	fdr.fileData = fileData;
	fdr.errorCode = &m_lastError;
	fdr.etalonMD5 = m_cfgFilesInfo[fileName].md5;
	fdr.needUncompress = m_cfgFilesInfo[fileName].compressed;

	m_downloadQueue.append(fdr);

	if (isTransferInProgress() == false)
	{
		startDownload();
	}
}

void CfgLoader::slot_onTimer()
{
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
			// file exists from previous downloads
			// nothing to do
			//
			qDebug() << "File " << cfi.pathFileName << " already exists, md5 = " << cfi.md5;
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

	m_enableSignalUnknownClient = true;
}

void CfgLoader::onEndFileDownload(const QString fileName, Tcp::FileTransferResult errorCode, const QString md5)
{
	//

	emit signal_onEndFileDownload(fileName, errorCode);

	if (errorCode != Tcp::FileTransferResult::Ok)
	{
		emit signal_onEndFileDownloadError(fileName, errorCode);

		if (errorCode == Tcp::FileTransferResult::UnknownClient)
		{
			emitSignalUnknownClient();
		}
	}

	//

	onEndDownload(fileName, errorCode);

	if (errorCode != Tcp::FileTransferResult::Ok)
	{
		setFileReady(true);
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
			setFileReady(true);
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
			qDebug() << "Downloaded Configuration.xml";

			if (readConfigurationXml() == true)
			{
				m_configurationXmlReady = true;

				BuildFileInfoArray bfiArray;

				for(const CfgFileInfo& cfi : m_cfgFilesInfo)
				{
					Builder::BuildFileInfo bfi = cfi;

					bfiArray.append(bfi);
				}

				qDebug() << "Read Configuration.xml";

				emit signal_configurationReady(m_cfgFilesInfo[CONFIGURATION_XML_FILE_INDEX].fileData, bfiArray);
			}
		}
	}
	else
	{
		qDebug() << "Downloaded " << (m_currentDownloadRequest.isAutoRequest ? "(auto) :" : "(manual) :")  << fileName;

		if (m_currentDownloadRequest.isAutoRequest == false)
		{
			// emit signal_endFileDownload for "manual" requests only!
			//
			if(m_currentDownloadRequest.fileData == nullptr)
			{
				assert(false);
				m_currentDownloadRequest.setErrorCode(Tcp::FileTransferResult::InternalError);
				setFileReady(true);
			}
			else
			{
				if (readCfgFile(fileName, m_currentDownloadRequest.fileData, m_currentDownloadRequest.needUncompress) == false)
				{
					m_currentDownloadRequest.setErrorCode(Tcp::FileTransferResult::LocalFileReadingError);
					setFileReady(true);
				}
				else
				{
					setFileReady(true);
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

	m_mutex.lock();

	m_cfgFilesInfo.clear();
	m_fileIDPathMap.clear();

	CfgFileInfo cfi;

	cfi.pathFileName = m_configurationXmlPathFileName;
	cfi.size = fileData.size();
	cfi.md5 = Md5Hash::hashStr(fileData);
	cfi.fileData.swap(fileData);

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

		if (xmlReader.name() == "BuildInfo")
		{
			m_buildInfo.readFromXml(xmlReader);
			continue;
		}

		// find "file" element
		//
		if (xmlReader.name() != "File")
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

	m_mutex.unlock();

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

void CfgLoader::setFileReady(bool value)
{
	m_mutex.lock();

	m_fileReady = value;

	m_mutex.unlock();

	emit signal_fileReady();
}

QString CfgLoader::getFilePathNameByID(QString fileID) const
{
	QString pathFileName;

	m_mutex.lock();

	if (m_fileIDPathMap.contains(fileID))
	{
		pathFileName = m_fileIDPathMap[fileID];
	}

	m_mutex.unlock();

	return pathFileName;
}

void CfgLoader::emitSignalUnknownClient()
{
	if (m_enableSignalUnknownClient == true)
	{
		m_enableSignalUnknownClient = false;
		emit signal_unknownClient();
	}
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
	m_logger(logger),
	m_mutex(QMutex::Recursive)
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
		assert(false);
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

	AUTO_LOCK(m_mutex);

	return m_cfgLoader->getFileBlocked(pathFileName, fileData, errorStr);;
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

	AUTO_LOCK(m_mutex);

	return m_cfgLoader->getFileBlockedByID(fileID, fileData, errorStr);;
}

bool CfgLoaderThread::getFileByID(const QString& fileID, QByteArray* fileData)
{
	TEST_PTR_RETURN_FALSE(fileData);

	AUTO_LOCK(m_mutex);

	return m_cfgLoader->getFileByID(fileID, fileData);;
}

bool CfgLoaderThread::hasFileID(QString fileID) const
{
	AUTO_LOCK(m_mutex);

	return m_cfgLoader->hasFileID(fileID);
}

bool CfgLoaderThread::isFileReady()
{
	AUTO_LOCK(m_mutex);

	return m_cfgLoader->isFileReady();;
}

Builder::BuildInfo CfgLoaderThread::buildInfo()
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

	connect(m_cfgLoader, &CfgLoader::signal_unknownClient, this, &CfgLoaderThread::signal_unknownClient);
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
