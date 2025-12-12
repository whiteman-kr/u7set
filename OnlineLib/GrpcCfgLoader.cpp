#ifndef ONLINE_LIB_DOMAIN
#error Do not include this file in the project! Link OnlineLib instead.
#endif

#include <CommonLib/ConstStrings.h>

#include "GrpcCfgLoader.h"
#include "CircularLogger.h"
#include "../UtilsLib/XmlHelper.h"

// -------------------------------------------------------------------------------------
//
// GrpcCfgLoader::FileDownloadRequest struct implementation
//
// -------------------------------------------------------------------------------------

void GrpcCfgLoader::FileDownloadRequest::clear()
{
	pathFileName = "";
	etalonMD5 = "";
	isAutoRequest = false;
	isAsyncCall = false;
	isTestCfgRequest = false;
	fileData = nullptr;
}

// -------------------------------------------------------------------------------------
//
// CfgLoader class implementation
//
// -------------------------------------------------------------------------------------

GrpcCfgLoader::GrpcCfgLoader(const SoftwareInfo& softwareInfo,
						int appInstance,
						const std::vector<HostAddressPort>& serverAddrs,
						bool enableDownloadCfg,
						CircularLoggerShared logger) :
	m_swInfo(softwareInfo),
	m_appInstance(appInstance),
	m_serverAddrs(serverAddrs),
	m_enableDownloadCfg(enableDownloadCfg),
	m_log(logger)
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
//	shutdown();

	m_appEquipmentID = appEquipmentID;
	m_appInstance = appInstance;

	m_appDataPath = Separator::DIR + m_appEquipmentID + Separator::MINUS + QString::number(m_appInstance);

	m_rootFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + m_appDataPath;

	logMsg(QString("app cfg data root folder - %1").arg(m_rootFolder));

	m_cfgXmlFileName = Separator::DIR + m_appEquipmentID + Separator::DIR + File::CONFIGURATION_XML;

	readSavedConfiguration();

	resetStatuses();
}

bool GrpcCfgLoader::getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr)
{
	// TEST_PTR_RETURN_FALSE(fileData);
	// TEST_PTR_RETURN_FALSE(errorStr);

	// fileData->clear();
	// errorStr->clear();

	// bool result = false;

	// std::shared_ptr<QByteArray> localFileData = std::make_shared<QByteArray>();

	// m_getFileBlockedMutex.lock();

	// emit signal_getFile(pathFileName, localFileData, false);

	// bool res = m_fileReadyCondition.wait(&m_getFileBlockedMutex, 10000);

	// if (res == true)
	// {
	// 	if (getLastError() == Tcp::FileTransferResult::Ok)
	// 	{
	// 		fileData->swap(*localFileData.get());
	// 		result = true;
	// 	}
	// 	else
	// 	{
	// 		*errorStr = getLastErrorStr();
	// 		result = false;
	// 	}
	// }
	// else
	// {
	// 	*errorStr = tr("File reading timeout");
	// 	result = false;
	// }

	// m_getFileBlockedMutex.unlock();

//	return result;
	return true;
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

// bool CfgLoader::getFileAsync(const QString& pathFileName)
// {
// 	std::shared_ptr<QByteArray> fileData = std::make_shared<QByteArray>();

// 	emit signal_getFile(pathFileName, fileData, true);

// 	return true;
// }

// bool CfgLoader::getFileAsyncByID(const QString& fileID)
// {
// 	QString pathFileName = getFilePathNameByID(fileID);

// 	return getFileAsync(pathFileName);
// }

bool GrpcCfgLoader::hasFileID(QString fileID) const
{
	AUTO_LOCK(m_mutex);

	return m_fileIDPathMap.contains(fileID);
}

OnlineLib::BuildInfo GrpcCfgLoader::buildInfo()
{
	AUTO_LOCK(m_mutex);

	return m_buildInfo;
}

SessionParams GrpcCfgLoader::sessionParams() const
{
	AUTO_LOCK(m_mutex);

	return m_sessionParams;
}

QString GrpcCfgLoader::curSoftwareSettingsProfileName() const
{
	AUTO_LOCK(m_mutex);

	return m_sessionParams.currentSettingsProfile;
}

E::SoftwareRunMode GrpcCfgLoader::softwareRunMode() const
{
	AUTO_LOCK(m_mutex);

	return m_sessionParams.softwareRunMode;
}

QStringList GrpcCfgLoader::getSettingsProfiles() const
{
	AUTO_LOCK(m_mutex);

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

void GrpcCfgLoader::slot_enableDownloadConfiguration()
{
	m_enableDownloadCfg = true;
}

void GrpcCfgLoader::slot_getFile(QString fileName, std::shared_ptr<QByteArray> fileData, bool asyncCall)
{
	// if (fileData == nullptr)
	// {
	// 	assert(false);
	// 	return;
	// }

	// fileData->clear();

	// if (m_configurationXmlReady == false)
	// {
	// 	emitFileReady(fileName, Tcp::FileTransferResult::ConfigurationIsNotReady, nullptr, asyncCall);
	// 	return;
	// }

	// if (m_cfgFilesInfo.contains(fileName) == false)
	// {
	// 	emitFileReady(fileName, Tcp::FileTransferResult::FileIsNotAccessible, nullptr, asyncCall);
	// 	return;
	// }

	// const CfgFileInfo& cfgFileInfo = m_cfgFilesInfo.value(fileName);

	// if (readCfgFileIfExists(fileName, fileData.get(), cfgFileInfo.md5, cfgFileInfo.compressed) == true)
	// {
	// 	logMessage(QString("file %1 already exists, md5 = %2").
	// 				arg(fileName).arg(cfgFileInfo.md5));

	// 	emitFileReady(fileName, Tcp::FileTransferResult::Ok, fileData, asyncCall);
	// 	return;
	// }

	// // file is not exists
	// //

	// FileDownloadRequest fdr;

	// fdr.pathFileName = fileName;
	// fdr.isAutoRequest = false;			// manual request
	// fdr.isAsyncCall = asyncCall;
	// fdr.fileData = fileData;
	// fdr.etalonMD5 = cfgFileInfo.md5;
	// fdr.needUncompress = cfgFileInfo.compressed;

	// m_downloadQueue.emplace_back(fdr);

	// if (isTransferInProgress() == false)
	// {
	// 	startDownload();
	// }
}

void GrpcCfgLoader::slot_onTimer()
{
	// if (isConnected() == false)
	// {
	// 	return;
	// }

	// if (m_configurationXmlReady == false)
	// {
	// 	startConfigurationXmlLoading();
	// 	return;
	// }

	// if (m_autoDownloadIndex >= m_cfgFilesInfo.count() ||
	// 	isConnected() == false ||
	// 	m_configurationXmlReady == false ||
	// 	isTransferInProgress() ||
	// 	m_allFilesLoaded == true ||
	// 	m_downloadQueue.empty() == false)
	// {
	// 	return;
	// }

	// while(m_autoDownloadIndex < m_cfgFilesInfo.count())
	// {
	// 	CfgFileInfo& cfi = m_cfgFilesInfo[m_autoDownloadIndex];

	// 	if (isCfgFileIsExists(cfi.pathFileName, cfi.md5) == true)
	// 	{
	// 		// file exists from previous downloads, nothing to do
	// 		//
	// 		logMessage(QString("file %1 already exists, md5 = %2").
	// 									arg(cfi.pathFileName).arg(cfi.md5));
	// 	}
	// 	else
	// 	{
	// 		FileDownloadRequest fdr;

	// 		fdr.pathFileName = m_cfgFilesInfo[m_autoDownloadIndex].pathFileName;
	// 		fdr.etalonMD5 = m_cfgFilesInfo[m_autoDownloadIndex].md5;
	// 		fdr.isAutoRequest = true;

	// 		m_downloadQueue.emplace_back(fdr);

	// 		startDownload();

	// 		m_autoDownloadIndex++;

	// 		break;
	// 	}

	// 	m_autoDownloadIndex++;
	// }
}

void GrpcCfgLoader::processGetSessionParamsReply(const char* replyData, quint32 replyDataSize)
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

	logMsg(QString("current software settings profile - %1, run mode - %2").
				  arg(m_sessionParams.currentSettingsProfile).
				  arg(E::valueToString<E::SoftwareRunMode>(m_sessionParams.softwareRunMode)));

	startConfigurationXmlLoading();
}

void GrpcCfgLoader::sendGetSessionParamsRequest()
{
//	sendRequest(RQID_GET_SESSION_PARAMS);
}

void GrpcCfgLoader::onThreadStarted()
{
	Q_ASSERT(m_grpcFileClient == nullptr);

	changeAppAndInitPaths(m_swInfo.equipmentID(), m_appInstance);

	startGrpcFileClient();


//	m_timer.setInterval(2000);
//	m_timer.start();

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
														QStringLiteral("GrpcCfgLoader"), m_log);

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

void GrpcCfgLoader::shutdown()
{
}

void GrpcCfgLoader::startDownload()
{
	// if (m_downloadQueue.empty())
	// {
	// 	assert(false);
	// 	return;
	// }

	// m_currentDownloadRequest = m_downloadQueue.front();

	// m_downloadQueue.pop_front();

	// onStartDownload(m_currentDownloadRequest.pathFileName);

	// slot_downloadFile(m_currentDownloadRequest.pathFileName);	// TcpFileTransfer::slot_downloadFile
}

void GrpcCfgLoader::resetStatuses()
{
	m_downloadQueue.clear();
	m_currentDownloadRequest.clear();

	m_cfgXmlMd5 = "";

	m_configurationXmlReady = false;
	m_autoDownloadIndex = 1;		// index 0 - Configuration.xml
	m_allFilesLoaded = false;
}

// void GrpcCfgLoader::onEndFileDownload(const QString fileName, Tcp::FileTransferResult errorCode, const QString md5)
// {
	// onEndDownload(fileName, errorCode);

	// if (errorCode != Tcp::FileTransferResult::Ok)
	// {
	// 	emitFileReady(fileName, errorCode, nullptr, m_currentDownloadRequest.isAsyncCall);
	// 	return;
	// }

	// if (m_currentDownloadRequest.etalonMD5.isEmpty() == true)
	// {
	// 	// can be empty for Configuration.xml file only!!!
	// 	//
	// 	assert(m_currentDownloadRequest.pathFileName == m_cfgXmlFileName);
	// }
	// else
	// {
	// 	if (m_currentDownloadRequest.etalonMD5 != md5)
	// 	{
	// 		emitFileReady(fileName, Tcp::FileTransferResult::FileDataCorrupted,
	// 					  nullptr, m_currentDownloadRequest.isAsyncCall);
	// 		return;
	// 	}
	// }

	// if (fileName == m_cfgXmlFileName)
	// {
	// 	// Configuration.xml is loaded
	// 	//
	// 	if (m_currentDownloadRequest.isTestCfgRequest)
	// 	{
	// 		if (m_cfgFilesInfo[CONFIGURATION_XML_FILE_INDEX].md5 != md5)
	// 		{
	// 			// configuration changed !!!!!!!!!!!!
	// 		}
	// 	}
	// 	else
	// 	{
	// 		bool result = true;

	// 		if (readConfigurationXml() == true)
	// 		{
	// 			m_configurationXmlReady = true;

	// 			logMessage(Separator::EMPTY_STR);
	// 			logMessage(QString("loading configuration: project %1, buildNo %2, build date %3...").
	// 							arg(m_buildInfo.project).arg(m_buildInfo.buildNo).arg(m_buildInfo.dateTimeStr()));
	// 			logMessage(Separator::EMPTY_STR);

	// 			BuildFileInfoArray bfiArray;

	// 			for(const CfgFileInfo& cfi : m_cfgFilesInfo)
	// 			{
	// 				const OnlineLib::BuildFileInfo& bfi = cfi;

	// 				bfiArray.emplace_back(bfi);
	// 			}

	// 			std::shared_ptr<const SoftwareSettings> curSettingsProfile = getCurrentSettingsProfile<SoftwareSettings>();

	// 			if (curSettingsProfile != nullptr)
	// 			{
	// 				logMessage(QString("current software settings profile '%1' read - Ok").
	// 											arg(m_sessionParams.currentSettingsProfile));

	// 				logMessage("read Configuration.xml - Ok");

	// 				emit signal_configurationReady(m_cfgFilesInfo[CONFIGURATION_XML_FILE_INDEX].fileData,
	// 											   bfiArray,
	// 											   m_sessionParams,
	// 											   curSettingsProfile);
	// 			}
	// 			else
	// 			{
	// 				logError(QString("reading software settings profile '%1' - FAILED").
	// 											arg(m_sessionParams.currentSettingsProfile));
	// 				result = false;
	// 			}
	// 		}
	// 		else
	// 		{
	// 			result = false;
	// 		}

	// 		if (result == false)
	// 		{
	// 			logError("reading Configuration.xml - FAILED");
	// 		}
	// 	}
	// }
	// else
	// {
	// 	logMessage(QString("downloaded %1 %2").
	// 								arg(m_currentDownloadRequest.isAutoRequest ? "(auto) :" : "(manual) :").
	// 								arg(fileName));

	// 	if (m_currentDownloadRequest.isAutoRequest == false)
	// 	{
	// 		// emit signal_endFileDownload for "manual" requests only!
	// 		//
	// 		if(m_currentDownloadRequest.fileData == nullptr)
	// 		{
	// 			assert(false);
	// 			emitFileReady(fileName, Tcp::FileTransferResult::InternalError,
	// 						  nullptr, m_currentDownloadRequest.isAsyncCall);
	// 		}
	// 		else
	// 		{
	// 			if (readCfgFile(fileName, m_currentDownloadRequest.fileData.get(),
	// 							m_currentDownloadRequest.needUncompress) == false)
	// 			{
	// 				emitFileReady(fileName, Tcp::FileTransferResult::LocalFileReadingError,
	// 							  nullptr, m_currentDownloadRequest.isAsyncCall);
	// 			}
	// 			else
	// 			{
	// 				Q_ASSERT(m_currentDownloadRequest.fileData != nullptr);
	// 				emitFileReady(fileName, Tcp::FileTransferResult::Ok,
	// 							  m_currentDownloadRequest.fileData, m_currentDownloadRequest.isAsyncCall);
	// 				m_currentDownloadRequest.fileData.reset();
	// 			}
	// 		}
	// 	}
	// }

	// if (m_autoDownloadIndex == m_cfgFilesInfo.count())
	// {
	// 	m_allFilesLoaded = true;
	// }

	// if (m_downloadQueue.empty() == false)
	// {
	// 	startDownload();
	// }
//}

bool GrpcCfgLoader::startConfigurationXmlLoading()
{
	// if (m_enableDownloadCfg == false)
	// {
	// 	return true;
	// }

	// if (isConnected() == false)
	// {
	// 	return true;
	// }

	// if (m_transferInProgress == true)
	// {
	// 	return true;
	// }

	// m_configurationXmlReady = false;

	// FileDownloadRequest fdr;

	// fdr.pathFileName = m_cfgXmlFileName;

	// m_downloadQueue.push_front(fdr);

	// startDownload();

	return true;
}

bool GrpcCfgLoader::readCfgXmlFile(const QByteArray& fileData)
{
	AUTO_LOCK(m_mutex);

	m_cfgFilesInfo.clear();
	m_buildFilesInfo.clear();
	m_fileIDPathMap.clear();

	XmlReadHelper xmlReader(fileData);

	bool res = m_buildInfo.readFromXml(xmlReader);

	if (res == false)
	{
		DEBUG_LOG_ERR(m_log, QString("Can't read <BuildInfo> section in file %1!").arg(m_cfgXmlFileName));
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

void GrpcCfgLoader::readSavedConfiguration()
{
}

bool GrpcCfgLoader::readCfgFile(const QString& pathFileName, QByteArray* fileData, bool needUncompress)
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

void GrpcCfgLoader::configurationChanged()
{
	m_enableDownloadCfg = false;			// waiting for call slot_enableDownloadConfiguration

	emit signal_configurationChanged();
}

void GrpcCfgLoader::emitFileReady(const QString& fileName,
							  Tcp::FileTransferResult errorCode,
							  std::shared_ptr<QByteArray> fileData,
							  bool asyncCall)
{
	m_lastError = errorCode;

	if (asyncCall == true)
	{
		emit signal_fileReady(fileName, errorCode, fileData);
	}
	else
	{
		m_fileReadyCondition.wakeAll();
	}
}

QString GrpcCfgLoader::getFilePathNameByID(const QString& fileID) const
{
	AUTO_LOCK(m_mutex);

	auto it = m_fileIDPathMap.find(fileID);

	if (it != m_fileIDPathMap.end())
	{
		return it->second;
	}

	return QString();
}

void GrpcCfgLoader::logMsg(const QString& msg)
{
	DEBUG_LOG_MSG(m_log, QString("GrpcCfgLoader: %1").arg(msg));
}

void GrpcCfgLoader::logWrn(const QString& wrn)
{
	DEBUG_LOG_WRN(m_log, QString("GrpcCfgLoader: %1").arg(wrn));
}

void GrpcCfgLoader::logErr(const QString& err)
{
	DEBUG_LOG_ERR(m_log, QString("GrpcCfgLoader: %1").arg(err));
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
									bool enableDownloadCfg,
									std::shared_ptr<CircularLogger> logger) :
	m_softwareInfo(softwareInfo),
	m_appInstance(appInstance),
	m_enableDownloadCfg(enableDownloadCfg),
	m_logger(logger)
{
//	qDebug() << "CfgLoaderThread::CfgLoaderThread";
	addServerAddrs(serverAddressPort1, serverAddressPort2);

	AUTO_LOCK(m_mutex);

	initThread();
}

GrpcCfgLoaderThread::~GrpcCfgLoaderThread()
{
	AUTO_LOCK(m_mutex);
	shutdownThread();

	qDebug() << "CfgLoaderThread::~CfgLoaderThread";
}

void GrpcCfgLoaderThread::start()
{
	AUTO_LOCK(m_mutex);

	if (m_thread == nullptr || m_grpcCfgLoader == nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	m_thread->start();
}

void GrpcCfgLoaderThread::quitAndWait()
{
	AUTO_LOCK(m_mutex);
	shutdownThread();
}

void GrpcCfgLoaderThread::enableDownloadConfiguration()
{
	AUTO_LOCK(m_mutex);

	//m_grpcCfgLoader->slot_enableDownloadConfiguration();
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

// bool CfgLoaderThread::getFileAsync(const QString& pathFileName)
// {
// 	AUTO_LOCK(m_mutex);

// 	return m_cfgLoader->getFileAsync(pathFileName);
// }

// bool CfgLoaderThread::getFileAsyncByID(const QString& fileID)
// {
// 	AUTO_LOCK(m_mutex);

// 	return m_cfgLoader->getFileAsyncByID(fileID);
// }

bool GrpcCfgLoaderThread::hasFileID(QString fileID) const
{
	AUTO_LOCK(m_mutex);

	return m_grpcCfgLoader->hasFileID(fileID);
}

OnlineLib::BuildInfo GrpcCfgLoaderThread::buildInfo()
{
	AUTO_LOCK(m_mutex);

	return m_grpcCfgLoader->buildInfo();
}

QString GrpcCfgLoaderThread::getLastErrorStr()
{
	AUTO_LOCK(m_mutex);

//	return m_grpcCfgLoader->getLastErrorStr();
	return QString();
}

Tcp::ConnectionState GrpcCfgLoaderThread::getConnectionState()
{
	AUTO_LOCK(m_mutex);

//	return m_grpcCfgLoader->getConnectionState();

	return Tcp::ConnectionState{};
}

HostAddressPort GrpcCfgLoaderThread::getCurrentServerAddressPort()
{
	AUTO_LOCK(m_mutex);

//	return m_grpcCfgLoader->currentServerAddressPort();
	return HostAddressPort{};
}

void GrpcCfgLoaderThread::setConnectionParams(const SoftwareInfo& softwareInfo,
										  const HostAddressPort& serverAddressPort1,
										  const HostAddressPort& serverAddressPort2,
										  bool enableDownloadConfiguration)
{
	m_softwareInfo = softwareInfo;

	addServerAddrs(serverAddressPort1, serverAddressPort2);

	m_enableDownloadCfg = enableDownloadConfiguration;

	AUTO_LOCK(m_mutex);

	shutdownThread();
	initThread();
	start();
}

SessionParams GrpcCfgLoaderThread::sessionParams() const
{
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
	Q_ASSERT(m_grpcCfgLoader == nullptr);
	Q_ASSERT(m_thread == nullptr);

	m_grpcCfgLoader = new GrpcCfgLoader(m_softwareInfo,
								m_appInstance,
								m_serverAddrs,
								m_enableDownloadCfg,
								m_logger);

	m_thread = new SimpleThread;

	m_thread->addWorker(m_grpcCfgLoader); // this instance of CfgLoader will be deleted during SimpleThread destruction

	connect(m_grpcCfgLoader, &GrpcCfgLoader::signal_configurationReady, this, &GrpcCfgLoaderThread::signal_configurationReady);
	connect(m_grpcCfgLoader, &GrpcCfgLoader::signal_unknownClientID, this, &GrpcCfgLoaderThread::signal_unknownClientID);
	connect(m_grpcCfgLoader, &GrpcCfgLoader::signal_wrongClientHostname, this, &GrpcCfgLoaderThread::signal_wrongClientHostname);

//	connect(m_cfgLoader, &CfgLoader::signal_fileReady, this, &CfgLoaderThread::signal_fileReady);
}

void GrpcCfgLoaderThread::shutdownThread()
{
	// restartThread can be == nullptr

	if (m_thread == nullptr)
	{
		return;
	}

	m_thread->quitAndWait();			// m_cfgLoader will be deleted here

	qDebug() << "CfgLoaderThread quited";

	delete m_thread;

	m_thread = nullptr;
	m_grpcCfgLoader = nullptr;
}

